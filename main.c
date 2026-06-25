#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "piloto.h"
#include "carrera.h"
#include "estadisticas.h"
#include "escuderia.h"
#include "puntos.h"

static void inicializarSistema(Puntos* pts);
static int mostrarMenu(void);
static void menuEstadisticas(void);
static void menuCarrera(Puntos* pts);
static void menuPilotosPorEscuderia(void);
static void finalizarTemporada(void);
static void menuGestionRegistros(void);

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
            //recalcularPuntosPilotos(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN);
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
            printf("\n=============================================\n");
            printf("        EXPORTANDO DATOS DEL CAMPEONATO      \n");
            printf("=============================================\n");
            printf("[*] Exportando pilotos...\n");
            exportarPilotosTxt(RUTA_PILOTO_BIN, RUTA_PILOTO_EXP_TXT);

            printf("[*] Exportando historial de carreras...\n");
            exportarCarrerasTxt(RUTA_CARRERA_BIN, RUTA_CARRERA_EXP_TXT);

            printf("\n[OK] Exportacion completa de la temporada.\n");
            printf("=============================================\n\n");
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

    destruirPuntos(&vPuntos);
    printf("Hasta luego.\n");
    return 0;
}

/**
 * inicializarSistema
 * Punto de arranque del sistema. Realiza:
 *   1. srand() con time(NULL) para la simulacion aleatoria.
 *   2. Genera pilotos.txt con el lote inicial y lo convierte a .bin.
 *   3. Genera escuderias.txt y lo convierte a escuderias.dat.
 *   4. Carga la tabla de puntos desde puntos.txt en la struct Puntos.
 */
static void inicializarSistema(Puntos* pts)
{
    printf("--- Inicializando sistema ---\n");

    srand((unsigned)time(NULL));

    /* Pilotos: txt -> bin */
    if (generarArchivoPilotosTxt(RUTA_PILOTO_TXT) == TODO_OK)
    {
        printf("[OK] Generado '%s'\n", RUTA_PILOTO_TXT);

        if (cargarArchivoPilotos(RUTA_PILOTO_TXT, RUTA_PILOTO_BIN) > 0)
            printf("[OK] Generado '%s'\n", RUTA_PILOTO_BIN);
        else
            printf("[!] Error al generar '%s'\n", RUTA_PILOTO_BIN);
    }
    else
        printf("[!] Error al generar '%s'\n", RUTA_PILOTO_TXT);

    /* Escuderias: txt -> bin */
    if (generarArchivoEscuderiasTxt(RUTA_ESCUDERIA_TXT) == TODO_OK)
    {
        printf("[OK] Generado '%s'\n", RUTA_ESCUDERIA_TXT);

        if (convertirArchivoTxtABin(RUTA_ESCUDERIA_TXT, RUTA_ESCUDERIA_BIN, sizeof(Escuderia), trozarEscuderiaTxt) == TODO_OK)
            printf("[OK] Generado '%s'\n", RUTA_ESCUDERIA_BIN);
        else
            printf("[!] Error al generar '%s'\n", RUTA_ESCUDERIA_BIN);
    }
    else
        printf("[!] Error al generar '%s'\n", RUTA_ESCUDERIA_TXT);

    /* Puntos: carga tabla desde .txt (la genera si no existe) */
    if (cargarConfigPuntos(RUTA_PUNTOS_TXT, pts) == TODO_OK)
        printf("[OK] Tabla de puntos cargada desde '%s'\n", RUTA_PUNTOS_TXT);
    else
        printf("[!] Error al cargar la tabla de puntos\n");

    printf("----------------------------\n\n");
}

/**
 * mostrarMenu
 * Imprime el menu principal y lee la opcion del usuario con scanf.
 * Retorna el entero ingresado (0 = salir).
 */
