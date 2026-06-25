#ifndef PUNTOS_H_INCLUDED
#define PUNTOS_H_INCLUDED

#include "vector.h"

#define RUTA_PUNTOS_TXT       "puntos.txt"
#define CAP_INICIAL_PUNTOS    10   /* capacidad inicial del vector */
#define NO_PUNTUA             0
typedef struct
{
    tVector tabla; /* vector de int: tabla[0] = puntos pos 1, etc. */
}Puntos;

/* Inicializa con la tabla estándar de F1 (10 posiciones) */
void inicializarPuntosDefault(Puntos* vPuntos);

/* Guarda la tabla en disco como texto (un entero por línea) */
int guardarConfigPuntos(const char* ruta, const Puntos* vPuntos);

/* Carga desde .txt; si no existe usa defaults y lo persiste */
int cargarConfigPuntos(const char* ruta, Puntos* vPuntos);

/* Devuelve los puntos para una posición (base 1). 0 si fuera de rango */
int puntosParaPosicion(const Puntos* vPuntos, int posicion);

/* Muestra la tabla por pantalla */
void mostrarConfigPuntos(const Puntos* vPuntos);

/* Libera el vector interno — llamar al final del programa */
void destruirPuntos(Puntos* vPuntos);

#endif // PUNTOS_H_INCLUDED

