#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "piloto.h"
#include "escuderia.h"
#include "utilidades.h"
#include "indice.h"

/* =========================================================
   Funciones estaticas auxiliares
   ========================================================= */

static void pedirDatosPiloto   (Piloto* p);
static int  ingresarEstadoBaja (void);
static void mostrarCamposPiloto(const Piloto* p);

/**
 * pedirDatosPiloto  [static]
 * Solicita nombre, nacionalidad, id_escuderia y fecha de nacimiento.
 * Llama a limpiarBuffer() antes de leerCadena() para descartar
 * residuos de scanf.
 */
static void pedirDatosPiloto(Piloto* p)
{
    limpiarBuffer();

    printf("Nombre: ");
    leerCadena(p->nombre, TAM_NOMBRE_PILOTO);

    printf("Nacionalidad: ");
    leerCadena(p->nacionalidad, TAM_NACIONALIDAD);

    printf("ID Escuderia: ");
    scanf("%u", &p->id_escuderia);

    do
    {
        printf("Fecha nacimiento (AAAAMMDD): ");
        scanf("%llu", &p->fechaNacimiento);
    }
    while (!esFechaValida(p->fechaNacimiento));
}

/**
 * ingresarEstadoBaja  [static]
 * Muestra las opciones de tipo de baja y retorna el caracter
 * de estado: ESTADO_RETIRADO_PILOTO o ESTADO_SUSPENDIDO_PILOTO.
 */
static int ingresarEstadoBaja(void)
{
    int op;

    printf("  Tipo de baja:\n");
    printf("  [1] Retirado    (R)\n");
    printf("  [2] Suspendido  (S)\n");
    printf("  Opcion: ");
    scanf("%d", &op);

    return (op == 1) ? ESTADO_RETIRADO_PILOTO : ESTADO_SUSPENDIDO_PILOTO;
}

/**
 * mostrarCamposPiloto  [static]
 * Muestra los campos editables del piloto numerados para que
 * el usuario elija cual modificar.
 */
static void mostrarCamposPiloto(const Piloto* p)
{
    printf("\n  Datos actuales del piloto ID %u:\n", p->id);
    printf("  [1] Nombre          : %s\n",  p->nombre);
    printf("  [2] Nacionalidad    : %s\n",  p->nacionalidad);
    printf("  [3] ID Escuderia    : %u\n",  p->id_escuderia);
    printf("  [4] Fecha nacimiento: %llu\n", p->fechaNacimiento);
    printf("  [5] Estado          : %c\n",  p->estado);
    printf("  [0] Salir\n");
}

/* =========================================================
   Generacion del lote inicial
   ========================================================= */

/**
 * generarArchivoPilotosTxt
 * Crea pilotos.txt con el lote inicial usando generarArchivoTexto()
 * y escribirPilotoTxt() como Accion.
 * Retorna TODO_OK o ERR_ARCH.
 */
int generarArchivoPilotosTxt(const char* rutaTxt)
{
    Piloto lote[11] =
    {
        {1,  "Max Verstappen",  "Neerlandes",  1, 0, ESTADO_ACTIVO_PILOTO,    19970930},
        {2,  "Lando Norris",    "Britanico",   2, 0, ESTADO_RETIRADO_PILOTO,  19991113},
        {3,  "Charles Leclerc", "Monegasco",   3, 0, ESTADO_ACTIVO_PILOTO,    19971016},
        {4,  "Oscar Piastri",   "Australiano", 2, 0, ESTADO_ACTIVO_PILOTO,    20010406},
        {5,  "Carlos Sainz",    "Espanol",     4, 0, ESTADO_ACTIVO_PILOTO,    19940901},
        {6,  "George Russell",  "Britanico",   1, 0, ESTADO_ACTIVO_PILOTO,    19980215},
        {7,  "Lewis Hamilton",  "Britanico",   3, 0, ESTADO_ACTIVO_PILOTO,    19850107},
        {8,  "Fernando Alonso", "Espanol",     5, 0, ESTADO_RETIRADO_PILOTO,  19810729},
        {9,  "Lance Stroll",    "Canadiense",  5, 0, ESTADO_ACTIVO_PILOTO,    19981029},
        {10, "Nico Hulkenberg", "Aleman",      6, 0, ESTADO_RETIRADO_PILOTO,  19870819},
        {11, "Juan Fernandez",  "Argentino",   6, 0, ESTADO_ACTIVO_PILOTO,    19870819}
    };

    return generarArchivoTexto(rutaTxt, lote, 11, sizeof(Piloto), escribirPilotoTxt);
}

