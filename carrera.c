#include <stdio.h>
#include <string.h>
#include "carrera.h"
#include "piloto.h"


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
        /* Mejora #8: guard contra desbordamiento del vector */
        if (c->resultados.ce >= c->resultados.tope)
            break;

        if (fread(&rp, sizeof(ResultadoPiloto), 1, fCarrera) != 1)
            return ERR_ARCH;

        memcpy((char*)c->resultados.vec +
               (c->resultados.ce * sizeof(ResultadoPiloto)),
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
 * Solicita circuito y fecha.
 * Mejora #4: verifica retorno de scanf antes de validar fecha.
 */
static void pedirDatosBase(Carrera* c)
{
    int leido;

    printf("\nNombre del circuito: ");
    limpiarBuffer();
    leerCadena(c->info.circuito, TAM_NOMBRE_CIRCUITO);

    do
    {
        printf("Fecha de la carrera (AAAAMMDD): ");
        leido = scanf("%llu", &c->info.fecha);

        if (leido != 1)
        {
            limpiarBuffer();
            printf("[!] Entrada invalida. Ingrese solo numeros (AAAAMMDD).\n");
            c->info.fecha = 0;
            continue;
        }

        limpiarBuffer();

        if (!esFechaValida(c->info.fecha))
            printf("[!] Fecha invalida. Use formato AAAAMMDD. (ej: 20260301)\n");

    }
    while (!esFechaValida(c->info.fecha));
}

/*
 * pedirEstadoResultado
 * Mejora #3: verifica retorno de scanf y limpia buffer ante entrada no numerica.
 */
static int pedirEstadoResultado(void)
{
    int estado;
    int leido;

    do
    {
        printf("Estado [1=FIN 2=DNF 3=DNS 4=DSQ]: ");
        leido = scanf("%d", &estado);

        if (leido != 1)
        {
            limpiarBuffer();
            printf("[!] Entrada invalida. Ingrese un numero del 1 al 4.\n");
            estado = 0;
            continue;
        }

        limpiarBuffer();

        if (estado < RES_FIN || estado > RES_DSQ)
            printf("[!] Estado invalido. Ingrese un valor entre 1 y 4.\n");

    }
    while (estado < RES_FIN || estado > RES_DSQ);

    return estado;
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
        rp         = (ResultadoPiloto*)obtenerElementoVector(
                         (tVector*)&c->resultados, k);
        encontrado = (rp->id_piloto == idPiloto);
        k++;
    }

    return encontrado;
}

/*
 * acumularPuntosDesdeCarrera
 * Suma los puntos de todos los resultados de una carrera
 * al vector de pilotos (busqueda binaria por id).
 */
static void acumularPuntosDesdeCarrera(tVector* vPilotos, const Carrera* c)
{
    size_t           i;
    ResultadoPiloto* rp;
    Piloto*          piloto;

    for (i = 0; i < (size_t)c->info.cant_resultados; i++)
    {
        rp     = (ResultadoPiloto*)obtenerElementoVector(
                     (tVector*)&c->resultados, i);
        piloto = (Piloto*)busquedaBinariaVector(vPilotos,
                                               &rp->id_piloto,
                                               compararUnsigned);

        if (piloto && rp->puntos > 0)
            piloto->puntos_acumulados += (unsigned)rp->puntos;
    }
}

/*
 * estadoResultadoATexto
 */
static const char* estadoResultadoATexto(int estado)
{
    switch (estado)
    {
    case RES_FIN: return "FIN";
    case RES_DNF: return "DNF";
    case RES_DNS: return "DNS";
    case RES_DSQ: return "DSQ";
    default:      return "???";
    }
}

/*
 * ordenarResultadosFINPrimero
 * Ordena el vector de resultados: primero los RES_FIN (en orden
 * de ingreso = orden de llegada), luego los demas (DNF/DNS/DSQ).
 * Usa insertion sort estable para mantener el orden relativo.
 */
static void ordenarResultadosFINPrimero(tVector* vRes)
{
    size_t           i, j;
    ResultadoPiloto  tmp;
    ResultadoPiloto* actual;
    ResultadoPiloto* anterior;

    for (i = 1; i < vRes->ce; i++)
    {
        actual = (ResultadoPiloto*)obtenerElementoVector(vRes, i);

        if (actual->estado_resultado == RES_FIN)
        {
            j = i;
            while (j > 0)
            {
                anterior = (ResultadoPiloto*)obtenerElementoVector(vRes, j - 1);
                if (anterior->estado_resultado != RES_FIN)
                {
                    memcpy(&tmp,    anterior, sizeof(ResultadoPiloto));
                    memcpy(anterior,
                           obtenerElementoVector(vRes, j),
                           sizeof(ResultadoPiloto));
                    memcpy(obtenerElementoVector(vRes, j), &tmp,
                           sizeof(ResultadoPiloto));
                    j--;
                }
                else
                    break;
            }
        }
    }
}

