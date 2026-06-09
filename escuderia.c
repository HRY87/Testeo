#include <stdio.h>
#include <string.h>
#include "escuderia.h"
#include "utilidades.h"


/* =========================================================
                    Lote de prueba inicial
   ========================================================= */

/**
 * generarArchivoEscuderiasTxt
 * Crea el archivo escuderia.txt con el lote inicial.
 * Usa generarArchivoTexto() + escribirEscuderiaTxt().
 * Retorna TODO_OK o ERR_ARCH.
 */
int generarArchivoEscuderiasTxt(const char* rutaTxt)
{
    Escuderia lote[6] = {
        {1, "RBR", "Red Bull Racing", "Austria",    ESTADO_ESCUDERIA_ACTIVA},
        {2, "MCL", "McLaren",         "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
        {3, "FER", "Ferrari",         "Italia",     ESTADO_ESCUDERIA_ACTIVA},
        {4, "WLF", "Williams",        "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
        {5, "AMR", "Aston Martin",    "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
        {6, "SAU", "Sauber",          "Suiza",      ESTADO_ESCUDERIA_ACTIVA}
    };

    return generarArchivoTexto(rutaTxt, lote, 6, sizeof(Escuderia), escribirEscuderiaTxt);
}

/* =========================================================
            ABM directo sobre archivo binario
   ========================================================= */

/*
 * buscarEscuderiaEnBin
 * Recorre el .bin hasta encontrar la escuderia con idBuscado.
 * Carga 'dest' y retorna el offset. Retorna -1L si no existe.
 */
long buscarEscuderiaEnBin(const char* rutaBin, unsigned idBuscado, Escuderia* dest)
{
    Escuderia e;
    long      offset;
    FILE*     fBin;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return -1L;

    while (1)
    {
        offset = ftell(fBin);
        if (fread(&e, sizeof(Escuderia), 1, fBin) != 1)
            break;

        if (e.id == idBuscado)
        {
            if (dest)
                *dest = e;
            fclose(fBin);
            return offset;
        }
    }

    fclose(fBin);
    return -1L;
}

/*
 * darBajaEscuderia
 * Pone estado = ESTADO_ESCUDERIA_INACTIVA.
 * Solo sobreescribe ese registro con fseek+fwrite.
 */
int darBajaEscuderia(const char* rutaBin, unsigned idEscuderia)
{
    Escuderia e;
    long      offset;
    FILE*     fBin;

    offset = buscarEscuderiaEnBin(rutaBin, idEscuderia, &e);
    if (offset < 0)
    {
        printf("[!] Escuderia ID %u no encontrada.\n", idEscuderia);
        return NO_ENCONTRADO;
    }

    if (e.estado == ESTADO_ESCUDERIA_INACTIVA)
    {
        printf("[!] La escuderia '%s' ya esta inactiva.\n", e.nombre);
        return ERR_LINEA;
    }

    e.estado = ESTADO_ESCUDERIA_INACTIVA;

    fBin = fopen(rutaBin, "r+b");
    if (!fBin)
        return ERR_ARCH;

    fseek(fBin, offset, SEEK_SET);
    fwrite(&e, sizeof(Escuderia), 1, fBin);
    fclose(fBin);

    printf("[OK] Escuderia '%s' dada de baja.\n", e.nombre);
    return TODO_OK;
}

/*
 * modificarEscuderia
 * Muestra el registro y permite editar campo por campo.
 * Solo sobreescribe ese registro en disco.
 */
int modificarEscuderia(const char* rutaBin, unsigned idEscuderia)
{
    Escuderia e;
    long      offset;
    FILE*     fBin;
    int       opcion;
    int       continuar;
    int       estadoNuevo;

    offset = buscarEscuderiaEnBin(rutaBin, idEscuderia, &e);
    if (offset < 0)
    {
        printf("[!] Escuderia ID %u no encontrada.\n", idEscuderia);
        return NO_ENCONTRADO;
    }

    continuar = 1;

    while (continuar)
    {
        printf("\n--- Modificar Escuderia ID %u ---\n", e.id);
        printf("  1. Codigo    [%s]\n",  e.codigo);
        printf("  2. Nombre    [%s]\n",  e.nombre);
        printf("  3. Pais      [%s]\n",  e.pais);
        printf("  4. Estado    [%s]\n",  e.estado == ESTADO_ESCUDERIA_ACTIVA
                                         ? "Activa" : "Inactiva");
        printf("  0. Confirmar y guardar\n");
        printf("Campo a modificar: ");
        scanf("%d", &opcion);
        limpiarBuffer();

        switch (opcion)
        {
        case 1:
            printf("Nuevo codigo (max 3 chars): ");
            leerCadena(e.codigo, TAM_CODIGO);
            break;

        case 2:
            printf("Nuevo nombre: ");
            leerCadena(e.nombre, TAM_NOMBRE_ESCUDERIA);
            break;

        case 3:
            printf("Nuevo pais: ");
            leerCadena(e.pais, TAM_PAIS);
            break;

        case 4:
            printf("Nuevo estado (1=Activa, 0=Inactiva): ");
            scanf("%d", &estadoNuevo);
            limpiarBuffer();
            if (estadoNuevo == ESTADO_ESCUDERIA_ACTIVA ||
                estadoNuevo == ESTADO_ESCUDERIA_INACTIVA)
            {
                e.estado = estadoNuevo;
            }
            else
                printf("[!] Estado invalido.\n");
            break;

        case 0:
            continuar = 0;
            break;

        default:
            printf("[!] Opcion invalida.\n");
            break;
        }
    }

    fBin = fopen(rutaBin, "r+b");
    if (!fBin)
        return ERR_ARCH;

    fseek(fBin, offset, SEEK_SET);
    fwrite(&e, sizeof(Escuderia), 1, fBin);
    fclose(fBin);

    printf("[OK] Escuderia '%s' modificada correctamente.\n", e.nombre);
    return TODO_OK;
}

/* =========================================================
            Punteros a funcion del TDA Escuderia
   ========================================================= */

/**
 * trozarEscuderiaTxt  [TxtABin]
 * Parsea una linea CSV con el formato:
 *   id,codigo,nombre,pais,estado
 * Retorna TODO_OK si se leyeron los 5 campos, ERR_LINEA si no.
 */
int trozarEscuderiaTxt(char* linea, void* reg)
{
    Escuderia* e   = (Escuderia*)reg;
    char*   act = strchr(linea, '\n');

    if(!act)
        return ERR_LINEA;

    *act = '\0';

    /* Estado */
    act = strrchr(linea, SEP_TXT);
    e->estado = *(act + 1);
    *act = '\0';

    /* Pais */
    act = strrchr(linea, SEP_TXT);
    copiarCadena(e->pais, act + 1, TAM_PAIS);
    *act = '\0';

    /* Nombre */
    act = strrchr(linea, SEP_TXT);
    copiarCadena(e->nombre, act + 1, TAM_NOMBRE_ESCUDERIA);
    *act = '\0';

    /* Codigo */
    act = strrchr(linea, SEP_TXT);
    copiarCadena(e->codigo, act + 1, TAM_CODIGO);
    *act = '\0';

    /* Id — lo que queda al inicio */
    sscanf(linea, "%u", &e->id);

    return TODO_OK;
}

/**
 * escuderiaBinATxt  [BinATxt]
 * Escribe una Escuderia en formato CSV en el archivo de texto.
 */
void escuderiaBinATxt(const void* dato, FILE* archTxt)
{
    const Escuderia* e = (const Escuderia*)dato;

    fprintf(archTxt, "%u%c%s%c%s%c%s%c%d\n",
            e->id,     SEP_TXT,
            e->codigo, SEP_TXT,
            e->nombre, SEP_TXT,
            e->pais,   SEP_TXT,
            e->estado);
}

/**
 * escribirEscuderiaTxt  [Accion]
 * Misma logica que escuderiaBinATxt pero con firma Accion.
 * Se usa con generarArchivoTexto() para el lote inicial.
 */
int escribirEscuderiaTxt(void* archTxt, const void* dato)
{
    escuderiaBinATxt(dato, (FILE*)archTxt);
    return TODO_OK;
}

/**
 * mostrarEscuderia  [Mostrar]
 * Imprime una Escuderia formateada por pantalla.
 */
void mostrarEscuderia(const void* dato)
{
    const Escuderia* e = (const Escuderia*)dato;

    printf("%-4u  [%s]  %-25s  %-15s  %s\n",
           e->id,
           e->codigo,
           e->nombre,
           e->pais,
           e->estado == ESTADO_ESCUDERIA_ACTIVA ? "Activa" : "Inactiva");
}

/**
 * esEscuderiaActiva  [Filter]
 * Retorna 1 si la escuderia tiene estado ACTIVA, 0 si no.
 */
int esEscuderiaActiva(const void* dato)
{
    return (((const Escuderia*)dato)->estado == ESTADO_ESCUDERIA_ACTIVA);
}