/* =========================================================
   Listado
   ========================================================= */

/**
 * listarPilotos
 * Imprime el encabezado de tabla y delega a mostrarArchivoBinario()
 * con mostrarPiloto como Mostrar.
 * Retorna TODO_OK o ERR_ARCH.
 */
int listarPilotos(const char* rutaBin)
{
    int resp;

    printf("\n");
    printf("=============================================================\n");
    printf("  LISTADO DE PILOTOS - TEMPORADA\n");
    printf("=============================================================\n");
    printf("%-4s  %-28s  %-10s  %s\n", "ID", "Nombre", "Estado", "Puntos");
    printf("-------------------------------------------------------------\n");

    resp = mostrarArchivoBinario(rutaBin, sizeof(Piloto), mostrarPiloto);

    printf("-------------------------------------------------------------\n");

    return resp;
}

/**
 * RankingPiloto
 * Genera el ranking de pilotos activos ordenado por puntos (mayor a menor).
 * Construye un arreglo dinamico de PilotoRef (puntos + offset),
 * ordena con qsort y muestra accediendo por offset.
 * Retorna TODO_OK, ERR_ARCH o ERR_MEM.
 */
int RankingPiloto(const char* rutaBin)
{
    FILE*      fBin;
    Piloto     pil;
    PilotoRef* refs;
    PilotoRef* p;
    long       offset;
    size_t     cap;
    size_t     n;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    cap  = CAPACIDAD_MINIMA;
    n    = 0;
    refs = (PilotoRef*)malloc(cap * sizeof(PilotoRef));

    if (!refs)
    {
        fclose(fBin);
        return ERR_MEM;
    }

    offset = 0L;
    while (fread(&pil, sizeof(Piloto), 1, fBin) == 1)
    {
        if (n == cap)
        {
            PilotoRef* tmp;
            cap *= 2;
            tmp = (PilotoRef*)realloc(refs, cap * sizeof(PilotoRef));

            if (!tmp)
            {
                free(refs);
                fclose(fBin);
                return ERR_MEM;
            }
            refs = tmp;
        }

        if (esPilotoActivos(&pil))
        {
            (refs + n)->puntos = pil.puntos_acumulados;
            (refs + n)->offset = offset;
            n++;
        }

        offset += (long)sizeof(Piloto);
    }

    qsort(refs, n, sizeof(PilotoRef), cmp_desc);

    printf("\n");
    printf("=============================================================\n");
    printf("  RANKING DE PILOTOS - TEMPORADA\n");
    printf("=============================================================\n");
    printf("%-4s  %-28s  %-10s  %s\n", "ID", "Nombre", "Estado", "Puntos");
    printf("-------------------------------------------------------------\n");

    for (p = refs; p < refs + n; p++)
    {
        fseek(fBin, p->offset, SEEK_SET);
        fread(&pil, sizeof(Piloto), 1, fBin);
        printf("%-4u  %-28s  %-10c  %u\n",
               pil.id, pil.nombre, pil.estado, pil.puntos_acumulados);
    }

    free(refs);
    fclose(fBin);
    return TODO_OK;
}

