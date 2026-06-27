#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "vector.h"

/**Dinamico**/

/**
 * crearVector
 * Reserva en heap un bloque contiguo para 'capacidad' elementos
 * de 'tamElem' bytes cada uno. Inicializa ce=0 y guarda
 * tamElem y tope en la estructura.
 * Retorna TODO_OK o SIN_MEM si malloc falla.
 */
int crearVector(tVector* v, size_t tamElem, size_t capacidad)
{
    v->vec = malloc(tamElem * capacidad);

    if(!v->vec)
        return SIN_MEM;

    v->ce = 0;
    v->tamElem = tamElem;
    v->cap = capacidad;

    return TODO_OK;
}

/**
 * destruirVector
 * Libera el bloque de memoria del vector y resetea todos
 * los campos a 0. Debe llamarse siempre al terminar de usar
 * el vector para evitar memory leaks.
 */
void destruirVector(tVector* v)
{
    free(v->vec);

    v->ce = 0;
    v->tamElem = 0;
    v->cap = 0;
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
 * insertarVectorOrd
 * Inserta 'dato' manteniendo el vector ordenado segun el criterio
 * de 'cmp' (insertion sort por busqueda lineal).
 * Paso a paso:
 *   1. Busca la posicion correcta avanzando mientras cmp() > 0.
 *   2. Desplaza todos los elementos posteriores un lugar hacia
 *      adelante para abrir el hueco (con aritmetica de punteros).
 *   3. Copia el dato en la posicion encontrada.
 * Retorna VEC_LLENO si no hay espacio, TODO_OK si inserto bien.
 */
int insertarVectorOrd(tVector* v, void* dato, Comparar cmp)
{
    char* act = (char*)v->vec;
    char* fin = (char*)v->vec + (v->ce * v->tamElem);

    if(v->ce == v->cap)
    {
        if(redimensionarVector(v, v->cap * FACTOR_INCREMENTAL) == SIN_MEM)
            return SIN_MEM;
    }

     /** Busca la posicion donde dato debe insertarse */
    while(act < fin && cmp(dato, act) > 0)
    {
        act += v->tamElem;
    }

    /** Desplaza hacia adelante desde el final hasta la posicion encontrada */
    while(fin > act)
    {
        memcpy(fin, fin - v->tamElem, v->tamElem);
        fin -= v->tamElem;
    }

    memcpy(act, dato, v->tamElem); /** Copia el nuevo elemento en su lugar */

    v->ce++;

    return TODO_OK;
}

/**
 * insertarFinalVector
 * Agrega 'dato' al final del vector sin mantener orden.
 * Retorna VEC_LLENO si alcanzo la capacidad maxima, TODO_OK si inserto.
 */
int insertarFinalVector(tVector* v, const void* dato)
{
    if(v->ce == v->cap)
    {
        if(redimensionarVector(v, v->cap * FACTOR_INCREMENTAL) == SIN_MEM)
            return SIN_MEM;
    }

    memcpy((char*)v->vec + (v->ce * v->tamElem), dato, v->tamElem);
    v->ce++;

    return TODO_OK;
}

/**
 * busquedaBinariaVector
 * Busca 'clave' en el vector ORDENADO usando busqueda binaria.
 * Paso a paso:
 *   1. Calcula el indice medio y compara con comparar().
 *   2. Si cmp == 0: encontrado, retorna el puntero al elemento.
 *   3. Si cmp < 0: descarta la mitad derecha (der = mid).
 *   4. Si cmp > 0: descarta la mitad izquierda (izq = mid + 1).
 * Retorna puntero al elemento encontrado, o NULL si no existe.
 */
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

        if(resp == 0)
            return obtenerElementoVector(v, mid); /**Encontrado*/

        if(resp < 0)
            der = mid; /** La clave esta en la mitad izquierda */
        else
            izq = mid + 1; /** La clave esta en la mitad derecha   */
    }

    return NULL;
}

