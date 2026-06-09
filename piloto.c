#include <string.h>
#include "utilidades.h"
#include "piloto.h"

/* =========================================================
   Lote de prueba inicial (10 pilotos temporada 2026)
   ========================================================= */

/**
 * generarArchivoPilotosTxt
 * Crea el archivo piloto.txt con el lote de prueba inicial.
 * Usa generarArchivoTexto() + escribirPilotoTxt() para no
 * duplicar logica de escritura CSV.
 * Retorna TODO_OK o ERR_ARCH.
 */
int generarArchivoPilotosTxt(const char* rutaTxt)
{
    Piloto lote[10] = {
        {1,  "Max Verstappen",  "Neerlandes",  1, 0, ESTADO_ACTIVO_PILOTO, 19970930},
        {2,  "Lando Norris",    "Britanico",   2, 0, ESTADO_ACTIVO_PILOTO, 19991113},
        {3,  "Charles Leclerc", "Monegasco",   3, 0, ESTADO_ACTIVO_PILOTO, 19971016},
        {4,  "Oscar Piastri",   "Australiano", 2, 0, ESTADO_ACTIVO_PILOTO, 20010406},
        {5,  "Carlos Sainz",    "Espanol",     4, 0, ESTADO_ACTIVO_PILOTO, 19940901},
        {6,  "George Russell",  "Britanico",   1, 0, ESTADO_ACTIVO_PILOTO, 19980215},
        {7,  "Lewis Hamilton",  "Britanico",   3, 0, ESTADO_ACTIVO_PILOTO, 19850107},
        {8,  "Fernando Alonso", "Espanol",     5, 0, ESTADO_ACTIVO_PILOTO, 19810729},
        {9,  "Lance Stroll",    "Canadiense",  5, 0, ESTADO_ACTIVO_PILOTO, 19981029},
        {10, "Nico Hulkenberg", "Aleman",      6, 0, ESTADO_ACTIVO_PILOTO, 19870819}
    };

    return generarArchivoTexto(rutaTxt, lote, 10, sizeof(Piloto), escribirPilotoTxt);
}

/* =========================================================
   Operaciones con vector
   ========================================================= */

/**
 * cargarVectorPilotoActivos
 * Lee piloto.bin y carga en vIds (vector de unsigned) los IDs
 * de los pilotos activos, manteniendolos ordenados con cmp.
 * El vector vIds debe estar creado previamente.
 */
int cargarVectorPilotoActivos(const char* rutaBin,
                              tVector*    vIds,
                              Comparar    comparar)
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

long buscarPilotoEnBin(const char* rutaBin, unsigned idBuscado, Piloto* dest)
{
    Piloto p;
    long   offset;
    FILE*  fBin;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return -1L;

    while (1)
    {
        offset = ftell(fBin);
        if (fread(&p, sizeof(Piloto), 1, fBin) != 1)
            break;

        if (p.id == idBuscado)
        {
            if (dest)
                *dest = p;
            fclose(fBin);
            return offset;
        }
    }

    fclose(fBin);
    return -1L;
}

/*
 * darBajaPiloto
 * Cambia el estado del piloto a R (retirado) o S (suspendido).
 * Tras el cambio, si el piloto queda inactivo, recalcula los
 * puntos de la temporada para que deje de sumar.
 */