/**
 * listarPilotosPorEscuderia
 * Verifica que la escuderia exista y este activa, luego lista
 * solo los pilotos que pertenecen a ella.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int listarPilotosPorEscuderia(const char* rutaPiloto, const char* rutaEscuderia, unsigned idEscuderia)
{
    Escuderia esc;
    Piloto    piloto;
    FILE*     fEsc;
    FILE*     fPil;
    int       encontrado;
    int       cont;

    encontrado = 0;
    cont       = 0;

    fEsc = fopen(rutaEscuderia, "rb");
    if (!fEsc)
        return ERR_ARCH;

    while (fread(&esc, sizeof(Escuderia), 1, fEsc) == 1 && !encontrado)
    {
        if (esc.id == idEscuderia)
            encontrado = 1;
    }
    fclose(fEsc);

    if (!encontrado)
    {
        printf("[!] Escuderia con ID %u no encontrada.\n", idEscuderia);
        return NO_ENCONTRADO;
    }

    if (esc.estado != ESTADO_ESCUDERIA_ACTIVA)
    {
        printf("[!] La escuderia [%s] %s esta inactiva.\n", esc.codigo, esc.nombre);
        return TODO_OK;
    }

    fPil = fopen(rutaPiloto, "rb");
    if (!fPil)
        return ERR_ARCH;

    printf("\n");
    printf("=============================================================\n");
    printf("  PILOTOS DE: [%s] %s\n", esc.codigo, esc.nombre);
    printf("=============================================================\n");
    printf("%-4s  %-28s  %-12s  %-6s  %s\n",
           "ID", "Nombre", "Nacionalidad", "Estado", "Puntos");
    printf("-------------------------------------------------------------\n");

    while (fread(&piloto, sizeof(Piloto), 1, fPil) == 1)
    {
        if (piloto.id_escuderia == idEscuderia)
        {
            printf("%-4u  %-28s  %-12s  %-6c  %u\n",
                   piloto.id, piloto.nombre, piloto.nacionalidad,
                   piloto.estado, piloto.puntos_acumulados);
            cont++;
        }
    }

    if (cont == 0)
        printf("  (Sin pilotos registrados para esta escuderia)\n");

    printf("-------------------------------------------------------------\n");
    printf("  Total: %d piloto(s)\n\n", cont);

    fclose(fPil);
    return TODO_OK;
}

/* =========================================================
   Punteros a funcion del TDA Piloto
   ========================================================= */

/**
 * mostrarPiloto  [Mostrar]
 * Imprime una fila de piloto en formato tabla.
 */
void mostrarPiloto(const void* dato)
{
    const Piloto* p = (const Piloto*)dato;

    printf("%-4u  %-28s  %-10c  %u\n",
           p->id, p->nombre, p->estado, p->puntos_acumulados);
}

/**
 * trozarPilotoTxt  [TxtABin]
 * Parsea una linea con formato separado por SEP_TXT de atras hacia
 * adelante: id|nombre|nacionalidad|id_escuderia|puntos|estado|fecha.
 * Retorna TODO_OK o ERR_LINEA.
 */
int trozarPilotoTxt(char* linea, void* reg)
{
    Piloto* p   = (Piloto*)reg;
    char*   act = strchr(linea, '\n');

    if (!act)
        return ERR_LINEA;

    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    sscanf(act + 1, "%llu", &p->fechaNacimiento);
    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    sscanf(act + 1, " %c", &p->estado);
    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    sscanf(act + 1, "%u", &p->puntos_acumulados);
    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    sscanf(act + 1, "%u", &p->id_escuderia);
    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    copiarCadena(p->nacionalidad, act + 1, TAM_NACIONALIDAD);
    *act = '\0';

    act = strrchr(linea, SEP_TXT);
    copiarCadena(p->nombre, act + 1, TAM_NOMBRE_PILOTO);
    *act = '\0';

    sscanf(linea, "%u", &p->id);

    return TODO_OK;
}

/**
 * pilotoBinATxt  [BinATxt]
 * Escribe un Piloto en formato texto con separador SEP_TXT.
 * Mismo formato que trozarPilotoTxt espera para reconstruir el .bin.
 * Retorna TODO_OK o ERR_ARCH.
 */
int pilotoBinATxt(const void* dato, FILE* archTxt)
{
    const Piloto* p = (const Piloto*)dato;

    if (!dato || !archTxt)
        return ERR_ARCH;

    fprintf(archTxt, "%u%c%s%c%s%c%u%c%u%c%c%c%llu\n",
            p->id,                SEP_TXT,
            p->nombre,            SEP_TXT,
            p->nacionalidad,      SEP_TXT,
            p->id_escuderia,      SEP_TXT,
            p->puntos_acumulados, SEP_TXT,
            p->estado,            SEP_TXT,
            p->fechaNacimiento);

    return TODO_OK;
}

/**
 * escribirPilotoTxt  [Accion]
 * Misma logica que pilotoBinATxt pero con firma Accion.
 * Se usa con generarArchivoTexto() para el lote inicial.
 * Retorna TODO_OK o ERR_ARCH.
 */
int escribirPilotoTxt(void* archivo, const void* dato)
{
    return pilotoBinATxt(dato, (FILE*)archivo);
}

/**
 * esPilotoActivos  [Filter]
 * Retorna 1 si el piloto tiene estado ACTIVO, 0 si no.
 */
int esPilotoActivos(const void* dato)
{
    return (((const Piloto*)dato)->estado == ESTADO_ACTIVO_PILOTO);
}

