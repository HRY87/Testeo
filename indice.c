#include <stdio.h>
#include <stdlib.h>
#include "indice.h"
#include "utilidades.h"
#include "piloto.h"
#include "vector.h"


/* ------------------------------------------------------------------
   cmpEntradaId  [Comparar — static]
   ------------------------------------------------------------------ */
static int cmpEntradaId(const void* a, const void* b)
{
    unsigned ia = ((const IndiceEntrada*)a)->id;
    unsigned ib = ((const IndiceEntrada*)b)->id;

    if (ia < ib) return -1;
    if (ia > ib) return  1;
    return 0;
}

/* ------------------------------------------------------------------
   guardarIdxDesdeVector  [static]
   Persiste el contenido del tVector en el .idx.
   ------------------------------------------------------------------ */
static int guardarIdxDesdeVector(const char* rutaIdx, tVector* v)
{
    FILE* f = fopen(rutaIdx, "wb");

    if (!f)
        return ERR_ARCH;

    fwrite(v->vec, v->tamElem, v->ce, f);
    fclose(f);

    return TODO_OK;
}

/* ------------------------------------------------------------------
   construirIndicePilotos
   ------------------------------------------------------------------ */
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

    if (crearVector(&vIdx, sizeof(IndiceEntrada), CAP_MAX) != TODO_OK)
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

/* ------------------------------------------------------------------
   buscarPilotoPorIndice
   fDat llega abierto desde el llamador. Esta funcion NO lo cierra.
   Abre el .idx internamente (solo lectura) y lo cierra antes de
   retornar, igual que buscarRegistroPorId hace con su FILE*.
   ------------------------------------------------------------------ */
long buscarPilotoPorIndice(const char* rutaIdx, FILE* fDat, unsigned id, Piloto* dest)
{
    tVector        vIdx;
    IndiceEntrada  clave;
    IndiceEntrada* encontrada;
    long           resultado;

    if (crearVector(&vIdx, sizeof(IndiceEntrada), CAP_MAX) != TODO_OK)
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

    if (resultado == -1L || !dest)
        return resultado;

    /* fDat ya esta abierto: posicionar y leer directamente */
    fseek(fDat, resultado, SEEK_SET);
    fread(dest, sizeof(Piloto), 1, fDat);

    return resultado;
}

/* ------------------------------------------------------------------
   insertarEntradaIndice
   ------------------------------------------------------------------ */
int insertarEntradaIndice(const char* rutaIdx, unsigned id, long offset)
{
    tVector       vIdx;
    IndiceEntrada nueva;
    int           resp;

    if (crearVector(&vIdx, sizeof(IndiceEntrada), CAP_MAX) != TODO_OK)
        return SIN_MEM;

    cargarVectorDesdeBin(rutaIdx, &vIdx);

    nueva.id     = id;
    nueva.offset = offset;

    insertarVectorOrd(&vIdx, &nueva, cmpEntradaId);

    resp = guardarIdxDesdeVector(rutaIdx, &vIdx);
    destruirVector(&vIdx);

    return resp;
}

/* ------------------------------------------------------------------
   eliminarEntradaIndice
   ------------------------------------------------------------------ */
static unsigned idAExcluir = 0;

static int filtroExcluirId(const void* dato)
{
    return (((const IndiceEntrada*)dato)->id != idAExcluir);
}

int eliminarEntradaIndice(const char* rutaIdx, unsigned id)
{
    tVector vOrigen;
    tVector vDestino;
    int resp;

    if (crearVector(&vOrigen,  sizeof(IndiceEntrada), CAP_MAX) != TODO_OK)
        return SIN_MEM;

    if (crearVector(&vDestino, sizeof(IndiceEntrada), CAP_MAX) != TODO_OK)
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

    /**Se usa una variable global porque la firma de filtre no me permite agregar otro parametro**/
    idAExcluir = id;
    filtrarVector(&vOrigen, &vDestino, filtroExcluirId);

    resp = guardarIdxDesdeVector(rutaIdx, &vDestino);

    destruirVector(&vOrigen);
    destruirVector(&vDestino);

    return resp;
}
