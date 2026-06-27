#ifndef ESCUDERIA_H_INCLUDED
#define ESCUDERIA_H_INCLUDED

#include <stdio.h>
#include "utilidades.h"

#define RUTA_ESCUDERIA_BIN      "escuderias.dat"
#define RUTA_ESCUDERIA_TXT      "escuderias.txt"
#define RUTA_ESCUDERIA_EXP_TXT  "escuderia_exportada.txt"
#define RUTA_BAJAS_ESCUDERIA    "bajas_escuderias.txt"

#define ESTADO_ESCUDERIA_ACTIVA   1
#define ESTADO_ESCUDERIA_INACTIVA 0

#define TAM_CODIGO           4
#define TAM_NOMBRE_ESCUDERIA 30
#define TAM_PAIS             50

typedef struct
{
    unsigned id;
    char     codigo[TAM_CODIGO];
    char     nombre[TAM_NOMBRE_ESCUDERIA];
    char     pais[TAM_PAIS];
    int      estado;   /* 1: activa, 0: inactiva */
} Escuderia;

/* Generacion del lote inicial */
int generarArchivoEscuderiasTxt(const char* rutaTxt);

/* Punteros a funcion del TDA Escuderia */
int  trozarEscuderiaTxt  (char* linea, void* reg);          /* TxtABin */
int  escuderiaBinATxt    (const void* dato, FILE* archTxt); /* BinATxt */
int  escribirEscuderiaTxt(void* archTxt, const void* dato); /* Accion  */
void mostrarEscuderia    (const void* dato);                /* Mostrar */
int  esEscuderiaActiva   (const void* dato);                /* Filter  */

/* ABM */
int altaEscuderia     (const char* rutaBin);
int bajaEscuderia     (const char* rutaBin, const char* rutaBajasTxt);
int modificarEscuderia(const char* rutaBin);

/* Validacion: retorna 1 si el id existe en el .bin, 0 si no. */
int esEscuderiaValida(const char* rutaBin, unsigned id);

#endif /* ESCUDERIA_H_INCLUDED */