/**
 * sumarPuntos  [Reduce]
 * Acumula en *(unsigned*)acumulador los puntos del piloto.
 * Retorna TODO_OK.
 */
int sumarPuntos(void* acumulador, const void* dato)
{
    *(unsigned*)acumulador += ((const Piloto*)dato)->puntos_acumulados;
    return TODO_OK;
}

/**
 * extraerIdPuntos  [Map]
 * Transforma un Piloto en unsigned[2]: [0]=id, [1]=puntos.
 * Retorna TODO_OK.
 */
int extraerIdPuntos(void* dest, const void* orig)
{
    unsigned*     resultado = (unsigned*)dest;
    const Piloto* p         = (const Piloto*)orig;

    resultado[COL_ID_PILOTO] = p->id;
    resultado[COL_PUNTOS]    = p->puntos_acumulados;

    return TODO_OK;
}

/**
 * cmp_desc  [Comparar]
 * Compara dos PilotoRef por puntos en orden DESCENDENTE.
 * Retorna 1 si b > a, -1 si b < a, 0 si iguales.
 */
int cmp_desc(const void* a, const void* b)
{
    const PilotoRef* ra = (const PilotoRef*)a;
    const PilotoRef* rb = (const PilotoRef*)b;

    if (rb->puntos > ra->puntos) return  1;
    if (rb->puntos < ra->puntos) return -1;
    return 0;
}

/* =========================================================
   Utilitaria para simulacion de carreras
   ========================================================= */

/**
 * cargarVectorPilotoActivos
 * Recorre el .bin e inserta ordenado en vIds solo los IDs de
 * pilotos con estado ACTIVO.
 * Retorna TODO_OK o ERR_ARCH.
 */
int cargarVectorPilotoActivos(const char* rutaBin, tVector* vIds, Comparar comparar)
{
    Piloto piloto;
    FILE*  fPiloto;

    fPiloto = fopen(rutaBin, "rb");
    if (!fPiloto)
        return ERR_ARCH;

    while (fread(&piloto, sizeof(Piloto), 1, fPiloto) == 1)
    {
        if (piloto.estado == ESTADO_ACTIVO_PILOTO)
            insertarVectorOrd(vIds, &piloto.id, comparar);
    }

    fclose(fPiloto);
    return TODO_OK;
}

/* =========================================================
   ABM
   ========================================================= */

/**
 * altaPiloto
 * Pide ID y datos, asigna estado Activo y puntos=0, agrega al
 * final del .bin y reconstruye el indice.
 * Retorna TODO_OK o ERR_ARCH.
 */
int altaPiloto(const char* rutaBin)
{
    Piloto nuevo;
    FILE*  fBin;

    printf("Ingresar ID: ");
    scanf("%u", &nuevo.id);

    nuevo.puntos_acumulados = 0;
    nuevo.estado            = ESTADO_ACTIVO_PILOTO;

    printf("\n--- ALTA DE PILOTO (ID asignado: %u) ---\n", nuevo.id);
    pedirDatosPiloto(&nuevo);

    fBin = fopen(rutaBin, "ab");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de pilotos.\n");
        return ERR_ARCH;
    }

    fwrite(&nuevo, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    /* Reconstruir el indice despues de cada alta */
    construirIndicePilotos(rutaBin, RUTA_INDICE_PILOTO);

    printf("[OK] Piloto '%s' registrado con ID %u.\n", nuevo.nombre, nuevo.id);
    return TODO_OK;
}

