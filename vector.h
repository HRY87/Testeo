#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "utilidades.h"

#define VEC_LLENO               3
#define VEC_VACIO               4
#define NO_EXISTE               5

#define CAPACIDAD_MINIMA        4
#define FACTOR_INCREMENTAL      2

#define ES_VECTOR_LLENO(X, Y)   ((X) == (Y) ? VEC_LLENO : 0)
#define ES_VECTOR_VACIO(X)      ((X) == 0   ? VEC_VACIO : 0)

typedef int  (*Map)   (void* destino, const void* origen);
typedef int  (*Filter)(const void* dato);
typedef int  (*Reduce)(void* acumulador, const void* dato);

typedef struct
{
    void*  vec;      /* Bloque contiguo de elementos en heap */
    size_t ce;       /* Cantidad de elementos cargados       */
    size_t tamElem;  /* Tamanio de cada elemento en bytes    */
    size_t cap;     /* Capacidad maxima                     */
} tVector;

/* Ciclo de vida */
int crearVector(tVector* v, size_t tamElem, size_t capacidad);
int redimensionarVector(tVector* v, size_t nuevaCap);
void destruirVector(tVector* v);

/* Insercion */
int insertarVectorOrd   (tVector* v, void* dato, Comparar cmp);
int insertarFinalVector (tVector* v, const void* dato);

/* Busqueda */
void* busquedaBinariaVector(tVector* v, void* clave, Comparar comparar);
void* obtenerElementoVector(tVector* v, size_t pos);

/* Persistencia */
int cargarVectorDesdeBin(const char* rutaBin, tVector* v);
int guardarVectorEnBin  (const char* rutaBin, tVector* v);

/* Operaciones funcionales genericas */
int filtrarVector(tVector* origen, tVector* destino, Filter filtro);
int reducirVector(tVector* v, void* acumulador, Reduce reducir);
int mapearVector(tVector* origen, tVector* destino, size_t tamDestino, Map mapear);

/* Auxiliares */
void mostrarVector(tVector* v, Mostrar mostrar);
void generarResultadoAleatorioVector(tVector* v);

#endif /* VECTOR_H_INCLUDED */
