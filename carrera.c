#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "carrera.h"
#include "piloto.h"
#include "utilidades.h"
#include "vector.h"
#include "puntos.h"

/* =========================================================
   Ciclo de vida del TDA Carrera
   ========================================================= */

/*
 * inicializarCarrera
 * Reserva el vector interno de Resultado con capacidad cantPilotos.
 * Debe llamarse antes de usar carrera.resultados.
 */
int inicializarCarrera(Carrera* c, int cantPilotos)
{
    c->cant_resultados = 0;
    return crearVector(&c->resultados, sizeof(Resultado), cantPilotos);
}

/*
 * destruirCarrera
 * Libera la memoria del vector interno.
 * Llamar siempre que se termine de usar una Carrera cargada desde archivo.
 */
void destruirCarrera(Carrera* c)
{
    destruirVector(&c->resultados);
    c->cant_resultados = 0;
}

/* =========================================================
   Serializacion
   No se puede usar fread/fwrite directo sobre Carrera porque
   tVector tiene un void* interno que apunta al heap.
   Estas dos funciones encapsulan toda la I/O de Carrera.
   ========================================================= */

/*
 * escribirCarrera
 * Guarda primero los campos fijos del struct (todo menos tVector)
 * y despues los datos del heap (el arreglo de Resultado).
 */
int escribirCarrera(FILE* f, const Carrera* c)
{
    /* Tamanio de los campos fijos: todo el struct menos el tVector */
    size_t tamCabecera = sizeof(Carrera) - sizeof(tVector);

    if (fwrite(c, tamCabecera, 1, f) != 1)
        return ERR_ARCH;

    /* Datos dinamicos: cant_resultados structs Resultado */
    if (fwrite(c->resultados.vec, sizeof(Resultado), c->cant_resultados, f)
            != (size_t)c->cant_resultados)
        return ERR_ARCH;

    return TODO_OK;
}

/*
 * leerCarrera
 * Lee primero la cabecera fija, luego reserva memoria y lee los Resultado.
 * Retorna ERR_ARCH en EOF o error, SIN_MEM si falla malloc.
 * El llamador es responsable de llamar destruirCarrera() cuando termine.
 */
int leerCarrera(FILE* f, Carrera* c)
{
    size_t tamCabecera = sizeof(Carrera) - sizeof(tVector);

    if (fread(c, tamCabecera, 1, f) != 1)
        return ERR_ARCH;   /* EOF o error de lectura */

    /* Reservar vector con exactamente cant_resultados elementos */
    if (crearVector(&c->resultados, sizeof(Resultado), c->cant_resultados) != TODO_OK)
        return SIN_MEM;

    if (fread(c->resultados.vec, sizeof(Resultado), c->cant_resultados, f)
            != (size_t)c->cant_resultados)
    {
        destruirVector(&c->resultados);
        return ERR_ARCH;
    }

    c->resultados.ce = c->cant_resultados;
    return TODO_OK;
}

/* =========================================================
   Funciones auxiliares internas
   ========================================================= */

/*
 * generarIdCarrera
 * Recorre el archivo contando registros y retorna el proximo ID.
 * El FILE* debe estar abierto en modo lectura binaria.
 */
int generarIdCarrera(FILE* fCarrera)
{
    Carrera c;
    int     id = 1;

    rewind(fCarrera);

    while (leerCarrera(fCarrera, &c) == TODO_OK)
    {
        id++;
        destruirCarrera(&c);
    }

    return id;
}

/* =========================================================
   Mostrar
   ========================================================= */

/*
 * mostrarCarrera  [Mostrar]
 * Imprime encabezado de la carrera y la tabla de resultados.
 * Recibe const void* para ser compatible con mostrarVector/mostrarArchivoBinario.
 * ATENCION: como mostrarArchivoBinario usa fread directo, no puede usarse
 * para Carrera. Ver el loop en menuCarrera() en main.c.
 */
