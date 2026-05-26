#ifndef UTILIDADES_H_INCLUDED
#define UTILIDADES_H_INCLUDED

#define TODO_OK         0
#define ERR_ARCH        1
#define ERR_CAD         2
#define NO_ENCONTRADO   4

typedef int(*Comparar)(const void* d1, const void* d2);
typedef void(*Mostrar)(const void* d);


int copiarCadena(char* dest, const char* src, size_t n);
void intercambiar(void* d1, void* d2, size_t tam);
int mostrarArchivoBinario(const char* rutaBin, void* dato, size_t tamElem, Mostrar mostrar);

/**Puntero a funcion**/
int compararUnsigned(const void* a, const void* b);
#endif // UTILIDADES_H_INCLUDED
