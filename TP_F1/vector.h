#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "utilidades.h"

#define SIN_MEM     1
#define DUPLICADO   2
#define VEC_LLENO   3
#define VEC_VACIO   4
#define NO_EXISTE   5

#define ES_VECTOR_LLENO(X,Y)((X) == (Y) ? VEC_LLENO : 0)
#define ES_VECTOR_VACIO(X)((X) == 0 ? VEC_VACIO : 0)
typedef struct
{
    void* vec; //Memoria donde estan los elementos
    size_t ce; //Cantidad de elementos
    size_t tamElem; //Tamanio de cada elemento
    size_t tope; //Capacidad maxima
}tVector;

/**Dinamico**/
int crearVector(tVector* v, size_t tamElem, size_t capacidad);
void destruirVector(tVector* v);

int insertarVectorOrd(tVector* v, void* dato, Comparar cmp);
void* busquedaBinariaVector(tVector* v, void* clave, Comparar cmp);
void* obtenerElementoVector(tVector* v, size_t pos);

/**Auxiliares**/
void mostrarVector(tVector* v, Mostrar mostrar);
void generarResultadoAleatorioVector(tVector* v);

#endif // VECTOR_H_INCLUDED
