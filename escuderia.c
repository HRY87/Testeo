#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <string.h>
#include "escuderia.h"
#include "utilidades.h"
/* =========================================================
   Funciones estaticas auxiliares
   ========================================================= */

static void pedirDatosEscuderia  (Escuderia* e);
static void mostrarCamposEscuderia(const Escuderia* e);

/**
 * pedirDatosEscuderia  [static]
 * Solicita codigo, nombre y pais. Llama a limpiarBuffer() antes de
 * cada leerCadena() para descartar residuos de scanf.
 */
static void pedirDatosEscuderia(Escuderia* e)
{
    limpiarBuffer();

    printf("Codigo (max 3 letras): ");
    leerCadena(e->codigo, TAM_CODIGO);

    limpiarBuffer();
    printf("Nombre: ");
    leerCadena(e->nombre, TAM_NOMBRE_ESCUDERIA);

    printf("Pais: ");
    leerCadena(e->pais, TAM_PAIS);
}

/**
 * mostrarCamposEscuderia  [static]
 * Muestra los campos editables numerados para que el usuario
 * elija cual modificar. Estado se traduce a texto.
 */
static void mostrarCamposEscuderia(const Escuderia* e)
{
    printf("\n  Datos actuales de la escuderia ID %u:\n", e->id);
    printf("  [1] Codigo : %s\n", e->codigo);
    printf("  [2] Nombre : %s\n", e->nombre);
    printf("  [3] Pais   : %s\n", e->pais);
    printf("  [4] Estado : %s\n", (e->estado == ESTADO_ESCUDERIA_ACTIVA ? "Activo" : "Inactivo"));
    printf("  [0] Salir\n");
}

/* =========================================================
   Generacion del lote inicial
   ========================================================= */

/**
 * generarArchivoEscuderiasTxt
 * Crea escuderias.txt con el lote inicial usando generarArchivoTexto()
 * y escribirEscuderiaTxt() como Accion.
 * Retorna TODO_OK o ERR_ARCH.
 */
