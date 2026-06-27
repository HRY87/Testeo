#include <stdio.h>
#include <stdlib.h>
#include "indice.h"
#include "utilidades.h"
#include "piloto.h"
#include "vector.h"

/* =========================================================
   Auxiliares estaticas
   ========================================================= */

/**
 * cmpEntradaId  [Comparar  static]
 * Compara dos IndiceEntrada por id. Retorna -1, 0 o 1.
 */
static int cmpEntradaId(const void* a, const void* b)
{
    unsigned ia = ((const IndiceEntrada*)a)->id;
    unsigned ib = ((const IndiceEntrada*)b)->id;

    if (ia < ib) return -1;
    if (ia > ib) return  1;
    return 0;
}

/**
 * guardarIdxDesdeVector  [static]
 * Persiste el contenido del tVector en el archivo .idx.
 * Retorna TODO_OK o ERR_ARCH.
 */
static int guardarIdxDesdeVector(const char* rutaIdx, tVector* v)
{
    FILE* f = fopen(rutaIdx, "wb");

    if (!f)
        return ERR_ARCH;

    fwrite(v->vec, v->tamElem, v->ce, f);
    fclose(f);

    return TODO_OK;
}

/* =========================================================
   Funciones publicas
   ========================================================= */

/**
 * construirIndicePilotos
 * Lee pilotos.dat de principio a fin y genera pilotos.idx
 * ordenado por id ascendente.
 * Retorna TODO_OK, ERR_ARCH o SIN_MEM.
 */
int construirIndicePilotos(const char* rutaDat, const char* rutaIdx)
{
    FILE*         fDat;
    Piloto        pil;
    tVector       vIdx;
    IndiceEntrada entrada;
    long          offset;
    int           resp;

    fDat = fopen(rutaDat, "rb");
    if (!fDat)
        return ERR_ARCH;

    if (crearVector(&vIdx, sizeof(IndiceEntrada), CAPACIDAD_MINIMA) != TODO_OK)
    {
        fclose(fDat);
        return SIN_MEM;
    }

    offset = 0L;
    while (fread(&pil, sizeof(Piloto), 1, fDat) == 1)
    {
        entrada.id     = pil.id;
        entrada.offset = offset;
        insertarVectorOrd(&vIdx, &entrada, cmpEntradaId);
        offset += (long)sizeof(Piloto);
    }
    fclose(fDat);

    resp = guardarIdxDesdeVector(rutaIdx, &vIdx);
    destruirVector(&vIdx);

    return resp;
}

/**
 * buscarPilotoPorIndice
 * Carga el .idx, aplica busqueda binaria y, si encuentra el id,
 * posiciona fDat con fseek y lee el Piloto en *dest.
 * fDat llega abierto desde el llamador; esta funcion NO lo cierra.
 * Retorna el offset en el .dat o -1L si no existe.
 */
long buscarPilotoPorIndice(const char* rutaIdx, FILE* fDat, unsigned id, Piloto* dest)
{
    tVector        vIdx;
    IndiceEntrada  clave;
    IndiceEntrada* encontrada;
    long           resultado;

    if (crearVector(&vIdx, sizeof(IndiceEntrada), CAPACIDAD_MINIMA) != TODO_OK)
        return -1L;

    if (cargarVectorDesdeBin(rutaIdx, &vIdx) != TODO_OK)
    {
        destruirVector(&vIdx);
        return -1L;
    }

    clave.id     = id;
    clave.offset = 0L;

    encontrada = (IndiceEntrada*)busquedaBinariaVector(&vIdx, &clave, cmpEntradaId);
    resultado  = (encontrada != NULL) ? encontrada->offset : -1L;

    destruirVector(&vIdx);

    if (resultado == -1L || dest == NULL)
        return resultado;

    fseek(fDat, resultado, SEEK_SET);
    fread(dest, sizeof(Piloto), 1, fDat);

    return resultado;
}

/**
 * insertarEntradaIndice
 * Carga el .idx, inserta la nueva entrada ordenada y lo persiste.
 * Retorna TODO_OK, ERR_ARCH o SIN_MEM.
 */
int insertarEntradaIndice(const char* rutaIdx, unsigned id, long offset)
{
    tVector       vIdx;
    IndiceEntrada nueva;
    int           resp;

    if (crearVector(&vIdx, sizeof(IndiceEntrada), CAPACIDAD_MINIMA) != TODO_OK)
        return SIN_MEM;

    cargarVectorDesdeBin(rutaIdx, &vIdx);

    nueva.id     = id;
    nueva.offset = offset;
    insertarVectorOrd(&vIdx, &nueva, cmpEntradaId);

    resp = guardarIdxDesdeVector(rutaIdx, &vIdx);
    destruirVector(&vIdx);

    return resp;
}

/**
 * eliminarEntradaIndice
 * Carga el .idx, filtra la entrada con el id dado y persiste el resultado.
 * Usa una variable estatica como parametro del filtro porque la firma
 * Filter no admite parametros extra.
 * Retorna TODO_OK, ERR_ARCH o SIN_MEM.
 */
static unsigned idAExcluir = 0;

static int filtroExcluirId(const void* dato)
{
    return (((const IndiceEntrada*)dato)->id != idAExcluir);
}

int eliminarEntradaIndice(const char* rutaIdx, unsigned id)
{
    tVector vOrigen;
    tVector vDestino;
    int     resp;

    if (crearVector(&vOrigen,  sizeof(IndiceEntrada), CAPACIDAD_MINIMA) != TODO_OK)
        return SIN_MEM;

    if (crearVector(&vDestino, sizeof(IndiceEntrada), CAPACIDAD_MINIMA) != TODO_OK)
    {
        destruirVector(&vOrigen);
        return SIN_MEM;
    }

    if (cargarVectorDesdeBin(rutaIdx, &vOrigen) != TODO_OK)
    {
        destruirVector(&vOrigen);
        destruirVector(&vDestino);
        return ERR_ARCH;
    }

    idAExcluir = id;
    filtrarVector(&vOrigen, &vDestino, filtroExcluirId);

    resp = guardarIdxDesdeVector(rutaIdx, &vDestino);

    destruirVector(&vOrigen);
    destruirVector(&vDestino);

    return resp;
}
