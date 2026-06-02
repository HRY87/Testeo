#ifndef PILOTO_H_INCLUDED
#define PILOTO_H_INCLUDED

#include "vector.h"

/* =========================================================
   Rutas de archivos del TDA Piloto
   ========================================================= */
#define RUTA_PILOTO_TXT  "piloto.txt"
#define RUTA_PILOTO_BIN  "piloto.bin"

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
   Indices de columnas en la matriz resultados de Carrera
   (definidos aqui porque Piloto los necesita para reducir)
   ========================================================= */
#define COL_ID_PILOTO  0
#define COL_PUNTOS     1

/* =========================================================
   Estructura Piloto
   Corresponde exactamente al registro binario en piloto.bin
   ========================================================= */
typedef struct
{
    unsigned           id;
    char               nombre[TAM_NOMBRE_PILOTO];
    char               nacionalidad[TAM_NACIONALIDAD];
    unsigned           id_escuderia;
    unsigned           puntos_acumulados;
    char               estado;           /* A, R o S */
    unsigned long long fechaNacimiento;  /* AAAAMMDD  */
} Piloto;

/* =========================================================
   Generacion y carga de archivos (inicializacion)
   ========================================================= */

/* Genera el lote inicial de prueba en piloto.txt */
int generarArchivoPilotosTxt(const char* rutaTxt);

/* =========================================================
   Operaciones con vector de pilotos
   ========================================================= */

/* Carga en vPilotos (ordenado por id) solo los pilotos activos */
int cargarVectorPilotoActivos(const char* rutaBin,
                              tVector*    vIds,
                              Comparar    comparar);

/* =========================================================
   Punteros a funcion del TDA Piloto
   Estas funciones son las "implementaciones especificas" que
   se pasan a las funciones genericas de utilidades y vector.
   ========================================================= */

/* TxtABin: parsea una linea CSV y carga un Piloto */
int  trozarPilotoTxt(char* linea, void* reg);

/* BinATxt: escribe un Piloto en formato CSV al archivo txt */
void pilotoBinATxt(const void* dato, FILE* archTxt);

/* Mostrar: imprime un Piloto formateado por pantalla */
void mostrarPiloto(const void* dato);

/* Accion (para generarArchivoTexto): escribe Piloto en FILE* */
int  escribirPilotoTxt(void* archTxt, const void* dato);

/* Filter: retorna 1 si el piloto esta activo */
int  esPilotoActivo(const void* dato);

/* Reduce: suma los puntos del piloto al acumulador unsigned */
int  sumarPuntos(void* acumulador, const void* dato);

/* Map: extrae {id, puntos} de un Piloto a un array unsigned[2] */
int  extraerIdPuntos(void* dest, const void* orig);
#endif // PILOTO_H_INCLUDED
