#include <stdio.h>
#include "puntos.h"
#include "utilidades.h"

/* =========================================================
                Tabla de puntos configurable
   ========================================================= */

/* Tabla estandar F1: top 10 */
static const int puntos_f1_default[10] = {25, 18, 15, 12, 10, 8, 6, 4, 2, 1};

void inicializarPuntosDefault(Puntos* cfg)
{
    int i;

    cfg->posiciones = 10;
    for (i = 0; i < cfg->posiciones; i++)
        cfg->tabla[i] = puntos_f1_default[i];

    /* Rellenar el resto con 0 */
    for (i = cfg->posiciones; i < MAX_POSICIONES_PUNTOS; i++)
        cfg->tabla[i] = 0;
}

int guardarConfigPuntos(const char* ruta, const Puntos* cfg)
{
    FILE* f = fopen(ruta, "wb");
    if (!f)
        return ERR_ARCH;

    fwrite(cfg, sizeof(Puntos), 1, f);
    fclose(f);
    return TODO_OK;
}

int cargarConfigPuntos(const char* ruta, Puntos* cfg)
{
    FILE* f = fopen(ruta, "rb");
    if (!f)
    {
        /* No existe: usar defaults y persistir para proximas veces */
        inicializarPuntosDefault(cfg);
        guardarConfigPuntos(ruta, cfg);
        return TODO_OK;
    }

    if (fread(cfg, sizeof(Puntos), 1, f) != 1)
    {
        fclose(f);
        inicializarPuntosDefault(cfg);
        return ERR_ARCH;
    }

    fclose(f);
    return TODO_OK;
}

int puntosParaPosicion(const Puntos* cfg, int posicion)
{
    if (!cfg || posicion < 1 || posicion > cfg->posiciones)
        return 0;
    return cfg->tabla[posicion - 1];
}

void mostrarConfigPuntos(const Puntos* cfg)
{
    int i;

    printf("Tabla de puntos actual (%d posiciones):\n", cfg->posiciones);
    printf("  Pos | Pts\n");
    printf("  ----+----\n");
    for (i = 0; i < cfg->posiciones; i++)
        printf("  %3d | %3d\n", i + 1, cfg->tabla[i]);
}
