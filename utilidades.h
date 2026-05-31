#ifndef UTILIDADES_H_INCLUDED
#define UTILIDADES_H_INCLUDED

#define TODO_OK         0
#define ERR_ARCH        1
#define ERR_LINEA       2
#define NO_ENCONTRADO   4

/**Defines para fecha**/
#define ANIO_BASE       1601
#define ES_ANIO_BISIESTO(X)(((X) % 4 == 0 && (X) % 100 != 0) || ((X) % 400 == 0))

typedef int(*Comparar)(const void* d1, const void* d2);
typedef void(*Mostrar)(const void* d);

typedef int (*Accion)(void* accion, const void* dato);

typedef int(*Map)(void* destino, const void* origen);
typedef int(*Filter)(const void* dato);
typedef int(*Reduce)(void* acumulador, const void* dato);

/**Puntero a funcion para conversion generico**/
typedef void (*BinATxt)(const void* dato, FILE* archTxt);
typedef int (*TxtABin)(char* linea, void* dato);

/**Funciones generales**/
int copiarCadena(char* dest, const char* src, size_t n);
int leerCadena(char* dest, size_t n);;
void intercambiar(void* d1, void* d2, size_t tam);
void limpiarBuffer(void);

/**Funciones para Archivos**/
int generarArchivoTexto(const char* rutaTxt, const void* datos, size_t cantElem, size_t tamElem, Accion accion);
int convertirArchivoBinATxt(const char* rutaBin, const char* rutaTxt, size_t tamElem, BinATxt binATxt);
int convertirArchivoTxtABin(const char* rutaTxt, const char* rutaBin, size_t tamElem, TxtABin txtABin);
int procesarArchivoBinario(const char* rutaBin, void* datos, size_t tamElem, Filter filtrar, Accion procesar);
int mostrarArchivoBinario(const char* rutaBin, size_t tamElem, Mostrar mostrar);

/**Funciones para fecha**/
int diasPorMes(unsigned mes, unsigned anio);
int esFechaValida(unsigned long long fecha);

/**Puntero a funcion**/
int compararUnsigned(const void* a, const void* b);

#endif // UTILIDADES_H_INCLUDED
