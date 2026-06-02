#include <memory.h>
#include "utilidades.h"

#define TAM_LINEA       256   /* Buffer maximo para leer una linea de texto */

/* =========================================================
            Funciones de cadena y utiles generales
   ========================================================= */

/**
 * copiarCadena
 * Copia hasta n-1 caracteres de src a dest y agrega '\0'.
 * Retorna TODO_OK o ERR_LINEA si algun puntero es NULL.
 */
int copiarCadena(char* dest, const char* src, size_t n)
{
    char*       d    = dest;
    const char* s    = src;
    size_t      cont = 0;

    if (!d || !s)
        return ERR_LINEA;

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
 * Lee una linea de stdin en dest (max n chars incluido '\0').
 * Reemplaza el '\n' que deja fgets por '\0'.
 * Retorna TODO_OK o ERR_LINEA.
 */
int leerCadena(char* dest, size_t n)
{
    char* pos;

    if (!dest || n == 0)
        return ERR_LINEA;

    if (!fgets(dest, (int)n, stdin))
        return ERR_LINEA;

    pos = dest;
    while (*pos && *pos != '\n')
        pos++;
    *pos = '\0';

    return TODO_OK;
}

/**
 * limpiarBuffer
 * Descarta todo lo que quedo en stdin hasta '\n' o EOF.
 * Necesario despues de scanf() para evitar lecturas basura.
 */
void limpiarBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * intercambiar
 * Intercambia dos bloques de memoria de tamanio tam.
 * Usa un buffer auxiliar temporal en heap.
 */
void intercambiar(void* d1, void* d2, size_t tam)
{
    void* aux = malloc(tam);
    if (!aux) return;

    memcpy(aux, d1,  tam);
    memcpy(d1,  d2,  tam);
    memcpy(d2,  aux, tam);

    free(aux);
}

/* =========================================================
            Funciones genericas para archivos
   ========================================================= */

/**
 * generarArchivoTexto
 * Recorre un array de structs en memoria y escribe cada uno
 * en un .txt usando la funcion 'escribir' del TDA correspondiente.
 * El puntero 'escribir' recibe (FILE*, const void*).
 */
int generarArchivoTexto(const char* rutaTxt,
                        const void* datos,
                        size_t cantElem,
                        size_t tamElem,
                        Accion escribir)
{
    size_t       i;
    const char*  pLec;
    FILE*        fTxt;

    fTxt = fopen(rutaTxt, "wt");
    if (!fTxt)
        return ERR_ARCH;

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
 * convertirArchivoTxtABin
 * Lee linea a linea un .txt y convierte cada una a registro
 * binario usando la funcion 'txtABin' del TDA.
 * Si txtABin retorna ERR_LINEA, la linea se descarta (no aborta).
 */
int convertirArchivoTxtABin(const char* rutaTxt,
                            const char* rutaBin,
                            size_t tamElem,
                            TxtABin txtABin)
{
    char*  linea;
    void*  reg;
    int    resp;
    FILE*  fTxt;
    FILE*  fBin;

    fTxt = fopen(rutaTxt, "rt");
    if (!fTxt)
        return ERR_ARCH;

    fBin = fopen(rutaBin, "wb");
    if (!fBin)
    {
        fclose(fTxt);
        return ERR_ARCH;
    }

    reg   = malloc(tamElem);
    linea = (char*)malloc(TAM_LINEA);

    if (!reg || !linea)
    {
        free(reg);
        free(linea);
        fclose(fTxt);
        fclose(fBin);
        return SIN_MEM;
    }

    while (fgets(linea, TAM_LINEA, fTxt))
    {
        resp = txtABin(linea, reg);
        if (resp == TODO_OK)
            fwrite(reg, tamElem, 1, fBin);
        /* Si es ERR_LINEA simplemente se omite esa linea */
    }

    free(reg);
    free(linea);
    fclose(fTxt);
    fclose(fBin);

    return TODO_OK;
}

/**
 * convertirArchivoBinATxt
 * Lee registro a registro un .bin y convierte cada uno a texto
 * usando la funcion 'binATxt' del TDA.
 */
int convertirArchivoBinATxt(const char* rutaBin,
                            const char* rutaTxt,
                            size_t tamElem,
                            BinATxt binATxt)
{
    void*  reg;
    FILE*  fBin;
    FILE*  fTxt;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    fTxt = fopen(rutaTxt, "wt");
    if (!fTxt)
    {
        fclose(fBin);
        return ERR_ARCH;
    }

    reg = malloc(tamElem);
    if (!reg)
    {
        fclose(fBin);
        fclose(fTxt);
        return SIN_MEM;
    }

    while (fread(reg, tamElem, 1, fBin) == 1)
        binATxt(reg, fTxt);

    free(reg);
    fclose(fBin);
    fclose(fTxt);

    return TODO_OK;
}

/**
 * mostrarArchivoBinario
 * Recorre un .bin y llama mostrar() por cada registro.
 * Generica: sirve para Piloto, Escuderia o Carrera.
 */
int mostrarArchivoBinario(const char* rutaBin,
                          size_t tamElem,
                          Mostrar mostrar)
{
    void*  dato;
    FILE*  fBin;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    dato = malloc(tamElem);
    if (!dato)
    {
        fclose(fBin);
        return SIN_MEM;
    }

    while (fread(dato, tamElem, 1, fBin) == 1)
        mostrar(dato);

    free(dato);
    fclose(fBin);

    return TODO_OK;
}

/**
 * procesarArchivoBinario
 * Recorre un .bin, aplica filtrar() a cada registro y si pasa
 * llama procesar(datos, registro).
 * Uso tipico: acumular puntos de carreras activas en un vector.
 */
int procesarArchivoBinario(const char* rutaBin,
                           void* datos,
                           size_t tamElem,
                           Filter filtrar,
                           Accion procesar)
{
    void*  reg;
    FILE*  fBin;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    reg = malloc(tamElem);
    if (!reg || !filtrar)
    {
        free(reg);
        fclose(fBin);
        return SIN_MEM;
    }

    while (fread(reg, tamElem, 1, fBin) == 1)
    {
        if (filtrar(reg))
            procesar(datos, reg);
    }

    free(reg);
    fclose(fBin);

    return TODO_OK;
}

/* =========================================================
            Funciones para fechas (formato AAAAMMDD)
   ========================================================= */

/**
 * diasPorMes
 * Retorna los dias del mes dado, considerando anios bisiestos.
 */
int diasPorMes(unsigned mes, unsigned anio)
{
    static const int dias[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

    if (mes == 2)
        return ES_ANIO_BISIESTO(anio) ? 29 : 28;

    return dias[mes];
}

/**
 * esFechaValida
 * Valida que una fecha AAAAMMDD sea coherente.
 * Retorna 1 si es valida, 0 si no.
 */
int esFechaValida(unsigned long long fecha)
{
    unsigned anio = (unsigned)(fecha / 10000);
    unsigned mes  = (unsigned)(fecha / 100 % 100);
    unsigned dia  = (unsigned)(fecha % 100);

    if (anio > ANIO_BASE &&
            mes  >= 1 && mes <= 12 &&
            dia  >= 1 && dia <= (unsigned)diasPorMes(mes, anio))
        return 1;

    return 0;
}

/* =========================================================
                Comparadores genericos
   ========================================================= */

/** Compara dos unsigned por valor */
int compararUnsigned(const void* a, const void* b)
{
    return (int)(*(const unsigned*)a - *(const unsigned*)b);
}

/** Compara dos int por valor */
int compararInt(const void* a, const void* b)
{
    return (*(const int*)a - *(const int*)b);
}

