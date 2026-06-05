#include <stdio.h>
#include <string.h>
#include "piloto.h"

/* =========================================================
   Lote inicial de prueba
   ========================================================= */

static const Piloto lote_pilotos[] =
{
    {1,  "Max Verstappen",  "Neerlandes",  1, 0, ESTADO_ACTIVO_PILOTO, 19970930ULL},
    {2,  "Lando Norris",    "Britanico",   2, 0, ESTADO_ACTIVO_PILOTO, 19991113ULL},
    {3,  "Charles Leclerc", "Monegasco",   3, 0, ESTADO_ACTIVO_PILOTO, 19971016ULL},
    {4,  "Oscar Piastri",   "Australiano", 2, 0, ESTADO_ACTIVO_PILOTO, 20010406ULL},
    {5,  "Carlos Sainz",    "Espanol",     4, 0, ESTADO_ACTIVO_PILOTO, 19940901ULL},
    {6,  "George Russell",  "Britanico",   1, 0, ESTADO_ACTIVO_PILOTO, 19980215ULL},
    {7,  "Lewis Hamilton",  "Britanico",   3, 0, ESTADO_ACTIVO_PILOTO, 19850107ULL},
    {8,  "Fernando Alonso", "Espanol",     5, 0, ESTADO_ACTIVO_PILOTO, 19810729ULL},
    {9,  "Lance Stroll",    "Canadiense",  5, 0, ESTADO_ACTIVO_PILOTO, 19981029ULL},
    {10, "Nico Hulkenberg",  "Aleman",     6, 0, ESTADO_ACTIVO_PILOTO, 19870819ULL}
};

#define CANT_PILOTOS_LOTE \
    (sizeof(lote_pilotos) / sizeof(lote_pilotos[0]))

/* =========================================================
   Generacion del archivo de texto inicial
   ========================================================= */

int generarArchivoPilotosTxt(const char* rutaTxt)
{
    return generarArchivoTexto(rutaTxt,
                               lote_pilotos,
                               CANT_PILOTOS_LOTE,
                               sizeof(Piloto),
                               escribirPilotoTxt);
}

/* =========================================================
   Operaciones con vector de pilotos
   ========================================================= */

/*
 * cargarVectorPilotoActivos
 * Carga en vIds (ordenado por id) solo los IDs de pilotos activos.
 */
int cargarVectorPilotoActivos(const char* rutaBin,
                              tVector*    vIds,
                              Comparar    comparar)
{
    Piloto p;
    FILE*  fBin;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    while (fread(&p, sizeof(Piloto), 1, fBin) == 1)
    {
        if (p.estado == ESTADO_ACTIVO_PILOTO)
            insertarVectorOrd(vIds, &p.id, comparar);
    }

    fclose(fBin);
    return TODO_OK;
}

/* =========================================================
   Helpers internos de ABM
   ========================================================= */

/*
 * generarIdPiloto
 * Busca el maximo ID en el archivo y retorna maximo+1.
 * Retorna 1 si el archivo esta vacio.
 */
static unsigned generarIdPiloto(const char* rutaBin)
{
    Piloto   p;
    FILE*    fBin;
    unsigned maxId = 0;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return 1;

    while (fread(&p, sizeof(Piloto), 1, fBin) == 1)
    {
        if (p.id > maxId)
            maxId = p.id;
    }

    fclose(fBin);
    return maxId + 1;
}

/*
 * buscarPilotoPorId
 * Carga el piloto con el ID dado en 'dest'.
 * Retorna TODO_OK o NO_ENCONTRADO.
 */
static int buscarPilotoPorId(const char* rutaBin,
                             unsigned    id,
                             Piloto*     dest)
{
    Piloto p;
    FILE*  fBin;
    int    encontrado = 0;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    while (!encontrado && fread(&p, sizeof(Piloto), 1, fBin) == 1)
    {
        if (p.id == id)
        {
            *dest     = p;
            encontrado = 1;
        }
    }

    fclose(fBin);
    return encontrado ? TODO_OK : NO_ENCONTRADO;
}

/*
 * pedirFechaNacimiento
 * Pide la fecha hasta que sea valida.
 */
static unsigned long long pedirFechaNacimiento(void)
{
    unsigned long long fecha;

    do
    {
        printf("  Fecha de nacimiento (AAAAMMDD): ");
        leerUnsignedLongLong(&fecha);

        if (!esFechaValida(fecha))
            printf("  [!] Fecha invalida. Use formato AAAAMMDD (ej: 19970930).\n");

    } while (!esFechaValida(fecha));

    return fecha;
}

/* =========================================================
   ABM
   ========================================================= */

