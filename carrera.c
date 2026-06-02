#include <stdio.h>
#include <string.h>
#include "carrera.h"
#include "piloto.h"

/* =========================================================
   Tabla de puntos F1 (posicion 1..10, indice 0 = sin puntos)
   ========================================================= */
static const int PUNTOS_F1[11] = {0, 25, 18, 15, 12, 10, 8, 6, 4, 2, 1};

/* =========================================================
   Ciclo de vida
   ========================================================= */

/**
 * inicializarCarrera
 * Prepara el tVector interno con 'capacidad' elementos.
 * Debe llamarse antes de cualquier uso de la Carrera.
 */
int inicializarCarrera(Carrera* c, int capacidad)
{
    memset(&c->info, 0, sizeof(CarreraHeader));
    return crearVector(&c->resultados, sizeof(ResultadoPiloto), (size_t)capacidad);
}

/**
 * destruirCarrera
 * Libera el tVector interno. No toca la memoria de la struct.
 */
void destruirCarrera(Carrera* c)
{
    destruirVector(&c->resultados);
}

/* =========================================================
   Serializacion: la UNICA interfaz con carrera.bin
   ========================================================= */

/**
 * escribirCarrera
 * Formato en disco:
 *   [ CarreraHeader ][ ResultadoPiloto * cant_resultados ]
 *
 * Precondicion: fCarrera apunta al final del archivo (fseek SEEK_END).
 */
int escribirCarrera(FILE* fCarrera, const Carrera* c)
{
    size_t i;
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

/**
 * leerCarrera
 * Lee el siguiente bloque [ CarreraHeader + N * ResultadoPiloto ]
 * desde la posicion actual del archivo.
 *
 * El tVector de 'c' DEBE estar ya inicializado con capacidad
 * suficiente (>= cant_resultados que se va a leer).
 *
 * Retorna TODO_OK si leyo una carrera completa,
 *         ERR_ARCH si llego al fin de archivo o hubo error.
 */
int leerCarrera(FILE* fCarrera, Carrera* c)
{
    int    i;
    ResultadoPiloto rp;

    /* Resetear el vector para reutilizar la carrera en un loop */
    c->resultados.ce = 0;

    if (fread(&c->info, sizeof(CarreraHeader), 1, fCarrera) != 1)
        return ERR_ARCH;   /* fin de archivo o error */

    for (i = 0; i < c->info.cant_resultados; i++)
    {
        if (fread(&rp, sizeof(ResultadoPiloto), 1, fCarrera) != 1)
            return ERR_ARCH;

        /* insertarVectorOrd no aplica aqui: el orden de llegada
           ES el orden de posicion de la carrera. */
        memcpy((char*)c->resultados.vec + (c->resultados.ce * sizeof(ResultadoPiloto)),
               &rp, sizeof(ResultadoPiloto));
        c->resultados.ce++;
    }

    return TODO_OK;
}

/* =========================================================
   Helpers internos
   ========================================================= */

/**
 * pedirDatosBase
 * Solicita circuito y fecha al usuario y los carga en c.
 * Factorizado para no repetir entre manual y aleatorio.
 */
static void pedirDatosBase(Carrera* c)
{
    limpiarBuffer();

    printf("\nNombre del circuito: ");
    leerCadena(c->info.circuito, TAM_NOMBRE_CIRCUITO);

    do {
        printf("Fecha de la carrera (AAAAMMDD): ");
        scanf("%llu", &c->info.fecha);
    } while (!esFechaValida(c->info.fecha));
}

/**
 * pedirTipoSancion
 * Muestra opciones y retorna el tipo de sancion elegido.
 */
static int pedirTipoSancion(int* carreras_out)
{
    int tipo;
    int carr = 0;

    printf("\n  Tipo de sancion:\n");
    printf("    1. Descuento de %d puntos\n",      PUNTOS_PENALIZACION_DSQ);
    printf("    2. Saltear carrera(s)\n");
    printf("    3. Puntos + saltear carrera(s)\n");
    printf("  Opcion: ");
    scanf("%d", &tipo);

    if (tipo < 1 || tipo > 3) tipo = SANCION_PUNTOS;

    if (tipo == SANCION_CARRERA || tipo == SANCION_AMBAS)
    {
        printf("  Carreras a saltear: ");
        scanf("%d", &carr);
        if (carr < 1) carr = 1;
    }

    *carreras_out = carr;
    return tipo;
}

/* =========================================================
   Alta de carrera
   ========================================================= */

/**
 * registrarCarreraAleatoria
 * Solicita circuito y fecha; genera posiciones con Fisher-Yates.
 * Llama a gestionarSancionesDSQ para los resultados DSQ
 * (en modo aleatorio no hay DSQ porque todos terminan,
 *  pero la firma queda preparada para futuras extensiones).
 */
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
    if (resp != TODO_OK)
    {
        fclose(fCarrera);
        destruirCarrera(&nueva);
        return resp;
    }

    fseek(fCarrera, 0L, SEEK_END);
    resp = escribirCarrera(fCarrera, &nueva);

    fclose(fCarrera);
    destruirCarrera(&nueva);

    return resp;
}

