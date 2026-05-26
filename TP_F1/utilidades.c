#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "utilidades.h"
#include "piloto.h"


/**Funciones generales**/
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

int leerCadena(char* dest, size_t n)
{
    char* pos;

    if(!dest || n == 0)
        return ERR_CAD;

    if(!fgets(dest, (int)n, stdin))
        return ERR_CAD;

    /** Reemplazar el \n que deja fgets por \0 **/
    pos = dest;
    while(*pos && *pos != '\n')
        pos++;

    *pos = '\0';

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

void limpiarBuffer(void)
{
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

/**Funciones para Archivos**/
int generarArchivoTexto(const char* rutaTxt, const void* datos, size_t cantElem, size_t tamElem, Accion escribir)
{
    size_t i;
    const char* pLec = (const char*)datos;

    FILE* fTxt = fopen(rutaTxt, "wt");

    if(!fTxt)
        return ERR_ARCH;

    for(i = 0; i < cantElem; i++)
    {
        escribir(fTxt, pLec);
        pLec += tamElem;
    }

    fclose(fTxt);
    return TODO_OK;
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

/**Funciones para fecha**/
int diasPorMes(unsigned mes, unsigned anio)
{
    static const int dias[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if(mes == 2)
        return (ES_ANIO_BISIESTO(anio)) ? 29 : 28;

    return dias[mes];
}

int esFechaValida(unsigned long long fecha)
{
    unsigned anio = (unsigned)(fecha / 10000);
    unsigned mes  = (unsigned)(fecha / 100 % 100);
    unsigned dia  = (unsigned)(fecha % 100);

    if(anio > 1601 && (mes >= 1 && mes <= 12) && (dia >= 1 && dia <= diasPorMes(mes, anio)))
        return 1;

    return 0;
}

/**Puntero a funcion**/
int compararUnsigned(const void* a, const void* b)
{
    return(*(unsigned*)a - *(unsigned*)b);
}

int escribirPilotoTxt(void* accion, const void* dato)
{
    FILE* txt = (FILE*)accion;
    const Piloto* p = (const Piloto*)dato;

    if(!txt || !p)
        return ERR_ARCH;

    fprintf(txt, "%u,%s,%s,%u,%u,%c,%llu\n",
            p->id,
            p->nombre,
            p->nacionalidad,
            p->id_escuderia,
            p->puntos_acumulados,
            p->estado,
            p->fechaNacimiento);

    return TODO_OK;
}
