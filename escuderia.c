#include <stdio.h>
#include <string.h>
#include "escuderia.h"

/* =========================================================
   Lote inicial de prueba
   ========================================================= */

static const Escuderia lote_escuderias[] =
{
    {1, "RBR", "Red Bull Racing", "Austria",    ESTADO_ESCUDERIA_ACTIVA},
    {2, "MCL", "McLaren",         "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
    {3, "FER", "Ferrari",         "Italia",      ESTADO_ESCUDERIA_ACTIVA},
    {4, "WLF", "Williams",        "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
    {5, "AMR", "Aston Martin",    "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
    {6, "SAU", "Sauber",          "Suiza",       ESTADO_ESCUDERIA_ACTIVA}
};

#define CANT_ESCUDERIAS_LOTE \
    (sizeof(lote_escuderias) / sizeof(lote_escuderias[0]))

/* =========================================================
   Generacion del archivo de texto inicial
   ========================================================= */

int generarArchivoEscuderiasTxt(const char* rutaTxt)
{
    return generarArchivoTexto(rutaTxt,
                               lote_escuderias,
                               CANT_ESCUDERIAS_LOTE,
                               sizeof(Escuderia),
                               escribirEscuderiaTxt);
}

/* =========================================================
   Helpers internos
   ========================================================= */

static unsigned generarIdEscuderia(const char* rutaBin)
{
    Escuderia e;
    FILE*     fBin;
    unsigned  maxId = 0;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return 1;

    while (fread(&e, sizeof(Escuderia), 1, fBin) == 1)
    {
        if (e.id > maxId)
            maxId = e.id;
    }

    fclose(fBin);
    return maxId + 1;
}

static int buscarEscuderiaPorId(const char* rutaBin,
                                unsigned    id,
                                Escuderia*  dest)
{
    Escuderia e;
    FILE*     fBin;
    int       encontrado = 0;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    while (!encontrado && fread(&e, sizeof(Escuderia), 1, fBin) == 1)
    {
        if (e.id == id)
        {
            *dest      = e;
            encontrado = 1;
        }
    }

    fclose(fBin);
    return encontrado ? TODO_OK : NO_ENCONTRADO;
}

/* =========================================================
   ABM
   ========================================================= */

int altaEscuderia(const char* rutaBin)
{
    Escuderia nueva;
    FILE*     fBin;

    memset(&nueva, 0, sizeof(Escuderia));

    nueva.id = generarIdEscuderia(rutaBin);

    printf("\n--- ALTA DE ESCUDERIA (ID asignado: %u) ---\n", nueva.id);

    printf("  Codigo (max 3 chars): ");
    leerCadena(nueva.codigo, TAM_CODIGO);

    printf("  Nombre: ");
    leerCadena(nueva.nombre, TAM_NOMBRE_ESCUDERIA);

    printf("  Pais: ");
    leerCadena(nueva.pais, TAM_PAIS);

    nueva.estado = ESTADO_ESCUDERIA_ACTIVA;

    fBin = fopen(rutaBin, "ab");
    if (!fBin)
        return ERR_ARCH;

    fwrite(&nueva, sizeof(Escuderia), 1, fBin);
    fclose(fBin);

    printf("  [OK] Escuderia '%s' agregada con ID %u.\n",
           nueva.nombre, nueva.id);

    return TODO_OK;
}

int bajaEscuderia(const char* rutaBin)
{
    unsigned  id;
    Escuderia e;
    int       resp;

    printf("\n--- BAJA DE ESCUDERIA ---\n");
    printf("  ID de la escuderia a dar de baja: ");
    leerUnsigned(&id, 1, 9999);

    resp = buscarEscuderiaPorId(rutaBin, id, &e);

    if (resp == NO_ENCONTRADO)
    {
        printf("  [!] No existe escuderia con ID %u.\n", id);
        return NO_ENCONTRADO;
    }

    if (e.estado == ESTADO_ESCUDERIA_INACTIVA)
    {
        printf("  [!] La escuderia '%s' ya esta inactiva.\n", e.nombre);
        return TODO_OK;
    }

    printf("  Escuderia: %s | Estado actual: %s\n",
           e.nombre,
           e.estado == ESTADO_ESCUDERIA_ACTIVA ? "Activa" : "Inactiva");
    printf("  Confirmar baja logica? (1=Si / 0=No): ");

    {
        int confirm = 0;
        leerInt(&confirm, 0, 1);
        if (confirm == 0)
        {
            printf("  [i] Operacion cancelada.\n");
            return TODO_OK;
        }
    }

    {
        FILE*     fBin;
        Escuderia tmp;
        long      posBaja = -1;

        fBin = fopen(rutaBin, "r+b");
        if (!fBin) return ERR_ARCH;

        while (fread(&tmp, sizeof(Escuderia), 1, fBin) == 1)
        {
            if (tmp.id == id)
            {
                posBaja    = ftell(fBin) - (long)sizeof(Escuderia);
                tmp.estado = ESTADO_ESCUDERIA_INACTIVA;
                fseek(fBin, posBaja, SEEK_SET);
                fwrite(&tmp, sizeof(Escuderia), 1, fBin);
                break;
            }
        }

        fclose(fBin);

        if (posBaja < 0)
            return NO_ENCONTRADO;
    }

    printf("  [OK] Escuderia '%s' dada de baja.\n", e.nombre);
    return TODO_OK;
}

int modificarEscuderia(const char* rutaBin)
{
    unsigned  id;
    Escuderia e;
    int       resp;
    int       campo;

    printf("\n--- MODIFICACION DE ESCUDERIA ---\n");
    printf("  ID de la escuderia a modificar: ");
    leerUnsigned(&id, 1, 9999);

    resp = buscarEscuderiaPorId(rutaBin, id, &e);

    if (resp == NO_ENCONTRADO)
    {
        printf("  [!] No existe escuderia con ID %u.\n", id);
        return NO_ENCONTRADO;
    }

    mostrarEscuderia(&e);

    printf("\n  Que campo desea modificar?\n");
    printf("    1. Codigo\n");
    printf("    2. Nombre\n");
    printf("    3. Pais\n");
    printf("    4. Estado (activar/desactivar)\n");
    printf("    0. Cancelar\n");
    printf("  Opcion: ");
    leerInt(&campo, 0, 4);

    if (campo == 0)
    {
        printf("  [i] Operacion cancelada.\n");
        return TODO_OK;
    }

    switch (campo)
    {
    case 1:
        printf("  Nuevo codigo (max 3 chars): ");
        leerCadena(e.codigo, TAM_CODIGO);
        break;

    case 2:
        printf("  Nuevo nombre: ");
        leerCadena(e.nombre, TAM_NOMBRE_ESCUDERIA);
        break;

    case 3:
        printf("  Nuevo pais: ");
        leerCadena(e.pais, TAM_PAIS);
        break;

    case 4:
        printf("  Nuevo estado (1=Activa / 0=Inactiva): ");
        leerInt(&e.estado, 0, 1);
        break;

    default:
        break;
    }

    {
        FILE*     fBin;
        Escuderia tmp;
        long      posReg = -1;

        fBin = fopen(rutaBin, "r+b");
        if (!fBin) return ERR_ARCH;

        while (fread(&tmp, sizeof(Escuderia), 1, fBin) == 1)
        {
            if (tmp.id == id)
            {
                posReg = ftell(fBin) - (long)sizeof(Escuderia);
                fseek(fBin, posReg, SEEK_SET);
                fwrite(&e, sizeof(Escuderia), 1, fBin);
                break;
            }
        }

        fclose(fBin);

        if (posReg < 0)
            return NO_ENCONTRADO;
    }

    printf("  [OK] Escuderia ID %u modificada correctamente.\n", id);
    return TODO_OK;
}

/* =========================================================
   Exportacion
   ========================================================= */

int exportarEscuderiasATxt(const char* rutaBin, const char* rutaTxt)
{
    return convertirArchivoBinATxt(rutaBin, rutaTxt,
                                   sizeof(Escuderia),
                                   escuderiaBinATxt);
}

/* =========================================================
   Punteros a funcion del TDA Escuderia
   ========================================================= */

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

int escribirEscuderiaTxt(void* archTxt, const void* dato)
{
    escuderiaBinATxt(dato, (FILE*)archTxt);
    return TODO_OK;
}

void mostrarEscuderia(const void* dato)
{
    const Escuderia* e = (const Escuderia*)dato;

    printf("----------------------------------\n");
    printf("ID          : %u\n",  e->id);
    printf("Codigo      : %s\n",  e->codigo);
    printf("Nombre      : %s\n",  e->nombre);
    printf("Pais        : %s\n",  e->pais);
    printf("Estado      : %s\n",  e->estado == ESTADO_ESCUDERIA_ACTIVA
           ? "Activa" : "Inactiva");
}

int esEscuderiaActiva(const void* dato)
{
    return (((const Escuderia*)dato)->estado == ESTADO_ESCUDERIA_ACTIVA);
}

void escuderiaObtenerClave(const void* registro, void* claveDestino)
{
    *(unsigned*)claveDestino = ((const Escuderia*)registro)->id;
}
