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
static int  mostrarSubMenuABM(const char* entidad);
static void menuABMPiloto(void);
static void menuABMEscuderia(void);
static void menuABMCarrera(void);
static void editarTablaPuntos(Puntos* vPuntos);
/* =========================================================
   Punto de entrada
   ========================================================= */
int main(void)
{
    int opcion;
    int subOpcion;
    Puntos vPuntos;

    inicializarSistema();

    /* Cargar tabla de puntos una sola vez al inicio.
       Si el archivo no existe, cargarConfigPuntos genera
       los valores F1 por defecto y los persiste. */
    cargarConfigPuntos(RUTA_PUNTOS_BIN, &vPuntos);
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
                                              compararUnsigned, &vPuntos) == TODO_OK)
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
                                           compararUnsigned, &vPuntos) == TODO_OK)
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

        /* --- 5. ABM Pilotos --- */
        case 5:
            menuABMPiloto();
            break;

        /* --- 6. ABM Escuderias --- */
        case 6:
            menuABMEscuderia();
            break;

        /* --- 7. ABM Carreras --- */
        case 7:
            menuABMCarrera();
            break;

        /* --- 8. Configurar tabla de puntos --- */
        case 8:
            editarTablaPuntos(&vPuntos);
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

    if (generarArchivoPilotosTxt(RUTA_PILOTO_TXT) == TODO_OK)
    {
        printf("[OK] Generado '%s'\n", RUTA_PILOTO_TXT);

        if (convertirArchivoTxtABin(RUTA_PILOTO_TXT,
                                    RUTA_PILOTO_BIN,
                                    sizeof(Piloto),
                                    trozarPilotoTxt) == TODO_OK)
            printf("[OK] Generado '%s'\n", RUTA_PILOTO_BIN);
    }

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
   Menus principales
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
    printf("|-------------------------------------|\n");
    printf("|  5. ABM Pilotos                     |\n");
    printf("|  6. ABM Escuderias                  |\n");
    printf("|  7. ABM Carreras                    |\n");
    printf("|  8. Configurar tabla de puntos      |\n");
    printf("|-------------------------------------|\n");
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

/*
 * mostrarSubMenuABM
 * Menu generico de alta/baja/modificacion.
 * El alta no aparece aqui: para piloto ya se genera desde txt,
 * para escuderia idem. El alta de carrera esta en opcion 2.
 */
static int mostrarSubMenuABM(const char* entidad)
{
    int op;

    printf("\n  +-----------------------------+\n");
    printf("  |  ABM %-22s|\n", entidad);
    printf("  |-----------------------------|\n");
    printf("  |  1. Baja (inactivar)        |\n");
    printf("  |  2. Modificar               |\n");
    printf("  |  0. Volver                  |\n");
    printf("  +-----------------------------+\n");
    printf("  Opcion: ");
    scanf("%d", &op);
    limpiarBuffer();

    return op;
}

/* =========================================================
   ABM Pilotos
   ========================================================= */
static void menuABMPiloto(void)
{
    int      opcion;
    unsigned idPiloto;

    /* Mostrar pilotos actuales para referencia */
    printf("\n=== PILOTOS ACTUALES ===\n");
    mostrarArchivoBinario(RUTA_PILOTO_BIN, sizeof(Piloto), mostrarPiloto);

    opcion = mostrarSubMenuABM("Pilotos");

    switch (opcion)
    {
    case 1:
        printf("ID del piloto a dar de baja: ");
        scanf("%u", &idPiloto);
        limpiarBuffer();
        darBajaPiloto(RUTA_PILOTO_BIN, RUTA_CARRERA_BIN, idPiloto);

        /* Recalcular puntos automaticamente tras la baja */
        printf("[..] Recalculando puntos de la temporada...\n");
        recalcularPuntosPilotos(RUTA_CARRERA_BIN,
                                RUTA_PILOTO_BIN,
                                filterEsCarreraActiva,
                                reduceAcumularPuntosCarrera);
        printf("[OK] Puntos actualizados.\n");

        printf("\n=== PILOTOS ACTUALIZADOS ===\n");
        mostrarArchivoBinario(RUTA_PILOTO_BIN, sizeof(Piloto), mostrarPiloto);
        break;

    case 2:
        printf("ID del piloto a modificar: ");
        scanf("%u", &idPiloto);
        limpiarBuffer();
        modificarPiloto(RUTA_PILOTO_BIN, idPiloto);

        printf("\n=== PILOTOS ACTUALIZADOS ===\n");
        mostrarArchivoBinario(RUTA_PILOTO_BIN, sizeof(Piloto), mostrarPiloto);
        break;

    case 0:
        break;

    default:
        printf("[!] Opcion invalida.\n");
        break;
    }
}

