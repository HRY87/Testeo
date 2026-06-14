#ifndef CARRERA_H_INCLUDED
#define CARRERA_H_INCLUDED

#include "utilidades.h"
#include "vector.h"
#include "puntos.h"

#define RUTA_CARRERA_BIN            "archivos/carreras.dat"
#define RUTA_CARRERA_TXT            "archivos/carreras.txt"
#define RUTA_CARRERA_EXP_TXT        "archivos/carrera_exportada.txt"
#define RUTA_BAJAS_CARRERA_TXT      "archivos/bajas_carreras.txt"

#define ESTADO_CARRERA_INACTIVA     0
#define ESTADO_CARRERA_ACTIVA       1
#define TAM_NOMBRE_CIRCUITO         20
#define MAX_PILOTOS_CARRERA         20  /* tope de validacion para ingreso manual */
#define MAX_CARRERAS_TEMPORADA      30
#define POS_LIMITE_PUNTOS_CARRERA   10

/* =========================================================
   TDA Resultado — una fila de la antigua matriz resultados
   ========================================================= */
typedef struct
{
    int id_piloto;
    int puntos;
} Resultado;

/* =========================================================
   TDA Carrera
   resultados es ahora un tVector de Resultado (heap dinamico)
   ========================================================= */
typedef struct
{
    int                id;
    char               circuito[TAM_NOMBRE_CIRCUITO];
    unsigned long long fecha;
    int                estado;
    int                cant_resultados;
    tVector            resultados;   /* tVector de Resultado */
} Carrera;

/* =========================================================
   Ciclo de vida del TDA Carrera
   ========================================================= */
int  inicializarCarrera(Carrera* c, int cantPilotos);
void destruirCarrera(Carrera* c);

/* =========================================================
   Serializacion — reemplazan fread/fwrite directo
   ========================================================= */
int escribirCarrera(FILE* f, const Carrera* c);
int leerCarrera(FILE* f, Carrera* c);

/* =========================================================
   Funciones del modulo
   ========================================================= */
int  registrarCarreraAleatoria(const char* rutaCarrera, const char* rutaPiloto, Comparar comparar, const Puntos* pts);
int  registrarCarreraManual(const char* rutaCarrera, const char* rutaPiloto, Comparar comparar, const Puntos* pts);
int  cargarResultadosCarreraAleatorios (const char* rutaPiloto, Carrera* nueva, Comparar comparar, const Puntos* pts);
int  generarIdCarrera(FILE* fCarrera);
int  recalcularPuntosPilotos(const char* rutaCarrera, const char* rutaPiloto);
int  exportarCarrerasTxt(const char* rutaBin, const char* rutaTxtExportado);

/* Puntero a funcion */
void mostrarCarrera (const void* dato);

/* ABM */
int bajaCarrera(const char* rutaBin, const char* rutaBajasTxt);
int modificarCarrera(const char* rutaBin);

int  mostrarTodasLasCarreras(const char* rutaBin);
#endif // CARRERA_H_INCLUDED
