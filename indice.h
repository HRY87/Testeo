#ifndef INDICE_H_INCLUDED
#define INDICE_H_INCLUDED

#include <stdio.h>
#include "utilidades.h"
#include "piloto.h"

#define RUTA_INDICE_PILOTO  "pilotos.idx"

/* Entrada del indice: clave (id) + posicion en el .dat.
   El archivo .idx es un arreglo contiguo de IndiceEntrada[]
   siempre ordenado por id ascendente. */
typedef struct
{
    unsigned id;
    long     offset;
} IndiceEntrada;

/* Reconstruye pilotos.idx leyendo pilotos.dat de principio a fin.
   Llamar despues de cualquier alta que modifique el .dat.
   Retorna TODO_OK, ERR_ARCH o SIN_MEM. */
int construirIndicePilotos(const char* rutaDat, const char* rutaIdx);

/* Acceso directo por ID usando busqueda binaria sobre el .idx.
   fDat debe llegar abierto (rb o rb+); esta funcion NO lo cierra.
   Carga el registro en *dest si dest != NULL.
   Retorna el offset en el .dat o -1L si no existe. */
long buscarPilotoPorIndice(const char* rutaIdx, FILE* fDat, unsigned id, Piloto* dest);

/* Actualizaciones incrementales del indice.
   insertarEntradaIndice: agrega una entrada manteniendo el orden.
   eliminarEntradaIndice: quita la entrada con el id dado.
   Ambas reescriben el .idx completo.
   Retornan TODO_OK, ERR_ARCH o SIN_MEM. */
int insertarEntradaIndice(const char* rutaIdx, unsigned id, long offset);
int eliminarEntradaIndice(const char* rutaIdx, unsigned id);

#endif /* INDICE_H_INCLUDED */