/*
 * completarConDNS
 * Agrega como DNS (0 puntos) todos los pilotos del vector vIdsActivos
 * que no hayan sido ingresados todavia en la carrera.
 */
static void completarConDNS(Carrera* c, const tVector* vIdsActivos)
{
    size_t           k;
    unsigned         idActivo;
    ResultadoPiloto  rp;

    for (k = 0; k < vIdsActivos->ce; k++)
    {
        idActivo = *(unsigned*)obtenerElementoVector((tVector*)vIdsActivos, k);

        if (!esPilotoDuplicado(c, idActivo))
        {
            if (c->resultados.ce >= c->resultados.tope)
                break;

            rp.id_piloto        = idActivo;
            rp.estado_resultado = RES_DNS;
            rp.puntos           = 0;

            memcpy((char*)c->resultados.vec +
                   (c->resultados.ce * sizeof(ResultadoPiloto)),
                   &rp, sizeof(ResultadoPiloto));

            c->resultados.ce++;
            c->info.cant_resultados++;
        }
    }
}

/* =========================================================
   Alta de carrera
   ========================================================= */

int registrarCarreraAleatoria(const char*         rutaCarrera,
                              const char*         rutaPiloto,
                              Comparar            comparar,
                              const Puntos* cfg)
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

    /* Mejora #9: avisar si la fecha es anterior a la ultima carrera */
    {
        Carrera tmp;
        Carrera ultima;
        int     hayUltima = 0;

        if (inicializarCarrera(&tmp,   MAX_PILOTOS_CARRERA) == TODO_OK &&
            inicializarCarrera(&ultima, MAX_PILOTOS_CARRERA) == TODO_OK)
        {
            FILE* fLec = fopen(rutaCarrera, "rb");
            if (fLec)
            {
                while (leerCarrera(fLec, &tmp) == TODO_OK)
                {
                    ultima.info             = tmp.info;
                    ultima.resultados.ce    = tmp.resultados.ce;
                    memcpy(ultima.resultados.vec, tmp.resultados.vec,
                           tmp.resultados.ce * sizeof(ResultadoPiloto));
                    hayUltima = 1;
                }
                fclose(fLec);
            }

            if (hayUltima && nueva.info.fecha < ultima.info.fecha)
                printf("[!] Advertencia: la fecha ingresada es anterior "
                       "a la ultima carrera registrada.\n");

            destruirCarrera(&tmp);
            destruirCarrera(&ultima);
        }
    }

    resp = cargarResultadosAleatorios(rutaPiloto, &nueva, comparar, cfg);

    if (resp == TODO_OK)
    {
        fseek(fCarrera, 0L, SEEK_END);
        resp = escribirCarrera(fCarrera, &nueva);
    }

    fclose(fCarrera);
    destruirCarrera(&nueva);

    return resp;
}

