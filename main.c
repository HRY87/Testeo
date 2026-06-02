#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "piloto.h"
#include "escuderia.h"
#include "carrera.h"

/* =========================================================
   Prototipos de funciones de menu e inicializacion
   (static: solo visibles en este archivo)
   ========================================================= */
static void inicializarSistema(void);
static int  mostrarMenu(void);

/* =========================================================
   Punto de entrada
   ========================================================= */
int main(void)
{
    int opcion;

    inicializarSistema();

    opcion = mostrarMenu();

    while (opcion != 0)
    {
        switch (opcion)
        {
        /* Funcionalidad 1: listar pilotos con sus puntos actuales */
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

        /* Funcionalidad 2: registrar carrera y actualizar puntos */
        case 2:
            if (registrarCarrera(RUTA_CARRERA_BIN,
                                 RUTA_PILOTO_BIN,
                                 compararUnsigned) == TODO_OK)
            {
                actualizarPuntosUltimaCarrera(RUTA_CARRERA_BIN,
                                             RUTA_PILOTO_BIN,
                                             filterEsCarreraActiva,
                                             reduceAcumularPuntosCarrera);

                printf("\n=== CARRERAS REGISTRADAS ===\n");
                mostrarArchivoBinario(RUTA_CARRERA_BIN,
                                      sizeof(Carrera),
                                      mostrarCarrera);
            }
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
   Genera los archivos de texto y binarios si no existen.
   Se regeneran SIEMPRE en esta version de prueba para
   garantizar datos limpios al arrancar.
   ========================================================= */
static void inicializarSistema(void)
{
    printf("--- Inicializando sistema ---\n");

    srand((unsigned)time(NULL));

    /* Lote de prueba PILOTO: txt -> bin */
    if (generarArchivoPilotosTxt(RUTA_PILOTO_TXT) == TODO_OK)
    {
        printf("[OK] Generado '%s'\n", RUTA_PILOTO_TXT);

        if (convertirArchivoTxtABin(RUTA_PILOTO_TXT,
                                    RUTA_PILOTO_BIN,
                                    sizeof(Piloto),
                                    trozarPilotoTxt) == TODO_OK)
        {
            printf("[OK] Generado '%s'\n", RUTA_PILOTO_BIN);
        }
    }

    /* Lote de prueba ESCUDERIA: txt -> bin */
    if (generarArchivoEscuderiasTxt(RUTA_ESCUDERIA_TXT) == TODO_OK)
    {
        printf("[OK] Generado '%s'\n", RUTA_ESCUDERIA_TXT);

        if (convertirArchivoTxtABin(RUTA_ESCUDERIA_TXT,
                                    RUTA_ESCUDERIA_BIN,
                                    sizeof(Escuderia),
                                    trozarEscuderiaTxt) == TODO_OK)
        {
            printf("[OK] Generado '%s'\n", RUTA_ESCUDERIA_BIN);
        }
    }

    printf("----------------------------\n\n");
}

/* =========================================================
                        Menu principal
   ========================================================= */
static int mostrarMenu(void)
{
    int opcion;

    printf("+==================================+\n");
    printf("|   GESTION TEMPORADA F1 2026      |\n");
    printf("|==================================|\n");
    printf("|  1. Listar pilotos y sus puntos  |\n");
    printf("|  2. Registrar carrera            |\n");
    printf("|  0. Salir                        |\n");
    printf("+==================================+\n");
    printf("Opcion: ");
    scanf("%d", &opcion);

    return opcion;
}
