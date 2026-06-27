#ifndef ESTADISTICAS_H_INCLUDED
#define ESTADISTICAS_H_INCLUDED

#include <stdio.h>
#include "carrera.h"
#include "utilidades.h"

typedef struct
{
    unsigned id_piloto;         /* ID del piloto consultado              */
    int      victorias;         /* cantidad de primeros puestos          */
    int      mejor_posicion;    /* minima posicion alcanzada             */
    int      peor_posicion;     /* maxima posicion alcanzada             */
    int      suma_posiciones;   /* acumulador para calcular el promedio  */
    int      cant_carreras;     /* carreras activas en las que participo */
    int      puntos_totales;    /* suma de todos los puntos obtenidos    */
} EstadisticaPiloto;

/*Calculo de estadisticas*/
int calcularEstadisticasPiloto(const char* archivo, unsigned id_piloto, EstadisticaPiloto* estadistica);

/*Consulta por carrera*/
int mejorYPeorPosicion(const char* rutaBin, int id_carrera);

/*Mostrar Vector*/
void mostrarEstadisticasPiloto(EstadisticaPiloto* estadistica);

#endif /* ESTADISTICAS_H_INCLUDED */
