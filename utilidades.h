#ifndef UTILIDADES_H_INCLUDED
#define UTILIDADES_H_INCLUDED

#define TODO_OK         0
#define ERR_ARCH        1
#define ERR_CAD         2
#define NO_ENCONTRADO   4
#define ERR_LINEA       3

/**Defines para fecha**/
#define ANIO_BASE       1601
#define ES_ANIO_BISIESTO(X)(((X) % 4 == 0 && (X) % 100 != 0) || ((X) % 400 == 0))
#define TAM_LINEA       256

/* =========================================================
   Separador de campos en archivos de texto.
   Cambiar aqui afecta a TODO el proyecto: trozar*, escribir*
   ========================================================= */
#define SEP_TXT         '|'

typedef int(*Comparar)(const void* d1, const void* d2);
typedef void(*Mostrar)(const void* d);
typedef int (*Accion)(void* accion, const void* dato);
/* Convierte un registro binario a texto (escribe en FILE*) */
typedef void(*BinATxt)(const void* dato, FILE* archTxt);
/* Convierte una linea de texto en un registro binario.
   Retorna TODO_OK si la linea es valida, ERR_LINEA si no. */
typedef int(*TxtABin)(char* linea, void* registro);

/**Funciones generales**/
int copiarCadena(char* dest, const char* src, size_t n);
int leerCadena(char* dest, size_t n);;
void intercambiar(void* d1, void* d2, size_t tam);
void limpiarBuffer(void);

/**Funciones para Archivos**/
int generarArchivoTexto(const char* rutaTxt, const void* datos, size_t cantElem, size_t tamElem, Accion accion);
int mostrarArchivoBinario(const char* rutaBin, void* dato, size_t tamElem, Mostrar mostrar);

/* Convierte un .txt a .bin usando la funcion txtABin del TDA */
int convertirArchivoTxtABin(const char* rutaTxt, const char* rutaBin, size_t tamElem, TxtABin txtABin);

/**Funciones para fecha**/
int diasPorMes(unsigned mes, unsigned anio);
int esFechaValida(unsigned long long fecha);

/**Puntero a funcion**/
int compararUnsigned(const void* a, const void* b);
int escribirPilotoTxt(void* accion, const void* dato);

/**Recibe solo el FILE* (usar rewind al inicio)**/
long buscarRegistroPorId(FILE* fBin, unsigned id, void* reg, size_t tamElem);
#endif // UTILIDADES_H_INCLUDED
