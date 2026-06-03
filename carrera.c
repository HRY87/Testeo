#include <stdio.h>
#include <string.h>
#include "carrera.h"
#include "piloto.h"

/* =========================================================
   Tabla de puntos F1 (posicion 1..10, indice 0 = sin puntos)
   ========================================================= */
static const int puntos_f1[11] = {0, 25, 18, 15, 12, 10, 8, 6, 4, 2, 1};

/* =========================================================
                    Ciclo de vida
   ========================================================= */

int inicializarCarrera(Carrera* c, int capacidad)
{
    memset(&c->info, 0, sizeof(CarreraHeader));
    return crearVector(&c->resultados, sizeof(ResultadoPiloto), (size_t)capacidad);
}

void destruirCarrera(Carrera* c)
{
    destruirVector(&c->resultados);
}

/* =========================================================
   Serializacion: unica interfaz con carrera.bin
   ========================================================= */

int escribirCarrera(FILE* fCarrera, const Carrera* c)
{
    size_t                 i;
    const ResultadoPiloto* rp;

    if (fwrite(&c->info, sizeof(CarreraHeader), 1, fCarrera) != 1)
        return ERR_ARCH;

    for (i = 0; i < (size_t)c->info.cant_resultados; i++)
    {
        rp = (const ResultadoPiloto*)obtenerElementoVector(
                 (tVector*)&c->resultados, i);

        if (fwrite(rp, sizeof(ResultadoPiloto), 1, fCarrera) != 1)
            return ERR_ARCH;
    }

    return TODO_OK;
}

int leerCarrera(FILE* fCarrera, Carrera* c)
{
    int             i;
    ResultadoPiloto rp;

    c->resultados.ce = 0;

    if (fread(&c->info, sizeof(CarreraHeader), 1, fCarrera) != 1)
        return ERR_ARCH;

    for (i = 0; i < c->info.cant_resultados; i++)
    {
        if (fread(&rp, sizeof(ResultadoPiloto), 1, fCarrera) != 1)
            return ERR_ARCH;

        memcpy((char*)c->resultados.vec + (c->resultados.ce * sizeof(ResultadoPiloto)),
               &rp, sizeof(ResultadoPiloto));
        c->resultados.ce++;
    }

    return TODO_OK;
}

/* =========================================================
                        Helpers internos
   ========================================================= */

/*
 * pedirDatosBase
 * Solicita circuito y fecha. Reintenta la fecha hasta que sea valida.
 * Usa leerCadena para el string y scanf para el numero.
 */
static void pedirDatosBase(Carrera* c)
{
    printf("\nNombre del circuito: ");
    limpiarBuffer();
    leerCadena(c->info.circuito, TAM_NOMBRE_CIRCUITO);

    do
    {
        printf("Fecha de la carrera (AAAAMMDD): ");
        scanf("%llu", &c->info.fecha);
        limpiarBuffer();

        if (!esFechaValida(c->info.fecha))
            printf("[!] Fecha invalida. Use formato AAAAMMDD.\n");

    }
    while (!esFechaValida(c->info.fecha));
}

/*
 * pedirEstadoResultado
 * Solicita el estado (1-4) con reintento hasta valor valido.
 * Se justifica como funcion porque el rango debe validarse.
 */
static int pedirEstadoResultado(void)
{
    int estado;

    do
    {
        printf("Estado [1=FIN 2=DNF 3=DNS 4=DSQ]: ");
        scanf("%d", &estado);
        limpiarBuffer();

        if (estado < RES_FIN || estado > RES_DSQ)
            printf("[!] Estado invalido. Ingrese un valor entre 1 y 4.\n");

    }
    while (estado < RES_FIN || estado > RES_DSQ);

    return estado;
}

/*
 * pedirTipoSancion
 * Solicita tipo (1-3) y cantidad de carreras si aplica.
 * Se justifica como funcion por la logica condicional de carreras.
 */