/**
 * bajaPiloto
 * Busca el piloto por ID con el indice. Si esta activo, cambia su
 * estado a Retirado o Suspendido y sobreescribe el registro.
 * Registra la baja en el archivo de texto de bajas.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int bajaPiloto(const char* rutaBin, const char* rutaBajasTxt)
{
    Piloto   piloto;
    FILE*    fBin;
    FILE*    fBajas;
    long     offset;
    unsigned id;

    fBin = fopen(rutaBin, "rb+");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de pilotos.\n");
        return ERR_ARCH;
    }

    printf("\n--- BAJA DE PILOTO ---\n");
    printf("ID del piloto: ");
    scanf("%u", &id);

    offset = buscarPilotoPorIndice(RUTA_INDICE_PILOTO, fBin, id, &piloto);

    if (offset == -1L)
    {
        fclose(fBin);
        printf("[!] Piloto con ID %u no encontrado.\n", id);
        return NO_ENCONTRADO;
    }

    if (piloto.estado != ESTADO_ACTIVO_PILOTO)
    {
        fclose(fBin);
        printf("[!] El piloto '%s' ya tiene estado '%c'.\n", piloto.nombre, piloto.estado);
        return TODO_OK;
    }

    printf("  Piloto encontrado: %s\n", piloto.nombre);
    piloto.estado = (char)ingresarEstadoBaja();

    fseek(fBin, offset, SEEK_SET);
    fwrite(&piloto, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    fBajas = fopen(rutaBajasTxt, "at");
    if (fBajas)
    {
        fprintf(fBajas, "PILOTO|%u|%s|%c\n", piloto.id, piloto.nombre, piloto.estado);
        fclose(fBajas);
    }

    printf("[OK] Estado del piloto '%s' actualizado a '%c'.\n", piloto.nombre, piloto.estado);
    return TODO_OK;
}

/**
 * modificarPiloto
 * Busca el piloto por ID con el indice, presenta un submenu de
 * campos y repite hasta que el usuario elija 0. Sobreescribe el
 * registro completo al finalizar.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int modificarPiloto(const char* rutaBin)
{
    Piloto             piloto;
    FILE*              fBin;
    long               offset;
    unsigned           id;
    int                campo;
    unsigned long long fechaNueva;
    int                nuevoEstado;
    int                okEsc;

    fBin = fopen(rutaBin, "rb+");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de pilotos.\n");
        return ERR_ARCH;
    }

    printf("\n--- MODIFICAR PILOTO ---\n");
    printf("ID del piloto: ");
    scanf("%u", &id);

    offset = buscarPilotoPorIndice(RUTA_INDICE_PILOTO, fBin, id, &piloto);

    if (offset == -1L)
    {
        fclose(fBin);
        printf("[!] Piloto con ID %u no encontrado.\n", id);
        return NO_ENCONTRADO;
    }

    mostrarCamposPiloto(&piloto);
    printf("Campo a modificar: ");
    scanf("%d", &campo);

    while (campo != 0)
    {
        limpiarBuffer();

        switch (campo)
        {
        case 1:
            printf("Nuevo nombre: ");
            leerCadena(piloto.nombre, TAM_NOMBRE_PILOTO);
            break;

        case 2:
            printf("Nueva nacionalidad: ");
            leerCadena(piloto.nacionalidad, TAM_NACIONALIDAD);
            break;

        case 3:
            okEsc = 0;
            do
            {
                printf("Nuevo ID Escuderia: ");
                if (scanf("%u", &piloto.id_escuderia) != 1)
                {
                    printf("[!] Entrada invalida.\n");
                    limpiarBuffer();
                }
                else
                {
                    limpiarBuffer();
                    okEsc = esEscuderiaValida(RUTA_ESCUDERIA_BIN, piloto.id_escuderia);
                    if (!okEsc)
                        printf("[!] Escuderia con ID %u no existe.\n", piloto.id_escuderia);
                }
            }
            while (!okEsc);
            break;

        case 4:
            do
            {
                printf("Nueva fecha nacimiento (AAAAMMDD): ");
                scanf("%llu", &fechaNueva);
            }
            while (!esFechaValida(fechaNueva));
            piloto.fechaNacimiento = fechaNueva;
            break;

        case 5:
            nuevoEstado = 0;
            do
            {
                printf("  [1] Activo      (A)\n");
                printf("  [2] Retirado    (R)\n");
                printf("  [3] Suspendido  (S)\n");
                printf("Nuevo Estado: ");
                if (scanf("%d", &nuevoEstado) != 1)
                {
                    printf("[!] Entrada invalida.\n");
                    limpiarBuffer();
                    nuevoEstado = 0;
                }
            }
            while (nuevoEstado < 1 || nuevoEstado > 3);

            piloto.estado = ((nuevoEstado == 1) ? ESTADO_ACTIVO_PILOTO
                          :(nuevoEstado == 2) ? ESTADO_RETIRADO_PILOTO: ESTADO_SUSPENDIDO_PILOTO);
            break;

        default:
            printf("[!] Campo invalido.\n");
            break;
        }

        mostrarCamposPiloto(&piloto);
        printf("Campo a modificar: ");
        scanf("%d", &campo);
    }

    fseek(fBin, offset, SEEK_SET);
    fwrite(&piloto, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    printf("[OK] Piloto ID %u modificado correctamente.\n", piloto.id);
    return TODO_OK;
}