int altaPiloto(const char* rutaBin)
{
    Piloto   nuevo;
    FILE*    fBin;
    unsigned idEsc;

    memset(&nuevo, 0, sizeof(Piloto));

    nuevo.id = generarIdPiloto(rutaBin);

    printf("\n--- ALTA DE PILOTO (ID asignado: %u) ---\n", nuevo.id);

    printf("  Nombre completo: ");
    leerCadena(nuevo.nombre, TAM_NOMBRE_PILOTO);

    printf("  Nacionalidad: ");
    leerCadena(nuevo.nacionalidad, TAM_NACIONALIDAD);

    printf("  ID de escuderia: ");
    leerUnsigned(&idEsc, 1, 9999);
    nuevo.id_escuderia = idEsc;

    nuevo.fechaNacimiento = pedirFechaNacimiento();

    /* Estado inicial siempre activo */
    nuevo.estado            = ESTADO_ACTIVO_PILOTO;
    nuevo.puntos_acumulados = 0;

    fBin = fopen(rutaBin, "ab");
    if (!fBin)
        return ERR_ARCH;

    fwrite(&nuevo, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    printf("  [OK] Piloto '%s' agregado con ID %u.\n",
           nuevo.nombre, nuevo.id);

    return TODO_OK;
}

int bajaPiloto(const char* rutaBin)
{
    unsigned id;
    Piloto   p;
    int      resp;

    printf("\n--- BAJA DE PILOTO ---\n");
    printf("  ID del piloto a dar de baja: ");
    leerUnsigned(&id, 1, 9999);

    resp = buscarPilotoPorId(rutaBin, id, &p);

    if (resp == NO_ENCONTRADO)
    {
        printf("  [!] No existe piloto con ID %u.\n", id);
        return NO_ENCONTRADO;
    }

    if (p.estado == ESTADO_RETIRADO_PILOTO)
    {
        printf("  [!] El piloto '%s' ya esta retirado.\n", p.nombre);
        return TODO_OK;
    }

    printf("  Piloto: %s | Estado actual: %c\n", p.nombre, p.estado);
    printf("  Confirmar baja logica (nuevo estado = R)? (1=Si / 0=No): ");

    {
        int confirm = 0;
        leerInt(&confirm, 0, 1);
        if (confirm == 0)
        {
            printf("  [i] Operacion cancelada.\n");
            return TODO_OK;
        }
    }

    resp = modificarRegistroEnBin(rutaBin,
                                  sizeof(Piloto),
                                  &id,
                                  compararUnsigned,
                                  pilotoObtenerClave,
                                  /* lambda inline via funcion estatica */
                                  NULL);

    /*
     * modificarRegistroEnBin necesita una ModificarReg.
     * Como el cambio es simple (un solo campo), usamos
     * una variable estatica auxiliar para pasarle el estado.
     * Patron: escribir directamente con r+b para el caso simple.
     */
    {
        FILE* fBin;
        Piloto tmp;
        long   posBaja = -1;

        fBin = fopen(rutaBin, "r+b");
        if (!fBin) return ERR_ARCH;

        while (fread(&tmp, sizeof(Piloto), 1, fBin) == 1)
        {
            if (tmp.id == id)
            {
                posBaja = ftell(fBin) - (long)sizeof(Piloto);
                tmp.estado = ESTADO_RETIRADO_PILOTO;
                fseek(fBin, posBaja, SEEK_SET);
                fwrite(&tmp, sizeof(Piloto), 1, fBin);
                break;
            }
        }

        fclose(fBin);

        if (posBaja < 0)
            return NO_ENCONTRADO;
    }

    printf("  [OK] Piloto '%s' dado de baja (estado R).\n", p.nombre);
    return TODO_OK;
}

int modificarPiloto(const char* rutaBin)
{
    unsigned id;
    Piloto   p;
    int      resp;
    int      campo;

    printf("\n--- MODIFICACION DE PILOTO ---\n");
    printf("  ID del piloto a modificar: ");
    leerUnsigned(&id, 1, 9999);

    resp = buscarPilotoPorId(rutaBin, id, &p);

    if (resp == NO_ENCONTRADO)
    {
        printf("  [!] No existe piloto con ID %u.\n", id);
        return NO_ENCONTRADO;
    }

    mostrarPiloto(&p);

    printf("\n  Que campo desea modificar?\n");
    printf("    1. Nombre\n");
    printf("    2. Nacionalidad\n");
    printf("    3. ID Escuderia\n");
    printf("    4. Estado (A/R/S)\n");
    printf("    5. Fecha de nacimiento\n");
    printf("    0. Cancelar\n");
    printf("  Opcion: ");
    leerInt(&campo, 0, 5);

    if (campo == 0)
    {
        printf("  [i] Operacion cancelada.\n");
        return TODO_OK;
    }

    switch (campo)
    {
    case 1:
        printf("  Nuevo nombre: ");
        leerCadena(p.nombre, TAM_NOMBRE_PILOTO);
        break;

    case 2:
        printf("  Nueva nacionalidad: ");
        leerCadena(p.nacionalidad, TAM_NACIONALIDAD);
        break;

    case 3:
        printf("  Nuevo ID de escuderia: ");
        leerUnsigned(&p.id_escuderia, 1, 9999);
        break;

    case 4:
        printf("  Nuevo estado (A=Activo / R=Retirado / S=Suspendido): ");
        leerChar(&p.estado, "ARS");
        break;

    case 5:
        p.fechaNacimiento = pedirFechaNacimiento();
        break;

    default:
        break;
    }

    /* Sobreescribir el registro en disco */
    {
        FILE* fBin;
        Piloto tmp;
        long   posReg = -1;

        fBin = fopen(rutaBin, "r+b");
        if (!fBin) return ERR_ARCH;

        while (fread(&tmp, sizeof(Piloto), 1, fBin) == 1)
        {
            if (tmp.id == id)
            {
                posReg = ftell(fBin) - (long)sizeof(Piloto);
                fseek(fBin, posReg, SEEK_SET);
                fwrite(&p, sizeof(Piloto), 1, fBin);
                break;
            }
        }

        fclose(fBin);

        if (posReg < 0)
            return NO_ENCONTRADO;
    }

    printf("  [OK] Piloto ID %u modificado correctamente.\n", id);
    return TODO_OK;
}

/* =========================================================
   Exportacion
   ========================================================= */

int exportarPilotosATxt(const char* rutaBin, const char* rutaTxt)
{
    return convertirArchivoBinATxt(rutaBin, rutaTxt,
                                   sizeof(Piloto),
                                   pilotoBinATxt);
}

/* =========================================================
   Punteros a funcion del TDA Piloto
   ========================================================= */

int trozarPilotoTxt(char* linea, void* reg)
{
    Piloto* p   = (Piloto*)reg;
    char*   act = strchr(linea, '\n');

    if(!act)
        return ERR_LINEA;

    *act = '\0';

    /* fechaNacimiento */
    act = strrchr(linea, SEP_TXT);
    sscanf(act + 1, "%llu", &p->fechaNacimiento);
    *act = '\0';

    /* estado */
    act = strrchr(linea, SEP_TXT);
    p->estado = *(act + 1);
    *act = '\0';

    /* puntos_acumulados */
    act = strrchr(linea, SEP_TXT);
    sscanf(act + 1, "%u", &p->puntos_acumulados);
    *act = '\0';

    /* id_escuderia */
    act = strrchr(linea, SEP_TXT);
    sscanf(act + 1, "%u", &p->id_escuderia);
    *act = '\0';

    /* nacionalidad */
    act = strrchr(linea, SEP_TXT);
    copiarCadena(p->nacionalidad, act + 1, TAM_NACIONALIDAD);
    *act = '\0';

    /* nombre */
    act = strrchr(linea, SEP_TXT);
    copiarCadena(p->nombre, act + 1, TAM_NOMBRE_PILOTO);
    *act = '\0';

    /* id — lo que queda al inicio */
    sscanf(linea, "%u", &p->id);

    return TODO_OK;
}

void pilotoBinATxt(const void* dato, FILE* archTxt)
{
    const Piloto* p = (const Piloto*)dato;

    fprintf(archTxt, "%u%c%s%c%s%c%u%c%u%c%c%c%llu\n",
            p->id,                SEP_TXT,
            p->nombre,            SEP_TXT,
            p->nacionalidad,      SEP_TXT,
            p->id_escuderia,      SEP_TXT,
            p->puntos_acumulados, SEP_TXT,
            p->estado,            SEP_TXT,
            p->fechaNacimiento);
}

void mostrarPiloto(const void* dato)
{
    const Piloto* p    = (const Piloto*)dato;
    unsigned      dia  = (unsigned)(p->fechaNacimiento % 100);
    unsigned      mes  = (unsigned)(p->fechaNacimiento / 100 % 100);
    unsigned      anio = (unsigned)(p->fechaNacimiento / 10000);

    printf("----------------------------------\n");
    printf("ID          : %u\n",             p->id);
    printf("Nombre      : %s\n",             p->nombre);
    printf("Nacionalidad: %s\n",             p->nacionalidad);
    printf("Escuderia   : %u\n",             p->id_escuderia);
    printf("Puntos      : %u\n",             p->puntos_acumulados);
    printf("Estado      : %c\n",             p->estado);
    printf("Nacimiento  : %02u/%02u/%04u\n", dia, mes, anio);
}

int escribirPilotoTxt(void* archTxt, const void* dato)
{
    pilotoBinATxt(dato, (FILE*)archTxt);
    return TODO_OK;
}

int esPilotoActivo(const void* dato)
{
    return (((const Piloto*)dato)->estado == ESTADO_ACTIVO_PILOTO);
}

int sumarPuntos(void* acumulador, const void* dato)
{
    *(unsigned*)acumulador += ((const Piloto*)dato)->puntos_acumulados;
    return TODO_OK;
}

int extraerIdPuntos(void* dest, const void* orig)
{
    unsigned* d = (unsigned*)dest;
    const Piloto* p = (const Piloto*)orig;
    d[0] = p->id;
    d[1] = p->puntos_acumulados;
    return TODO_OK;
}

void pilotoObtenerClave(const void* registro, void* claveDestino)
{
    *(unsigned*)claveDestino = ((const Piloto*)registro)->id;
}