void mostrarCarrera(const void* dato)
{
    const Carrera* c = (const Carrera*)dato;
    Resultado*     r;
    int            i;

    printf("\n");
    printf("  +------------------------------------------+\n");
    printf("  |  Carrera #%-3d  %-20s     |\n", c->id, c->circuito);
    printf("  |  Fecha: %llu   Estado: %s         |\n",
           c->fecha,
           c->estado == ESTADO_CARRERA_ACTIVA ? "Activa  " : "Cancelada");
    printf("  +--------+------------+--------+\n");
    printf("  | Pos    | ID Piloto  | Puntos |\n");
    printf("  +--------+------------+--------+\n");

    for (i = 0; i < c->cant_resultados; i++)
    {
        r = (Resultado*)obtenerElementoVector((tVector*)&c->resultados, i);
        printf("  | %-6d | %-10d | %-6d |\n", i + 1, r->id_piloto, r->puntos);
    }

    printf("  +--------+------------+--------+\n");
}

/* =========================================================
   Alta de carrera — resultado aleatorio
   ========================================================= */

/*
 * cargarResultadosCarreraAleatorios
 * Carga los IDs de pilotos activos en un vector, los baraja con
 * Fisher-Yates y asigna puntos F1 segun posicion.
 */
int cargarResultadosCarreraAleatorios(const char* rutaPiloto, Carrera* nueva,
                                       Comparar comparar, const Puntos* pts)
{
    tVector  vIds;
    unsigned idPiloto;
    int      i;
    int      puntos;
    Resultado res;

    if (crearVector(&vIds, sizeof(unsigned), MAX_PILOTOS_CARRERA) != TODO_OK)
        return SIN_MEM;

    if (cargarVectorPilotoActivos(rutaPiloto, &vIds, comparar) != TODO_OK)
    {
        destruirVector(&vIds);
        return ERR_ARCH;
    }

    /* Barajar con Fisher-Yates para orden aleatorio */
    generarResultadoAleatorioVector(&vIds);

    nueva->cant_resultados = (int)vIds.ce;

    if (inicializarCarrera(nueva, nueva->cant_resultados) != TODO_OK)
    {
        destruirVector(&vIds);
        return SIN_MEM;
    }

    for (i = 0; i < nueva->cant_resultados; i++)
    {
        idPiloto = *(unsigned*)obtenerElementoVector(&vIds, i);

        puntos = obtenerPuntosPorPosicion(pts, i + 1);

        res.id_piloto = (int)idPiloto;
        res.puntos    = puntos;

        insertarFinalVector(&nueva->resultados, &res);
    }

    destruirVector(&vIds);
    return TODO_OK;
}

/*
 * registrarCarreraAleatoria
 * Pide circuito y fecha, genera resultados aleatorios y guarda en .dat.
 */
int registrarCarreraAleatoria(const char* rutaCarrera, const char* rutaPiloto,
                               Comparar comparar, const Puntos* pts)
{
    Carrera nueva;
    FILE*   fCarrera;
    int     ret;

    /* Inicializar campos fijos — el vector se inicializa en cargarResultados */
    nueva.resultados.vec = NULL;
    nueva.cant_resultados = 0;

    fCarrera = fopen(rutaCarrera, "ab+");
    if (!fCarrera)
        return ERR_ARCH;

    nueva.id     = generarIdCarrera(fCarrera);
    nueva.estado = ESTADO_CARRERA_ACTIVA;

    limpiarBuffer();
    printf("Circuito: ");
    leerCadena(nueva.circuito, TAM_NOMBRE_CIRCUITO);

    do
    {
        printf("Fecha (AAAAMMDD): ");
        scanf("%llu", &nueva.fecha);
    }
    while (!esFechaValida(nueva.fecha));

    ret = cargarResultadosCarreraAleatorios(rutaPiloto, &nueva, comparar, pts);

    if (ret == TODO_OK)
    {
        fseek(fCarrera, 0, SEEK_END);
        escribirCarrera(fCarrera, &nueva);
        printf("[OK] Carrera #%d en %s registrada con %d pilotos.\n",
               nueva.id, nueva.circuito, nueva.cant_resultados);
    }

    destruirCarrera(&nueva);
    fclose(fCarrera);
    return ret;
}