/* =========================================================
   ABM Escuderias
   ========================================================= */
static void menuABMEscuderia(void)
{
    int      opcion;
    unsigned idEscuderia;

    printf("\n=== ESCUDERIAS ACTUALES ===\n");
    mostrarArchivoBinario(RUTA_ESCUDERIA_BIN, sizeof(Escuderia), mostrarEscuderia);

    opcion = mostrarSubMenuABM("Escuderias");

    switch (opcion)
    {
    case 1:
        printf("ID de la escuderia a dar de baja: ");
        scanf("%u", &idEscuderia);
        limpiarBuffer();
        darBajaEscuderia(RUTA_ESCUDERIA_BIN, idEscuderia);

        printf("\n=== ESCUDERIAS ACTUALIZADAS ===\n");
        mostrarArchivoBinario(RUTA_ESCUDERIA_BIN, sizeof(Escuderia), mostrarEscuderia);
        break;

    case 2:
        printf("ID de la escuderia a modificar: ");
        scanf("%u", &idEscuderia);
        limpiarBuffer();
        modificarEscuderia(RUTA_ESCUDERIA_BIN, idEscuderia);

        printf("\n=== ESCUDERIAS ACTUALIZADAS ===\n");
        mostrarArchivoBinario(RUTA_ESCUDERIA_BIN, sizeof(Escuderia), mostrarEscuderia);
        break;

    case 0:
        break;

    default:
        printf("[!] Opcion invalida.\n");
        break;
    }
}

/* =========================================================
   ABM Carreras
   ========================================================= */
static void menuABMCarrera(void)
{
    int opcion;
    int idCarrera;

    printf("\n=== CARRERAS ACTUALES ===\n");
    listarTodasLasCarreras(RUTA_CARRERA_BIN);

    opcion = mostrarSubMenuABM("Carreras");

    switch (opcion)
    {
    case 1:
        printf("ID de la carrera a dar de baja: ");
        scanf("%d", &idCarrera);
        limpiarBuffer();
        /* darBajaCarrera ya recalcula puntos internamente */
        darBajaCarrera(RUTA_CARRERA_BIN, RUTA_PILOTO_BIN, idCarrera);

        printf("\n=== CARRERAS ACTUALIZADAS ===\n");
        listarTodasLasCarreras(RUTA_CARRERA_BIN);
        break;

    case 2:
        printf("ID de la carrera a modificar: ");
        scanf("%d", &idCarrera);
        limpiarBuffer();
        modificarCarrera(RUTA_CARRERA_BIN, idCarrera);

        printf("\n=== CARRERAS ACTUALIZADAS ===\n");
        listarTodasLasCarreras(RUTA_CARRERA_BIN);
        break;

    case 0:
        break;

    default:
        printf("[!] Opcion invalida.\n");
        break;
    }
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
static void editarTablaPuntos(Puntos* vPuntos)
{
    int i;
    int nuevasCantPos;
    int nuevoPts;
    int confirmacion;
    int leido;

    printf("\n=== CONFIGURACION DE TABLA DE PUNTOS ===\n");
    mostrarConfigPuntos(vPuntos);

    printf("\nCantidad de posiciones con puntos (1-%d, actual: %d): ",
           MAX_POSICIONES_PUNTOS, vPuntos->posiciones);

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

        vPuntos->tabla[i] = nuevoPts;
    }

    /* Limpiar posiciones que quedaron fuera del nuevo rango */
    for (i = nuevasCantPos; i < MAX_POSICIONES_PUNTOS; i++)
        vPuntos->tabla[i] = 0;

    vPuntos->posiciones = nuevasCantPos;

    printf("\nTabla nueva:\n");
    mostrarConfigPuntos(vPuntos);

    printf("Confirmar cambios? (1=Si / 0=No): ");
    leido = scanf("%d", &confirmacion);
    limpiarBuffer();

    if (leido == 1 && confirmacion == 1)
    {
        if (guardarConfigPuntos(RUTA_PUNTOS_BIN, vPuntos) == TODO_OK)
            printf("[OK] Tabla guardada en '%s'.\n\n", RUTA_PUNTOS_BIN);
        else
            printf("[!] Error al guardar la tabla.\n\n");
    }
    else
    {
        /* Recargar desde disco para descartar cambios en memoria */
        cargarConfigPuntos(RUTA_PUNTOS_BIN, vPuntos);
        printf("[i] Cambios descartados.\n\n");
    }
}

