#ifndef UTILIDADES_H_INCLUDED
#define UTILIDADES_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#define ERR_NO_ENCONTRADO        -1 /* la carrera o piloto buscado no existe */
#define TODO_OK                   0
#define ERR_ARCHIVO               1
#define ERR_CADENA                2
#define ERR_LINEA                 3
#define ERR_MEMORIA               4


/* Formato de fecha almacenado como entero AAAAMMDD */
#define ES_ANIO_BISIESTO(X) (((X) % 4 == 0 && (X) % 100 != 0) || ((X) % 400 == 0))
#define TAM_LINEA       256

/* Separador de campos en archivos de texto.
   Cambiar aqui afecta a TODO el proyecto. */
#define SEP_TXT         '|'

typedef int  (*Comparar)(const void* d1, const void* d2);
typedef void (*Mostrar) (const void* d);
typedef int  (*Accion)  (void* archivo, const void* dato);
typedef int  (*BinATxt) (const void* dato, FILE* archTxt);  /* Convierte un registro binario a texto (escribe en FILE*.*/
typedef int  (*TxtABin) (char* linea, void* registro);      /* Convierte una linea de texto en un registro binario.*/

/* Funciones generales */
int copiarCadena(char* dest, const char* src, size_t n);
int leerCadena(char* dest, size_t n);
void intercambiar(void* d1, void* d2, size_t tam);
void limpiarBuffer(void);

/* Funciones para archivos */
int generarArchivoTexto(const char* rutaTxt, const void* datos, size_t cantElem, size_t tamElem, Accion escribir);
int mostrarArchivoBinario(const char* rutaBin, size_t tamElem, Mostrar mostrar);
int convertirArchivoTxtABin(const char* rutaTxt, const char* rutaBin, size_t tamElem, TxtABin trozarLinea);
int convertirArchivoBinATxt(const char* rutaBin, const char* rutaTxt, size_t tamElem, BinATxt escribirRegistro);

/* Funciones para fecha */
int diasPorMes(unsigned mes, unsigned anio);
int esFechaValida(unsigned long long fecha);

/* Comparador generico de unsigned: compatible con qsort e insertarVectorOrd. */
int compararUnsigned(const void* a, const void* b);

/* Busqueda lineal generica por clave en archivo binario.
   Compara los primeros tamClave bytes de cada registro con clave.
   Retorna el offset del registro encontrado o -1L si no existe. */
long buscarRegistroPorId(FILE* fBin, const void* clave, void* reg, size_t tamElem, size_t tamClave);

/* Retorna 1 si el archivo en ruta existe y puede abrirse, 0 si no. */
int archivoExiste(const char* ruta);

#endif /* UTILIDADES_H_INCLUDED */