static int mostrarMenu(void)
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
/* Submenú de estadisticas: calcula y muestra stats de un piloto o el 1ro/ultimo de una carrera */
static void menuEstadisticas(void)
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
        mostrarEstadisticasPiloto(&stats); // imprime el resumen calculado
        break;
    case 2:
        printf("Ingrese ID de la carrera: ");
        scanf("%d", &id_carrera);
        mejorYPeorPosicion(RUTA_CARRERA_BIN, id_carrera); // muestra el 1ro y ultimo de esa carrera
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
 * En ambos casos, al terminar recalcula los puntos de todos los
 * pilotos y muestra el historial de carreras actualizado.
 */
/* Submenú para registrar una carrera: permite elegir entre simulacion aleatoria o ingreso manual.
   Tras registrar, recalcula los puntos de todos los pilotos y muestra el historial. */
static void menuCarrera(Puntos* pts)
{
    int op;
    Carrera carrera;

    printf("\n  +---------------------------+\n");
    printf("  |     REGISTRAR CARRERA     |\n");
    printf("  |---------------------------|\n");
    printf("  |  1. Simulacion aleatoria  |\n");
    printf("  |  2. Ingreso manual        |\n");
    printf("  |  0. volver                |\n");
    printf("  +---------------------------+\n");
    printf("  Opcion: ");
    scanf("%d", &op);

    switch(op)
    {
    case 1:
        registrarCarreraAleatoria(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN, compararUnsigned, pts);
        recalcularPuntosPilotos(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN); // actualiza puntos tras la carrera
        mostrarArchivoBinario(RUTA_CARRERA_BIN, &carrera, sizeof(Carrera), mostrarCarrera);
        break;

    case 2:
        registrarCarreraManual(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN, compararUnsigned, pts);
        recalcularPuntosPilotos(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN);
        mostrarArchivoBinario(RUTA_CARRERA_BIN, &carrera, sizeof(Carrera), mostrarCarrera);
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
/* Lista todas las escuderias disponibles, pide al usuario que elija una y muestra sus pilotos */
static void menuPilotosPorEscuderia(void)
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
        mostrarEscuderia(&esc); // imprime cada escuderia del .bin
    fclose(fEsc);

    printf("\nIngrese ID de escuderia (0 para volver): ");
    scanf("%d", &idEsc);

    if (idEsc == 0)
        return;

    listarPilotosPorEscuderia(RUTA_PILOTO_BIN, RUTA_ESCUDERIA_BIN, (unsigned)idEsc);
}

/**
 * FinalizarTemporada
 * Cierra la temporada actual:
 *   1. Pide nombre para el archivo de pilotos y renombra pilotos.txt.
 *   2. Pide nombre para el archivo de carreras y renombra carreras.txt.
 *   3. Elimina los tres .bin para que la proxima ejecucion arranque limpia.
 * Usa aritmetica de punteros para concatenar la extension ".txt"
 * sin funciones de cadena de biblioteca.
 */
/* Cierra la temporada: renombra los .txt de pilotos y carreras con el nombre elegido
   y borra los .bin para que la proxima ejecucion arranque limpia */
static void finalizarTemporada(void)
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

    exportarPilotosTxt(RUTA_PILOTO_BIN, RUTA_PILOTO_EXP_TXT); /* genera el .txt desde el .bin */
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

    exportarCarrerasTxt(RUTA_CARRERA_BIN, RUTA_CARRERA_EXP_TXT); /* genera el .txt desde el .bin */
    if (rename(RUTA_CARRERA_EXP_TXT, nomarchivocar) == 0)
        printf("[OK] Archivo de carreras guardado como '%s'\n", nomarchivocar);
    else
        printf("[!] Error al guardar el archivo de carreras\n");

    /* Elimina los .bin para que la proxima temporada arranque limpia */
    remove(RUTA_CARRERA_BIN);
    remove(RUTA_PILOTO_BIN);
    remove(RUTA_ESCUDERIA_BIN);

    printf("\n[OK] Temporada finalizada.\n");
}

/**
 * menuGestionRegistros
 * Submenú de ABM. Delega cada opcion a la funcion correspondiente:
 *   1-3: alta/baja/modificar piloto
 *   4-6: alta/baja/modificar escuderia
 *   7-8: baja/modificar carrera
 */
static void menuGestionRegistros(void)
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
