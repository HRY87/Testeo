#include <stdio.h>
#include <string.h>
#include "puntos.h"
#include "utilidades.h"

// Tabla estándar de F1: top 10
#define CANT_DEFAULT_F1  10
static const int puntos_f1_default[CANT_DEFAULT_F1] = {25, 18, 15, 12, 10, 8, 6, 4, 2, 1};


/* ------------------------------------------------------------- *
 *  inicializarPuntosDefault                                     *
 *  Crea el vector interno y lo llena con los valores F1.        *
 * ------------------------------------------------------------- */
void inicializarPuntosDefault(Puntos* vPuntos)
{
    int i;

    crearVector(&vPuntos->tabla, sizeof(int), CAP_INICIAL_PUNTOS);

    for (i = 0; i < CANT_DEFAULT_F1; i++)
        insertarFinalVector(&vPuntos->tabla, &puntos_f1_default[i]);
}


/* ------------------------------------------------------------- *
 *  guardarConfigPuntos                                          *
 *  Recorre el vector y escribe un entero por línea en el .txt.  *
 *  Retorna TODO_OK o ERR_ARCH.                                  *
 * ------------------------------------------------------------- */
int guardarConfigPuntos(const char* ruta, const Puntos* vPuntos)
{
    FILE*  f;
    int*   elem;
    size_t i;

    f = fopen(ruta, "wt");
    if (!f)
        return ERR_ARCHIVO;

    for (i = 0; i < vPuntos->tabla.ce; i++)
    {
        elem = (int*)obtenerElementoVector((tVector*)&vPuntos->tabla, i);
        if (elem)
            fprintf(f, "%d\n", *elem);
    }

    fclose(f);
    return TODO_OK;
}


/* ------------------------------------------------------------- *
 *  cargarConfigPuntos                                           *
 *  Crea el vector interno y lo llena desde el .txt.             *
 *  - Ignora líneas vacías y comentarios (#).                    *
 *  - El vector crece dinámicamente: no hay límite fijo.         *
 *  Si el archivo no existe: carga defaults y lo persiste.       *
 *  Si el archivo existe pero está vacío: igual que arriba.      *
 *  Retorna TODO_OK o ERR_ARCH.                                  *
 * ------------------------------------------------------------- */
int cargarConfigPuntos(const char* ruta, Puntos* vPuntos)
{
    FILE* f;
    char  linea[TAM_LINEA];
    int   valor;
    int   resp;
    int   cuenta = 0;

    resp = crearVector(&vPuntos->tabla, sizeof(int), CAP_INICIAL_PUNTOS);
    if (resp != TODO_OK)
        return resp;

    f = fopen(ruta, "rt");
    if (!f)
    {
        /* Archivo inexistente: usar defaults y persistirlos */
        for (int i = 0; i < CANT_DEFAULT_F1; i++)
            insertarFinalVector(&vPuntos->tabla, &puntos_f1_default[i]);

        guardarConfigPuntos(ruta, vPuntos);
        return TODO_OK;
    }

    while (fgets(linea, sizeof(linea), f) != NULL)
    {
        /* Ignorar comentarios y líneas vacías */
        if (linea[0] == '#' || linea[0] == '\n' || linea[0] == '\r')
            continue;

        if (sscanf(linea, "%d", &valor) == 1)
        {
            insertarFinalVector(&vPuntos->tabla, &valor);
            cuenta++;
        }
    }

    fclose(f);

    if (cuenta == 0)
    {
        /* Archivo vacío o sin datos válidos: usar defaults */
        for (int i = 0; i < CANT_DEFAULT_F1; i++)
            insertarFinalVector(&vPuntos->tabla, &puntos_f1_default[i]);

        guardarConfigPuntos(ruta, vPuntos);
    }

    return TODO_OK;
}


/* ------------------------------------------------------------- *
 *  puntosParaPosicion                                           *
 *  Devuelve los puntos de la posición dada (base 1).            *
 *  Retorna 0 si la posición está fuera de rango o p es NULL.    *
 * ------------------------------------------------------------- */
int puntosParaPosicion(const Puntos* vPuntos, int posicion)
{
    int* elem;

    if (!vPuntos || posicion < 1)
        return NO_PUNTUA;

    elem = (int*)obtenerElementoVector((tVector*)&vPuntos->tabla, (size_t)(posicion - 1));

    if (!elem)
        return NO_PUNTUA;

    return *elem;
}


/* ------------------------------------------------------------- *
 *  mostrarConfigPuntos                                          *
 *  Imprime la tabla completa en formato columnar.               *
 * ------------------------------------------------------------- */
void mostrarConfigPuntos(const Puntos* vPuntos)
{
    int*   elem;
    size_t i;

    printf("  Tabla de puntos (%zu posiciones con puntos):\n", vPuntos->tabla.ce);
    printf("  Pos | Pts\n");
    printf("  ----+----\n");

    for (i = 0; i < vPuntos->tabla.ce; i++)
    {
        elem = (int*)obtenerElementoVector((tVector*)&vPuntos->tabla, i);
        if (elem)
            printf("  %3zu | %3d\n", i + 1, *elem);
    }
}


/* ------------------------------------------------------------- *
 *  destruirPuntos                                               *
 *  Libera el vector interno. Llamar al final del programa.      *
 * ------------------------------------------------------------- */
void destruirPuntos(Puntos* vPuntos)
{
    destruirVector(&vPuntos->tabla);
}
