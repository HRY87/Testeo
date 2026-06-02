#include <stdio.h>
#include "sancion.h"
#include "piloto.h"

/* =========================================================
                Funciones del TDA Sancion
   ========================================================= */

/**
 * registrarSancion
 * Persiste una Sancion en modo append en sancion.bin.
 */
int registrarSancion(const char* rutaSancion, const Sancion* s)
{
    FILE* fBin;

    fBin = fopen(rutaSancion, "ab");
    if (!fBin)
        return ERR_ARCH;

    fwrite(s, sizeof(Sancion), 1, fBin);
    fclose(fBin);

    return TODO_OK;
}

/**
 * contarDSQPiloto
 * Cuenta cuantas sanciones tiene el piloto en sancion.bin.
 * Retorna la cantidad o -1 si el archivo no existe.
 */
int contarDSQPiloto(const char* rutaSancion, unsigned idPiloto)
{
    Sancion s;
    int     cant = 0;
    FILE*   fBin;

    fBin = fopen(rutaSancion, "rb");
    if (!fBin)
        return 0;   /* Sin archivo = 0 sanciones previas */

    while (fread(&s, sizeof(Sancion), 1, fBin) == 1)
    {
        if (s.id_piloto == idPiloto)
            cant++;
    }

    fclose(fBin);
    return cant;
}

/**
 * aplicarSancionDSQ
 *
 * Flujo:
 *   1. Construye y persiste la Sancion.
 *   2. Si tipo incluye SANCION_PUNTOS:
 *      abre piloto.bin, busca el registro por id (recorrido
 *      secuencial), descuenta PUNTOS_PENALIZACION_DSQ y
 *      reescribe el registro en la misma posicion.
 *   3. Cuenta DSQ acumulados. Si >= UMBRAL_DSQ_SUSPENSION
 *      cambia estado del piloto a 'S' y lo reescribe.
 *
 * Nota sobre la busqueda en .bin:
 *   Se hace un recorrido secuencial porque piloto.bin NO
 *   garantiza orden (se carga desde txt en orden de archivo).
 *   Si en el futuro se agrega un indice, reemplazar por
 *   busqueda directa via fseek.
 */
int aplicarSancionDSQ(const char*        rutaSancion,
                      const char*        rutaPiloto,
                      unsigned           idPiloto,
                      unsigned           idCarrera,
                      unsigned long long fechaCarrera,
                      int                tipo,
                      int                carreras)
{
    Sancion s;
    Piloto  piloto;
    FILE*   fPiloto;
    long    posEncontrado;
    int     encontrado;
    int     totalDSQ;

    /* --- 1. Construir y persistir la sancion --- */
    s.id_piloto  = idPiloto;
    s.id_carrera = idCarrera;
    s.tipo       = tipo;
    s.puntos     = (tipo == SANCION_PUNTOS || tipo == SANCION_AMBAS)
                   ? PUNTOS_PENALIZACION_DSQ : 0;
    s.carreras   = (tipo == SANCION_CARRERA || tipo == SANCION_AMBAS)
                   ? carreras : 0;
    s.fecha      = fechaCarrera;

    if (registrarSancion(rutaSancion, &s) != TODO_OK)
        return ERR_ARCH;

    /* --- 2 y 3. Modificar piloto en .bin --- */
    fPiloto = fopen(rutaPiloto, "r+b");
    if (!fPiloto)
        return ERR_ARCH;

    encontrado    = 0;
    posEncontrado = 0;

    /* Recorrido secuencial para localizar el piloto */
    while (fread(&piloto, sizeof(Piloto), 1, fPiloto) == 1)
    {
        if (piloto.id == idPiloto)
        {
            /* Guardar posicion para reescribir luego */
            posEncontrado = ftell(fPiloto) - (long)sizeof(Piloto);
            encontrado    = 1;
            break;
        }
    }

    if (!encontrado)
    {
        fclose(fPiloto);
        return NO_ENCONTRADO;
    }

    /* Descontar puntos si corresponde */
    if (tipo == SANCION_PUNTOS || tipo == SANCION_AMBAS)
    {
        if (piloto.puntos_acumulados >= (unsigned)PUNTOS_PENALIZACION_DSQ)
            piloto.puntos_acumulados -= (unsigned)PUNTOS_PENALIZACION_DSQ;
        else
            piloto.puntos_acumulados = 0;  /* no queda en negativo */
    }

    /* Verificar si supera el umbral de suspension */
    totalDSQ = contarDSQPiloto(rutaSancion, idPiloto);
    if (totalDSQ >= UMBRAL_DSQ_SUSPENSION)
        piloto.estado = ESTADO_SUSPENDIDO_PILOTO;

    /* Reescribir el registro en la misma posicion */
    fseek(fPiloto, posEncontrado, SEEK_SET);
    fwrite(&piloto, sizeof(Piloto), 1, fPiloto);

    fclose(fPiloto);
    return TODO_OK;
}

/* =========================================================
   Punteros a funcion del TDA Sancion
   ========================================================= */

void mostrarSancion(const void* dato)
{
    const Sancion* s = (const Sancion*)dato;
    const char*    tipoStr;

    switch (s->tipo)
    {
        case SANCION_PUNTOS:  tipoStr = "Descuento puntos";       break;
        case SANCION_CARRERA: tipoStr = "Saltear carrera(s)";     break;
        case SANCION_AMBAS:   tipoStr = "Puntos + carrera(s)";    break;
        default:              tipoStr = "Desconocido";             break;
    }

    printf("Piloto ID:%-4u  Carrera:%-3u  Tipo:%-20s  "
           "Puntos:%-4d  Carreras:%-2d  Fecha:%llu\n",
           s->id_piloto, s->id_carrera, tipoStr,
           s->puntos, s->carreras, s->fecha);
}

void sancionBinATxt(const void* dato, FILE* archTxt)
{
    const Sancion* s = (const Sancion*)dato;

    fprintf(archTxt, "%u%c%u%c%d%c%d%c%d%c%llu\n",
            s->id_piloto,  SEP_TXT,
            s->id_carrera, SEP_TXT,
            s->tipo,       SEP_TXT,
            s->puntos,     SEP_TXT,
            s->carreras,   SEP_TXT,
            s->fecha);
}
