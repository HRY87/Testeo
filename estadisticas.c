#include "estadisticas.h"
#include "carrera.h"
#include "piloto.h"

/* =========================================================
                    Calculo de estadisticas
   ========================================================= */

/**
 * calcularEstadisticasPiloto
 * Recorre TODO el archivo carreras.dat una sola vez y acumula,
 * para el piloto con id_piloto dado, las siguientes metricas:
 *   - victorias        : cantidad de veces que termino en posicion 1.
 *   - mejor_posicion   : minimo de posiciones registradas.
 *   - peor_posicion    : maximo de posiciones registradas.
 *   - suma_posiciones  : suma total para calcular el promedio externamente.
 *   - puntos_totales   : suma de puntos obtenidos (columna COL_PUNTOS).
 *   - cant_carreras    : cantidad de carreras en las que participo.
 * Las carreras con estado ESTADO_CARRERA_INACTIVA se omiten.
 * Retorna TODO_OK o ERR_ARCH si no pudo abrir el archivo.
 */
int calcularEstadisticasPiloto(const char* archivo, unsigned id_piloto, EstadisticaPiloto* estadistica)
{
    FILE*   pb;
    Carrera carrera;
    int     i;
    int     posicion;

    pb = fopen(archivo, "rb");
    if (pb == NULL)
        return ERR_ARCHIVO;

    /* inicializacion de todos los campos */
    estadistica->id_piloto       = id_piloto;
    estadistica->victorias       = 0;
    estadistica->suma_posiciones = 0;
    estadistica->cant_carreras   = 0;
    estadistica->mejor_posicion  = MAX_PILOTOS_CARRERA; /* arranca en el maximo posible */
    estadistica->peor_posicion   = 0;
    estadistica->puntos_totales  = 0;

    while (fread(&carrera, sizeof(Carrera), 1, pb) == 1)
    {
        /* solo procesa carreras activas */
        if (carrera.estado == ESTADO_CARRERA_ACTIVA)
        {
            for (i = 0; i < carrera.cant_resultados; i++)
            {
                if ((unsigned)carrera.resultados[i][COL_ID_PILOTO] == id_piloto)
                {
                    posicion = i + 1; /* la fila i corresponde a la posicion i+1 */

                    /* expresion booleana evita un if extra para victorias */
                    estadistica->victorias       += (posicion == 1);
                    estadistica->suma_posiciones += posicion;
                    estadistica->puntos_totales  += carrera.resultados[i][COL_PUNTOS];
                    estadistica->cant_carreras++;

                    if (posicion < estadistica->mejor_posicion)
                        estadistica->mejor_posicion = posicion;

                    if (posicion > estadistica->peor_posicion)
                        estadistica->peor_posicion = posicion;
                }
            }
        }
    }

    fclose(pb);
    return TODO_OK;
}

/* =========================================================
                    Consulta por carrera
   ========================================================= */

/**
 * mejorYPeorPosicion
 * Busca en carreras.dat la carrera con id_carrera dado.
 * Si existe y esta activa, muestra por pantalla:
 *   - El ID del piloto en la fila 0 (1er puesto).
 *   - El ID del piloto en la fila cant_resultados-1 (ultimo puesto).
 * Recorre el archivo secuencialmente hasta encontrar el ID.
 * Retorna TODO_OK, ERR_ARCH o ERR_NO_ENCONTRADO.
 */
int mejorYPeorPosicion(const char* rutaBin, int id_carrera)
{
    FILE*   pb;
    Carrera carrera;
    int     encontrado;

    pb = fopen(rutaBin, "rb");
    if (pb == NULL)
        return ERR_ARCHIVO;

    encontrado = 0;

    while (fread(&carrera, sizeof(Carrera), 1, pb) == 1 && !encontrado)
    {
        if (carrera.id == id_carrera && carrera.estado == ESTADO_CARRERA_ACTIVA)
        {
            encontrado = 1;

            printf("\n");
            printf("  +--------------------------------------+\n");
            printf("  |   RESULTADO CARRERA: %-14s |\n", carrera.circuito);
            printf("  +--------------------------------------+\n");
            printf("  | [1] Mejor posicion : ID %-10d |\n",carrera.resultados[0][COL_ID_PILOTO]);
            printf("  | [X] Peor posicion  : ID %-10d |\n", carrera.resultados[carrera.cant_resultados - 1][COL_ID_PILOTO]);
            printf("  +--------------------------------------+\n");
            printf("\n");
        }
    }

    fclose(pb);

    if (!encontrado)
    {
        printf("[!] Carrera no encontrada o cancelada.\n");
        return ERR_NO_ENCONTRADO;
    }

    return TODO_OK;
}

/* =========================================================
                      Mostrar estadisiticas
   ========================================================= */

/**
 * mostrarEstadisticasPiloto
 * Calcula los promedios a partir de suma_posiciones y puntos_totales
 * divididos por cant_carreras (cast a float para evitar division entera)
 * y muestra el resumen estadistico completo por pantalla.
 * Si cant_carreras == 0, informa que el piloto no tiene registros.
 * No retorna valor.
 */
void mostrarEstadisticasPiloto(EstadisticaPiloto* estadistica)
{
    float promedio_posiciones;
    float promedio_puntos;

    if (estadistica->cant_carreras == 0)
    {
        printf("[!] El piloto no tiene carreras registradas.\n");
        return;
    }

    /* cast a float para evitar division entera */
    promedio_posiciones = (float)estadistica->suma_posiciones / estadistica->cant_carreras;
    promedio_puntos     = (float)estadistica->puntos_totales  / estadistica->cant_carreras;

    printf("\n");
    printf("  +--------------------------------------+\n");
    printf("  |   ESTADISTICAS DEL PILOTO #%-5u     |\n", estadistica->id_piloto);
    printf("  +--------------------------------------+\n");
    printf("  | [V] Victorias          : %-10d |\n", estadistica->victorias);
    printf("  | [+] Mejor posicion     : %-10d |\n", estadistica->mejor_posicion);
    printf("  | [-] Peor posicion      : %-10d |\n", estadistica->peor_posicion);
    printf("  +--------------------------------------+\n");
    printf("  | [~] Prom. posiciones   : %-10.2f |\n", promedio_posiciones);
    printf("  | [~] Prom. puntos       : %-10.2f |\n", promedio_puntos);
    printf("  | [*] Puntos totales     : %-10d |\n", estadistica->puntos_totales);
    printf("  | [#] Carreras disputadas: %-10d |\n", estadistica->cant_carreras);
    printf("  +--------------------------------------+\n");
    printf("\n");
}
