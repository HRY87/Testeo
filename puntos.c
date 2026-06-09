#include <stdio.h>
#include "puntos.h"
#include "utilidades.h"

/* =========================================================
                Tabla de puntos configurable
   ========================================================= */

/* Tabla estandar F1: top 10 */
static const int puntos_f1_default[10] = {25, 18, 15, 12, 10, 8, 6, 4, 2, 1};

void inicializarPuntosDefault(Puntos* vPuntos)
{
    int i;

    vPuntos->posiciones = 10;
    for (i = 0; i < vPuntos->posiciones; i++)
        vPuntos->tabla[i] = puntos_f1_default[i];

    /* Rellenar el resto con 0 */
    for (i = vPuntos->posiciones; i < MAX_POSICIONES_PUNTOS; i++)
        vPuntos->tabla[i] = 0;
}

int guardarConfigPuntos(const char* ruta, const Puntos* vPuntos)
{
    FILE* f = fopen(ruta, "wb");
    if (!f)
        return ERR_ARCH;

    fwrite(vPuntos, sizeof(Puntos), 1, f);
    fclose(f);
    return TODO_OK;
}

int cargarConfigPuntos(const char* ruta, Puntos* vPuntos)
{
    FILE* f = fopen(ruta, "rb");
    if (!f)
    {
        /* No existe: usar defaults y persistir para proximas veces */
        inicializarPuntosDefault(vPuntos);
        guardarConfigPuntos(ruta, vPuntos);
        return TODO_OK;
    }

    if (fread(vPuntos, sizeof(Puntos), 1, f) != 1)
    {
        fclose(f);
        inicializarPuntosDefault(vPuntos);
        return ERR_ARCH;
    }

    fclose(f);
    return TODO_OK;
}

int puntosParaPosicion(const Puntos* vPuntos, int posicion)
{
    if (!vPuntos || posicion < 1 || posicion > vPuntos->posiciones)
        return 0;
    return vPuntos->tabla[posicion - 1];
}

void mostrarConfigPuntos(const Puntos* vPuntos)
{
    int i;

    printf("Tabla de puntos actual (%d posiciones):\n", vPuntos->posiciones);
    printf("  Pos | Pts\n");
    printf("  ----+----\n");
    for (i = 0; i < vPuntos->posiciones; i++)
        printf("  %3d | %3d\n", i + 1, vPuntos->tabla[i]);
}
