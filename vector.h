#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "utilidades.h"

/* =========================================================
            Codigos de retorno propios del vector
   ========================================================= */
#define VEC_LLENO   10
#define VEC_VACIO   11
#define DUPLICADO   12
#define NO_EXISTE   13


/* =========================================================
                    Macros de control
   ========================================================= */
#define ES_VECTOR_LLENO(ce, tope) ((ce) == (tope) ? VEC_LLENO : 0)
#define ES_VECTOR_VACIO(ce)       ((ce) == 0       ? VEC_VACIO : 0)

/* =========================================================
   TDA Vector dinamico generico
   Almacena elementos de cualquier tipo usando void*.
   El tamanio de cada elemento (tamElem) debe indicarse
   al crear el vector.
   ========================================================= */
typedef struct
{
    void*  vec;      /* Bloque de memoria con los elementos  */
    size_t ce;       /* Cantidad de elementos cargados       */
    size_t tamElem;  /* Tamanio en bytes de cada elemento    */
    size_t tope;     /* Capacidad maxima del vector          */
} tVector;

/* =========================================================
                        Ciclo de vida
   ========================================================= */
int  crearVector(tVector* v, size_t tamElem, size_t capacidad);
void destruirVector(tVector* v);

/* =========================================================
                    Insercion y acceso
   ========================================================= */

/* Inserta manteniendo orden segun cmp (insertion sort) */
int   insertarVectorOrd(tVector* v, void* dato, Comparar cmp);

/* Busqueda binaria: requiere vector ordenado con el mismo cmp */
void* busquedaBinariaVector(tVector* v, void* clave, Comparar cmp);

/* Acceso directo por indice (sin verificacion de rango) */
void* obtenerElementoVector(tVector* v, size_t pos);

/* =========================================================
        Persistencia: vector <-> archivo binario
   ========================================================= */
int cargarVectorDesdeBin(const char* rutaBin, tVector* v);
int guardarVectorEnBin  (const char* rutaBin, tVector* v);

/* =========================================================
            Operaciones funcionales genericas
   ========================================================= */

/* Copia al destino los elementos que pasan el filtro */
int filtrarVector(tVector* origen, tVector* destino, Filter filtro);

/* Acumula todos los elementos en un acumulador */
int reducirVector(tVector* v, void* acumulador, Reduce reducir);

/* Transforma cada elemento origen -> destino con nueva forma */
int mapearVector(tVector* origen, tVector* destino,
                 size_t tamDestino, Map mapear);

/* =========================================================
                        Auxiliares
   ========================================================= */

/* Muestra cada elemento usando la funcion mostrar del TDA */
void mostrarVector(tVector* v, Mostrar mostrar);

/* Mezcla aleatoria (Fisher-Yates): para simular resultados */
void generarResultadoAleatorioVector(tVector* v);

#endif // VECTOR_H_INCLUDED
