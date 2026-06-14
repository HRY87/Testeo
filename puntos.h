#ifndef PUNTOS_H_INCLUDED
#define PUNTOS_H_INCLUDED

#include "vector.h"

#define RUTA_PUNTOS_TXT  "archivos/puntos.txt"

/*
 * TDA Puntos
 * Usa tVector internamente para almacenar la tabla de forma dinámica.
 * El vector guarda int. Índice 0 = posición 0 (siempre 0 puntos).
 * Índice i = puntos para la posición i (base 1).
 *
 * Formato puntos.txt (una línea por posición, empezando en 0):
 *   0
 *   25
 *   18
 *   ...
 */
typedef struct
{
    tVector tabla; /* vector de int: tabla[i] = puntos para posición i */
} Puntos;

/* Genera puntos.txt con los defaults de F1 si no existe */
int  generarArchivoPuntosTxt(const char* ruta);

/* Carga desde .txt al vector; si no existe, genera el archivo y carga defaults */
int  cargarConfigPuntos(const char* ruta, Puntos* p);

/* Retorna puntos para una posición (base 1). 0 si fuera de rango. */
int  obtenerPuntosPorPosicion(const Puntos* p, int posicion);

/* Muestra la tabla por pantalla */
void mostrarConfigPuntos(const Puntos* p);

/* Libera el vector interno */
void destruirPuntos(Puntos* p);

#endif // PUNTOS_H_INCLUDED
