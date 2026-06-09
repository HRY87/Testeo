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
   Corresponde exactamente al registro binario en escuderia.bin
   ========================================================= */
typedef struct
{
    unsigned id;
    char     codigo[TAM_CODIGO];
    char     nombre[TAM_NOMBRE_ESCUDERIA];
    char     pais[TAM_PAIS];
    int      estado;  /* 1: activa, 0: inactiva */
} Escuderia;

/* =========================================================
   Generacion de archivos (inicializacion)
   ========================================================= */

/* Genera el lote inicial de prueba en escuderia.txt */
int generarArchivoEscuderiasTxt(const char* rutaTxt);

/* =========================================================
            ABM directo sobre archivo binario
   ========================================================= */

/* Busca una escuderia por ID.
   Retorna el offset del registro o -1L si no existe. */
long buscarEscuderiaEnBin(const char* rutaBin, unsigned idBuscado, Escuderia* dest);

/* Baja logica: pone estado = ESTADO_ESCUDERIA_INACTIVA */
int darBajaEscuderia(const char* rutaBin, unsigned idEscuderia);

/* Modificacion campo a campo, sobreescribe solo ese registro */
int modificarEscuderia(const char* rutaBin, unsigned idEscuderia);
/* =========================================================
   Punteros a funcion del TDA Escuderia
   ========================================================= */

/* TxtABin: parsea una linea CSV y carga una Escuderia */
int  trozarEscuderiaTxt(char* linea, void* reg);

/* BinATxt: escribe una Escuderia en formato CSV al txt */
void escuderiaBinATxt(const void* dato, FILE* archTxt);

/* Accion (para generarArchivoTexto): escribe Escuderia en FILE* */
int  escribirEscuderiaTxt(void* archTxt, const void* dato);

/* Mostrar: imprime una Escuderia formateada por pantalla */
void mostrarEscuderia(const void* dato);

/* Filter: retorna 1 si la escuderia esta activa */
int  esEscuderiaActiva(const void* dato);

#endif // ESCUDERIA_H_INCLUDED
