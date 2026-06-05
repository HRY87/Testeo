#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "piloto.h"
#include "escuderia.h"
#include "carrera.h"
#include "puntos.h"

/* =========================================================
   Prototipos internos
   ========================================================= */
static void inicializarSistema(void);
static int  mostrarMenu(void);
static int  mostrarSubMenuCarrera(void);
static void editarTablaPuntos(ConfigPuntos* cfg);

/* =========================================================
   Punto de entrada
   ========================================================= */
int main(void)
{
    int          opcion;
    int          subOpcion;
    ConfigPuntos cfg;

    inicializarSistema();

    /* Cargar tabla de puntos una sola vez al inicio.
       Si el archivo no existe, cargarConfigPuntos genera
       los valores F1 por defecto y los persiste. */
    cargarConfigPuntos(RUTA_PUNTOS_BIN, &cfg);

    opcion = mostrarMenu();

    while (opcion != 0)
    {
        switch (opcion)
        {
        /* --- 1. Listar pilotos con puntos recalculados --- */
        case 1:
            recalcularPuntosPilotos(RUTA_CARRERA_BIN,
                                    RUTA_PILOTO_BIN,
                                    filterEsCarreraActiva);

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
                                              compararUnsigned,
                                              &cfg) == TODO_OK)
                {
                    actualizarPuntosUltimaCarrera(RUTA_CARRERA_BIN,
                                                  RUTA_PILOTO_BIN,
                                                  filterEsCarreraActiva);
                    printf("\n=== CARRERAS REGISTRADAS ===\n");
                    listarTodasLasCarreras(RUTA_CARRERA_BIN);
                }
            }
            else if (subOpcion == 2)
            {
                if (registrarCarreraManual(RUTA_CARRERA_BIN,
                                           RUTA_PILOTO_BIN,
                                           compararUnsigned,
                                           &cfg) == TODO_OK)
                {
                    actualizarPuntosUltimaCarrera(RUTA_CARRERA_BIN,
                                                  RUTA_PILOTO_BIN,
                                                  filterEsCarreraActiva);
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

        /* --- 5. Configurar tabla de puntos --- */
        case 5:
            editarTablaPuntos(&cfg);
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
    printf("|  5. Configurar tabla de puntos      |\n");
    printf("|  0. Salir                           |\n");
    printf("+=====================================+\n");
    printf("Opcion: ");
    scanf("%d", &opcion);
    limpiarBuffer();

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
    limpiarBuffer();

    return op;
}

/* =========================================================
   Edicion de la tabla de puntos
   ========================================================= */

/*
 * editarTablaPuntos
 * Permite al usuario ver la tabla actual, cambiar la cantidad
 * de posiciones con puntos y editar cada valor.
 * Persiste los cambios en RUTA_PUNTOS_BIN al confirmar.
 */
static void editarTablaPuntos(ConfigPuntos* cfg)
{
    int i;
    int nuevasCantPos;
    int nuevoPts;
    int confirmacion;
    int leido;

    printf("\n=== CONFIGURACION DE TABLA DE PUNTOS ===\n");
    mostrarConfigPuntos(cfg);

    printf("\nCantidad de posiciones con puntos (1-%d, actual: %d): ",
           MAX_POSICIONES_PUNTOS, cfg->posiciones);

    leido = scanf("%d", &nuevasCantPos);
    limpiarBuffer();

    if (leido != 1 || nuevasCantPos < 1 || nuevasCantPos > MAX_POSICIONES_PUNTOS)
    {
        printf("[!] Valor invalido. No se realizaron cambios.\n");
        return;
    }

    printf("\nIngrese los puntos para cada posicion (0 = sin puntos):\n");

    for (i = 0; i < nuevasCantPos; i++)
    {
        do
        {
            printf("  Posicion %2d: ", i + 1);
            leido = scanf("%d", &nuevoPts);
            limpiarBuffer();

            if (leido != 1 || nuevoPts < 0)
                printf("  [!] Valor invalido. Ingrese un numero >= 0.\n");

        } while (leido != 1 || nuevoPts < 0);

        cfg->tabla[i] = nuevoPts;
    }

    /* Limpiar posiciones que quedaron fuera del nuevo rango */
    for (i = nuevasCantPos; i < MAX_POSICIONES_PUNTOS; i++)
        cfg->tabla[i] = 0;

    cfg->posiciones = nuevasCantPos;

    printf("\nTabla nueva:\n");
    mostrarConfigPuntos(cfg);

    printf("Confirmar cambios? (1=Si / 0=No): ");
    leido = scanf("%d", &confirmacion);
    limpiarBuffer();

    if (leido == 1 && confirmacion == 1)
    {
        if (guardarConfigPuntos(RUTA_PUNTOS_BIN, cfg) == TODO_OK)
            printf("[OK] Tabla guardada en '%s'.\n\n", RUTA_PUNTOS_BIN);
        else
            printf("[!] Error al guardar la tabla.\n\n");
    }
    else
    {
        /* Recargar desde disco para descartar cambios en memoria */
        cargarConfigPuntos(RUTA_PUNTOS_BIN, cfg);
        printf("[i] Cambios descartados.\n\n");
    }
}
