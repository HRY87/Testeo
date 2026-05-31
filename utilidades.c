#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "utilidades.h"
#include "piloto.h"

#define TAM_LINEA       256

/**Funciones generales**/
int copiarCadena(char* dest, const char* src, size_t n)
{
    char* d = dest;
    char* s = (char*)src;
    size_t cont = 0;

    if(!d || !src)
        return ERR_LINEA;

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
        return ERR_LINEA;

    if(!fgets(dest, (int)n, stdin))
        return ERR_LINEA;

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

int convertirArchivoBinATxt(const char* rutaBin, const char* rutaTxt, size_t tamElem, BinATxt binATxt)
{
    FILE* fBin = fopen(rutaBin, "rb");

    if(!fBin)
    {
        return ERR_ARCH;
    }

    FILE* fTxt = fopen(rutaTxt, "wt");

    if(!fTxt)
    {
        fclose(fBin);
        return ERR_ARCH;
    }

    void* reg = malloc(tamElem);

    if(!reg)
    {
        fclose(fBin);
        fclose(fTxt);
        return SIN_MEM;
    }

    fread(reg, tamElem, 1, fBin);

    while(!feof(fBin))
    {
        binATxt(reg, fTxt); //Funcion de conversion bin a txt
        fread(reg, tamElem, 1, fBin);
    }

    free(reg);

    fclose(fBin);
    fclose(fTxt);

    return TODO_OK;
}

int convertirArchivoTxtABin(const char* rutaTxt, const char* rutaBin, size_t tamElem, TxtABin txtABin)
{
    int resp = TODO_OK;
    char* linea; //Buffer temporal para guarda la linea de texto

    FILE* fTxt = fopen(rutaTxt, "rt");

    if(!fTxt)
    {
        return ERR_ARCH;
    }

    FILE* fBin = fopen(rutaBin, "wb");

    if(!fBin)
    {
        fclose(fTxt);
        return ERR_ARCH;
    }

    void* reg = malloc(tamElem);

    if(!reg)
    {
        fclose(fBin);
        fclose(fTxt);
        return SIN_MEM;
    }

    linea =  (char*)malloc(TAM_LINEA); //No olvidar reservar memoria

    fgets(linea, TAM_LINEA, fTxt);
    while(!feof(fTxt) && (resp != ERR_LINEA))
    {
        resp = txtABin(linea, reg);

        if(resp == TODO_OK)
        {
            fwrite(reg, tamElem, 1, fBin);
        }

        fgets(linea, TAM_LINEA, fTxt);
    }

    fclose(fBin);
    fclose(fTxt);

    free(reg);
    free(linea);

    return TODO_OK;
}

int procesarArchivoBinario(const char* rutaBin, void* datos, size_t tamElem, Filter filtrar, Accion procesar)
{
    void* reg;

    FILE* fBin = fopen(rutaBin, "rb");

    if(!fBin)
        return ERR_ARCH;

    reg = malloc(tamElem);

    if(!reg || !filtrar)
    {
        fclose(fBin);
        return SIN_MEM;
    }

    while(fread(reg, tamElem, 1, fBin))
    {
        if(filtrar(reg))
            procesar(datos, reg);
    }

    free(reg);
    fclose(fBin);

    return TODO_OK;
}
int mostrarArchivoBinario(const char* rutaBin, size_t tamElem, Mostrar mostrar)
{
    FILE* fBin = fopen(rutaBin, "rb");
    if(!fBin)
        return ERR_ARCH;

    void* dato = malloc(tamElem);

    if(!dato)
    {
        fclose(fBin);
        return SIN_MEM;
    }

    while(fread(dato, tamElem, 1, fBin))
    {
        mostrar(dato);
    }

    free(dato);
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


