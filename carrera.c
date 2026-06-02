#include <string.h>
#include "carrera.h"
#include "piloto.h"

/* =========================================================
   Tabla de puntos F1 (posicion 1 a 10, indice 0 = sin puntos)
   ========================================================= */
static const int PUNTOS_F1[11] = {0, 25, 18, 15, 12, 10, 8, 6, 4, 2, 1};

/* =========================================================
   Funcion auxiliar interna: carga un vector con todos los
   pilotos del .bin (sin filtrar, en orden de archivo).
   Se usa para actualizar los puntos acumulados.
   ========================================================= */
int cargarVectorPilotos(const char* rutaPiloto, tVector* vPilotos)
{
    if (crearVector(vPilotos, sizeof(Piloto), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if (cargarVectorDesdeBin(rutaPiloto, vPilotos))
    {
        destruirVector(vPilotos);
        return ERR_ARCH;
    }

    return TODO_OK;
}

/* =========================================================
   Alta de carrera
   ========================================================= */

/**
 * registrarCarrera
 * Solicita circuito y fecha al usuario, genera resultados
 * aleatorios con los pilotos activos y escribe la carrera
 * en modo append en carrera.bin.
 */
int registrarCarrera(const char* rutaCarrera,
                     const char* rutaPiloto,
                     Comparar    comparar)
{
    Carrera nueva;
    FILE*   fCarrera;

    fCarrera = fopen(rutaCarrera, "ab+");
    if (!fCarrera)
        return ERR_ARCH;

    memset(&nueva, 0, sizeof(Carrera));
    nueva.id     = generarIdCarrera(fCarrera);
    nueva.estado = ESTADO_CARRERA_ACTIVA;

    /* Limpiar el '\n' que dejo el scanf del menu */
    limpiarBuffer();

    printf("\nNombre del circuito: ");
    leerCadena(nueva.circuito, TAM_NOMBRE_CIRCUITO);

    do {
        printf("\nFecha de la carrera (AAAAMMDD): ");
        scanf("%llu", &nueva.fecha);
    } while (!esFechaValida(nueva.fecha));

    /* Generar posiciones aleatorias con los pilotos activos */
    if (cargarResultadosCarreraAleatorios(rutaPiloto, &nueva, comparar))
    {
        fclose(fCarrera);
        return ERR_ARCH;
    }

    fseek(fCarrera, 0L, SEEK_END);
    fwrite(&nueva, sizeof(Carrera), 1, fCarrera);
    fclose(fCarrera);

    return TODO_OK;
}

/**
 * cargarResultadosCarreraAleatorios
 * Carga los IDs de pilotos activos en un vector, los mezcla
 * aleatoriamente (Fisher-Yates) y asigna puntos F1 segun posicion.
 */
int cargarResultadosCarreraAleatorios(const char* rutaPiloto,
                                      Carrera*    nueva,
                                      Comparar    comparar)
{
    tVector  vIds;
    int      i;
    unsigned pos;

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

    nueva->cant_resultados = (int)vIds.ce;

    for (i = 0; i < nueva->cant_resultados; i++)
    {
        pos = *(unsigned*)obtenerElementoVector(&vIds, i);

        nueva->resultados[i][COL_ID_PILOTO] = (int)pos;
        /* Solo el top POS_LIMITE_PUNTOS_CARRERA suma puntos */
        nueva->resultados[i][COL_PUNTOS] =
            (i < POS_LIMITE_PUNTOS_CARRERA) ? PUNTOS_F1[i + 1] : 0;
    }

    destruirVector(&vIds);
    return TODO_OK;
}

/**
 * generarIdCarrera
 * Retorna el ID para la nueva carrera:
 *  - Si el archivo esta vacio: retorna 1.
 *  - Si no: lee el ID de la ultima carrera y retorna id+1.
 */
int generarIdCarrera(FILE* fCarrera)
{
    Carrera ultima;

    fseek(fCarrera, 0, SEEK_END);

    if (ftell(fCarrera) < (long)sizeof(Carrera))
        return 1;

    fseek(fCarrera, -(long)sizeof(Carrera), SEEK_END);
    fread(&ultima, sizeof(Carrera), 1, fCarrera);

    return (ultima.id + 1);
}

/* =========================================================
   Actualizacion de puntos
   ========================================================= */

/**
 * recalcularPuntosPilotos
 * Recalcula los puntos desde cero:
 *  1. Carga todos los pilotos en un vector.
 *  2. Resetea sus puntos a 0.
 *  3. Recorre carrera.bin y acumula solo las activas.
 *  4. Persiste el vector actualizado en piloto.bin.
 * Usar al inicio de temporada o al cancelar una carrera.
 */
int recalcularPuntosPilotos(const char* rutaCarrera,
                            const char* rutaPiloto,
                            Filter      filtrar,
                            Reduce      reducir)
{
    tVector vPilotos;
    char*   act;
    char*   fin;

    if (cargarVectorPilotos(rutaPiloto, &vPilotos))
        return ERR_ARCH;

    /* Resetear puntos de todos los pilotos */
    act = (char*)vPilotos.vec;
    fin = act + (vPilotos.ce * vPilotos.tamElem);
    while (act < fin)
    {
        ((Piloto*)act)->puntos_acumulados = 0;
        act += vPilotos.tamElem;
    }

    /* Acumular puntos de todas las carreras activas */
    procesarArchivoBinario(rutaCarrera, &vPilotos,
                           sizeof(Carrera), filtrar, reducir);

    guardarVectorEnBin(rutaPiloto, &vPilotos);
    destruirVector(&vPilotos);

    return TODO_OK;
}

/**
 * actualizarPuntosUltimaCarrera
 * Suma solo los puntos de la ultima carrera registrada.
 * Mas eficiente que recalcularPuntosPilotos para el flujo normal:
 * registrar carrera -> llamar a esta funcion.
 */
int actualizarPuntosUltimaCarrera(const char* rutaCarrera,
                                  const char* rutaPiloto,
                                  Filter      filtrar,
                                  Reduce      reducir)
{
    tVector vPilotos;
    Carrera carrera;
    FILE*   fCarrera;

    if (cargarVectorPilotos(rutaPiloto, &vPilotos))
        return SIN_MEM;

    fCarrera = fopen(rutaCarrera, "rb");
    if (!fCarrera)
    {
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    /* Posicionarse directamente en la ultima carrera */
    fseek(fCarrera, -(long)sizeof(Carrera), SEEK_END);
    fread(&carrera, sizeof(Carrera), 1, fCarrera);
    fclose(fCarrera);

    if (filtrar(&carrera))
        reducir(&vPilotos, &carrera);

    guardarVectorEnBin(rutaPiloto, &vPilotos);
    destruirVector(&vPilotos);

    return TODO_OK;
}

/* =========================================================
   Punteros a funcion del TDA Carrera
   ========================================================= */

/**
 * mostrarCarrera  [Mostrar]
 * Imprime la carrera con sus resultados completos.
 */
void mostrarCarrera(const void* dato)
{
    const Carrera* c    = (const Carrera*)dato;
    unsigned       anio = (unsigned)(c->fecha / 10000);
    unsigned       mes  = (unsigned)(c->fecha / 100 % 100);
    unsigned       dia  = (unsigned)(c->fecha % 100);
    int            i;

    printf("----------------------------------\n");
    printf("ID Carrera  : %d\n",  c->id);
    printf("Circuito    : %s\n",  c->circuito);
    printf("Fecha       : %02u/%02u/%04u\n", dia, mes, anio);
    printf("Estado      : %s\n",  c->estado == ESTADO_CARRERA_ACTIVA ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n", c->cant_resultados);

    for (i = 0; i < c->cant_resultados; i++)
    {
        printf("  Pos %2d | Piloto ID: %3d | Puntos: %2d\n",
               i + 1,
               c->resultados[i][COL_ID_PILOTO],
               c->resultados[i][COL_PUNTOS]);
    }
}

/**
 * filterEsCarreraActiva  [Filter]
 * Retorna 1 si la carrera esta activa, 0 si fue cancelada.
 */
int filterEsCarreraActiva(const void* dato)
{
    return (((const Carrera*)dato)->estado == ESTADO_CARRERA_ACTIVA);
}

/**
 * reduceAcumularPuntosCarrera  [Reduce]
 * Recibe un tVector de Pilotos como acumulador y una Carrera.
 * Para cada resultado de la carrera, busca el piloto por ID
 * (busqueda binaria) y le suma los puntos correspondientes.
 */
int reduceAcumularPuntosCarrera(void* acumulador, const void* dato)
{
    tVector*       vPilotos = (tVector*)acumulador;
    const Carrera* carrera  = (const Carrera*)dato;
    Piloto*        piloto;
    unsigned       idBuscar;
    size_t         i;

    for (i = 0; i < (size_t)carrera->cant_resultados; i++)
    {
        idBuscar = (unsigned)carrera->resultados[i][COL_ID_PILOTO];

        /* El vector esta ordenado por id: busqueda binaria O(log n) */
        piloto = (Piloto*)busquedaBinariaVector(vPilotos,
                                               &idBuscar,
                                               compararUnsigned);
        if (piloto)
            piloto->puntos_acumulados += (unsigned)carrera->resultados[i][COL_PUNTOS];
    }

    return TODO_OK;
}