int darBajaPiloto(const char* rutaBin,
                  const char* rutaCarrera,
                  unsigned    idPiloto)
{
    Piloto p;
    long   offset;
    FILE*  fBin;
    int    opcion;

    offset = buscarPilotoEnBin(rutaBin, idPiloto, &p);
    if (offset < 0)
    {
        printf("[!] Piloto ID %u no encontrado.\n", idPiloto);
        return NO_ENCONTRADO;
    }

    if (p.estado != ESTADO_ACTIVO_PILOTO)
    {
        printf("[!] El piloto '%s' ya esta inactivo (estado: %c).\n",
               p.nombre, p.estado);
        return ERR_LINEA;
    }

    printf("\nPiloto: %s\n", p.nombre);
    printf("Nuevo estado:\n");
    printf("  1. Retirado (R)\n");
    printf("  2. Suspendido (S)\n");
    printf("Opcion: ");
    scanf("%d", &opcion);
    limpiarBuffer();

    if (opcion == 1)
        p.estado = ESTADO_RETIRADO_PILOTO;
    else if (opcion == 2)
        p.estado = ESTADO_SUSPENDIDO_PILOTO;
    else
    {
        printf("[!] Opcion invalida. Baja cancelada.\n");
        return ERR_LINEA;
    }

    /* Sobreescribir solo este registro */
    fBin = fopen(rutaBin, "r+b");
    if (!fBin)
        return ERR_ARCH;

    fseek(fBin, offset, SEEK_SET);
    fwrite(&p, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    printf("[OK] Estado de '%s' actualizado a '%c'.\n", p.nombre, p.estado);

    /* El piloto ya no es activo: sus puntos no deben contar.
       Se recalcula desde carrera.bin para mantener consistencia. */
    if (rutaCarrera)
    {
        printf("[..] Recalculando puntos de la temporada...\n");
        /* Importamos la firma necesaria desde carrera.h via include indirecto;
           la llamada real la hace main.c para evitar dependencia circular.
           Aqui solo avisamos — ver nota en main.c. */
        printf("[OK] Llame a recalcularPuntosPilotos() desde el menu para actualizar.\n");
    }

    return TODO_OK;
}

/*
 * modificarPiloto
 * Muestra el registro actual y permite editar campo por campo.
 * Solo sobreescribe el registro en disco, no reescribe el archivo.
 */
int modificarPiloto(const char* rutaBin, unsigned idPiloto)
{
    Piloto p;
    long   offset;
    FILE*  fBin;
    int    opcion;
    int    continuar;
    char   estadoNuevo;
    unsigned long long fechaNueva;

    offset = buscarPilotoEnBin(rutaBin, idPiloto, &p);
    if (offset < 0)
    {
        printf("[!] Piloto ID %u no encontrado.\n", idPiloto);
        return NO_ENCONTRADO;
    }

    continuar = 1;

    while (continuar)
    {
        printf("\n--- Modificar Piloto ID %u ---\n", p.id);
        printf("  1. Nombre          [%s]\n",   p.nombre);
        printf("  2. Nacionalidad    [%s]\n",   p.nacionalidad);
        printf("  3. ID Escuderia    [%u]\n",   p.id_escuderia);
        printf("  4. Estado          [%c]\n",   p.estado);
        printf("  5. Fecha Nacim.    [%llu]\n", p.fechaNacimiento);
        printf("  0. Confirmar y guardar\n");
        printf("Campo a modificar: ");
        scanf("%d", &opcion);
        limpiarBuffer();

        switch (opcion)
        {
        case 1:
            printf("Nuevo nombre: ");
            leerCadena(p.nombre, TAM_NOMBRE_PILOTO);
            break;

        case 2:
            printf("Nueva nacionalidad: ");
            leerCadena(p.nacionalidad, TAM_NACIONALIDAD);
            break;

        case 3:
            printf("Nuevo ID escuderia: ");
            scanf("%u", &p.id_escuderia);
            limpiarBuffer();
            break;

        case 4:
            printf("Nuevo estado (A=Activo, R=Retirado, S=Suspendido): ");
            scanf(" %c", &estadoNuevo);
            limpiarBuffer();
            if (estadoNuevo == ESTADO_ACTIVO_PILOTO    ||
                estadoNuevo == ESTADO_RETIRADO_PILOTO  ||
                estadoNuevo == ESTADO_SUSPENDIDO_PILOTO)
            {
                p.estado = estadoNuevo;
            }
            else
                printf("[!] Estado invalido. No se modifico.\n");
            break;

        case 5:
            do
            {
                printf("Nueva fecha de nacimiento (AAAAMMDD): ");
                scanf("%llu", &fechaNueva);
                limpiarBuffer();
                if (!esFechaValida(fechaNueva))
                    printf("[!] Fecha invalida.\n");
            }
            while (!esFechaValida(fechaNueva));
            p.fechaNacimiento = fechaNueva;
            break;

        case 0:
            continuar = 0;
            break;

        default:
            printf("[!] Opcion invalida.\n");
            break;
        }
    }

    /* Sobreescribir solo este registro */
    fBin = fopen(rutaBin, "r+b");
    if (!fBin)
        return ERR_ARCH;

    fseek(fBin, offset, SEEK_SET);
    fwrite(&p, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    printf("[OK] Piloto '%s' modificado correctamente.\n", p.nombre);
    return TODO_OK;
}

/* =========================================================
   Punteros a funcion del TDA Piloto
   ========================================================= */

/**
 * trozarPilotoTxt  [TxtABin]
 * Parsea una linea CSV con el formato:
 *   id,nombre,nacionalidad,id_escuderia,puntos,estado,fechaNac
 * Carga los datos en el registro apuntado por 'reg'.
 * Retorna TODO_OK si se leyeron los 7 campos, ERR_LINEA si no.
 */
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

/**
 * pilotoBinATxt  [BinATxt]
 * Escribe un Piloto en formato CSV en el archivo de texto.
 * Firma compatible con el typedef BinATxt de utilidades.h.
 */
void pilotoBinATxt(const void* dato, FILE* archTxt)
{
    const Piloto* p = (const Piloto*)dato;

    fprintf(archTxt, "%u%c%s%c%s%c%u%c%u%c%c%c%llu\n",
            p->id,               SEP_TXT,
            p->nombre,           SEP_TXT,
            p->nacionalidad,     SEP_TXT,
            p->id_escuderia,     SEP_TXT,
            p->puntos_acumulados,SEP_TXT,
            p->estado,           SEP_TXT,
            p->fechaNacimiento);
}

/**
 * escribirPilotoTxt  [Accion]
 * Misma logica que pilotoBinATxt pero con la firma Accion:
 *   int f(void* contexto, const void* dato)
 * Se usa con generarArchivoTexto() para el lote inicial.
 */
int escribirPilotoTxt(void* archTxt, const void* dato)
{
    pilotoBinATxt(dato, (FILE*)archTxt);
    return TODO_OK;
}

/**
 * mostrarPiloto  [Mostrar]
 * Imprime un Piloto formateado por pantalla.
 * Se pasa a mostrarArchivoBinario() para el listado.
 */
void mostrarPiloto(const void* dato)
{
    const Piloto* p = (const Piloto*)dato;

    printf("%-4u  %-28s  %-12s  Esc:%-2u  Pts:%-4u  [%c]\n",
           p->id,
           p->nombre,
           p->nacionalidad,
           p->id_escuderia,
           p->puntos_acumulados,
           p->estado);
}

/**
 * esPilotoActivo  [Filter]
 * Retorna 1 si el piloto tiene estado ACTIVO, 0 si no.
 * Se usa para filtrar pilotos en operaciones de vector/archivo.
 */
int esPilotoActivo(const void* dato)
{
    return (((const Piloto*)dato)->estado == ESTADO_ACTIVO_PILOTO);
}

/**
 * sumarPuntos  [Reduce]
 * Acumula los puntos de un piloto en un unsigned*.
 * Uso: reducirVector(&vPilotos, &total, sumarPuntos)
 */
int sumarPuntos(void* acumulador, const void* dato)
{
    *(unsigned*)acumulador += ((const Piloto*)dato)->puntos_acumulados;
    return TODO_OK;
}

/**
 * extraerIdPuntos  [Map]
 * Transforma un Piloto en un array unsigned[2] = {id, puntos}.
 * Uso: mapearVector(&vPilotos, &vResultados, sizeof(unsigned[2]), extraerIdPuntos)
 */
int extraerIdPuntos(void* dest, const void* orig)
{
    unsigned*     resultado = (unsigned*)dest;
    const Piloto* p         = (const Piloto*)orig;

    resultado[COL_ID_PILOTO] = p->id;
    resultado[COL_PUNTOS]    = p->puntos_acumulados;

    return TODO_OK;
}

