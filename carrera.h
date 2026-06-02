#ifndef CARRERA_H_INCLUDED
#define CARRERA_H_INCLUDED

#include "utilidades.h"
#include "vector.h"

/* =========================================================
            Rutas de archivos del TDA Carrera
   ========================================================= */
#define RUTA_CARRERA_BIN  "carrera.bin"
#define RUTA_CARRERA_TXT  "carrera.txt"

/* =========================================================
                Estado de la carrera
   ========================================================= */
#define ESTADO_CARRERA_ACTIVA   1
#define ESTADO_CARRERA_INACTIVA 0

/* =========================================================
                Constantes de la estructura
   ========================================================= */
#define TAM_NOMBRE_CIRCUITO      20
#define MAX_PILOTOS_CARRERA      20
#define MAX_CARRERAS_TEMPORADA   30
#define POS_LIMITE_PUNTOS_CARRERA 10  /* Solo top 10 suman puntos */

/* =========================================================
   Estructura Carrera
   resultados[i][0] = id del piloto en posicion i
   resultados[i][1] = puntos obtenidos en posicion i
   ========================================================= */
typedef struct
{
    int                id;
    char               circuito[TAM_NOMBRE_CIRCUITO];
    unsigned long long fecha;
    int                estado;
    int                cant_resultados;
    int                resultados[MAX_PILOTOS_CARRERA][2];
} Carrera;

/* =========================================================
                    Alta de carrera
   ========================================================= */

/* Solicita datos al usuario, genera resultados aleatorios
   y persiste la nueva carrera en carrera.bin */
int registrarCarrera(const char* rutaCarrera,
                     const char* rutaPiloto,
                     Comparar    comparar);

/* Genera el ID de la proxima carrera (ultimo id + 1) */
int generarIdCarrera(FILE* fCarrera);
int cargarResultadosCarreraAleatorios(const char* rutaPiloto, Carrera* nueva, Comparar comparar);

/* =========================================================
                Actualizacion de puntos
   ========================================================= */

/* Recalcula puntos de todos los pilotos desde cero
   (recorre todo carrera.bin). Usar al inicio de temporada
   o al cancelar una carrera. */
int recalcularPuntosPilotos(const char* rutaCarrera,
                            const char* rutaPiloto,
                            Filter      filtrar,
                            Reduce      reducir);

/* Actualiza puntos sumando solo la ultima carrera registrada.
   Mas eficiente: usar inmediatamente despues de registrar. */
int actualizarPuntosUltimaCarrera(const char* rutaCarrera,
                                  const char* rutaPiloto,
                                  Filter      filtrar,
                                  Reduce      reducir);

/* =========================================================
            Punteros a funcion del TDA Carrera
   ========================================================= */

/* Mostrar: imprime una Carrera con sus resultados */
void mostrarCarrera(const void* dato);

/* Filter: retorna 1 si la carrera esta activa */
int  filterEsCarreraActiva(const void* dato);

/* Reduce: acumula los puntos de una carrera en un tVector de Pilotos */
int  reduceAcumularPuntosCarrera(void* acumulador, const void* dato);

#endif // CARRERA_H_INCLUDED
