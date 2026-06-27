#ifndef INDICE_H_INCLUDED
#define INDICE_H_INCLUDED

#include <stdio.h>
#include "utilidades.h"
#include "piloto.h"

#define RUTA_INDICE_PILOTO  "pilotos.idx"

/* Indice : clave (id) + posicion en el .dat.
   El archivo .idx es un arreglo contiguo siempre ordenado por id ascendente. */

typedef struct
{
    unsigned id;
    long     offset;
}Indice;

/*Creacion de archivo indice*/
int construirIndicePilotos(const char* rutaDat, const char* rutaIdx);

/*Busqueda directa con el indice*/
long buscarPilotoPorIndice(const char* rutaIdx, FILE* fDat, unsigned id, Piloto* dest);

/*Actualizacion del archivo indice*/
int insertarEntradaIndice(const char* rutaIdx, unsigned id, long offset);
int eliminarEntradaIndice(const char* rutaIdx, unsigned id);

#endif /* INDICE_H_INCLUDED */
