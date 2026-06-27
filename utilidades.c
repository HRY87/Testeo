#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilidades.h"

/* =========================================================
   Funciones generales
   ========================================================= */

/**
 * copiarCadena
 * Copia hasta n-1 caracteres de src en dest, garantizando
 * terminacion en '\0'. Usa aritmetica de punteros.
 * Retorna TODO_OK, o ERR_CAD si algun puntero es NULL.
 */
int copiarCadena(char* dest, const char* src, size_t n)
{
    char*       d    = dest;
    const char* s    = src;
    size_t      cont = 0;

    if (!d || !s)
        return ERR_CADENA;

    while (*s && cont < n - 1)
    {
        *d++ = *s++;
        cont++;
    }
    *d = '\0';

    return TODO_OK;
}

/**
 * leerCadena
 * Lee una linea desde stdin con fgets y elimina el '\n' final.
 * Retorna TODO_OK, o ERR_CAD si la lectura falla o los parametros son invalidos.
 */
int leerCadena(char* dest, size_t n)
{
    char* pos;

    if (!dest || n == 0)
        return ERR_CADENA;

    if (!fgets(dest, (int)n, stdin))
        return ERR_CADENA;

    pos = dest;
    while (*pos && *pos != '\n')
        pos++;
    *pos = '\0';

    return TODO_OK;
}

/**
 * intercambiar
 * Intercambia dos bloques de memoria de tam bytes.
 * Usa un buffer temporal en heap; si malloc falla no hace nada.
 */
void intercambiar(void* d1, void* d2, size_t tam)
{
    void* aux = malloc(tam);

    if (!aux)
        return;

    memcpy(aux, d1, tam);
    memcpy(d1,  d2, tam);
    memcpy(d2, aux, tam);

    free(aux);
}

/**
 * limpiarBuffer
 * Descarta caracteres pendientes en stdin hasta '\n' o EOF.
 * Debe llamarse antes de leerCadena() para evitar residuos de scanf.
 */
void limpiarBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* =========================================================
   Funciones para archivos
   ========================================================= */

/**
 * generarArchivoTexto
 * Crea (o sobreescribe) rutaTxt y escribe cada elemento del arreglo
 * datos usando el puntero a funcion escribir (firma Accion).
 * Retorna TODO_OK o ERR_ARCH.
 */
int generarArchivoTexto(const char* rutaTxt, const void* datos, size_t cantElem, size_t tamElem, Accion escribir)
{
    const char* pLec;
    size_t      i;
    FILE*       fTxt;

    fTxt = fopen(rutaTxt, "wt");
    if (!fTxt)
        return ERR_ARCHIVO;

    pLec = (const char*)datos;
    for (i = 0; i < cantElem; i++)
    {
        escribir(fTxt, pLec);
        pLec += tamElem;
    }

    fclose(fTxt);
    return TODO_OK;
}

/**
 * mostrarArchivoBinario
 * Abre rutaBin y llama a mostrar() para cada registro leido.
 * Retorna TODO_OK o ERR_ARCH.
 */
int mostrarArchivoBinario(const char* rutaBin, size_t tamElem, Mostrar mostrar)
{
    void* dato;
    FILE* fBin;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCHIVO;

    dato = malloc(tamElem);
    if (!dato)
    {
        fclose(fBin);
        return ERR_MEMORIA;
    }

    while (fread(dato, tamElem, 1, fBin) == 1)
        mostrar(dato);

    free(dato);
    fclose(fBin);
    return TODO_OK;
}

/**
 * convertirArchivoTxtABin
 * Lee rutaTxt linea a linea, convierte cada una con trozarLinea()
 * y escribe el resultado en rutaBin. Lineas invalidas se descartan.
 * Retorna TODO_OK, ERR_ARCH o SIN_MEM.
 */
