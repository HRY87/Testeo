#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "utilidades.h"

int copiarCadena(char* dest, const char* src, size_t n)
{
    char* d = dest;
    char* s = (char*)src;
    size_t cont = 0;

    if(!d || !src)
        return ERR_CAD;

    /**Es n - 1, ya que hay que dejar espacio para el \0**/
    while(*s && cont < n - 1)
    {
        *d = *s;
        d++;
        s++;
        cont++;
    }

    *d = '\0';

    return TODO_OK;
}

void intercambiar(void* d1, void* d2, size_t tam)
{
    void* aux = malloc(tam);

    if(!aux)
        return;

    memcpy(aux, d1, tam);
    memcpy(d1, d2, tam);
    memcpy(d2, aux, tam);

    free(aux);
}

int mostrarArchivoBinario(const char* rutaBin, void* dato, size_t tamElem, Mostrar mostrar)
{
    FILE* fBin = fopen(rutaBin, "rb");

    if(!fBin)
        return ERR_ARCH;

    while(fread(dato, tamElem, 1, fBin) == 1)
    {
        mostrar(dato);
    }

    fclose(fBin);

    return TODO_OK;
}
/**Puntero a funcion**/
int compararUnsigned(const void* a, const void* b)
{
    return(*(unsigned*)a - *(unsigned*)b);
}

