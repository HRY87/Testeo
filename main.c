#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "piloto.h"
#include "carrera.h"
#include "escuderia.h"

static void inicializarSistema(void);
static int mostrarMenu(void);

int main()
{
    int opcion;

    inicializarSistema();

    opcion = mostrarMenu();

    while (opcion != 0)
    {
        switch (opcion)
        {
        case 1:
            recalcularPuntosPilotos(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN, filterEsCarreraActiva, reduceAcumularPuntosCarrera);
            mostrarArchivoBinario(RUTA_PILOTO_BIN, sizeof(Piloto), mostrarPiloto);
            break;
        case 2:
            registrarCarrera(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN, compararUnsigned);
            actualizarPuntosUltimaCarrera(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN, filterEsCarreraActiva, reduceAcumularPuntosCarrera);
            mostrarArchivoBinario(RUTA_CARRERA_BIN, sizeof(Carrera), mostrarCarrera);
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


static void inicializarSistema(void)
{
    printf("--- Inicializando sistema ---\n");

    srand((unsigned)time(NULL));

    /**Creacion de lote de prueba para PILOTO**/
    if(generarArchivoPilotosTxt(RUTA_PILOTO_TXT) == TODO_OK)
    {
        printf("[OK] Se genero '%s'.\n", RUTA_PILOTO_TXT);

        if(convertirArchivoTxtABin(RUTA_PILOTO_TXT, RUTA_PILOTO_BIN, sizeof(Piloto), trozarPilotoTxt) == TODO_OK)
        {
            printf("[OK] Se genero '%s'.\n", RUTA_PILOTO_BIN);
        }
    }

     /**Creacion de lote de prueba para ESCUDERIA**/
    if(generarArchivoEscuderiasTxt(RUTA_ESCUDERIA_TXT) == TODO_OK)
    {
        printf("[OK] Se genero '%s'.\n", RUTA_ESCUDERIA_TXT);
        if(convertirArchivoTxtABin(RUTA_ESCUDERIA_TXT, RUTA_ESCUDERIA_BIN, sizeof(Escuderia), trozarEscuderiaTxt) == TODO_OK)
        {
            printf("[OK] Se genero '%s'.\n", RUTA_ESCUDERIA_BIN);
        }
    }

    printf("----------------------------\n\n");
}

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