static void pedirTipoSancion(int* tipo, int* carreras)
{
    printf("\n  Tipo de sancion:\n");
    printf("    1. Descuento de %d puntos\n", PUNTOS_PENALIZACION_DSQ);
    printf("    2. Saltear carrera(s)\n");
    printf("    3. Puntos + saltear carrera(s)\n");

    do
    {
        printf("  Opcion: ");
        scanf("%d", tipo);
        limpiarBuffer();

        if (*tipo < SANCION_PUNTOS || *tipo > SANCION_AMBAS)
            printf("[!] Opcion invalida. Ingrese 1, 2 o 3.\n");

    }
    while (*tipo < SANCION_PUNTOS || *tipo > SANCION_AMBAS);

    *carreras = 0;

    if (*tipo == SANCION_CARRERA || *tipo == SANCION_AMBAS)
    {
        printf("  Carreras a saltear: ");
        scanf("%d", carreras);
        limpiarBuffer();

        if (*carreras < 1)
        {
            printf("[!] Debe ser al menos 1. Se asigna 1.\n");
            *carreras = 1;
        }
    }
}

/*
 * esPilotoDuplicado
 * Retorna 1 si idPiloto ya esta registrado en la carrera.
 */
static int esPilotoDuplicado(const Carrera* c, unsigned idPiloto)
{
    size_t           k;
    ResultadoPiloto* rp;
    int              encontrado;

    encontrado = 0;
    k          = 0;

    while (k < c->resultados.ce && !encontrado)
    {
        rp         = (ResultadoPiloto*)obtenerElementoVector((tVector*)&c->resultados, k);
        encontrado = (rp->id_piloto == idPiloto);
        k++;
    }

    return encontrado;
}

/*
 * acumularPuntosDesdeCarrera
 * Suma los puntos de todos los resultados de una carrera
 * en el vector de pilotos. Factorizado para evitar duplicacion
 * entre recalcularPuntosPilotos y actualizarPuntosUltimaCarrera.
 */
static void acumularPuntosDesdeCarrera(tVector* vPilotos, const Carrera* c)
{
    size_t           i;
    ResultadoPiloto* rp;
    Piloto*          piloto;

    for (i = 0; i < (size_t)c->info.cant_resultados; i++)
    {
        rp     = (ResultadoPiloto*)obtenerElementoVector((tVector*)&c->resultados, i);
        piloto = (Piloto*)busquedaBinariaVector(vPilotos, &rp->id_piloto, compararUnsigned);

        if (piloto && rp->puntos > 0)
            piloto->puntos_acumulados += (unsigned)rp->puntos;
    }
}

/*
 * estadoResultadoATexto
 * Convierte el entero de estado a su cadena para mostrar.
 */
static const char* estadoResultadoATexto(int estado)
{
    switch (estado)
    {
    case RES_FIN:
        return "FIN";
    case RES_DNF:
        return "DNF";
    case RES_DNS:
        return "DNS";
    case RES_DSQ:
        return "DSQ";
    default:
        return "???";
    }
}

/* =========================================================
   Alta de carrera
   ========================================================= */