/* =========================================================
   Alta de carrera — ingreso manual
   ========================================================= */

/*
 * registrarCarreraManual
 * El usuario ingresa posicion por posicion el ID de piloto.
 * Los puntos se asignan automaticamente segun la tabla F1.
 */
int registrarCarreraManual(const char* rutaCarrera, const char* rutaPiloto,
                            Comparar comparar, const Puntos* pts)
{
    Carrera  nueva;
    tVector  vIds;
    FILE*    fCarrera;
    unsigned idIngresado;
    int      cantPilotos;
    int      i;
    int      puntos;
    Resultado res;

    nueva.resultados.vec  = NULL;
    nueva.cant_resultados = 0;

    fCarrera = fopen(rutaCarrera, "ab+");
    if (!fCarrera)
        return ERR_ARCH;

    nueva.id     = generarIdCarrera(fCarrera);
    nueva.estado = ESTADO_CARRERA_ACTIVA;

    limpiarBuffer();
    printf("Circuito: ");
    leerCadena(nueva.circuito, TAM_NOMBRE_CIRCUITO);

    do
    {
        printf("Fecha (AAAAMMDD): ");
        scanf("%llu", &nueva.fecha);
    }
    while (!esFechaValida(nueva.fecha));

    /* Cargar IDs validos para validar el ingreso */
    if (crearVector(&vIds, sizeof(unsigned), MAX_PILOTOS_CARRERA) != TODO_OK)
    {
        fclose(fCarrera);
        return SIN_MEM;
    }
    cargarVectorPilotoActivos(rutaPiloto, &vIds, comparar);

    do
    {
        printf("Cantidad de pilotos que terminaron la carrera (max %d): ", MAX_PILOTOS_CARRERA);
        scanf("%d", &cantPilotos);
    }
    while (cantPilotos < 1 || cantPilotos > MAX_PILOTOS_CARRERA);

    if (inicializarCarrera(&nueva, cantPilotos) != TODO_OK)
    {
        destruirVector(&vIds);
        fclose(fCarrera);
        return SIN_MEM;
    }

    for (i = 0; i < cantPilotos; i++)
    {
        do
        {
            printf("Posicion %d — ID piloto: ", i + 1);
            scanf("%u", &idIngresado);
        }
        while (!busquedaBinariaVector(&vIds, &idIngresado, comparar));

        puntos = obtenerPuntosPorPosicion(pts, i + 1);

        res.id_piloto = (int)idIngresado;
        res.puntos    = puntos;

        insertarFinalVector(&nueva.resultados, &res);
        nueva.cant_resultados++;
    }

    fseek(fCarrera, 0, SEEK_END);
    escribirCarrera(fCarrera, &nueva);

    printf("[OK] Carrera #%d en %s registrada con %d pilotos.\n",
           nueva.id, nueva.circuito, nueva.cant_resultados);

    destruirCarrera(&nueva);
    destruirVector(&vIds);
    fclose(fCarrera);
    return TODO_OK;
}

/* =========================================================
   Recalcular puntos de pilotos
   ========================================================= */

/*
 * recalcularPuntosPilotos
 * Carga todos los pilotos en un vector, resetea sus puntos a 0,
 * recorre carrera.dat acumulando puntos por piloto y guarda el vector.
 */
