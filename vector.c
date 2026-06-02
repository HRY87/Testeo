#include <memory.h>
#include "vector.h"

/* =========================================================
                        Ciclo de vida
   ========================================================= */

/**
 * crearVector
 * Reserva memoria para un vector de 'capacidad' elementos
 * de 'tamElem' bytes cada uno. Inicializa ce=0.
 * Retorna TODO_OK o SIN_MEM.
 */
int crearVector(tVector* v, size_t tamElem, size_t capacidad)
{
    v->vec = malloc(tamElem * capacidad);
    if (!v->vec)
        return SIN_MEM;

    v->ce      = 0;
    v->tamElem = tamElem;
    v->tope    = capacidad;

    return TODO_OK;
}

/**
 * destruirVector
 * Libera la memoria del vector y resetea sus campos.
 */
void destruirVector(tVector* v)
{
    free(v->vec);
    v->vec     = NULL;
    v->ce      = 0;
    v->tamElem = 0;
    v->tope    = 0;
}

/* =========================================================
                    Insercion y acceso
   ========================================================= */

/**
 * insertarVectorOrd
 * Inserta 'dato' en la posicion correcta segun cmp,
 * desplazando los elementos mayores hacia la derecha.
 * Requiere que el vector ya este ordenado con el mismo cmp.
 * Retorna TODO_OK o VEC_LLENO.
 */
int insertarVectorOrd(tVector* v, void* dato, Comparar cmp)
{
    char* act;
    char* fin;

    if(v->ce == v->tope)
        return VEC_LLENO;

    act = (char*)v->vec;
    fin = act + (v->ce * v->tamElem);

    /* Buscar posicion de insercion */
    while (act < fin && cmp(dato, act) > 0)
        act += v->tamElem;

    /* Desplazar elementos para hacer lugar */
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
 * busquedaBinariaVector
 * Busca 'clave' en un vector ordenado con el mismo criterio
 * que se uso al insertar. Retorna puntero al elemento o NULL.
 */
void* busquedaBinariaVector(tVector* v, void* clave, Comparar comparar)
{
    size_t izq, der, mid;
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
 * Retorna puntero al elemento en la posicion 'pos'.
 * No verifica rango: el llamador es responsable.
 */
void* obtenerElementoVector(tVector* v, size_t pos)
{
    return (char*)v->vec + (pos * v->tamElem);
}

/* =========================================================
            Persistencia: vector <-> archivo binario
   ========================================================= */

/**
 * cargarVectorDesdeBin
 * Lee registros del .bin hasta llenar el vector o agotar
 * el archivo. Los elementos NO se insertan ordenados;
 * se cargan en el orden del archivo.
 */
int cargarVectorDesdeBin(const char* rutaBin, tVector* v)
{
    char* act;
    char* fin;
    FILE* fBin;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    act = (char*)v->vec;
    fin = act + (v->tope * v->tamElem);

    while (act < fin && fread(act, v->tamElem, 1, fBin) == 1)
    {
        v->ce++;
        act += v->tamElem;
    }

    fclose(fBin);
    return TODO_OK;
}

/**
 * guardarVectorEnBin
 * Escribe todos los elementos del vector en un .bin
 * (sobreescribe el archivo si ya existe).
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
    fin = act + (v->ce * v->tamElem);

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
 * Recorre 'origen' y copia a 'destino' los elementos
 * que pasan el filtro. Destino debe estar creado previamente.
 * Retorna TODO_OK o VEC_LLENO si destino se llena.
 */
int filtrarVector(tVector* origen, tVector* destino, Filter filtro)
{
    char* act;
    char* fin;

    if (!origen || !destino || !filtro)
        return SIN_MEM;

    act = (char*)origen->vec;
    fin = act + (origen->ce * origen->tamElem);

    while (act < fin)
    {
        if (filtro(act))
        {
            if (destino->ce == destino->tope)
                return VEC_LLENO;

            memcpy((char*)destino->vec + (destino->ce * destino->tamElem),
                   act, destino->tamElem);
            destino->ce++;
        }
        act += origen->tamElem;
    }

    return TODO_OK;
}

/**
 * reducirVector
 * Recorre el vector y acumula informacion en 'acumulador'
 * usando la funcion 'reducir' del TDA.
 */
int reducirVector(tVector* v, void* acumulador, Reduce reducir)
{
    char* act;
    char* fin;

    if (!v || !acumulador || !reducir)
        return SIN_MEM;

    act = (char*)v->vec;
    fin = act + (v->ce * v->tamElem);

    while (act < fin)
    {
        reducir(acumulador, act);
        act += v->tamElem;
    }

    return TODO_OK;
}

/**
 * mapearVector
 * Transforma cada elemento de 'origen' en uno de 'destino'
 * usando la funcion 'mapear'. Los elementos destino pueden
 * tener un tamanio diferente (tamDestino).
 */
int mapearVector(tVector* origen, tVector* destino,
                 size_t tamDestino, Map mapear)
{
    char* actOrigen;
    char* finOrigen;
    char* actDestino;

    if (!origen || !destino || !mapear)
        return SIN_MEM;

    if (origen->ce > destino->tope)
        return VEC_LLENO;

    actOrigen  = (char*)origen->vec;
    finOrigen  = actOrigen + (origen->ce * origen->tamElem);
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
 * Recorre el vector mostrando cada elemento con mostrar().
 * Util para debug o listados rapidos.
 */
void mostrarVector(tVector* v, Mostrar mostrar)
{
    char*  act = (char*)v->vec;
    size_t i;

    for (i = 0; i < v->ce; i++)
    {
        mostrar(act);
        act += v->tamElem;
    }
}

/**
 * generarResultadoAleatorioVector
 * Mezcla aleatoriamente los elementos con el algoritmo
 * Fisher-Yates. Se usa para simular resultados de carrera.
 * Requiere haber llamado srand() previamente.
 */
void generarResultadoAleatorioVector(tVector* v)
{
    size_t i, j;

    for (i = v->ce - 1; i > 0; i--)
    {
        j = (size_t)(rand() % (int)(i + 1));
        intercambiar(obtenerElementoVector(v, i),
                     obtenerElementoVector(v, j),
                     v->tamElem);
    }
}
