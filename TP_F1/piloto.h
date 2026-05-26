#ifndef PILOTO_H_INCLUDED
#define PILOTO_H_INCLUDED

#include "vector.h"

#define RUTA_PILOTO_TXT                "piloto.txt"
#define RUTA_PILOTO_BIN                "piloto.bin"
#define ESTADO_ACTIVO_PILOTO            'A'
#define ESTADO_RETIRADO_PILOTO          'R'
#define ESTADO_SUSPENDIDO_PILOTO        'S'
#define TAM_NOMBRE_PILOTO               30
#define TAM_NACIONALIDAD                30

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

int generarArchivoPilotosTxt(const char* rutaTxt);
int cargarArchivoPilotos(const char* pathTxt, const char* rutaBin);
size_t listarPilotos(const char* rutaBin);

/**Funciones para manejo de datos TDA vector**/
int esPilotoActivos(const void* dato);
int sumarPuntos(void* acumulador, const void* dato);
int extraerIdPuntos(void* dest, const void* orig);

int cargarVectorPilotoActivos(const char* rutaBin, tVector* vIds, Comparar comparar);
#endif // PILOTO_H_INCLUDED
