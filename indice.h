#ifndef INDICE_H_INCLUDED
#define INDICE_H_INCLUDED

#include <stdio.h>
#include "utilidades.h"
#include "piloto.h"

#define RUTA_INDICE_PILOTO  "pilotos.idx"

/* ------------------------------------------------------------------
   Entrada del indice: clave (id) + posicion en el .dat (offset).
   El archivo .idx es un arreglo contiguo de IndiceEntrada[],
   siempre ordenado por id ascendente.
   ------------------------------------------------------------------ */
typedef struct
{
    unsigned id;
    long     offset;
} IndiceEntrada;

/* ------------------------------------------------------------------
   Construccion / reconstruccion completa.
   Lee pilotos.dat de principio a fin y genera pilotos.idx ordenado.
   Llamar despues de cualquier alta que modifique el .dat.
   Retorna TODO_OK, ERR_ARCH o SIN_MEM.
   ------------------------------------------------------------------ */
int construirIndicePilotos(const char* rutaDat, const char* rutaIdx);

/* ------------------------------------------------------------------
   Acceso directo por ID.
   Recibe fDat YA ABIERTO por el llamador (rb o rb+).
   Carga el .idx, aplica busquedaBinariaVector y, si encuentra el id,
   hace fseek en fDat para leer el Piloto completo en *dest.
   Usa rewind() internamente sobre el .idx para garantizar lectura
   desde el inicio, igual que buscarRegistroPorId en utilidades.c.
   El llamador NO debe cerrar fDat dentro de esta funcion.
   Retorna el offset en el .dat (>= 0) o -1L si no existe.
   ------------------------------------------------------------------ */
long buscarPilotoPorIndice(const char* rutaIdx, FILE* fDat, unsigned id, Piloto* dest);

/* ------------------------------------------------------------------
   Actualizacion incremental.
   insertarEntradaIndice: agrega una entrada manteniendo el orden.
   eliminarEntradaIndice: quita la entrada con el id dado.
   Ambas reescriben el .idx completo.
   Retornan TODO_OK, ERR_ARCH o SIN_MEM.
   ------------------------------------------------------------------ */
int insertarEntradaIndice(const char* rutaIdx, unsigned id, long offset);
int eliminarEntradaIndice(const char* rutaIdx, unsigned id);

#endif /* INDICE_H_INCLUDED */
