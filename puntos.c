#include <stdio.h>
#include "puntos.h"
#include "utilidades.h"

#define CANT_DEFAULT        11
#define CAP_INICIAL_PUNTOS  32   /* capacidad inicial del vector, puede crecer */

static const int puntos_f1_default[CANT_DEFAULT] = {0, 25, 18, 15, 12, 10, 8, 6, 4, 2, 1};

/* =========================================================
   Genera puntos.txt con los valores F1 por defecto.
   ========================================================= */
int generarArchivoPuntosTxt(const char* ruta)
{
    FILE* f;
    int   i;

    f = fopen(ruta, "wt");
    if (!f)
        return ERR_ARCH;

    for (i = 0; i < CANT_DEFAULT; i++)
        fprintf(f, "%d\n", puntos_f1_default[i]);

    fclose(f);
    return TODO_OK;
}

/* =========================================================
   Carga la tabla desde .txt al vector interno.
   Si el archivo no existe, lo genera con defaults.
   ========================================================= */
int cargarConfigPuntos(const char* ruta, Puntos* p)
{
    FILE* f;
    int   valor;
    int   resp;

    resp = crearVector(&p->tabla, sizeof(int), CAP_INICIAL_PUNTOS);
    if (resp != TODO_OK)
        return resp;

    f = fopen(ruta, "rt");
    if (!f)
    {
        /* No existe: generar con defaults y cargar desde el arreglo estático */
        generarArchivoPuntosTxt(ruta);

        f = fopen(ruta, "rt");
        if (!f)
        {
            destruirVector(&p->tabla);
            return ERR_ARCH;
        }
    }

    while (fscanf(f, "%d\n", &valor) == 1)
    {
        resp = insertarFinalVector(&p->tabla, &valor);

        if (resp == VEC_LLENO)
        {
            /* No debería pasar con CAP_INICIAL_PUNTOS = 32, pero por las dudas */
            fclose(f);
            return VEC_LLENO;
        }
    }

    fclose(f);
    return TODO_OK;
}

/* =========================================================
   Retorna los puntos para una posición (base 1).
   tabla[0] = 0 (posición ficticia), tabla[1] = 1er lugar, etc.
   ========================================================= */
int obtenerPuntosPorPosicion(const Puntos* p, int posicion)
{
    int* elem;

    if (!p || posicion < 1)
        return 0;

    elem = (int*)obtenerElementoVector((tVector*)&p->tabla, (size_t)posicion);
    if (!elem)
        return 0;

    return *elem;
}

/* =========================================================
   Muestra la tabla por pantalla.
   ========================================================= */
void mostrarConfigPuntos(const Puntos* p)
{
    int    i;
    int*   elem;
    size_t total;

    total = p->tabla.ce;

    printf("  Tabla de puntos (%zu posiciones con puntos):\n", total > 0 ? total - 1 : 0);
    printf("  Pos | Pts\n");
    printf("  ----+----\n");

    for (i = 1; i < (int)total; i++)
    {
        elem = (int*)obtenerElementoVector((tVector*)&p->tabla, (size_t)i);
        if (elem)
            printf("  %3d | %3d\n", i, *elem);
    }
}

/* =========================================================
   Libera el vector interno.
   ========================================================= */
void destruirPuntos(Puntos* p)
{
    destruirVector(&p->tabla);
}