int convertirArchivoTxtABin(const char* rutaTxt, const char* rutaBin, size_t tamElem, TxtABin trozarLinea)
{
    char*  linea;
    void*  reg;
    FILE*  fTxt;
    FILE*  fBin;
    size_t exportados = 0;

    fTxt = fopen(rutaTxt, "rt");
    if (!fTxt)
        return ERR_ARCHIVO;

    fBin = fopen(rutaBin, "wb");
    if (!fBin)
    {
        fclose(fTxt);
        return ERR_ARCHIVO;
    }

    reg   = malloc(tamElem);
    linea = (char*)malloc(TAM_LINEA);

    if (!reg || !linea)
    {
        free(reg);
        free(linea);
        fclose(fTxt);
        fclose(fBin);
        return ERR_MEMORIA;
    }

    while (fgets(linea, TAM_LINEA, fTxt))
    {
        if (trozarLinea(linea, reg) == TODO_OK)
        {
            fwrite(reg, tamElem, 1, fBin);
            exportados++;
        }
    }

    free(reg);
    free(linea);
    fclose(fTxt);
    fclose(fBin);

    //printf("[OK] %lu registros escritos en %s.\n", (unsigned long)exportados, rutaBin);
    return TODO_OK;
}

/**
 * convertirArchivoBinATxt
 * Lee rutaBin registro a registro y escribe cada uno en rutaTxt
 * usando el puntero a funcion escribirRegistro (firma BinATxt).
 * Retorna TODO_OK, ERR_ARCH o ERR_MEM.
 */
int convertirArchivoBinATxt(const char* rutaBin, const char* rutaTxt, size_t tamElem, BinATxt escribirRegistro)
{
    FILE*  fBin;
    FILE*  fTxt;
    void*  reg;
    size_t exportados = 0;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCHIVO;

    fTxt = fopen(rutaTxt, "wt");
    if (!fTxt)
    {
        fclose(fBin);
        return ERR_ARCHIVO;
    }

    reg = malloc(tamElem);
    if (!reg)
    {
        fclose(fBin);
        fclose(fTxt);
        return ERR_MEMORIA;
    }

    while (fread(reg, tamElem, 1, fBin) == 1)
    {
        if (escribirRegistro(reg, fTxt) == TODO_OK)
            exportados++;
    }

    free(reg);
    fclose(fBin);
    fclose(fTxt);

    //printf("[OK] %lu registros escritos en %s.\n", (unsigned long)exportados, rutaTxt);
    return TODO_OK;
}

/* =========================================================
   Funciones para fecha
   ========================================================= */

/**
 * diasPorMes
 * Retorna la cantidad de dias del mes para el anio dado.
 * Considera anios bisiestos para febrero.
 */
int diasPorMes(unsigned mes, unsigned anio)
{
    static const int dias[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (mes == 2)
        return ES_ANIO_BISIESTO(anio) ? 29 : 28;

    return dias[mes];
}

/**
 * esFechaValida
 * Valida una fecha en formato AAAAMMDD.
 * Retorna 1 si es valida, 0 si no.
 */
int esFechaValida(unsigned long long fecha)
{
    unsigned anio = (unsigned)(fecha / 10000);
    unsigned mes  = (unsigned)(fecha / 100 % 100);
    unsigned dia  = (unsigned)(fecha % 100);

    if (anio > 1601 && mes >= 1 && mes <= 12 && dia >= 1 && dia <= (unsigned)diasPorMes(mes, anio))
        return 1;

    printf("\n  +==================================+\n");
    printf("  |  !! INGRESE UNA FECHA VALIDA !!  |\n");
    printf("  +==================================+\n\n");
    return 0;
}

/* =========================================================
   Punteros a funcion genericos
   ========================================================= */

/**
 * compararUnsigned  [Comparar]
 * Compara dos unsigned. Retorna -1, 0 o 1.
 */
int compararUnsigned(const void* a, const void* b)
{
    unsigned va = *(const unsigned*)a;
    unsigned vb = *(const unsigned*)b;

    if (va < vb) return -1;
    if (va > vb) return  1;
    return 0;
}

/**
 * buscarRegistroPorId
 * Busqueda lineal generica: compara los primeros tamClave bytes
 * de cada registro con clave usando memcmp.
 * Retorna el offset del registro encontrado o -1L.
 */
long buscarRegistroPorId(FILE* fBin, const void* clave, void* reg, size_t tamElem, size_t tamClave)
{
    long offset = 0L;

    rewind(fBin);

    while (fread(reg, tamElem, 1, fBin) == 1)
    {
        if (memcmp(reg, clave, tamClave) == 0)
            return offset;

        offset += (long)tamElem;
    }

    return -1L;
}

/**
 * archivoExiste
 * Intenta abrir ruta en modo lectura binaria.
 * Retorna 1 si existe, 0 si no.
 */
int archivoExiste(const char* ruta)
{
    FILE* f = fopen(ruta, "rb");

    if (f)
    {
        fclose(f);
        return 1;
    }
    return 0;
}
