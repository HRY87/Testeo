#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "vector.h"

/**Dinamico**/
int crearVector(tVector* v, size_t tamElem, size_t capacidad)
{
    v->vec = malloc(tamElem * capacidad);

    if(!v->vec)
        return SIN_MEM;

    v->ce = 0;
    v->tamElem = tamElem;
    v->tope = capacidad;

    return TODO_OK;
}

void destruirVector(tVector* v)
{
    free(v->vec);

    v->ce = 0;
    v->tamElem = 0;
    v->tope = 0;
}

int insertarVectorOrd(tVector* v, void* dato, Comparar cmp)
{
    char* act = (char*)v->vec;
    char* fin = (char*)v->vec + (v->ce * v->tamElem);

    if(v->ce == v->tope)
        return VEC_LLENO;

    while(act < fin && cmp(dato, act) > 0)
    {
        act += v->tamElem;
    }


    while(fin > act)
    {
        memcpy(fin, fin - v->tamElem, v->tamElem);
        fin -= v->tamElem;
    }

    memcpy(act, dato, v->tamElem);

    v->ce++;

    return TODO_OK;
}

void* busquedaBinariaVector(tVector* v, void* clave, Comparar comparar)
{
    size_t izq, der, mid;
    int resp;

    izq = 0;
    der = v->ce;

    while(izq < der)
    {
        mid = (izq + der)/2;

        resp = comparar(clave, obtenerElementoVector(v, mid));

        if(resp == 0)/**Encontrado**/
            return obtenerElementoVector(v, mid);

        if(resp < 0)
            der = mid;
        else
            izq = mid + 1;
    }

    return NULL;
}

void* obtenerElementoVector(tVector* v, size_t pos)
{
    return( (char*)v-> vec + (pos * v->tamElem));
}

/**Funciones para vector-archivo**/
int cargarVectorDesdeBin(const char* rutaBin, tVector* v)
{
    char* act = (char*)v->vec;
    char* fin = (char*)v->vec + (v->tope * v->tamElem);

    FILE* fBin = fopen(rutaBin, "rb");

    if(!fBin)
        return ERR_ARCH;

    while(act < fin && fread(act, v->tamElem, 1, fBin) == 1)
    {
        v->ce++;
        act += v->tamElem;
    }

    fclose(fBin);
    return TODO_OK;
}

int guardarVectorEnBin(const char* rutaBin, tVector* v)
{
    char* act = (char*)v->vec;
    char* fin = (char*)v->vec + (v->ce * v->tamElem);

    FILE* fBin = fopen(rutaBin, "wb");

    if(!fBin)
        return ERR_ARCH;

    while(act < fin)
    {
        fwrite(act, v->tamElem, 1, fBin);
        act += v->tamElem;
    }

    fclose(fBin);
    return TODO_OK;
}

/**Funciones genericas para manejo de datos**/
int filtrarVector(tVector* origen, tVector* destino, Filter filtro)
{
    char* act;
    char* fin;

    if(!origen || !destino || !filtro)
        return SIN_MEM;

    act = (char*)origen->vec;
    fin = (char*)origen->vec + (origen->ce * origen->tamElem);

    while(act < fin)
    {
        if(filtro(act))
        {
            if(destino->ce == destino->tope)
                return VEC_LLENO;

            memcpy((char*)destino->vec + (destino->ce * destino->tamElem), act, destino->tamElem);
            destino->ce++;
        }
        act += origen->tamElem;
    }

    return TODO_OK;
}

int reducirVector(tVector* v, void* acumulador, Reduce reducir)
{
    char* act;
    char* fin;

    if(!v || !acumulador || !reducir)
        return SIN_MEM;

    act = (char*)v->vec;
    fin = (char*)v->vec + (v->ce * v->tamElem);

    while(act < fin)
    {
        reducir(acumulador, act);
        act += v->tamElem;
    }

    return TODO_OK;
}

int mapearVector(tVector* origen, tVector* destino, size_t tamDestino, Map mapear)
{
    char* actOrigen;
    char* finOrigen;
    char* actDestino;

    if(!origen || !destino || !mapear)
        return SIN_MEM;

    if(origen->ce > destino->tope)
        return VEC_LLENO;

    actOrigen  = (char*)origen->vec;
    finOrigen  = (char*)origen->vec + (origen->ce * origen->tamElem);
    actDestino = (char*)destino->vec;

    while(actOrigen < finOrigen)
    {
        mapear(actDestino, actOrigen);
        actOrigen  += origen->tamElem;
        actDestino += tamDestino;
    }

    destino->ce = origen->ce;

    return TODO_OK;
}

/**Auxiliares**/
void mostrarVector(tVector* v, Mostrar mostrar)
{
    char* act = (char*)v->vec;
    for(int i = 0 ; i < v->ce; i++)
    {
        mostrar(act);
        printf("\t");
        act += v->tamElem;
    }

    puts("");
}

/**Algoritmo de Fisher-Yates**/
void generarResultadoAleatorioVector(tVector* v)
{
    size_t i, j;

    for(i = v->ce - 1; i > 0; i--)
    {
        j = (size_t)(rand() % (int)(i + 1));

        intercambiar(obtenerElementoVector(v, i),
                     obtenerElementoVector(v, j), v->tamElem);
    }
}
