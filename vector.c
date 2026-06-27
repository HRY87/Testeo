#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

/* =========================================================
   Ciclo de vida
   ========================================================= */

/**
 * crearVector
 * Reserva en heap un bloque para capacidad elementos de tamElem bytes.
 * Retorna TODO_OK o SIN_MEM.
 */
int crearVector(tVector* v, size_t tamElem, size_t capacidad)
{
    v->vec = malloc(tamElem * capacidad);

    if (!v->vec)
        return SIN_MEM;

    v->ce      = 0;
    v->tamElem = tamElem;
    v->cap    = capacidad;

    return TODO_OK;
}

int redimensionarVector(tVector* v, size_t nuevaCap)
{
    void* nVec = realloc(v->vec, nuevaCap * v->tamElem);

    if(!nVec)
    {
        free(nVec);
        return SIN_MEM;
    }

    //printf("Redimension de %lu a %lu\n", (long unsigned)v->tope, (long unsigned)nuevaCap);

    v->vec = nVec;
    v->cap = nuevaCap;

    return TODO_OK;
}

/**
 * destruirVector
 * Libera la memoria del vector y resetea todos los campos a 0.
 */
void destruirVector(tVector* v)
{
    free(v->vec);
    v->vec     = NULL;
    v->ce      = 0;
    v->tamElem = 0;
    v->cap    = 0;
}

/* =========================================================
   Insercion
   ========================================================= */

/**
 * insertarVectorOrd
 * Inserta dato manteniendo orden segun cmp (insertion sort lineal).
 * Busca la posicion, desplaza los elementos posteriores y copia.
 * Retorna VEC_LLENO si no hay espacio, TODO_OK si inserto.
 */
int insertarVectorOrd(tVector* v, void* dato, Comparar cmp)
{
    char* act;
    char* fin;

    if (v->ce == v->cap)
    {
        if(redimensionarVector(v, v->cap * FACTOR_INCREMENTAL) == SIN_MEM)
            return SIN_MEM;
    }

    act = (char*)v->vec;
    fin = (char*)v->vec + (v->ce * v->tamElem);

    while (act < fin && cmp(dato, act) > 0)
        act += v->tamElem;

    while (fin > act)
    {
        memcpy(fin, fin - v->tamElem, v->tamElem);
        fin -= v->tamElem;
    }

    memcpy(act, dato, v->tamElem);
    v->ce++;

    return TODO_OK;
}

/**
 * insertarFinalVector
 * Agrega dato al final sin mantener orden.
 * Retorna VEC_LLENO o TODO_OK.
 */
int insertarFinalVector(tVector* v, const void* dato)
{
    if (v->ce == v->cap)
    {
        if(redimensionarVector(v, v->cap * FACTOR_INCREMENTAL) == SIN_MEM)
            return SIN_MEM;
    }

    memcpy((char*)v->vec + (v->ce * v->tamElem), dato, v->tamElem);
    v->ce++;

    return TODO_OK;
}

/* =========================================================
   Busqueda
   ========================================================= */

/**
 * busquedaBinariaVector
 * Busqueda binaria sobre vector ORDENADO.
 * Retorna puntero al elemento encontrado, o NULL si no existe.
 */
void* busquedaBinariaVector(tVector* v, void* clave, Comparar comparar)
{
    size_t izq;
    size_t der;
    size_t mid;
    int    resp;

    izq = 0;
    der = v->ce;

    while (izq < der)
    {
        mid  = (izq + der) / 2;
        resp = comparar(clave, obtenerElementoVector(v, mid));

        if (resp == 0)
            return obtenerElementoVector(v, mid);

        if (resp < 0)
            der = mid;
        else
            izq = mid + 1;
    }

    return NULL;
}

/**
 * obtenerElementoVector
 * Retorna puntero al elemento en la posicion pos.
 */
void* obtenerElementoVector(tVector* v, size_t pos)
{
    return (char*)v->vec + (pos * v->tamElem);
}

/* =========================================================
   Persistencia
   ========================================================= */

/**
 * cargarVectorDesdeBin
 * Lee registros de tamElem bytes desde rutaBin hasta agotar el archivo,
 * redimensionando el vector si la cantidad de registros supera su
 * capacidad actual (igual criterio que insertarVectorOrd/insertarFinalVector).
 * Retorna TODO_OK, ERR_ARCH o SIN_MEM.
 */
