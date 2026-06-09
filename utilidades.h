#ifndef UTILIDADES_H_INCLUDED
#define UTILIDADES_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

/* =========================================================
   Codigos de retorno unificados para todo el proyecto
   ========================================================= */
#define TODO_OK         0
#define ERR_ARCH        1
#define ERR_LINEA       2
#define SIN_MEM         3
#define NO_ENCONTRADO   4

/* =========================================================
   Macros para fechas (formato AAAAMMDD como unsigned long long)
   ========================================================= */
#define ANIO_BASE       1601
#define ES_ANIO_BISIESTO(X)(((X) % 4 == 0 && (X) % 100 != 0) || ((X) % 400 == 0))

/* =========================================================
   Tamanio de buffer para lectura de lineas de texto
   ========================================================= */
#define TAM_LINEA       256

/* =========================================================
   Separador de campos en archivos de texto.
   Cambiar aqui afecta a TODO el proyecto: trozar*, escribir*
   ========================================================= */
#define SEP_TXT         '|'
/* =========================================================
                Punteros a funcion genericos
   Todos los modulos (Piloto, Escuderia, Carrera) deben
   proveer implementaciones de estos tipos cuando corresponda.
   ========================================================= */

/* Compara dos elementos. Retorna <0, 0 o >0 */
typedef int(*Comparar)(const void* d1, const void* d2);

/* Muestra un elemento por pantalla */
typedef void(*Mostrar)(const void* dato);

/* Accion sobre un dato (ej: escribir en archivo texto) */
typedef int(*Accion)(void* contexto, const void* dato);

/* Convierte una linea de texto en un registro binario.
   Retorna TODO_OK si la linea es valida, ERR_LINEA si no. */
typedef int(*TxtABin)(char* linea, void* registro);

/* Convierte un registro binario a texto (escribe en FILE*) */
typedef void(*BinATxt)(const void* dato, FILE* archTxt);

/* Filtra un elemento: retorna 1 si pasa el filtro, 0 si no */
typedef int  (*Filter)(const void* dato);

/* Reduce: acumula informacion de un elemento en acumulador */
typedef int(*Reduce)(void* acumulador, const void* dato);

/* Mapea un elemento de origen a destino (transformacion) */
typedef int(*Map)(void* destino, const void* origen);

/* =========================================================
                    Funciones de cadena
   ========================================================= */
int  copiarCadena(char* dest, const char* src, size_t n);
int  leerCadena(char* dest, size_t n);
void limpiarBuffer(void);
void intercambiar(void* d1, void* d2, size_t tam);

/* =========================================================
                Funciones para archivos
   ========================================================= */

/* Genera un .txt a partir de un array de structs en memoria */
int generarArchivoTexto(const char* rutaTxt,
                        const void* datos,
                        size_t cantElem,
                        size_t tamElem,
                        Accion escribir);

/* Convierte un .bin a .txt usando la funcion binATxt del TDA */
int convertirArchivoBinATxt(const char* rutaBin,
                            const char* rutaTxt,
                            size_t tamElem,
                            BinATxt binATxt);

/* Convierte un .txt a .bin usando la funcion txtABin del TDA */
int convertirArchivoTxtABin(const char* rutaTxt,
                            const char* rutaBin,
                            size_t tamElem,
                            TxtABin txtABin);

/* Recorre un .bin y llama mostrar() por cada registro */
int mostrarArchivoBinario(const char* rutaBin,
                          size_t tamElem,
                          Mostrar mostrar);

/* Recorre un .bin, filtra con filtrar() y procesa con procesar() */
int procesarArchivoBinario(const char* rutaBin,
                           void* datos,
                           size_t tamElem,
                           Filter filtrar,
                           Accion procesar);

/* =========================================================
                    Funciones para fechas
   ========================================================= */
int diasPorMes(unsigned mes, unsigned anio);
int esFechaValida(unsigned long long fecha);

/* =========================================================
            Comparadores genericos reutilizables
   ========================================================= */
int compararUnsigned(const void* a, const void* b);
int compararInt(const void* a, const void* b);

#endif // UTILIDADES_H_INCLUDED
