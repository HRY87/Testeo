#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include "puntos.h"

/* Imprime el menu principal y lee la opcion. Retorna la opcion (0 = salir) */
int mostrarMenu(void);

/* Submenus, cada uno con su propio scanf y switch interno */
void menuEstadisticas(void);
void menuCarrera(Puntos* pts);
void menuPilotosPorEscuderia(void);
void exportarTemporada(void);
void menuGestionRegistros(void);

/* Cierra la temporada: exporta/renombra archivos y borra los .bin/.idx */
void finalizarTemporada(void);

#endif // MENU_H_INCLUDED
