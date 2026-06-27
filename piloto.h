#ifndef PILOTO_H_INCLUDED
#define PILOTO_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

#define RUTA_PILOTO_TXT         "pilotos.txt"
#define RUTA_PILOTO_BIN         "pilotos.dat"
#define RUTA_PILOTO_EXP_TXT     "piloto_exportado.txt"
#define RUTA_BAJAS_PILOTO_TXT   "bajas_pilotos.txt"

#define ESTADO_ACTIVO_PILOTO        'A'
#define ESTADO_RETIRADO_PILOTO      'R'
#define ESTADO_SUSPENDIDO_PILOTO    'S'
#define TAM_NOMBRE_PILOTO           30
#define TAM_NACIONALIDAD            30


/* Columnas de la matriz resultados[][] */
#define COL_ID_PILOTO               0
#define COL_PUNTOS                  1

typedef struct
{
    unsigned           id;
    char               nombre[TAM_NOMBRE_PILOTO];
    char               nacionalidad[TAM_NACIONALIDAD];
    unsigned           id_escuderia;
    unsigned           puntos_acumulados;
    char               estado;           /* A, R o S */
    unsigned long long fechaNacimiento;
} Piloto;

/* Referencia liviana usada en RankingPiloto: puntos + posicion en .dat */
typedef struct
{
    unsigned puntos;
    long     offset;
} PilotoRef;

/* Generacion del lote inicial */
int generarArchivoPilotosTxt(const char* rutaTxt);

/* Listado */
int listarPilotos(const char* rutaBin);
int rankingPiloto(const char* rutaBin);
int listarPilotosPorEscuderia(const char* rutaPiloto, const char* rutaEscuderia, unsigned idEscuderia);

/* Punteros a funcion del TDA Piloto */
void mostrarPiloto(const void* dato);                   /* Mostrar */
int trozarPilotoTxt(char* linea, void* reg);            /* TxtABin*/
int pilotoBinATxt(const void* dato, FILE* archTxt);     /* BinATxt*/
int escribirPilotoTxt(void* archivo, const void* dato); /* Accion */
int esPilotoActivos(const void* dato);                  /* Filter*/
int sumarPuntos(void* acumulador, const void* dato);    /* Reduce */
int extraerIdPuntos(void* dest, const void* orig);      /* Map*/
int cmp_desc(const void* a, const void* b);             /* Comparar*/

/* Utilitaria para simulacion de carreras */
int cargarVectorPilotoActivos(const char* rutaBin, tVector* vIds, Comparar comparar);

/*Exportar datos de temporada*/
int exportarPilotosTxt(const char* rutaBin, const char* rutaTxtExportado);

/* ABM */
int altaPiloto(const char* rutaBin);
int bajaPiloto(const char* rutaBin, const char* rutaBajasTxt);
int modificarPiloto(const char* rutaBin);

#endif /* PILOTO_H_INCLUDED */