/**
 * registrarCarreraManual
 *
 * El usuario ingresa, para cada piloto que participo:
 *   - ID del piloto
 *   - Estado del resultado (F/N/D/Q)
 *   - Puntos obtenidos (sugeridos automaticamente segun posicion,
 *     pero el usuario puede modificarlos)
 *
 * Para los pilotos con estado DSQ se invoca aplicarSancionDSQ()
 * al finalizar el ingreso.
 *
 * El ingreso termina cuando el usuario ingresa id = 0.
 */
int registrarCarreraManual(const char* rutaCarrera,
                           const char* rutaPiloto,
                           const char* rutaSancion,
                           Comparar    comparar)
{
    Carrera          nueva;
    ResultadoPiloto  rp;
    FILE*            fCarrera;
    int              resp;
    int              pos;         /* posicion actual (1-based) */
    unsigned         idPiloto;
    char             estadoChar;
    int              puntosSug;   /* puntos sugeridos por posicion */
    int              tipoSancion;
    int              carrerasSancion;

    /* Array local para guardar los DSQ y procesarlos al final */
    unsigned dsqIds[MAX_PILOTOS_CARRERA];
    int      dsqCant = 0;
    int      k;

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
    limpiarBuffer();

    printf("\nIngreso de resultados (ID piloto = 0 para terminar)\n");
    printf("Estados: F=Termino  N=DNF  D=DNS  Q=DSQ\n");
    printf("--------------------------------------------\n");

    pos = 1;

    for (;;)
    {
        if (nueva.resultados.ce >= nueva.resultados.tope)
        {
            printf("[!] Capacidad maxima de pilotos alcanzada.\n");
            break;
        }

        printf("Posicion %2d - ID piloto (0 = fin): ", pos);
        scanf("%u", &idPiloto);

        if (idPiloto == 0)
            break;

        /* Estado del resultado */
        printf("           Estado [F/N/D/Q]: ");
        scanf(" %c", &estadoChar);

        /* Normalizar a mayuscula */
        if (estadoChar >= 'a' && estadoChar <= 'z')
            estadoChar = (char)(estadoChar - 'a' + 'A');

        /* Validar estado */
        if (estadoChar != RES_FIN  && estadoChar != RES_DNF &&
            estadoChar != RES_DNS  && estadoChar != RES_DSQ)
        {
            printf("[!] Estado invalido, se asigna F.\n");
            estadoChar = RES_FIN;
        }

        /* Puntos sugeridos segun posicion y estado */
        if (estadoChar == RES_FIN && pos <= POS_LIMITE_PUNTOS_CARRERA)
            puntosSug = PUNTOS_F1[pos];
        else
            puntosSug = 0;

        printf("           Puntos (sugerido %2d, Enter para aceptar, -1 editar): ",
               puntosSug);
        {
            int pInput;
            scanf("%d", &pInput);
            rp.puntos = (pInput == -1) ? puntosSug : pInput;
        }

        rp.id_piloto        = idPiloto;
        rp.estado_resultado = (int)estadoChar;

        /* Agregar al vector directamente (orden de ingreso = posicion) */
        memcpy((char*)nueva.resultados.vec +
               (nueva.resultados.ce * sizeof(ResultadoPiloto)),
               &rp, sizeof(ResultadoPiloto));
        nueva.resultados.ce++;
        nueva.info.cant_resultados++;

        /* Registrar DSQ para procesar al final */
        if (estadoChar == RES_DSQ && dsqCant < MAX_PILOTOS_CARRERA)
            dsqIds[dsqCant++] = idPiloto;

        pos++;
    }

    /* Persistir la carrera */
    fseek(fCarrera, 0L, SEEK_END);
    resp = escribirCarrera(fCarrera, &nueva);
    fclose(fCarrera);

    /* Procesar sanciones DSQ */
    for (k = 0; k < dsqCant; k++)
    {
        printf("\n[DSQ] Piloto ID %u - Elegir sancion:\n", dsqIds[k]);
        tipoSancion = pedirTipoSancion(&carrerasSancion);

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

/**
 * cargarResultadosAleatorios
 * Carga IDs de pilotos activos, los mezcla con Fisher-Yates
 * y asigna puntos F1 segun posicion. Todos terminan con RES_FIN.
 */
int cargarResultadosAleatorios(const char* rutaPiloto,
                               Carrera*    c,
                               Comparar    comparar)
{
    tVector         vIds;
    int             i;
    unsigned        idPos;
    ResultadoPiloto rp;

    if (crearVector(&vIds, sizeof(unsigned), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if (cargarVectorPilotoActivos(rutaPiloto, &vIds, comparar))
    {
        destruirVector(&vIds);
        return ERR_ARCH;
    }

    if (vIds.ce == 0)
    {
        destruirVector(&vIds);
        return ERR_ARCH;
    }

    generarResultadoAleatorioVector(&vIds);

    c->info.cant_resultados = (int)vIds.ce;

    for (i = 0; i < c->info.cant_resultados; i++)
    {
        idPos = *(unsigned*)obtenerElementoVector(&vIds, i);

        rp.id_piloto        = idPos;
        rp.estado_resultado = (int)RES_FIN;
        rp.puntos           = (i < POS_LIMITE_PUNTOS_CARRERA)
                              ? PUNTOS_F1[i + 1] : 0;

        memcpy((char*)c->resultados.vec + (i * sizeof(ResultadoPiloto)),
               &rp, sizeof(ResultadoPiloto));
        c->resultados.ce++;
    }

    destruirVector(&vIds);
    return TODO_OK;
}

/**
 * generarIdCarrera
 * Lee el ultimo CarreraHeader del archivo para obtener el id.
 * Si el archivo esta vacio retorna 1.
 */
int generarIdCarrera(FILE* fCarrera)
{
    long          tam;

    fseek(fCarrera, 0, SEEK_END);
    tam = ftell(fCarrera);

    if (tam < (long)sizeof(CarreraHeader))
        return 1;

    /* Leer el ULTIMO header: retroceder desde el final.
       No podemos saltar directamente porque los bloques tienen
       tamanio variable; hay que recorrer desde el principio
       y quedarse con el ultimo id leido. */
    {
        int    ultimoId = 1;
        Carrera tmp;

        fseek(fCarrera, 0, SEEK_SET);

        if (inicializarCarrera(&tmp, MAX_PILOTOS_CARRERA) == TODO_OK)
        {
            while (leerCarrera(fCarrera, &tmp) == TODO_OK)
                ultimoId = tmp.info.id;

            destruirCarrera(&tmp);
        }

        return ultimoId + 1;
    }
}

/* =========================================================
   Actualizacion de puntos en piloto.bin
   ========================================================= */

/**
 * recalcularPuntosPilotos
 * 1. Carga todos los pilotos en un vector.
 * 2. Resetea puntos a 0.
 * 3. Recorre carrera.bin acumulando solo carreras activas.
 * 4. Persiste el vector actualizado.
 */
int recalcularPuntosPilotos(const char* rutaCarrera,
                            const char* rutaPiloto,
                            Filter      filtrar,
                            Reduce      reducir)
{
    tVector vPilotos;
    Carrera c;
    FILE*   fCarrera;
    char*   act;
    char*   fin;
    size_t  i;
    ResultadoPiloto* rp;
    Piloto*          piloto;

    if (crearVector(&vPilotos, sizeof(Piloto), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if (cargarVectorDesdeBin(rutaPiloto, &vPilotos))
    {
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    /* Resetear puntos de todos los pilotos */
    act = (char*)vPilotos.vec;
    fin = act + (vPilotos.ce * vPilotos.tamElem);
    while (act < fin)
    {
        ((Piloto*)act)->puntos_acumulados = 0;
        act += vPilotos.tamElem;
    }

    if (inicializarCarrera(&c, MAX_PILOTOS_CARRERA))
    {
        destruirVector(&vPilotos);
        return SIN_MEM;
    }

    fCarrera = fopen(rutaCarrera, "rb");
    if (!fCarrera)
    {
        destruirVector(&vPilotos);
        destruirCarrera(&c);
        return ERR_ARCH;
    }

    /* Recorrer todas las carreras y acumular puntos */
    while (leerCarrera(fCarrera, &c) == TODO_OK)
    {
        /* Solo carreras activas */
        if (!filtrar(&c.info))
            continue;

        /* Acumular puntos: busqueda binaria por id en vPilotos */
        for (i = 0; i < (size_t)c.info.cant_resultados; i++)
        {
            rp = (ResultadoPiloto*)obtenerElementoVector(&c.resultados, i);
            piloto = (Piloto*)busquedaBinariaVector(&vPilotos,
                                                    &rp->id_piloto,
                                                    compararUnsigned);
            if (piloto && rp->puntos > 0)
                piloto->puntos_acumulados += (unsigned)rp->puntos;
        }
    }

    fclose(fCarrera);

    guardarVectorEnBin(rutaPiloto, &vPilotos);
    destruirCarrera(&c);
    destruirVector(&vPilotos);

    return TODO_OK;
}

/**
 * actualizarPuntosUltimaCarrera
 * Mas eficiente: lee solo la ultima carrera y acumula.
 */
int actualizarPuntosUltimaCarrera(const char* rutaCarrera,
                                  const char* rutaPiloto,
                                  Filter      filtrar,
                                  Reduce      reducir)
{
    tVector vPilotos;
    Carrera actual;
    Carrera ultima;
    FILE*   fCarrera;
    int     hayUltima = 0;
    size_t  i;
    ResultadoPiloto* rp;
    Piloto*          piloto;

    (void)reducir; /* la reduccion se hace directamente aqui */

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

    /* Recorrer hasta el final para obtener la ultima carrera */
    while (leerCarrera(fCarrera, &actual) == TODO_OK)
    {
        /* Intercambiar actual <-> ultima copiando el header y el vector */
        {
            CarreraHeader tmpH = ultima.info;
            tVector       tmpV = ultima.resultados;

            ultima.info        = actual.info;
            ultima.resultados  = actual.resultados;

            actual.info        = tmpH;
            actual.resultados  = tmpV;
        }
        hayUltima = 1;
    }

    fclose(fCarrera);
    destruirCarrera(&actual);  /* liberar el vector del temporal */

    if (hayUltima && filtrar(&ultima.info))
    {
        for (i = 0; i < (size_t)ultima.info.cant_resultados; i++)
        {
            rp = (ResultadoPiloto*)obtenerElementoVector(&ultima.resultados, i);
            piloto = (Piloto*)busquedaBinariaVector(&vPilotos,
                                                    &rp->id_piloto,
                                                    compararUnsigned);
            if (piloto && rp->puntos > 0)
                piloto->puntos_acumulados += (unsigned)rp->puntos;
        }
    }

    guardarVectorEnBin(rutaPiloto, &vPilotos);
    destruirCarrera(&ultima);
    destruirVector(&vPilotos);

    return TODO_OK;
}

/* =========================================================
   Punteros a funcion del TDA Carrera
   ========================================================= */

/**
 * mostrarCarrera  [Mostrar]
 * Recibe un CarreraHeader* (no Carrera*) porque
 * procesarArchivoBinario trabaja con tamElem = sizeof(CarreraHeader).
 *
 * Para mostrar con resultados usar mostrarCarreraCompleta().
 */
void mostrarCarrera(const void* dato)
{
    const CarreraHeader* h    = (const CarreraHeader*)dato;
    unsigned             anio = (unsigned)(h->fecha / 10000);
    unsigned             mes  = (unsigned)(h->fecha / 100 % 100);
    unsigned             dia  = (unsigned)(h->fecha % 100);

    printf("----------------------------------\n");
    printf("ID Carrera  : %d\n",  h->id);
    printf("Circuito    : %s\n",  h->circuito);
    printf("Fecha       : %02u/%02u/%04u\n", dia, mes, anio);
    printf("Estado      : %s\n",  h->estado == ESTADO_CARRERA_ACTIVA
                                  ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n", h->cant_resultados);
}

/**
 * mostrarCarreraCompleta
 * Version extendida que incluye los resultados detallados.
 * Requiere abrir el archivo y leer la carrera completa.
 */
void mostrarCarreraCompleta(const Carrera* c)
{
    int                    i;
    const ResultadoPiloto* rp;
    const char*            estStr;
    unsigned               anio = (unsigned)(c->info.fecha / 10000);
    unsigned               mes  = (unsigned)(c->info.fecha / 100 % 100);
    unsigned               dia  = (unsigned)(c->info.fecha % 100);

    printf("==================================\n");
    printf("ID Carrera  : %d\n",  c->info.id);
    printf("Circuito    : %s\n",  c->info.circuito);
    printf("Fecha       : %02u/%02u/%04u\n", dia, mes, anio);
    printf("Estado      : %s\n",  c->info.estado == ESTADO_CARRERA_ACTIVA
                                  ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n", c->info.cant_resultados);
    printf("----------------------------------\n");

    for (i = 0; i < (int)c->resultados.ce; i++)
    {
        rp = (const ResultadoPiloto*)obtenerElementoVector(
                 (tVector*)&c->resultados, i);

        switch ((char)rp->estado_resultado)
        {
            case RES_FIN: estStr = "FIN"; break;
            case RES_DNF: estStr = "DNF"; break;
            case RES_DNS: estStr = "DNS"; break;
            case RES_DSQ: estStr = "DSQ"; break;
            default:      estStr = "???"; break;
        }

        printf("  Pos %2d | Piloto ID: %3u | Pts: %2d | %s\n",
               i + 1,
               rp->id_piloto,
               rp->puntos,
               estStr);
    }
}

/**
 * listarTodasLasCarreras
 * Recorre carrera.bin y muestra cada carrera completa.
 */
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

/**
 * filterEsCarreraActiva  [Filter]
 * Opera sobre CarreraHeader* porque procesarArchivoBinario
 * trabaja a nivel de header (tamElem = sizeof(CarreraHeader)).
 */
int filterEsCarreraActiva(const void* dato)
{
    return (((const CarreraHeader*)dato)->estado == ESTADO_CARRERA_ACTIVA);
}

/**
 * reduceAcumularPuntosCarrera  [Reduce]
 *
 * ATENCION: esta funcion recibe un CarreraHeader*, NO una Carrera*.
 * Los resultados NO estan disponibles en este contexto porque
 * procesarArchivoBinario solo lee sizeof(CarreraHeader) por registro.
 *
 * Para acumular puntos con resultados variables usar
 * recalcularPuntosPilotos() que lee carreras completas.
 *
 * Esta implementacion queda como stub para compatibilidad.
 * Ver recalcularPuntosPilotos() para la logica real.
 */
int reduceAcumularPuntosCarrera(void* acumulador, const void* dato)
{
    /* Ver comentario arriba: la acumulacion real se hace en
       recalcularPuntosPilotos con leerCarrera() completa. */
    (void)acumulador;
    (void)dato;
    return TODO_OK;
}
