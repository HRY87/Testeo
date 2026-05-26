#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "utilidades.h"

#define SIN_MEM     1
#define DUPLICADO   2
#define VEC_LLENO   3
#define VEC_VACIO   4
#define NO_EXISTE   5

#define ES_VECTOR_LLENO(X,Y)    ((X) == (Y) ? VEC_LLENO : 0)
#define ES_VECTOR_VACIO(X)      ((X) == 0 ? VEC_VACIO : 0)

typedef int(*Map)(void* destino, const void* origen);
typedef int(*Filter)(const void* dato);
typedef int(*Reduce)(void* acumulador, const void* dato);

typedef struct
{
    void* vec; //Memoria donde estan los elementos
    size_t ce; //Cantidad de elementos
    size_t tamElem; //Tamanio de cada elemento
    size_t tope; //Capacidad maxima
}tVector;

int crearVector(tVector* v, size_t tamElem, size_t capacidad);
void destruirVector(tVector* v);
int insertarVectorOrd(tVector* v, void* dato, Comparar cmp);
void* busquedaBinariaVector(tVector* v, void* clave, Comparar cmp);
void* obtenerElementoVector(tVector* v, size_t pos);

/**Funciones para vector-archivo**/
int cargarVectorDesdeBin(const char* rutaBin, tVector* v);
int guardarVectorEnBin  (const char* rutaBin, tVector* v);

/**Funciones genericas para manejo de datos**/
int filtrarVector(tVector* origen, tVector* destino, Filter filtro);
int reducirVector(tVector* v, void* acumulador, Reduce reducir);
int mapearVector (tVector* origen, tVector* destino, size_t tamDestino, Map mapear);

/**Auxiliares**/
void mostrarVector(tVector* v, Mostrar mostrar);
void generarResultadoAleatorioVector(tVector* v);

#endif // VECTOR_H_INCLUDED
