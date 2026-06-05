#ifndef ESCUDERIA_H_INCLUDED
#define ESCUDERIA_H_INCLUDED

#include "utilidades.h"

/* =========================================================
   Rutas de archivos del TDA Escuderia
   ========================================================= */
#define RUTA_ESCUDERIA_BIN  "archivos/escuderia.dat"
#define RUTA_ESCUDERIA_TXT  "archivos/escuderia.txt"

/* =========================================================
   Estados de la escuderia (campo 'estado')
   ========================================================= */
#define ESTADO_ESCUDERIA_ACTIVA   1
#define ESTADO_ESCUDERIA_INACTIVA 0

/* =========================================================
   Tamanios de campos de cadena
   ========================================================= */
#define TAM_CODIGO           4
#define TAM_NOMBRE_ESCUDERIA 30
#define TAM_PAIS             50

/* =========================================================
   Estructura Escuderia
   ========================================================= */
typedef struct
{
    unsigned id;
    char     codigo[TAM_CODIGO];
    char     nombre[TAM_NOMBRE_ESCUDERIA];
    char     pais[TAM_PAIS];
    int      estado;
} Escuderia;

/* =========================================================
   Generacion de archivos (inicializacion)
   ========================================================= */
int generarArchivoEscuderiasTxt(const char* rutaTxt);

/* =========================================================
   ABM sobre escuderia.dat
   ========================================================= */

/* Alta: pide datos al usuario y agrega la escuderia al .dat. */
int altaEscuderia(const char* rutaBin);

/* Baja logica: cambia estado a 0 (inactiva). */
int bajaEscuderia(const char* rutaBin);

/* Modificacion: busca por ID y permite editar campos. */
int modificarEscuderia(const char* rutaBin);

/* =========================================================
   Exportacion a texto al finalizar la temporada
   ========================================================= */
int exportarEscuderiasATxt(const char* rutaBin, const char* rutaTxt);

/* =========================================================
   Punteros a funcion del TDA Escuderia
   ========================================================= */
int  trozarEscuderiaTxt(char* linea, void* reg);
void escuderiaBinATxt(const void* dato, FILE* archTxt);
int  escribirEscuderiaTxt(void* archTxt, const void* dato);
void mostrarEscuderia(const void* dato);
int  esEscuderiaActiva(const void* dato);

/* Callback para modificarRegistroEnBin */
void escuderiaObtenerClave(const void* registro, void* claveDestino);

#endif // ESCUDERIA_H_INCLUDED
