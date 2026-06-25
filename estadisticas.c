#include "estadisticas.h"
#include "carrera.h"


/**
 * calcularEstadisticasPiloto
 * Recorre TODO el archivo carreras.dat una sola vez y acumula,
 * para el piloto con id_piloto dado, las siguientes metricas:
 *   - victorias         (posicion == 1)
 *   - mejor_posicion    (minimo de posiciones)
 *   - peor_posicion     (maximo de posiciones)
 *   - suma_posiciones   (para calcular promedio externamente)
 *   - puntos_totales    (columna 1 de resultados[][1])
 *   - cant_carreras     (cuantas veces aparecio el piloto)
 * Las carreras canceladas (estado == 0) se saltan.
 * Retorna TODO_OK o ERR_ARCH si no pudo abrir el archivo.
 */
//Encontrar Maximo - Minimo - Promedio
/* Recorre todas las carreras del .bin y acumula las estadisticas del piloto indicado */
int calcularEstadisticasPiloto(const char *archivo, unsigned id_piloto, EstadisticaPiloto *estadistica)
{
    FILE *pb;
    Carrera carrera;
    int i;
    int posicion;

    pb = fopen(archivo, "rb");
    if (pb == NULL)
        return ERR_ARCH;

    // valores iniciales: mejor_posicion arranca en el maximo posible
    estadistica->id_piloto       = id_piloto;
    estadistica->victorias       = 0;
    estadistica->suma_posiciones = 0;
    estadistica->cant_carreras   = 0;
    estadistica->mejor_posicion  = MAX_PILOTOS_CARRERA;
    estadistica->peor_posicion   = 0;
    estadistica->puntos_totales  = 0;

    while (fread(&carrera, sizeof(Carrera), 1, pb) == 1)
    {
        if (carrera.estado == 0) // carrera cancelada, no cuenta
            continue;

        for (i = 0; i < carrera.cant_resultados; i++)
        {
            if (carrera.resultados[i][0] == id_piloto) // encontro al piloto en esta carrera
            {
                posicion = i + 1; // la fila i corresponde a la posicion i+1

                if (posicion == 1)
                    estadistica->victorias++;

                if (posicion < estadistica->mejor_posicion)
                    estadistica->mejor_posicion = posicion;

                if (posicion > estadistica->peor_posicion)
                    estadistica->peor_posicion = posicion;

                estadistica->suma_posiciones += posicion;
                estadistica->puntos_totales  += carrera.resultados[i][1]; // columna 1 = puntos
                estadistica->cant_carreras++;
            }
        }
    }

    fclose(pb);
    return 0;
}


/**
 * mejorYPeorPosicion
 * Busca en carreras.dat la carrera con id_carrera dado.
 * Si existe y esta activa, muestra:
 *   - El ID del piloto en la fila 0 (1er puesto)
 *   - El ID del piloto en la fila cant_resultados-1 (ultimo puesto)
 * Retorna TODO_OK, ERR_ARCH o ERR_NO_ENCONTRADO.
 */
/* Busca la carrera por ID y muestra el piloto que llego primero y el que llego ultimo */
int mejorYPeorPosicion(const char *rutaBin, int id_carrera)
{
    FILE *pb;
    Carrera carrera;

    pb = fopen(rutaBin, "rb");
    if (pb == NULL)
        return ERR_ARCH;

    while (fread(&carrera, sizeof(Carrera), 1, pb) == 1)
    {
        if (carrera.id == id_carrera && carrera.estado == 1)
        {
            printf("\n");
            printf("  +--------------------------------------+\n");
            printf("  |   RESULTADO CARRERA: %-14s |\n", carrera.circuito);
            printf("  +--------------------------------------+\n");
            printf("  | [1] Mejor posicion : ID %-10d |\n", carrera.resultados[0][0]);                            // fila 0 = 1er puesto
            printf("  | [X] Peor posicion  : ID %-10d |\n", carrera.resultados[carrera.cant_resultados - 1][0]); // ultima fila = ultimo puesto
            printf("  +--------------------------------------+\n");
            printf("\n");
            fclose(pb);
            return TODO_OK;
        }
    }

    fclose(pb);
    printf("[!] Carrera no encontrada.\n");
    return ERR_NO_ENCONTRADO;
}

/**
 * mostrarEstadisticasPiloto
 * Calcula los promedios a partir de suma_posiciones y
 * puntos_totales divididos por cant_carreras (con cast a float
 * para evitar division entera) y muestra el resumen completo
 * en pantalla. Si cant_carreras == 0, informa que no hay datos.
 */
/* Calcula promedios y muestra el resumen estadistico completo del piloto */
void mostrarEstadisticasPiloto(EstadisticaPiloto *estadistica)
{
    float promedio_posiciones;
    float promedio_puntos;

    if (estadistica->cant_carreras == 0)
    {
        printf("[!] El piloto no tiene carreras registradas.\n");
        return;
    }

    // division entera evitada con cast a float
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