int generarArchivoEscuderiasTxt(const char* rutaTxt)
{
    Escuderia lote[6] =
    {
        {1, "RBR", "Red Bull Racing", "Austria",     ESTADO_ESCUDERIA_ACTIVA},
        {2, "MCL", "McLaren",         "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
        {3, "FER", "Ferrari",         "Italia",      ESTADO_ESCUDERIA_ACTIVA},
        {4, "WLF", "Williams",        "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
        {5, "AMR", "Aston Martin",    "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
        {6, "SAU", "Sauber",          "Suiza",       ESTADO_ESCUDERIA_ACTIVA}
    };

    return generarArchivoTexto(rutaTxt, lote, 6, sizeof(Escuderia), escribirEscuderiaTxt);
}

/* =========================================================
   Punteros a funcion del TDA Escuderia
   ========================================================= */

/**
 * trozarEscuderiaTxt  [TxtABin]
 * Parsea una linea con formato id|codigo|nombre|pais|estado de
 * atras hacia adelante con strrchr, igual que el resto de los TDA.
 * Retorna TODO_OK o ERR_LINEA.
 */
int trozarEscuderiaTxt(char* linea, void* reg)
{
    Escuderia* e   = (Escuderia*)reg;
    char*      act = strchr(linea, '\n');

    if (!act)
        return ERR_LINEA;

    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    sscanf(act + 1, "%d", &e->estado);
    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    copiarCadena(e->pais, act + 1, TAM_PAIS);
    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    copiarCadena(e->nombre, act + 1, TAM_NOMBRE_ESCUDERIA);
    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    copiarCadena(e->codigo, act + 1, TAM_CODIGO);
    *act = '\0';

    sscanf(linea, "%u", &e->id);

    return TODO_OK;
}

/**
 * escuderiaBinATxt  [BinATxt]
 * Escribe una Escuderia en formato texto con separador SEP_TXT.
 * Mismo formato que trozarEscuderiaTxt espera.
 * Retorna TODO_OK o ERR_ARCH.
 */
int escuderiaBinATxt(const void* dato, FILE* archTxt)
{
    const Escuderia* e = (const Escuderia*)dato;

    if (!dato || !archTxt)
        return ERR_ARCHIVO;

    fprintf(archTxt, "%u%c%s%c%s%c%s%c%d\n",
            e->id,     SEP_TXT,
            e->codigo, SEP_TXT,
            e->nombre, SEP_TXT,
            e->pais,   SEP_TXT,
            e->estado);

    return TODO_OK;
}

/**
 * escribirEscuderiaTxt  [Accion]
 * Misma logica que escuderiaBinATxt pero con firma Accion.
 * Se usa con generarArchivoTexto() para el lote inicial.
 * Retorna TODO_OK o ERR_ARCH.
 */
int escribirEscuderiaTxt(void* archTxt, const void* dato)
{
    return escuderiaBinATxt(dato, (FILE*)archTxt);
}

/**
 * mostrarEscuderia  [Mostrar]
 * Imprime una Escuderia formateada por pantalla.
 */
void mostrarEscuderia(const void* dato)
{
    const Escuderia* e = (const Escuderia*)dato;

    printf("%-4u  [%s]  %-25s  %-15s  %s\n",
           e->id, e->codigo, e->nombre, e->pais,
           e->estado == ESTADO_ESCUDERIA_ACTIVA ? "Activa" : "Inactiva");
}

/**
 * esEscuderiaActiva  [Filter]
 * Retorna 1 si estado == ESTADO_ESCUDERIA_ACTIVA, 0 si no.
 */
int esEscuderiaActiva(const void* dato)
{
    return (((const Escuderia*)dato)->estado == ESTADO_ESCUDERIA_ACTIVA);
}

/* =========================================================
   ABM
   ========================================================= */

/**
 * altaEscuderia
 * Pide ID y datos, asigna estado Activo y agrega al final del .bin.
 * Retorna TODO_OK o ERR_ARCH.
 */
int altaEscuderia(const char* rutaBin)
{
    Escuderia nueva;
    FILE*     fBin;

    printf("Ingresar ID: ");
    scanf("%u", &nueva.id);
    nueva.estado = ESTADO_ESCUDERIA_ACTIVA;

    printf("\n--- ALTA DE ESCUDERIA (ID asignado: %u) ---\n", nueva.id);
    pedirDatosEscuderia(&nueva);

    fBin = fopen(rutaBin, "ab");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de escuderias.\n");
        return ERR_ARCHIVO;
    }

    fwrite(&nueva, sizeof(Escuderia), 1, fBin);
    fclose(fBin);

    printf("[OK] Escuderia '%s' registrada con ID %u.\n", nueva.nombre, nueva.id);
    return TODO_OK;
}

/**
 * bajaEscuderia
 * Busca la escuderia por ID. Si esta activa y el usuario confirma,
 * la marca como inactiva y sobreescribe el registro.
 * Registra la baja en el archivo de texto de bajas.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int bajaEscuderia(const char* rutaBin, const char* rutaBajasTxt)
{
    Escuderia escuderia;
    FILE*     fBin;
    FILE*     fBajas;
    long      offset;
    unsigned  id;
    int       confirmacion;

    fBin = fopen(rutaBin, "rb+");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de escuderias.\n");
        return ERR_ARCHIVO;
    }

    printf("\n--- BAJA DE ESCUDERIA ---\n");
    printf("ID de la escuderia: ");
    scanf("%u", &id);

    offset = buscarRegistroPorId(fBin, &id, &escuderia, sizeof(Escuderia), sizeof(unsigned));

    if (offset == -1L)
    {
        fclose(fBin);
        printf("[!] Escuderia con ID %u no encontrada.\n", id);
        return ERR_NO_ENCONTRADO;
    }

    if (escuderia.estado == ESTADO_ESCUDERIA_INACTIVA)
    {
        fclose(fBin);
        printf("[!] La escuderia '%s' ya se encuentra inactiva.\n", escuderia.nombre);
        return TODO_OK;
    }

    printf("  Escuderia encontrada: %s (%s)\n", escuderia.nombre, escuderia.codigo);
    printf("  Confirma la baja? [1] Si  [0] No: ");
    scanf("%d", &confirmacion);

    if (confirmacion != 1)
    {
        fclose(fBin);
        printf("[!] Baja cancelada.\n");
        return TODO_OK;
    }

    escuderia.estado = ESTADO_ESCUDERIA_INACTIVA;

    fseek(fBin, offset, SEEK_SET);
    fwrite(&escuderia, sizeof(Escuderia), 1, fBin);
    fclose(fBin);

    fBajas = fopen(rutaBajasTxt, "at");
    if (fBajas)
    {
        fprintf(fBajas, "ESCUDERIA|%u|%s|%s|INACTIVA\n",
                escuderia.id, escuderia.codigo, escuderia.nombre);
        fclose(fBajas);
    }

    printf("[OK] Escuderia '%s' dada de baja.\n", escuderia.nombre);
    return TODO_OK;
}

/**
 * modificarEscuderia
 * Busca la escuderia por ID, presenta un submenu de campos y repite
 * hasta que el usuario elija 0. Sobreescribe el registro al finalizar.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int modificarEscuderia(const char* rutaBin)
{
    Escuderia escuderia;
    FILE*     fBin;
    long      offset;
    unsigned  id;
    int       campo;

    fBin = fopen(rutaBin, "rb+");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de escuderias.\n");
        return ERR_ARCHIVO;
    }

    printf("\n--- MODIFICAR ESCUDERIA ---\n");
    printf("ID de la escuderia: ");
    scanf("%u", &id);

    offset = buscarRegistroPorId(fBin, &id, &escuderia, sizeof(Escuderia), sizeof(unsigned));

    if (offset == -1L)
    {
        fclose(fBin);
        printf("[!] Escuderia con ID %u no encontrada.\n", id);
        return ERR_NO_ENCONTRADO;
    }

    mostrarCamposEscuderia(&escuderia);
    printf("Campo a modificar: ");
    scanf("%d", &campo);

    while (campo != 0)
    {
        limpiarBuffer();

        switch (campo)
        {
        case 1:
            printf("Nuevo codigo (max 3 letras): ");
            leerCadena(escuderia.codigo, TAM_CODIGO);
            break;

        case 2:
            printf("Nuevo nombre: ");
            leerCadena(escuderia.nombre, TAM_NOMBRE_ESCUDERIA);
            break;

        case 3:
            printf("Nuevo pais: ");
            leerCadena(escuderia.pais, TAM_PAIS);
            break;

        case 4:
            printf("Nuevo Estado [1/0]: ");
            scanf("%d", &escuderia.estado);
            break;

        default:
            printf("[!] Campo invalido.\n");
            break;
        }

        mostrarCamposEscuderia(&escuderia);
        printf("Campo a modificar: ");
        scanf("%d", &campo);
    }

    fseek(fBin, offset, SEEK_SET);
    fwrite(&escuderia, sizeof(Escuderia), 1, fBin);
    fclose(fBin);

    printf("[OK] Escuderia ID %u modificada correctamente.\n", escuderia.id);
    return TODO_OK;
}

/**
 * esEscuderiaValida
 * Busca el id en el .bin. Retorna 1 si existe, 0 si no.
 */
int esEscuderiaValida(const char* rutaBin, unsigned id)
{
    Escuderia esc;
    FILE*     fBin;
    long      offset;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return 0;

    offset = buscarRegistroPorId(fBin, &id, &esc, sizeof(Escuderia), sizeof(unsigned));
    fclose(fBin);

    return (offset != -1L);
}
