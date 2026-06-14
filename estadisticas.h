#ifndef ESTADISTICAS_H_INCLUDED
#define ESTADISTICAS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "carrera.h"
#define ERR_NO_ENCONTRADO -1

typedef struct
{
    unsigned id_piloto;
    int   victorias;
    int   suma_posiciones;
    int   cant_carreras;
    int   mejor_posicion;
    int   peor_posicion;
    int   puntos_totales;

} EstadisticaPiloto;



int calcularEstadisticasPiloto(const char *archivo, unsigned id_piloto, EstadisticaPiloto *estadistica);
int mejorYPeorPosicion(const char *rutaBin, int id_carrera);
void mostrarEstadisticasPiloto(EstadisticaPiloto *estadistica);


#endif // ESTADISTICAS_H_INCLUDED
