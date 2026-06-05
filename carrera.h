#ifndef CARRERA_H_INCLUDED
#define CARRERA_H_INCLUDED

#include "utilidades.h"
#include "vector.h"
#include "resultado.h"
#include "puntos.h"
/* =========================================================
   Rutas de archivos del TDA Carrera
   ========================================================= */
#define RUTA_CARRERA_BIN  "archivos/carrera.dat"
#define RUTA_CARRERA_TXT  "archivos/carrera.txt"

/* =========================================================
   Estado de la carrera
   ========================================================= */
#define ESTADO_CARRERA_ACTIVA   1
#define ESTADO_CARRERA_INACTIVA 0

/* =========================================================
   Constantes
   ========================================================= */
#define TAM_NOMBRE_CIRCUITO       20
#define MAX_PILOTOS_CARRERA       20
#define MAX_CARRERAS_TEMPORADA    30

/* =========================================================
   Estructura de ENCABEZADO de carrera (parte fija en disco)

   En carrera.bin cada carrera se serializa como:
     [ CarreraHeader ][ ResultadoPiloto x cant_resultados ]

   Esto permite un numero variable de resultados por carrera
   sin desperdiciar memoria con una matriz fija.
   ========================================================= */
typedef struct
{
    int                id;
    char               circuito[TAM_NOMBRE_CIRCUITO];
    unsigned long long fecha;
    int                estado;
    int                cant_resultados;
} CarreraHeader;

/* =========================================================
   Estructura COMPLETA de Carrera (representacion en memoria)

   El tVector resultados vive en heap y se libera con
   destruirCarrera(). NUNCA se serializa el puntero vec
   directamente: usar escribirCarrera() / leerCarrera().
   ========================================================= */
typedef struct
{
    CarreraHeader info;
    tVector       resultados;   /* elementos: ResultadoPiloto */
} Carrera;

/* =========================================================
   Ciclo de vida de Carrera en memoria
   ========================================================= */

/* Inicializa el tVector interno; debe llamarse antes de usar */
int  inicializarCarrera(Carrera* c, int capacidad);

/* Libera el tVector interno */
void destruirCarrera(Carrera* c);

/* =========================================================
   Serializacion (disco <-> memoria)

   Estas funciones son el UNICO punto de contacto con el .bin.
   ========================================================= */

/* Escribe un Carrera completo (header + resultados) al final del archivo */
int escribirCarrera(FILE* fCarrera, const Carrera* c);

/* Lee el siguiente Carrera del archivo en la posicion actual.
   El tVector de 'c' debe estar ya inicializado con suficiente capacidad.
   Retorna TODO_OK, ERR_ARCH (fin de archivo) o SIN_MEM. */
int leerCarrera(FILE* fCarrera, Carrera* c);

/* =========================================================
   Alta de carrera
   ========================================================= */

/* Modo automatico: posiciones generadas con Fisher-Yates */
int registrarCarreraAleatoria(const char*         rutaCarrera,
                              const char*         rutaPiloto,
                              Comparar            comparar,
                              const ConfigPuntos* cfg);

/* Modo manual: el usuario ingresa cada piloto, estado y puntos */
int registrarCarreraManual(const char* rutaCarrera,
                           const char* rutaPiloto,
                           Comparar    comparar);

/* Generador de ID (ultimo id + 1, o 1 si el archivo esta vacio) */
int generarIdCarrera(FILE* fCarrera);

/* Carga los IDs de pilotos activos en un vector y los mezcla */
int cargarResultadosAleatorios(const char* rutaPiloto,
                               Carrera*    c,
                               Comparar    comparar);

/* =========================================================
   Actualizacion de puntos en piloto.bin
   ========================================================= */

/* Recalcula puntos desde cero (recorre todo carrera.bin) */
int recalcularPuntosPilotos(const char* rutaCarrera,
                            const char* rutaPiloto,
                            Filter      filtrar,
                            Reduce      reducir);

/* Suma solo los puntos de la ultima carrera (mas eficiente) */
int actualizarPuntosUltimaCarrera(const char* rutaCarrera,
                                  const char* rutaPiloto,
                                  Filter      filtrar,
                                  Reduce      reducir);

/* =========================================================
   Punteros a funcion del TDA Carrera
   ========================================================= */

/* Mostrar: imprime una Carrera con sus resultados */
void mostrarCarrera(const void* dato);

/* Mostrar completa (con resultados del tVector) */
void mostrarCarreraCompleta(const Carrera* c);

/* Recorre carrera.bin y muestra todas las carreras completas */
int  listarTodasLasCarreras(const char* rutaCarrera);

/* Filter: retorna 1 si la carrera esta activa */
int  filterEsCarreraActiva(const void* dato);

/* Reduce: acumula los puntos de una carrera en un tVector de Pilotos */
int  reduceAcumularPuntosCarrera(void* acumulador, const void* dato);

#endif // CARRERA_H_INCLUDED