/**
 * obtenerElementoVector
 * Retorna un puntero void* al elemento en la posicion 'pos'.
 * Calcula la direccion con aritmetica de punteros:
 *   base + pos * tamElem
 */
void* obtenerElementoVector(tVector* v, size_t pos)
{
    return( (char*)v-> vec + (pos * v->tamElem));
}

/**Funciones para vector-archivo**/

/**
 * cargarVectorDesdeBin
 * Abre el archivo binario y lee registros de tamElem bytes
 * hasta llenar el vector o llegar al fin del archivo.
 * Retorna TODO_OK o ERR_ARCH si no pudo abrir el archivo.
 */
int cargarVectorDesdeBin(const char* rutaBin, tVector* v)
{
    char* act = (char*)v->vec;
    char* fin = (char*)v->vec + (v->cap * v->tamElem);

    FILE* fBin = fopen(rutaBin, "rb");

    if(!fBin)
        return ERR_ARCH;

    /** Lee mientras haya espacio en el vector y datos en el archivo */
    while(act < fin && fread(act, v->tamElem, 1, fBin) == 1)
    {
        v->ce++;
        act += v->tamElem;
    }

    fclose(fBin);
    return TODO_OK;
}

/**
 * guardarVectorEnBin
 * Sobreescribe el archivo binario con el contenido actual
 * del vector (solo los ce elementos cargados).
 * Retorna TODO_OK o ERR_ARCH si no pudo abrir el archivo.
 */
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
/**
 * filtrarVector
 * Recorre 'origen' y copia en 'destino' solo los elementos
 * para los que filtro() retorna distinto de 0.
 * Si destino se llena antes de terminar, retorna VEC_LLENO.
 * Retorna SIN_MEM si alguno de los punteros es NULL.
 */
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
            if(destino->ce == destino->cap)
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
 * Aplica la funcion 'reducir' (firma Reduce) a cada elemento
 * del vector acumulando el resultado en 'acumulador'.
 * El tipo y valor inicial del acumulador los define el llamador.
 * Retorna SIN_MEM si alguno de los punteros es NULL, TODO_OK si no.
 */
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

/**
 * mapearVector
 * Transforma cada elemento de 'origen' aplicando mapear() y
 * almacena el resultado en 'destino'. Los elementos destino
 * pueden tener un tamanio distinto (tamDestino).
 * Retorna SIN_MEM si alguno de los punteros es NULL,
 * VEC_LLENO si destino no tiene capacidad suficiente.
 */
int mapearVector(tVector* origen, tVector* destino, size_t tamDestino, Map mapear)
{
    char* actOrigen;
    char* finOrigen;
    char* actDestino;

    if(!origen || !destino || !mapear)
        return SIN_MEM;

    if(origen->ce > destino->cap)
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
/**
 * mostrarVector
 * Recorre el vector con aritmetica de punteros y llama a
 * mostrar() para cada elemento. Imprime un tabulador entre
 * elementos y un salto de linea al final.
 */
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

/**
 * generarResultadoAleatorioVector  [Algoritmo de Fisher-Yates]
 * Baraja aleatoriamente los elementos del vector con
 * distribucion uniforme.
 * Paso a paso:
 *   Desde el ultimo elemento hacia el primero (i = ce-1 .. 1):
 *     1. Elige un indice j aleatorio entre 0 e i (inclusive).
 *     2. Intercambia los elementos en i y j.
 * Resultado: permutacion aleatoria uniforme de los elementos.
 */
void generarResultadoAleatorioVector(tVector* v)
{
    size_t i, j;

    for(i = v->ce - 1; i > 0; i--)
    {
        j = (size_t)(rand() % (int)(i + 1)); /** Indice aleatorio en [0, i] */

        intercambiar(obtenerElementoVector(v, i),obtenerElementoVector(v, j), v->tamElem);
    }
}
