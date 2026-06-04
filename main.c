#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "piloto.h"
#include "escuderia.h"
#include "carrera.h"

/* =========================================================
   Prototipos internos
   ========================================================= */
static void inicializarSistema(void);
static int  mostrarMenu(void);
static int  mostrarSubMenuCarrera(void);

/* =========================================================
   Punto de entrada
   ========================================================= */
int main(void)
{
    int opcion;
    int subOpcion;

    inicializarSistema();
    opcion = mostrarMenu();

    while (opcion != 0)
    {
        switch (opcion)
        {
        /* --- 1. Listar pilotos con puntos recalculados --- */
        case 1:
            recalcularPuntosPilotos(RUTA_CARRERA_BIN,
                                    RUTA_PILOTO_BIN,
                                    filterEsCarreraActiva,
                                    reduceAcumularPuntosCarrera);

            printf("\n=== PILOTOS - TEMPORADA 2026 ===\n");
            mostrarArchivoBinario(RUTA_PILOTO_BIN,
                                  sizeof(Piloto),
                                  mostrarPiloto);
            break;

        /* --- 2. Registrar carrera (manual o aleatoria) --- */
        case 2:
            subOpcion = mostrarSubMenuCarrera();

            if (subOpcion == 1)
            {
                if (registrarCarreraAleatoria(RUTA_CARRERA_BIN,
                                              RUTA_PILOTO_BIN,
                                              compararUnsigned) == TODO_OK)
                {
                    actualizarPuntosUltimaCarrera(RUTA_CARRERA_BIN,
                                                 RUTA_PILOTO_BIN,
                                                 filterEsCarreraActiva,
                                                 reduceAcumularPuntosCarrera);
                    printf("\n=== CARRERAS REGISTRADAS ===\n");
                    listarTodasLasCarreras(RUTA_CARRERA_BIN);
                }
            }
            else if (subOpcion == 2)
            {
                if (registrarCarreraManual(RUTA_CARRERA_BIN,
                                           RUTA_PILOTO_BIN,
                                           compararUnsigned) == TODO_OK)
                {
                    actualizarPuntosUltimaCarrera(RUTA_CARRERA_BIN,
                                                 RUTA_PILOTO_BIN,
                                                 filterEsCarreraActiva,
                                                 reduceAcumularPuntosCarrera);
                    printf("\n=== CARRERAS REGISTRADAS ===\n");
                    listarTodasLasCarreras(RUTA_CARRERA_BIN);
                }
            }
            break;

        /* --- 3. Listar escuderias --- */
        case 3:
            printf("\n=== ESCUDERIAS ===\n");
            mostrarArchivoBinario(RUTA_ESCUDERIA_BIN,
                                  sizeof(Escuderia),
                                  mostrarEscuderia);
            break;

        /* --- 4. Listar todas las carreras --- */
        case 4:
            printf("\n=== CARRERAS DE LA TEMPORADA ===\n");
            listarTodasLasCarreras(RUTA_CARRERA_BIN);
            break;

        default:
            printf("[!] Opcion invalida.\n\n");
            break;
        }

        opcion = mostrarMenu();
    }

    printf("Hasta luego.\n");
    return 0;
}

/* =========================================================
   Inicializacion del sistema
   ========================================================= */
static void inicializarSistema(void)
{
    printf("--- Inicializando sistema ---\n");

    srand((unsigned)time(NULL));

    /* Piloto: txt -> bin (siempre regenera para datos limpios) */
    if (generarArchivoPilotosTxt(RUTA_PILOTO_TXT) == TODO_OK)
    {
        printf("[OK] Generado '%s'\n", RUTA_PILOTO_TXT);

        if (convertirArchivoTxtABin(RUTA_PILOTO_TXT,
                                    RUTA_PILOTO_BIN,
                                    sizeof(Piloto),
                                    trozarPilotoTxt) == TODO_OK)
            printf("[OK] Generado '%s'\n", RUTA_PILOTO_BIN);
    }

    /* Escuderia: txt -> bin */
    if (generarArchivoEscuderiasTxt(RUTA_ESCUDERIA_TXT) == TODO_OK)
    {
        printf("[OK] Generado '%s'\n", RUTA_ESCUDERIA_TXT);

        if (convertirArchivoTxtABin(RUTA_ESCUDERIA_TXT,
                                    RUTA_ESCUDERIA_BIN,
                                    sizeof(Escuderia),
                                    trozarEscuderiaTxt) == TODO_OK)
            printf("[OK] Generado '%s'\n", RUTA_ESCUDERIA_BIN);
    }

    printf("----------------------------\n\n");
}

/* =========================================================
   Menus
   ========================================================= */
static int mostrarMenu(void)
{
    int opcion;

    printf("+=====================================+\n");
    printf("|    GESTION TEMPORADA F1 2026        |\n");
    printf("|=====================================|\n");
    printf("|  1. Listar pilotos y sus puntos     |\n");
    printf("|  2. Registrar carrera               |\n");
    printf("|  3. Listar escuderias               |\n");
    printf("|  4. Ver todas las carreras          |\n");
    printf("|  0. Salir                           |\n");
    printf("+=====================================+\n");
    printf("Opcion: ");
    scanf("%d", &opcion);

    return opcion;
}

static int mostrarSubMenuCarrera(void)
{
    int op;

    printf("\n  +---------------------------+\n");
    printf("  |  Tipo de registro         |\n");
    printf("  |---------------------------|\n");
    printf("  |  1. Simulacion aleatoria  |\n");
    printf("  |  2. Ingreso manual        |\n");
    printf("  +---------------------------+\n");
    printf("  Opcion: ");
    scanf("%d", &op);

    return op;
}
