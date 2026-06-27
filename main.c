#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "piloto.h"
#include "carrera.h"
#include "escuderia.h"
#include "puntos.h"
#include "indice.h"
#include "menu.h"

static void inicializarSistema(Puntos* pts);
static void inicializarPilotos(void);
static void inicializarEscuderias(void);
static void inicializarPuntos(Puntos* pts);
static void finalizarSesion(void);

int main()
{
    int opcion;
    Puntos vPuntos;

    inicializarSistema(&vPuntos);

    opcion = mostrarMenu();

    while (opcion != 0)
    {
        system("cls");
        switch (opcion)
        {
        case 1:
            listarPilotos(RUTA_PILOTO_BIN);
            break;
        case 2:
            menuCarrera(&vPuntos);
            break;
        case 3:
            RankingPiloto(RUTA_PILOTO_BIN);
            break;
        case 4:
            menuPilotosPorEscuderia();
            break;
        case 5:
            menuEstadisticas();
            break;
        case 6:
            exportarTemporada();
            break;
        case 7:
            finalizarTemporada();
            printf("\nLo esperamos la siguiente temporada!\n");
            exit(0);
            break;
        case 8:
            system("cls");
            menuGestionRegistros();
            break;
        default:
            printf("[!] Opcion invalida.\n\n");
            break;
        }
        opcion = mostrarMenu();
    }

    finalizarSesion();
    destruirPuntos(&vPuntos);
    printf("Hasta luego.\n");
    return 0;
}


/**
 * inicializarSistema
 * Punto de arranque del sistema:
 *   1. srand() con time(NULL) para la simulacion aleatoria.
 *   2. Inicializa pilotos, escuderias y la tabla de puntos.
 *   3. Recalcula los puntos contra el historial de carreras
 *      (carrera.dat puede seguir vivo de una sesion anterior
 *      cerrada con la opcion 0).
 */
static void inicializarSistema(Puntos* pts)
{
    printf("--- Inicializando sistema ---\n");

    srand((unsigned)time(NULL));

    inicializarPilotos();
    inicializarEscuderias();
    inicializarPuntos(pts);

    recalcularPuntosPilotos(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN);

    printf("----------------------------\n\n");
}

/**
 * inicializarPilotos
 * Genera el lote inicial de pilotos.txt solo si no existe todavia,
 * reconstruye pilotos.dat a partir del .txt y genera su indice.
 */
static void inicializarPilotos(void)
{
    if (!archivoExiste(RUTA_PILOTO_TXT))
    {
        generarArchivoPilotosTxt(RUTA_PILOTO_TXT);
        printf("[OK] Lote inicial generado en '%s'\n", RUTA_PILOTO_TXT);
    }

    if (convertirArchivoTxtABin(RUTA_PILOTO_TXT, RUTA_PILOTO_BIN, sizeof(Piloto), trozarPilotoTxt) != TODO_OK)
    {
        printf("[!] Error al generar '%s'\n", RUTA_PILOTO_BIN);
        return;
    }

    printf("[OK] Generado '%s'\n", RUTA_PILOTO_BIN);

    if (construirIndicePilotos(RUTA_PILOTO_BIN, RUTA_INDICE_PILOTO) == TODO_OK)
        printf("[OK] Indice generado '%s'\n", RUTA_INDICE_PILOTO);
    else
        printf("[!] Error al generar el indice de pilotos\n");
}

/**
 * inicializarEscuderias
 * Mismo criterio que inicializarPilotos pero sin indice.
 */
static void inicializarEscuderias(void)
{
    if (!archivoExiste(RUTA_ESCUDERIA_TXT))
    {
        generarArchivoEscuderiasTxt(RUTA_ESCUDERIA_TXT);
        printf("[OK] Lote inicial generado en '%s'\n", RUTA_ESCUDERIA_TXT);
    }

    if (convertirArchivoTxtABin(RUTA_ESCUDERIA_TXT, RUTA_ESCUDERIA_BIN, sizeof(Escuderia), trozarEscuderiaTxt) == TODO_OK)
        printf("[OK] Generado '%s'\n", RUTA_ESCUDERIA_BIN);
    else
        printf("[!] Error al generar '%s'\n", RUTA_ESCUDERIA_BIN);
}

/**
 * inicializarPuntos
 * Carga la tabla de puntos desde puntos.txt (la genera si no existe).
 */
static void inicializarPuntos(Puntos* pts)
{
    if (cargarConfigPuntos(RUTA_PUNTOS_TXT, pts) == TODO_OK)
        printf("[OK] Tabla de puntos cargada desde '%s'\n", RUTA_PUNTOS_TXT);
    else
        printf("[!] Error al cargar la tabla de puntos\n");
}

/**
 * finalizarSesion
 * Se ejecuta al salir con la opcion 0 (salida normal, NO finaliza
 * la temporada). Persiste el estado actual de pilotos y escuderias
 * en sus .txt de origen y elimina los .bin/.idx, para que la
 * proxima ejecucion los reconstruya desde el texto actualizado.
 * carrera.dat NO se borra aca: la temporada sigue activa hasta
 * que el usuario elija la opcion 7.
 */
static void finalizarSesion(void)
{
    convertirArchivoBinATxt(RUTA_PILOTO_BIN, RUTA_PILOTO_TXT, sizeof(Piloto), pilotoBinATxt);
    remove(RUTA_PILOTO_BIN);
    remove(RUTA_INDICE_PILOTO);

    convertirArchivoBinATxt(RUTA_ESCUDERIA_BIN, RUTA_ESCUDERIA_TXT, sizeof(Escuderia), escuderiaBinATxt);
    remove(RUTA_ESCUDERIA_BIN);
}
