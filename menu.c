#include <stdio.h>
#include <stdlib.h>
#include "menu.h"
#include "utilidades.h"
#include "piloto.h"
#include "escuderia.h"
#include "carrera.h"
#include "estadisticas.h"
#include "indice.h"

/**
 * mostrarMenu
 * Imprime el menu principal y lee la opcion del usuario con scanf.
 * Retorna el entero ingresado (0 = salir).
 */
int mostrarMenu(void)
{
    int opcion;
    printf("+==================================+\n");
    printf("|      GESTION TEMPORADA F1        |\n");
    printf("|==================================|\n");
    printf("|  1. Listar pilotos y sus puntos  |\n");
    printf("|  2. Registrar carrera            |\n");
    printf("|  3. Mostrar ranking de pilotos   |\n");
    printf("|  4. Pilotos por escuderia        |\n");
    printf("|  5. Estadisticas                 |\n");
    printf("|  6. Exportar temporada           |\n");
    printf("|  7. Finalizar temporada          |\n");
    printf("|  8. Gestion de registros         |\n");
    printf("|  0. Salir                        |\n");
    printf("+==================================+\n");
    printf("Opcion: ");
    scanf("%d", &opcion);

    return opcion;
}

/**
 * menuEstadisticas
 * Submenú de estadisticas. Segun la opcion:
 *   1 -> pide ID de piloto, llama a calcularEstadisticasPiloto()
 *        y muestra el resultado con mostrarEstadisticasPiloto().
 *   2 -> pide ID de carrera y llama a mejorYPeorPosicion().
 */
void menuEstadisticas(void)
{
    int subopcion;
    unsigned id_piloto;
    int id_carrera;
    EstadisticaPiloto stats;

    printf("+==========================================+\n");
    printf("|           ESTADISTICAS                   |\n");
    printf("|==========================================|\n");
    printf("|  1. Estadisticas de un piloto            |\n");
    printf("|  2. Mejor y peor posicion por carrera    |\n");
    printf("|  0. Volver                               |\n");
    printf("+==========================================+\n");
    printf("Opcion: ");
    scanf("%d", &subopcion);

    switch (subopcion)
    {
    case 1:
        printf("Ingrese ID del piloto: ");
        scanf("%u", &id_piloto);
        if (calcularEstadisticasPiloto(RUTA_CARRERA_BIN, id_piloto, &stats) == ERR_ARCH)
        {
            printf("[!] Error al abrir el archivo.\n");
            break;
        }
        mostrarEstadisticasPiloto(&stats);
        break;
    case 2:
        printf("Ingrese ID de la carrera: ");
        scanf("%d", &id_carrera);
        mejorYPeorPosicion(RUTA_CARRERA_BIN, id_carrera);
        break;
    case 0:
        break;
    default:
        printf("[!] Opcion invalida.\n");
        break;
    }
}

/**
 * menuCarrera
 * Submenú de registro de carreras. Segun la opcion:
 *   1 -> simulacion aleatoria con Fisher-Yates.
 *   2 -> ingreso manual de posiciones.
 * En ambos casos, al terminar aplica los puntos de la ultima
 * carrera y muestra el historial actualizado.
 */
void menuCarrera(Puntos* pts)
{
    int op;

    printf("\n  +---------------------------+\n");
    printf("  |     REGISTRAR CARRERA     |\n");
    printf("  |---------------------------|\n");
    printf("  |  1. Simulacion aleatoria  |\n");
    printf("  |  2. Ingreso manual        |\n");
    printf("  |  0. volver                |\n");
    printf("  +---------------------------+\n");
    printf("  Opcion: ");
    scanf("%d", &op);

    switch (op)
    {
    case 1:
        registrarCarreraAleatoria(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN, compararUnsigned, pts);
        aplicarPuntosUltimaCarrera(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN);
        mostrarArchivoBinario(RUTA_CARRERA_BIN, sizeof(Carrera), mostrarCarrera);
        break;

    case 2:
        registrarCarreraManual(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN, compararUnsigned, pts);
        aplicarPuntosUltimaCarrera(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN);
        mostrarArchivoBinario(RUTA_CARRERA_BIN, sizeof(Carrera), mostrarCarrera);
        break;

    case 0:
        break;

    default:
        printf("[!] Opcion invalida.\n");
        break;
    }
}

/**
 * menuPilotosPorEscuderia
 * Lista todas las escuderias del .bin, pide al usuario un ID
 * y llama a listarPilotosPorEscuderia() para mostrar sus pilotos.
 */
void menuPilotosPorEscuderia(void)
{
    Escuderia esc;
    FILE*     fEsc;
    int       idEsc;

    fEsc = fopen(RUTA_ESCUDERIA_BIN, "rb");
    if (!fEsc)
    {
        printf("[!] No se encontro el archivo de escuderias.\n");
        return;
    }

    printf("\n--- Escuderias disponibles ---\n");
    while (fread(&esc, sizeof(Escuderia), 1, fEsc) == 1)
        mostrarEscuderia(&esc);
    fclose(fEsc);

    printf("\nIngrese ID de escuderia (0 para volver): ");
    scanf("%d", &idEsc);

    if (idEsc == 0)
        return;

    listarPilotosPorEscuderia(RUTA_PILOTO_BIN, RUTA_ESCUDERIA_BIN, (unsigned)idEsc);
}

/**
 * exportarTemporada
 * Exporta pilotos, escuderias y carreras de .bin a .txt (opcion 6).
 */