int recalcularPuntosPilotos(const char* rutaCarrera, const char* rutaPiloto)
{
    tVector   vPilotos;
    Carrera   carrera;
    Piloto*   pPiloto;
    Resultado* r;
    FILE*     fCarrera;
    unsigned  id;
    int       i;

    if (crearVector(&vPilotos, sizeof(Piloto), CAP_MAX) != TODO_OK)
        return SIN_MEM;

    if (cargarVectorDesdeBin(rutaPiloto, &vPilotos) != TODO_OK)
    {
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    /* Resetear puntos */
    for (i = 0; i < (int)vPilotos.ce; i++)
    {
        pPiloto = (Piloto*)obtenerElementoVector(&vPilotos, i);
        pPiloto->puntos_acumulados = 0;
    }

    fCarrera = fopen(rutaCarrera, "rb");
    if (!fCarrera)
    {
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    while (leerCarrera(fCarrera, &carrera) == TODO_OK)
    {
        if (carrera.estado == ESTADO_CARRERA_ACTIVA)
        {
            for (i = 0; i < carrera.cant_resultados; i++)
            {
                r  = (Resultado*)obtenerElementoVector(&carrera.resultados, i);
                id = (unsigned)r->id_piloto;

                pPiloto = (Piloto*)busquedaBinariaVector(&vPilotos, &id, compararUnsigned);
                if (pPiloto)
                    pPiloto->puntos_acumulados += (unsigned)r->puntos;
            }
        }
        destruirCarrera(&carrera);
    }

    fclose(fCarrera);

    guardarVectorEnBin(rutaPiloto, &vPilotos);
    destruirVector(&vPilotos);

    return TODO_OK;
}

/* =========================================================
   Exportar carreras a texto
   ========================================================= */

/*
 * exportarCarrerasTxt
 * Formato: id,circuito,fecha,estado,cant_resultados,idPiloto:puntos-...
 */
int exportarCarrerasTxt(const char* rutaBin, const char* rutaTxtExportado)
{
    FILE*      fBin;
    FILE*      fTxt;
    Carrera    c;
    Resultado* r;
    int        i;
    int        exportadas = 0;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
    {
        printf("[!] No se encontro el archivo de carreras.\n");
        return ERR_ARCH;
    }

    fTxt = fopen(rutaTxtExportado, "wt");
    if (!fTxt)
    {
        fclose(fBin);
        return ERR_ARCH;
    }

    while (leerCarrera(fBin, &c) == TODO_OK)
    {
        fprintf(fTxt, "%d,%s,%llu,%d,%d",
                c.id, c.circuito, c.fecha, c.estado, c.cant_resultados);

        for (i = 0; i < c.cant_resultados; i++)
        {
            r = (Resultado*)obtenerElementoVector(&c.resultados, i);
            fprintf(fTxt, "%c%d:%d",
                    (i == 0 ? ',' : '-'), r->id_piloto, r->puntos);
        }

        fprintf(fTxt, "\n");
        exportadas++;
        destruirCarrera(&c);
    }

    fclose(fBin);
    fclose(fTxt);

    printf("[OK] Se exportaron %d carreras.\n", exportadas);
    return TODO_OK;
}

/* =========================================================
   ABM — Baja logica y modificacion
   ========================================================= */

/*
 * bajaCarrera
 * Busca la carrera por ID, la marca como inactiva y la reescribe.
 * Registra la baja en el archivo de texto de bajas.
 * ATENCION: como el formato es variable (cant_resultados puede variar),
 * se reescribe el archivo completo con la carrera modificada.
 */
int bajaCarrera(const char* rutaBin, const char* rutaBajasTxt)
{
    FILE*    fBin;
    FILE*    fTmp;
    FILE*    fBajas;
    Carrera  c;
    int      idBaja;
    int      encontrado = 0;

    printf("\n--- BAJA DE CARRERA ---\n");
    printf("ID de la carrera: ");
    scanf("%d", &idBaja);

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    fTmp = fopen("archivos/tmp_carreras.dat", "wb");
    if (!fTmp)
    {
        fclose(fBin);
        return ERR_ARCH;
    }

    while (leerCarrera(fBin, &c) == TODO_OK)
    {
        if (c.id == idBaja)
        {
            encontrado    = 1;
            c.estado      = ESTADO_CARRERA_INACTIVA;
        }
        escribirCarrera(fTmp, &c);
        destruirCarrera(&c);
    }

    fclose(fBin);
    fclose(fTmp);

    if (!encontrado)
    {
        remove("archivos/tmp_carreras.dat");
        printf("[!] Carrera con ID %d no encontrada.\n", idBaja);
        return NO_ENCONTRADO;
    }

    remove(rutaBin);
    rename("archivos/tmp_carreras.dat", rutaBin);

    fBajas = fopen(rutaBajasTxt, "at");
    if (fBajas)
    {
        fprintf(fBajas, "CARRERA|%d|INACTIVA\n", idBaja);
        fclose(fBajas);
    }

    printf("[OK] Carrera ID %d dada de baja.\n", idBaja);
    return TODO_OK;
}

/*
 * modificarCarrera
 * Permite cambiar circuito o fecha de una carrera existente.
 * Reescribe el archivo completo con la carrera modificada.
 */
int modificarCarrera(const char* rutaBin)
{
    FILE*   fBin;
    FILE*   fTmp;
    Carrera c;
    int     idMod;
    int     encontrado = 0;
    int     campo;

    printf("\n--- MODIFICAR CARRERA ---\n");
    printf("ID de la carrera: ");
    scanf("%d", &idMod);

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    fTmp = fopen("archivos/tmp_carreras.dat", "wb");
    if (!fTmp)
    {
        fclose(fBin);
        return ERR_ARCH;
    }

    while (leerCarrera(fBin, &c) == TODO_OK)
    {
        if (c.id == idMod)
        {
            encontrado = 1;

            printf("  [1] Circuito : %s\n", c.circuito);
            printf("  [2] Fecha    : %llu\n", c.fecha);
            printf("  [0] Salir\n");
            printf("Campo a modificar: ");
            scanf("%d", &campo);

            while (campo != 0)
            {
                limpiarBuffer();
                switch (campo)
                {
                case 1:
                    printf("Nuevo circuito: ");
                    leerCadena(c.circuito, TAM_NOMBRE_CIRCUITO);
                    break;
                case 2:
                    do
                    {
                        printf("Nueva fecha (AAAAMMDD): ");
                        scanf("%llu", &c.fecha);
                    }
                    while (!esFechaValida(c.fecha));
                    break;
                default:
                    printf("[!] Campo invalido.\n");
                    break;
                }
                printf("  [1] Circuito : %s\n", c.circuito);
                printf("  [2] Fecha    : %llu\n", c.fecha);
                printf("  [0] Salir\n");
                printf("Campo a modificar: ");
                scanf("%d", &campo);
            }
        }
        escribirCarrera(fTmp, &c);
        destruirCarrera(&c);
    }

    fclose(fBin);
    fclose(fTmp);

    if (!encontrado)
    {
        remove("archivos/tmp_carreras.dat");
        printf("[!] Carrera con ID %d no encontrada.\n", idMod);
        return NO_ENCONTRADO;
    }

    remove(rutaBin);
    rename("archivos/tmp_carreras.dat", rutaBin);

    printf("[OK] Carrera ID %d modificada.\n", idMod);
    return TODO_OK;
}

int  mostrarTodasLasCarreras(const char* rutaBin)
{
    FILE*   f;
    Carrera c;

    f = fopen(rutaBin, "rb");
    if (!f)
    {
        printf("[!] No hay carreras registradas.\n");
        return ERR_ARCH;
    }

    while (leerCarrera(f, &c) == TODO_OK)
    {
        mostrarCarrera(&c);
        destruirCarrera(&c);
    }

    fclose(f);
    return TODO_OK;
}
