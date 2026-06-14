#include "estadisticas.h"
#include "carrera.h"

/* Recorre todas las carreras del .dat y acumula estadisticas del piloto indicado */
int calcularEstadisticasPiloto(const char* archivo, unsigned id_piloto, EstadisticaPiloto* estadistica)
{
    FILE*      pb;
    Carrera    carrera;
    Resultado* r;
    int        i;
    int        posicion;

    pb = fopen(archivo, "rb");
    if (pb == NULL)
        return ERR_ARCH;

    estadistica->id_piloto       = id_piloto;
    estadistica->victorias       = 0;
    estadistica->suma_posiciones = 0;
    estadistica->cant_carreras   = 0;
    estadistica->mejor_posicion  = MAX_PILOTOS_CARRERA;
    estadistica->peor_posicion   = 0;
    estadistica->puntos_totales  = 0;

    /* leerCarrera reserva memoria — destruirCarrera la libera al final de cada iteracion */
    while (leerCarrera(pb, &carrera) == TODO_OK)
    {
        if (carrera.estado == ESTADO_CARRERA_ACTIVA)
        {
            for (i = 0; i < carrera.cant_resultados; i++)
            {
                r = (Resultado*)obtenerElementoVector(&carrera.resultados, i);

                if ((unsigned)r->id_piloto == id_piloto)
                {
                    posicion = i + 1;

                    if (posicion == 1)
                        estadistica->victorias++;

                    if (posicion < estadistica->mejor_posicion)
                        estadistica->mejor_posicion = posicion;

                    if (posicion > estadistica->peor_posicion)
                        estadistica->peor_posicion = posicion;

                    estadistica->suma_posiciones += posicion;
                    estadistica->puntos_totales  += r->puntos;
                    estadistica->cant_carreras++;
                }
            }
        }
        destruirCarrera(&carrera);   /* libera el vector interno de cada carrera */
    }

    fclose(pb);
    return TODO_OK;
}

/* Busca la carrera por ID y muestra el piloto que llego primero y el que llego ultimo */
int mejorYPeorPosicion(const char* rutaBin, int id_carrera)
{
    FILE*      pb;
    Carrera    carrera;
    Resultado* primero;
    Resultado* ultimo;

    pb = fopen(rutaBin, "rb");
    if (pb == NULL)
        return ERR_ARCH;

    while (leerCarrera(pb, &carrera) == TODO_OK)
    {
        if (carrera.id == id_carrera && carrera.estado == ESTADO_CARRERA_ACTIVA)
        {
            primero = (Resultado*)obtenerElementoVector(&carrera.resultados, 0);
            ultimo  = (Resultado*)obtenerElementoVector(&carrera.resultados,
                                                        carrera.cant_resultados - 1);

            printf("\n");
            printf("  +--------------------------------------+\n");
            printf("  |   RESULTADO CARRERA: %-14s |\n", carrera.circuito);
            printf("  +--------------------------------------+\n");
            printf("  | [1] Mejor posicion : ID %-10d |\n", primero->id_piloto);
            printf("  | [X] Peor posicion  : ID %-10d |\n", ultimo->id_piloto);
            printf("  +--------------------------------------+\n\n");

            destruirCarrera(&carrera);
            fclose(pb);
            return TODO_OK;
        }
        destruirCarrera(&carrera);
    }

    fclose(pb);
    printf("[!] Carrera no encontrada.\n");
    return ERR_NO_ENCONTRADO;
}

/* Calcula promedios y muestra el resumen estadistico completo del piloto */
void mostrarEstadisticasPiloto(EstadisticaPiloto* estadistica)
{
    float promedio_posiciones;
    float promedio_puntos;

    if (estadistica->cant_carreras == 0)
    {
        printf("[!] El piloto no tiene carreras registradas.\n");
        return;
    }

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
    printf("  +--------------------------------------+\n\n");
}
