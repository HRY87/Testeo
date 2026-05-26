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