int registrarCarreraAleatoria(const char* rutaCarrera,
                              const char* rutaPiloto,
                              const char* rutaSancion,
                              Comparar    comparar)
{
    Carrera nueva;
    FILE*   fCarrera;
    int     resp;

    if (inicializarCarrera(&nueva, MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    fCarrera = fopen(rutaCarrera, "ab+");
    if (!fCarrera)
    {
        destruirCarrera(&nueva);
        return ERR_ARCH;
    }

    nueva.info.id     = generarIdCarrera(fCarrera);
    nueva.info.estado = ESTADO_CARRERA_ACTIVA;

    pedirDatosBase(&nueva);

    resp = cargarResultadosAleatorios(rutaPiloto, &nueva, comparar);

    if (resp == TODO_OK)
    {
        fseek(fCarrera, 0L, SEEK_END);
        resp = escribirCarrera(fCarrera, &nueva);
    }

    fclose(fCarrera);
    destruirCarrera(&nueva);

    return resp;
}

int registrarCarreraManual(const char* rutaCarrera,
                           const char* rutaPiloto,
                           const char* rutaSancion,
                           Comparar    comparar)
{
    Carrera         nueva;
    ResultadoPiloto rp;
    FILE*           fCarrera;
    int             resp, pos;
    int             pInput, puntosSug;
    int             hayMas, tipoSancion, carrerasSancion;
    int             k, dsqCant; //Cantidad de descuento
    unsigned        dsqIds[MAX_PILOTOS_CARRERA];


    if (inicializarCarrera(&nueva, MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    fCarrera = fopen(rutaCarrera, "ab+");
    if (!fCarrera)
    {
        destruirCarrera(&nueva);
        return ERR_ARCH;
    }

    nueva.info.id              = generarIdCarrera(fCarrera);
    nueva.info.estado          = ESTADO_CARRERA_ACTIVA;
    nueva.info.cant_resultados = 0;
    dsqCant                    = 0;

    pedirDatosBase(&nueva);

    printf("\nIngreso de resultados (ID piloto = 0 para terminar)\n");
    printf("Estados: 1=FIN  2=DNF  3=DNS  4=DSQ\n");
    printf("--------------------------------------------\n");

    pos    = 1;
    hayMas = 1;

    while (hayMas && nueva.resultados.ce < nueva.resultados.tope)
    {
        printf("Posicion %2d - ID piloto (0 = fin): ", pos);
        scanf("%u", &rp.id_piloto);
        limpiarBuffer();

        if (rp.id_piloto == 0)
        {
            hayMas = 0;
        }
        else if (esPilotoDuplicado(&nueva, rp.id_piloto))
        {
            printf("[!] Piloto ID %u ya fue registrado en esta carrera.\n", rp.id_piloto);
        }
        else
        {
            rp.estado_resultado = pedirEstadoResultado();

            puntosSug = (rp.estado_resultado == RES_FIN && pos <= POS_LIMITE_PUNTOS_CARRERA)
                        ? puntos_f1[pos] : puntos_f1[0];

            printf("           Puntos (sugerido %2d, -1 para aceptar): ", puntosSug);
            scanf("%d", &pInput);
            limpiarBuffer();

            rp.puntos = (pInput < 0) ? puntosSug : pInput;

            memcpy((char*)nueva.resultados.vec +
                   (nueva.resultados.ce * sizeof(ResultadoPiloto)),
                   &rp, sizeof(ResultadoPiloto));

            nueva.resultados.ce++;
            nueva.info.cant_resultados++;

            if (rp.estado_resultado == RES_DSQ && dsqCant < MAX_PILOTOS_CARRERA)
                dsqIds[dsqCant++] = rp.id_piloto;

            pos++;
        }
    }

    if (nueva.resultados.ce >= nueva.resultados.tope)
        printf("[!] Capacidad maxima de pilotos alcanzada.\n");

    fseek(fCarrera, 0L, SEEK_END);
    resp = escribirCarrera(fCarrera, &nueva);
    fclose(fCarrera);

    for (k = 0; k < dsqCant; k++)
    {
        printf("\n[DSQ] Piloto ID %u - Elegir sancion:\n", dsqIds[k]);
        pedirTipoSancion(&tipoSancion, &carrerasSancion);

        aplicarSancionDSQ(rutaSancion,
                          rutaPiloto,
                          dsqIds[k],
                          (unsigned)nueva.info.id,
                          nueva.info.fecha,
                          tipoSancion,
                          carrerasSancion);
    }

    destruirCarrera(&nueva);
    return resp;
}

int cargarResultadosAleatorios(const char* rutaPiloto,
                               Carrera*    c,
                               Comparar    comparar)
{
    tVector         vIds;
    int             i;
    ResultadoPiloto rp;

    if (crearVector(&vIds, sizeof(unsigned), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if (cargarVectorPilotoActivos(rutaPiloto, &vIds, comparar) || vIds.ce == 0)
    {
        destruirVector(&vIds);
        return ERR_ARCH;
    }

    generarResultadoAleatorioVector(&vIds);

    c->info.cant_resultados = (int)vIds.ce;

    for (i = 0; i < c->info.cant_resultados; i++)
    {
        rp.id_piloto        = *(unsigned*)obtenerElementoVector(&vIds, i);
        rp.estado_resultado = RES_FIN;
        rp.puntos           = (i < POS_LIMITE_PUNTOS_CARRERA) ? puntos_f1[i + 1] : 0;

        memcpy((char*)c->resultados.vec + (i * sizeof(ResultadoPiloto)),
               &rp, sizeof(ResultadoPiloto));
        c->resultados.ce++;
    }

    destruirVector(&vIds);
    return TODO_OK;
}

int generarIdCarrera(FILE* fCarrera)
{
    int     ultimoId;
    Carrera tmp;

    fseek(fCarrera, 0, SEEK_END);

    if (ftell(fCarrera) < (long)sizeof(CarreraHeader))
        return 1;

    ultimoId = 1;
    fseek(fCarrera, 0, SEEK_SET);

    if (inicializarCarrera(&tmp, MAX_PILOTOS_CARRERA) == TODO_OK)
    {
        while (leerCarrera(fCarrera, &tmp) == TODO_OK)
            ultimoId = tmp.info.id;

        destruirCarrera(&tmp);
    }

    return ultimoId + 1;
}

/* =========================================================
   Actualizacion de puntos en piloto.bin
   ========================================================= */

int recalcularPuntosPilotos(const char* rutaCarrera,
                            const char* rutaPiloto,
                            Filter      filtrar,
                            Reduce      reducir)
{
    tVector  vPilotos;
    Carrera  c;
    FILE*    fCarrera;
    size_t   i;
    Piloto*  p;

    (void)reducir;

    if (crearVector(&vPilotos, sizeof(Piloto), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if (cargarVectorDesdeBin(rutaPiloto, &vPilotos))
    {
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    /* Resetear puntos de todos los pilotos */
    for (i = 0; i < vPilotos.ce; i++)
    {
        p = (Piloto*)obtenerElementoVector(&vPilotos, i);
        p->puntos_acumulados = 0;
    }

    if (inicializarCarrera(&c, MAX_PILOTOS_CARRERA))
    {
        destruirVector(&vPilotos);
        return SIN_MEM;
    }

    fCarrera = fopen(rutaCarrera, "rb");
    if (!fCarrera)
    {
        destruirCarrera(&c);
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    while (leerCarrera(fCarrera, &c) == TODO_OK)
    {
        if (filtrar(&c.info))
            acumularPuntosDesdeCarrera(&vPilotos, &c);
    }

    fclose(fCarrera);
    guardarVectorEnBin(rutaPiloto, &vPilotos);
    destruirCarrera(&c);
    destruirVector(&vPilotos);

    return TODO_OK;
}

int actualizarPuntosUltimaCarrera(const char* rutaCarrera,
                                  const char* rutaPiloto,
                                  Filter      filtrar,
                                  Reduce      reducir)
{
    tVector       vPilotos;
    Carrera       actual;
    Carrera       ultima;
    FILE*         fCarrera;
    int           hayUltima;
    CarreraHeader tmpH;
    tVector       tmpV;

    (void)reducir;

    if (crearVector(&vPilotos, sizeof(Piloto), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if (cargarVectorDesdeBin(rutaPiloto, &vPilotos))
    {
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    if (inicializarCarrera(&actual, MAX_PILOTOS_CARRERA) ||
            inicializarCarrera(&ultima, MAX_PILOTOS_CARRERA))
    {
        destruirVector(&vPilotos);
        return SIN_MEM;
    }

    fCarrera = fopen(rutaCarrera, "rb");
    if (!fCarrera)
    {
        destruirCarrera(&actual);
        destruirCarrera(&ultima);
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    hayUltima = 0;

    while (leerCarrera(fCarrera, &actual) == TODO_OK)
    {
        tmpH              = ultima.info;
        tmpV              = ultima.resultados;
        ultima.info       = actual.info;
        ultima.resultados = actual.resultados;
        actual.info       = tmpH;
        actual.resultados = tmpV;
        hayUltima         = 1;
    }

    fclose(fCarrera);
    destruirCarrera(&actual);

    if (hayUltima && filtrar(&ultima.info))
        acumularPuntosDesdeCarrera(&vPilotos, &ultima);

    guardarVectorEnBin(rutaPiloto, &vPilotos);
    destruirCarrera(&ultima);
    destruirVector(&vPilotos);

    return TODO_OK;
}

/* =========================================================
   Punteros a funcion del TDA Carrera
   ========================================================= */

void mostrarCarrera(const void* dato)
{
    const CarreraHeader* h    = (const CarreraHeader*)dato;
    unsigned             dia  = (unsigned)(h->fecha % 100);
    unsigned             mes  = (unsigned)(h->fecha / 100 % 100);
    unsigned             anio = (unsigned)(h->fecha / 10000);

    printf("----------------------------------\n");
    printf("ID Carrera  : %d\n",              h->id);
    printf("Circuito    : %s\n",              h->circuito);
    printf("Fecha       : %02u/%02u/%04u\n",  dia, mes, anio);
    printf("Estado      : %s\n",              h->estado == ESTADO_CARRERA_ACTIVA
           ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n",      h->cant_resultados);
}

void mostrarCarreraCompleta(const Carrera* c)
{
    int                    i;
    const ResultadoPiloto* rp;
    unsigned               dia  = (unsigned)(c->info.fecha % 100);
    unsigned               mes  = (unsigned)(c->info.fecha / 100 % 100);
    unsigned               anio = (unsigned)(c->info.fecha / 10000);

    printf("==================================\n");
    printf("ID Carrera  : %d\n",              c->info.id);
    printf("Circuito    : %s\n",              c->info.circuito);
    printf("Fecha       : %02u/%02u/%04u\n",  dia, mes, anio);
    printf("Estado      : %s\n",              c->info.estado == ESTADO_CARRERA_ACTIVA
           ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n",      c->info.cant_resultados);
    printf("----------------------------------\n");

    for (i = 0; i < (int)c->resultados.ce; i++)
    {
        rp = (const ResultadoPiloto*)obtenerElementoVector((tVector*)&c->resultados, i);

        printf("  Pos %2d | Piloto ID: %3u | Pts: %2d | %s\n",
               i + 1,
               rp->id_piloto,
               rp->puntos,
               estadoResultadoATexto(rp->estado_resultado));
    }
}

int listarTodasLasCarreras(const char* rutaCarrera)
{
    Carrera c;
    FILE*   fCarrera;

    fCarrera = fopen(rutaCarrera, "rb");
    if (!fCarrera)
        return ERR_ARCH;

    if (inicializarCarrera(&c, MAX_PILOTOS_CARRERA))
    {
        fclose(fCarrera);
        return SIN_MEM;
    }

    while (leerCarrera(fCarrera, &c) == TODO_OK)
        mostrarCarreraCompleta(&c);

    destruirCarrera(&c);
    fclose(fCarrera);

    return TODO_OK;
}

int filterEsCarreraActiva(const void* dato)
{
    return (((const CarreraHeader*)dato)->estado == ESTADO_CARRERA_ACTIVA);
}

int reduceAcumularPuntosCarrera(void* acumulador, const void* dato)
{
    (void)acumulador;
    (void)dato;
    return TODO_OK;
}