int cargarVectorDesdeBin(const char* rutaBin, tVector* v)
{
    void* act;
    FILE* fBin;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    act = (char*)v->vec + (v->ce * v->tamElem);

    while (fread(act, v->tamElem, 1, fBin) == 1)
    {
        v->ce++;

        if (v->ce == v->cap)
        {
            if (redimensionarVector(v, v->cap * FACTOR_INCREMENTAL) == SIN_MEM)
            {
                fclose(fBin);
                return SIN_MEM;
            }
        }

        act = (char*)v->vec + (v->ce * v->tamElem);
    }

    fclose(fBin);
    return TODO_OK;
}

/**
 * guardarVectorEnBin
 * Sobreescribe rutaBin con los ce elementos del vector.
 * Retorna TODO_OK o ERR_ARCH.
 */
int guardarVectorEnBin(const char* rutaBin, tVector* v)
{
    char* act;
    char* fin;
    FILE* fBin;

    fBin = fopen(rutaBin, "wb");
    if (!fBin)
        return ERR_ARCH;

    act = (char*)v->vec;
    fin = (char*)v->vec + (v->ce * v->tamElem);

    while (act < fin)
    {
        fwrite(act, v->tamElem, 1, fBin);
        act += v->tamElem;
    }

    fclose(fBin);
    return TODO_OK;
}

/* =========================================================
   Operaciones funcionales genericas
   ========================================================= */

/**
 * filtrarVector
 * Copia en destino solo los elementos de origen para los que
 * filtro() retorna distinto de 0.
 * Retorna TODO_OK, VEC_LLENO o SIN_MEM.
 */
int filtrarVector(tVector* origen, tVector* destino, Filter filtro)
{
    char* act;
    char* fin;

    if (!origen || !destino || !filtro)
        return SIN_MEM;

    act = (char*)origen->vec;
    fin = (char*)origen->vec + (origen->ce * origen->tamElem);

    while (act < fin)
    {
        if (filtro(act))
        {
            if (destino->ce == destino->cap)
                return VEC_LLENO;

            memcpy((char*)destino->vec + (destino->ce * destino->tamElem), act, destino->tamElem);
            destino->ce++;
        }
        act += origen->tamElem;
    }

    return TODO_OK;
}

/**
 * reducirVector
 * Aplica reducir() a cada elemento acumulando en acumulador.
 * Retorna TODO_OK o SIN_MEM.
 */
int reducirVector(tVector* v, void* acumulador, Reduce reducir)
{
    char* act;
    char* fin;

    if (!v || !acumulador || !reducir)
        return SIN_MEM;

    act = (char*)v->vec;
    fin = (char*)v->vec + (v->ce * v->tamElem);

    while (act < fin)
    {
        reducir(acumulador, act);
        act += v->tamElem;
    }

    return TODO_OK;
}

/**
 * mapearVector
 * Transforma cada elemento de origen con mapear() y lo guarda
 * en destino. Los elementos destino pueden tener tamano distinto.
 * Retorna TODO_OK, SIN_MEM o VEC_LLENO.
 */
int mapearVector(tVector* origen, tVector* destino, size_t tamDestino, Map mapear)
{
    char* actOrigen;
    char* finOrigen;
    char* actDestino;

    if (!origen || !destino || !mapear)
        return SIN_MEM;

    if (origen->ce > destino->cap)
        return VEC_LLENO;

    actOrigen  = (char*)origen->vec;
    finOrigen  = (char*)origen->vec + (origen->ce * origen->tamElem);
    actDestino = (char*)destino->vec;

    while (actOrigen < finOrigen)
    {
        mapear(actDestino, actOrigen);
        actOrigen  += origen->tamElem;
        actDestino += tamDestino;
    }

    destino->ce = origen->ce;
    return TODO_OK;
}

/* =========================================================
   Auxiliares
   ========================================================= */

/**
 * mostrarVector
 * Llama a mostrar() para cada elemento del vector.
 */
void mostrarVector(tVector* v, Mostrar mostrar)
{
    char*  act = (char*)v->vec;
    size_t i;

    for (i = 0; i < v->ce; i++)
    {
        mostrar(act);
        printf("\t");
        act += v->tamElem;
    }
    puts("");
}

/**
 * generarResultadoAleatorioVector  [Fisher-Yates]
 * Baraja los elementos del vector con distribucion uniforme.
 * Para i desde ce-1 hasta 1: elige j en [0,i] e intercambia i con j.
 */
void generarResultadoAleatorioVector(tVector* v)
{
    size_t i;
    size_t j;

    for (i = v->ce - 1; i > 0; i--)
    {
        j = (size_t)(rand() % (int)(i + 1));
        intercambiar(obtenerElementoVector(v, i), obtenerElementoVector(v, j), v->tamElem);
    }
}
