#ifndef PILOTO_H_INCLUDED
#define PILOTO_H_INCLUDED

#include "vector.h"

/* =========================================================
   Rutas de archivos del TDA Piloto
   ========================================================= */
#define RUTA_PILOTO_BIN  "archivos/piloto.dat"
#define RUTA_PILOTO_TXT  "archivos/piloto.txt"

/* =========================================================
   Estados del piloto (campo 'estado')
   ========================================================= */
#define ESTADO_ACTIVO_PILOTO     'A'
#define ESTADO_RETIRADO_PILOTO   'R'
#define ESTADO_SUSPENDIDO_PILOTO 'S'

/* =========================================================
   Tamanios de campos de cadena
   ========================================================= */
#define TAM_NOMBRE_PILOTO  30
#define TAM_NACIONALIDAD   30

/* =========================================================
   Estructura Piloto
   ========================================================= */
typedef struct
{
    unsigned           id;
    char               nombre[TAM_NOMBRE_PILOTO];
    char               nacionalidad[TAM_NACIONALIDAD];
    unsigned           id_escuderia;
    unsigned           puntos_acumulados;
    char               estado;
    unsigned long long fechaNacimiento;
} Piloto;

/* =========================================================
   Generacion de archivos (inicializacion)
   ========================================================= */
int generarArchivoPilotosTxt(const char* rutaTxt);

/* =========================================================
   Operaciones con vector de pilotos
   ========================================================= */
int cargarVectorPilotoActivos(const char* rutaBin,
                              tVector*    vIds,
                              Comparar    comparar);

/* =========================================================
   ABM sobre piloto.dat
   ========================================================= */

/* Alta: pide datos al usuario y agrega el piloto al .dat.
   Genera el ID automaticamente (maximo existente + 1). */
int altaPiloto(const char* rutaBin);

/* Baja logica: cambia estado a 'R' (Retirado).
   No elimina el registro fisicamente. */
int bajaPiloto(const char* rutaBin);

/* Modificacion: busca por ID y permite editar campos. */
int modificarPiloto(const char* rutaBin);

/* =========================================================
   Exportacion a texto al finalizar la temporada
   ========================================================= */
int exportarPilotosATxt(const char* rutaBin, const char* rutaTxt);

/* =========================================================
   Punteros a funcion del TDA Piloto
   ========================================================= */
int  trozarPilotoTxt(char* linea, void* reg);
void pilotoBinATxt(const void* dato, FILE* archTxt);
void mostrarPiloto(const void* dato);
int  escribirPilotoTxt(void* archTxt, const void* dato);
int  esPilotoActivo(const void* dato);
int  sumarPuntos(void* acumulador, const void* dato);
int  extraerIdPuntos(void* dest, const void* orig);

/* Callbacks para modificarRegistroEnBin */
void pilotoObtenerClave(const void* registro, void* claveDestino);
#endif // PILOTO_H_INCLUDED