int registrarCarreraManual(const char*         rutaCarrera,
                           const char*         rutaPiloto,
                           Comparar            comparar,
                           const Puntos* cfg)
{
    Carrera         nueva;
    ResultadoPiloto rp;
    tVector         vIdsActivos;
    FILE*           fCarrera;
    int             resp;
    int             pos;
    int             pInput;
    int             puntosSug;
    int             hayMas;
    int             leido;
    unsigned        idIngresado;
    int             totalActivos;

    if (inicializarCarrera(&nueva, MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    /* Cargar IDs activos para validacion y autocomplete */
    if (crearVector(&vIdsActivos, sizeof(unsigned), MAX_PILOTOS_CARRERA))
    {
        destruirCarrera(&nueva);
        return SIN_MEM;
    }

    if (cargarVectorPilotoActivos(rutaPiloto, &vIdsActivos, comparar) != TODO_OK
        || vIdsActivos.ce == 0)
    {
        printf("[!] No hay pilotos activos para registrar la carrera.\n");
        destruirVector(&vIdsActivos);
        destruirCarrera(&nueva);
        return VEC_VACIO;
    }

    totalActivos = (int)vIdsActivos.ce;

    fCarrera = fopen(rutaCarrera, "ab+");
    if (!fCarrera)
    {
        destruirVector(&vIdsActivos);
        destruirCarrera(&nueva);
        return ERR_ARCH;
    }

    nueva.info.id              = generarIdCarrera(fCarrera);
    nueva.info.estado          = ESTADO_CARRERA_ACTIVA;
    nueva.info.cant_resultados = 0;

    pedirDatosBase(&nueva);

    printf("\nTotal de pilotos activos: %d\n", totalActivos);
    printf("Ingreso de resultados (ID piloto = 0 para terminar;\n");
    printf("pilotos no ingresados se completaran como DNS)\n");
    printf("Estados: 1=FIN  2=DNF  3=DNS  4=DSQ\n");
    printf("DNF/DNS/DSQ asignan 0 puntos automaticamente.\n");
    printf("--------------------------------------------\n");

    pos    = 1;
    hayMas = 1;

    while (hayMas && (int)nueva.resultados.ce < totalActivos)
    {
        printf("Posicion %2d - ID piloto (0 = fin): ", pos);
        leido = scanf("%u", &idIngresado);

        if (leido != 1)
        {
            limpiarBuffer();
            printf("[!] Entrada invalida. Ingrese un numero.\n");
            continue;
        }
        limpiarBuffer();

        if (idIngresado == 0)
        {
            hayMas = 0;
        }
        /* Mejora #2 + #10: verificar que existe Y esta activo */
        else if (busquedaBinariaVector(&vIdsActivos, &idIngresado, comparar) == NULL)
        {
            printf("[!] Piloto ID %u no existe o no esta activo.\n",
                   idIngresado);
        }
        else if (esPilotoDuplicado(&nueva, idIngresado))
        {
            printf("[!] Piloto ID %u ya fue registrado en esta carrera.\n",
                   idIngresado);
        }
        else
        {
            rp.id_piloto        = idIngresado;
            rp.estado_resultado = pedirEstadoResultado();

            if (rp.estado_resultado != RES_FIN)
            {
                rp.puntos = 0;
                printf("           Puntos: 0 (estado %s)\n",
                       estadoResultadoATexto(rp.estado_resultado));
            }
            else
            {
                /* FIN: sugerir puntos segun posicion de la tabla configurable */
                puntosSug = puntosParaPosicion(cfg, pos);

                printf("           Puntos (sugerido %2d, -1 para aceptar): ",
                       puntosSug);
                leido = scanf("%d", &pInput);
                if (leido != 1)
                {
                    limpiarBuffer();
                    pInput = -1;
                }
                else
                    limpiarBuffer();

                rp.puntos = (pInput < 0) ? puntosSug : pInput;
            }

            memcpy((char*)nueva.resultados.vec +
                   (nueva.resultados.ce * sizeof(ResultadoPiloto)),
                   &rp, sizeof(ResultadoPiloto));

            nueva.resultados.ce++;
            nueva.info.cant_resultados++;
            pos++;
        }
    }

    /* Autocomplete: pilotos no ingresados -> DNS */
    if ((int)nueva.resultados.ce < totalActivos)
    {
        printf("[i] Completando %d piloto(s) restante(s) como DNS...\n",
               totalActivos - (int)nueva.resultados.ce);
        completarConDNS(&nueva, &vIdsActivos);
    }

    /* Ordenar: FIN primero, luego DNF/DNS/DSQ */
    ordenarResultadosFINPrimero(&nueva.resultados);

    fseek(fCarrera, 0L, SEEK_END);
    resp = escribirCarrera(fCarrera, &nueva);
    fclose(fCarrera);
    destruirVector(&vIdsActivos);
    destruirCarrera(&nueva);

    return resp;
}

int cargarResultadosAleatorios(const char*         rutaPiloto,
                               Carrera*            c,
                               Comparar            comparar,
                               const Puntos* cfg)
{
    tVector         vIds;
    int             i;
    ResultadoPiloto rp;

    if (crearVector(&vIds, sizeof(unsigned), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if (cargarVectorPilotoActivos(rutaPiloto, &vIds, comparar) != TODO_OK)
    {
        destruirVector(&vIds);
        return ERR_ARCH;
    }

    /* Mejora #1: vector vacio = error descriptivo */
    if (vIds.ce == 0)
    {
        printf("[!] No hay pilotos activos para generar la carrera.\n");
        destruirVector(&vIds);
        return VEC_VACIO;
    }

    generarResultadoAleatorioVector(&vIds);

    c->info.cant_resultados = (int)vIds.ce;

    for (i = 0; i < c->info.cant_resultados; i++)
    {
        rp.id_piloto        = *(unsigned*)obtenerElementoVector(&vIds, i);
        rp.estado_resultado = RES_FIN;
        rp.puntos           = puntosParaPosicion(cfg, i + 1);

        memcpy((char*)c->resultados.vec + (i * sizeof(ResultadoPiloto)),
               &rp, sizeof(ResultadoPiloto));
        c->resultados.ce++;
    }

    destruirVector(&vIds);
    return TODO_OK;
}

/*
 * generarIdCarrera
 * Mejora #5: busca el MAXIMO id, no el ultimo.
 */
int generarIdCarrera(FILE* fCarrera)
{
    Carrera tmp;
    int     maxId;

    fseek(fCarrera, 0, SEEK_END);

    if (ftell(fCarrera) < (long)sizeof(CarreraHeader))
        return 1;

    maxId = 0;
    fseek(fCarrera, 0, SEEK_SET);

    if (inicializarCarrera(&tmp, MAX_PILOTOS_CARRERA) == TODO_OK)
    {
        while (leerCarrera(fCarrera, &tmp) == TODO_OK)
        {
            if (tmp.info.id > maxId)
                maxId = tmp.info.id;
        }
        destruirCarrera(&tmp);
    }

    return maxId + 1;
}

/* =========================================================
   Actualizacion de puntos en piloto.bin
   ========================================================= */

/*
 * recalcularPuntosPilotos
 * Mejora #12: parametro Reduce eliminado, no se usaba.
 */
int recalcularPuntosPilotos(const char* rutaCarrera,
                            const char* rutaPiloto,
                            Filter      filtrar)
{
    tVector  vPilotos;
    Carrera  c;
    FILE*    fCarrera;
    size_t   i;
    Piloto*  p;

    if (crearVector(&vPilotos, sizeof(Piloto), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if (cargarVectorDesdeBin(rutaPiloto, &vPilotos))
    {
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    /* Resetear puntos antes de recalcular */
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

/*
 * actualizarPuntosUltimaCarrera
 * Mejora #7 + #12: sin swap manual, sin parametro Reduce.
 */
int actualizarPuntosUltimaCarrera(const char* rutaCarrera,
                                  const char* rutaPiloto,
                                  Filter      filtrar)
{
    tVector  vPilotos;
    Carrera  c;
    Carrera  ultima;
    FILE*    fCarrera;
    int      hayUltima;

    if (crearVector(&vPilotos, sizeof(Piloto), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if (cargarVectorDesdeBin(rutaPiloto, &vPilotos))
    {
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    if (inicializarCarrera(&c,      MAX_PILOTOS_CARRERA) ||
        inicializarCarrera(&ultima, MAX_PILOTOS_CARRERA))
    {
        destruirVector(&vPilotos);
        return SIN_MEM;
    }

    fCarrera = fopen(rutaCarrera, "rb");
    if (!fCarrera)
    {
        destruirCarrera(&c);
        destruirCarrera(&ultima);
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    hayUltima = 0;

    while (leerCarrera(fCarrera, &c) == TODO_OK)
    {
        ultima.info          = c.info;
        ultima.resultados.ce = c.resultados.ce;
        memcpy(ultima.resultados.vec, c.resultados.vec,
               c.resultados.ce * sizeof(ResultadoPiloto));
        hayUltima = 1;
    }

    fclose(fCarrera);
    destruirCarrera(&c);

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
    printf("ID Carrera  : %d\n",             h->id);
    printf("Circuito    : %s\n",             h->circuito);
    printf("Fecha       : %02u/%02u/%04u\n", dia, mes, anio);
    printf("Estado      : %s\n",             h->estado == ESTADO_CARRERA_ACTIVA
           ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n",     h->cant_resultados);
}

void mostrarCarreraCompleta(const Carrera* c)
{
    int                    i;
    const ResultadoPiloto* rp;
    unsigned               dia  = (unsigned)(c->info.fecha % 100);
    unsigned               mes  = (unsigned)(c->info.fecha / 100 % 100);
    unsigned               anio = (unsigned)(c->info.fecha / 10000);

    printf("==================================\n");
    printf("ID Carrera  : %d\n",             c->info.id);
    printf("Circuito    : %s\n",             c->info.circuito);
    printf("Fecha       : %02u/%02u/%04u\n", dia, mes, anio);
    printf("Estado      : %s\n",             c->info.estado == ESTADO_CARRERA_ACTIVA
           ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n",     c->info.cant_resultados);
    printf("----------------------------------\n");

    for (i = 0; i < (int)c->resultados.ce; i++)
    {
        rp = (const ResultadoPiloto*)obtenerElementoVector(
                 (tVector*)&c->resultados, i);

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

/* Mejora #12: stub de compatibilidad, no hace nada */
int reduceAcumularPuntosCarrera(void* acumulador, const void* dato)
{
    (void)acumulador;
    (void)dato;
    return TODO_OK;
}