void exportarTemporada(void)
{
    printf("\n=============================================\n");
    printf("        EXPORTANDO DATOS DEL CAMPEONATO      \n");
    printf("=============================================\n");
    printf("[*] Exportando pilotos...\n");
    convertirArchivoBinATxt(RUTA_PILOTO_BIN, RUTA_PILOTO_EXP_TXT, sizeof(Piloto), pilotoBinATxt);

    printf("[*] Exportando escuderias...\n");
    convertirArchivoBinATxt(RUTA_ESCUDERIA_BIN, RUTA_ESCUDERIA_EXP_TXT, sizeof(Escuderia), escuderiaBinATxt);

    printf("[*] Exportando historial de carreras...\n");
    convertirArchivoBinATxt(RUTA_CARRERA_BIN, RUTA_CARRERA_EXP_TXT, sizeof(Carrera), carreraBinATxt);

    printf("\n[OK] Exportacion completa de la temporada.\n");
    printf("=============================================\n\n");
}

/**
 * finalizarTemporada
 * Cierra la temporada actual (opcion 7):
 *   1. Pide nombre para el archivo de pilotos y lo exporta/renombra.
 *   2. Pide nombre para el archivo de carreras y lo exporta/renombra.
 *   3. Elimina los .bin/.idx para que la proxima temporada arranque
 *      completamente limpia (incluyendo carrera.dat).
 */
void finalizarTemporada(void)
{
    char  nomarchivopil[TAM_LINEA];
    char  nomarchivocar[TAM_LINEA];
    char* pos;

    /* Pilotos */
    printf("\nColoque nombre del archivo pilotos (sin espacios, sin extension):\n");
    scanf("%251s", nomarchivopil);

    pos = nomarchivopil;
    while (*pos) pos++;
    *pos++ = '.';
    *pos++ = 't';
    *pos++ = 'x';
    *pos++ = 't';
    *pos = '\0';

    convertirArchivoBinATxt(RUTA_PILOTO_BIN, RUTA_PILOTO_EXP_TXT, sizeof(Piloto), pilotoBinATxt);
    if (rename(RUTA_PILOTO_EXP_TXT, nomarchivopil) == 0)
        printf("[OK] Archivo de pilotos guardado como '%s'\n", nomarchivopil);
    else
        printf("[!] Error al guardar el archivo de pilotos\n");

    /* Carreras */
    printf("\nColoque nombre del archivo carreras (sin espacios, sin extension):\n");
    scanf("%251s", nomarchivocar);

    pos = nomarchivocar;
    while (*pos) pos++;
    *pos++ = '.';
    *pos++ = 't';
    *pos++ = 'x';
    *pos++ = 't';
    *pos = '\0';

    convertirArchivoBinATxt(RUTA_CARRERA_BIN, RUTA_CARRERA_EXP_TXT, sizeof(Carrera), carreraBinATxt);
    if (rename(RUTA_CARRERA_EXP_TXT, nomarchivocar) == 0)
        printf("[OK] Archivo de carreras guardado como '%s'\n", nomarchivocar);
    else
        printf("[!] Error al guardar el archivo de carreras\n");

    /* Temporada cerrada: arranca todo de cero la proxima vez */
    remove(RUTA_CARRERA_BIN);
    remove(RUTA_PILOTO_BIN);
    remove(RUTA_ESCUDERIA_BIN);
    remove(RUTA_INDICE_PILOTO);
    printf("\n[OK] Temporada finalizada.\n");
}

/**
 * menuGestionRegistros
 * Submenú de ABM. Delega cada opcion a la funcion correspondiente:
 *   1-3: alta/baja/modificar piloto
 *   4-6: alta/baja/modificar escuderia
 *   7-8: baja/modificar carrera
 */
void menuGestionRegistros(void)
{
    int op;

    printf("+==========================================+\n");
    printf("|         GESTION DE REGISTROS             |\n");
    printf("|==========================================|\n");
    printf("|  1. Alta piloto                          |\n");
    printf("|  2. Baja piloto                          |\n");
    printf("|  3. Modificar piloto                     |\n");
    printf("|  4. Alta escuderia                       |\n");
    printf("|  5. Baja escuderia                       |\n");
    printf("|  6. Modificar escuderia                  |\n");
    printf("|  7. Baja carrera                         |\n");
    printf("|  8. Modificar carrera                    |\n");
    printf("|  0. Volver                               |\n");
    printf("+==========================================+\n");
    printf("Opcion: ");
    scanf("%d", &op);

    switch (op)
    {
    case 1:
        altaPiloto(RUTA_PILOTO_BIN);
        break;

    case 2:
        bajaPiloto(RUTA_PILOTO_BIN, RUTA_BAJAS_PILOTO_TXT);
        break;

    case 3:
        modificarPiloto(RUTA_PILOTO_BIN);
        break;

    case 4:
        altaEscuderia(RUTA_ESCUDERIA_BIN);
        break;

    case 5:
        bajaEscuderia(RUTA_ESCUDERIA_BIN, RUTA_BAJAS_ESCUDERIA);
        break;

    case 6:
        modificarEscuderia(RUTA_ESCUDERIA_BIN);
        break;

    case 7:
        bajaCarrera(RUTA_CARRERA_BIN, RUTA_BAJAS_CARRERA_TXT);
        break;

    case 8:
        modificarCarrera(RUTA_CARRERA_BIN);
        break;

    case 0:
        break;

    default:
        printf("[!] Opcion invalida.\n");
        break;
    }
}
