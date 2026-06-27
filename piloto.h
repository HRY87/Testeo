#ifndef PILOTO_H_INCLUDED
#define PILOTO_H_INCLUDED

#include "vector.h"
#include <stdlib.h>
#include <stdio.h>

#define RUTA_PILOTO_TXT                "pilotos.txt"
#define RUTA_PILOTO_BIN                "pilotos.dat"
#define RUTA_PILOTO_EXP_TXT            "piloto_exportado.txt"
#define RUTA_BAJAS_PILOTO_TXT          "bajas_pilotos.txt"

#define ESTADO_ACTIVO_PILOTO            'A'
#define ESTADO_RETIRADO_PILOTO          'R'
#define ESTADO_SUSPENDIDO_PILOTO        'S'
#define TAM_NOMBRE_PILOTO               30
#define TAM_NACIONALIDAD                30
#define CAP_MAX                         64
#define ERR_MEM                         1

//No se donde #$!"#! meter esto
#define COL_ID_PILOTO               0
#define COL_PUNTOS                  1

typedef struct {
    unsigned id;
    char nombre[TAM_NOMBRE_PILOTO];
    char nacionalidad[TAM_NACIONALIDAD];
    unsigned id_escuderia;
    unsigned puntos_acumulados;
    char estado;    /** A, R o S **/
    unsigned long long fechaNacimiento;
}Piloto;

typedef struct {
    unsigned puntos;
    long offset;
} PilotoRef;


int generarArchivoPilotosTxt(const char* rutaTxt);
int listarPilotos(const char* rutaBin);
int RankingPiloto(const char* rutaBin);
int exportarPilotosTxt(const char* rutaBin, const char* rutaTxtExportado);

/**Funciones para manejo de datos TDA vector**/
void mostrarPiloto(const void* dato);
int cmp_desc(const void* a, const void* b);
int trozarPilotoTxt(char* linea, void* reg);
int esPilotoActivos(const void* dato);
int sumarPuntos(void* acumulador, const void* dato);
int extraerIdPuntos(void* dest, const void* orig);
int pilotoBinATxt(const void* dato, FILE* archTxt);
int cargarVectorPilotoActivos(const char* rutaBin, tVector* vIds, Comparar comparar);
int listarPilotosPorEscuderia(const char* rutaPiloto, const char* rutaEscuderia, unsigned idEscuderia);

/* --- ABM --- */
int altaPiloto(const char* rutaBin);
int bajaPiloto(const char* rutaBin, const char* rutaBajasTxt);
int modificarPiloto(const char* rutaBin);

#endif // PILOTO_H_INCLUDED
