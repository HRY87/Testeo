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
   ========================================================= */
#define SEP_TXT         '|'

/* =========================================================
                Punteros a funcion genericos
   ========================================================= */
typedef int  (*Comparar)(const void* d1, const void* d2);
typedef void (*Mostrar) (const void* dato);
typedef int  (*Accion)  (void* contexto, const void* dato);
typedef int  (*TxtABin) (char* linea, void* registro);
typedef void (*BinATxt) (const void* dato, FILE* archTxt);
typedef int  (*Filter)  (const void* dato);
typedef int  (*Reduce)  (void* acumulador, const void* dato);
typedef int  (*Map)     (void* destino, const void* origen);

/* =========================================================
                    Funciones de cadena
   ========================================================= */
int  copiarCadena(char* dest, const char* src, size_t n);
int  leerCadena(char* dest, size_t n);
void limpiarBuffer(void);
void intercambiar(void* d1, void* d2, size_t tam);

/* =========================================================
         Lectura segura de tipos numericos desde stdin

   Todas leen una linea completa, la convierten y validan.
   Si el usuario ingresa letras donde se pide un numero,
   o el valor esta fuera del rango [min, max], reintentan.
   Retornan TODO_OK cuando el valor es valido.

   Uso tipico:
       int opcion;
       leerInt(&opcion, 0, 5);

       unsigned id;
       leerUnsigned(&id, 1, 9999);

       unsigned long long fecha;
       leerUnsignedLongLong(&fecha);          <- sin rango, solo que sea numero

       char estado;
       leerChar(&estado, "ARS");              <- solo acepta A, R o S
   ========================================================= */

/* Lee un int en [min, max]. Reinicia si la entrada no es numerica
   o esta fuera de rango. */
int leerInt(int* dest, int min, int max);

/* Lee un unsigned en [min, max]. */
int leerUnsigned(unsigned* dest, unsigned min, unsigned max);

/* Lee un unsigned long long sin rango (solo valida que sea numerico). */
int leerUnsignedLongLong(unsigned long long* dest);

/* Lee un char que pertenezca a la cadena 'validos' (case-sensitive).
   Ejemplo: leerChar(&c, "ARS") acepta solo 'A', 'R' o 'S'. */
int leerChar(char* dest, const char* validos);

/* =========================================================
                Funciones para archivos
   ========================================================= */
int generarArchivoTexto(const char* rutaTxt,
                        const void* datos,
                        size_t cantElem,
                        size_t tamElem,
                        Accion escribir);

int convertirArchivoBinATxt(const char* rutaBin,
                            const char* rutaTxt,
                            size_t tamElem,
                            BinATxt binATxt);

int convertirArchivoTxtABin(const char* rutaTxt,
                            const char* rutaBin,
                            size_t tamElem,
                            TxtABin txtABin);

int mostrarArchivoBinario(const char* rutaBin,
                          size_t tamElem,
                          Mostrar mostrar);

int procesarArchivoBinario(const char* rutaBin,
                           void* datos,
                           size_t tamElem,
                           Filter filtrar,
                           Accion procesar);

/* =========================================================
   Modificacion de un registro en un archivo binario.

   Recorre el .bin buscando el registro cuya clave
   (extraida con 'obtenerClave') coincida con 'clave'
   segun 'comparar'. Cuando lo encuentra, llama a
   'modificar(registro)' y lo sobreescribe en disco.

   Retorna TODO_OK, NO_ENCONTRADO o ERR_ARCH.
   ========================================================= */
typedef void (*ObtenerClave)(const void* registro, void* claveDestino);
typedef void (*ModificarReg)(void* registro);

int modificarRegistroEnBin(const char*   rutaBin,
                           size_t        tamElem,
                           const void*   clave,
                           Comparar      comparar,
                           ObtenerClave  obtenerClave,
                           ModificarReg  modificar);

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
