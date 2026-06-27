#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilidades.h"
#include "piloto.h"


/**Funciones generales**/

/**
 * copiarCadena
 * Copia hasta n-1 caracteres de src en dest,
 * garantizando que dest quede terminado en '\0'.
 * Retorna TODO_OK si los punteros son validos, ERR_CAD si no.
 **/
int copiarCadena(char* dest, const char* src, size_t n)
{
    char* d = dest;
    char* s = (char*)src;
    size_t cont = 0;

    if(!d || !src)
        return ERR_CAD;

    /** Copia caracter a caracter hasta n-1 o fin de src **/
    while(*s && cont < n - 1)
    {
        *d = *s;
        d++;
        s++;
        cont++;
    }

    *d = '\0'; /** Asegura terminacion de cadena **/

    return TODO_OK;
}

/**
 * leerCadena
 * Lee una linea desde stdin con fgets y elimina el '\n' final.
 * Retorna TODO_OK si la lectura fue exitosa, ERR_CAD si hubo error o
 * los parametros son invalidos.
 **/
int leerCadena(char* dest, size_t n)
{
    char* pos;

    if(!dest || n == 0)
        return ERR_CAD;

    if(!fgets(dest, (int)n, stdin))
        return ERR_CAD;

    /** Avanza hasta el '\n' o el fin de cadena y lo reemplaza por '\0' **/
    pos = dest;
    while(*pos && *pos != '\n')
        pos++;

    *pos = '\0';

    return TODO_OK;
}

/**
 * intercambiar
 * Intercambia el contenido de dos bloques de memoria de 'tam' bytes.
 * Usa un buffer temporal en heap para no depender del tipo de dato.
 * No retorna valor; si malloc falla, no realiza el intercambio.
 */
void intercambiar(void* d1, void* d2, size_t tam)
{
    void* aux = malloc(tam);

    if(!aux)
        return;

    memcpy(aux, d1, tam); /** aux  <- d1 */
    memcpy(d1,  d2, tam); /** d1   <- d2 */
    memcpy(d2, aux, tam); /** d2   <- aux (valor original de d1) */

    free(aux);
}

/**
 * limpiarBuffer
 * Descarta todos los caracteres pendientes en stdin hasta
 * encontrar '\n' o EOF. Se usa antes de leerCadena() para
 * evitar que scanf deje residuos en el buffer.
 */
void limpiarBuffer(void)
{
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

/**Funciones para Archivos**/

/**
 * generarArchivoTexto
 * Crea (o sobreescribe) el archivo de texto en rutaTxt y escribe
 * cada elemento del arreglo 'datos' usando el puntero a funcion 'escribir' (firma Accion).
 * Avanza por el arreglo con aritmetica de punteros segun tamElem.
 * Retorna TODO_OK si todo salio bien, ERR_ARCH si no pudo abrir el archivo.
 */
int generarArchivoTexto(const char* rutaTxt, const void* datos, size_t cantElem, size_t tamElem, Accion escribir)
{
    size_t i;
    const char* pLec = (const char*)datos; /** Puntero byte para recorrer el arreglo generico */

    FILE* fTxt = fopen(rutaTxt, "wt");

    if(!fTxt)
        return ERR_ARCH;

    for(i = 0; i < cantElem; i++)
    {
        escribir(fTxt, pLec); /** Delega el formato de escritura al TDA correspondiente */
        pLec += tamElem;      /** Avanza al siguiente elemento */
    }

    fclose(fTxt);
    return TODO_OK;
}

/**
 * mostrarArchivoBinario
 * Abre el archivo binario en rutaBin y muestra cada registro
 * por pantalla usando el puntero a funcion 'mostrar' (firma Mostrar).
 * Lee de a un registro por iteracion en el buffer 'dato'.
 * Retorna TODO_OK o ERR_ARCH si no pudo abrir el archivo.
 */
int mostrarArchivoBinario(const char* rutaBin, size_t tamElem, Mostrar mostrar)
{
    void* dato;
    FILE* fBin;

    fBin = fopen(rutaBin, "rb");

    if(!fBin)
        return ERR_ARCH;

    dato = malloc(tamElem);

    if(!dato)
    {
        fclose(fBin);
        return ERR_MEM;
    }

    while(fread(dato, tamElem, 1, fBin) == 1)
    {
        mostrar(dato); /** Delega el formato de visualizacion al TDA */
    }

    free(dato);
    fclose(fBin);

    return TODO_OK;
}

/**Funciones para fecha**/

/**
 * diasPorMes
 * Retorna la cantidad de dias del mes indicado para el anio dado.
 * Febrero retorna 29 si el anio es bisiesto segun la macro ES_ANIO_BISIESTO
 * si no lo es retonar 28.
 */
int diasPorMes(unsigned mes, unsigned anio)
{
    static const int dias[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if(mes == 2)
        return (ES_ANIO_BISIESTO(anio)) ? 29 : 28;

    return dias[mes];
}

/**
 * esFechaValida
 * Valida que la fecha en formato AAAAMMDD sea coherente:
 * anio > 1601, mes entre 1-12, dia entre 1 y diasPorMes().
 * Imprime un cartel de error si la fecha es invalida.
 * Retorna 1 si es valida, 0 si no.
 */
int esFechaValida(unsigned long long fecha)
{
    /** Obtenemos los campos de la fecha numerica */
    unsigned anio = (unsigned)(fecha / 10000);
    unsigned mes  = (unsigned)(fecha / 100 % 100);
    unsigned dia  = (unsigned)(fecha % 100);

    if(anio > 1601 && (mes >= 1 && mes <= 12) && (dia >= 1 && dia <= diasPorMes(mes, anio)))
        return 1;

    printf("\n");
    printf("  +==================================+\n");
    printf("  |  !! INGRESE UNA FECHA VALIDA !!  |\n");
    printf("  +==================================+\n");
    printf("\n");
    return 0;

}

/**Puntero a funcion**/
/**
 * compararUnsigned  [Comparar]
 * Compara dos valores unsigned apuntados por a y b.
 * Retorna -1 si a < b, 1 si a > b, 0 si son iguales.
 * Compatible con qsort e insertarVectorOrd.
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
 * escribirPilotoTxt  [Accion]
 * Escribe un registro Piloto en formato .txt en el FILE* recibido.
 * Formato: id,nombre,nacionalidad,id_escuderia,puntos,estado,fechaNacimiento
 * Retorna TODO_OK, o ERR_ARCH si algun puntero es NULL.
 */
int escribirPilotoTxt(void* accion, const void* dato)
{
    FILE* txt = (FILE*)accion;
    const Piloto* p = (const Piloto*)dato;

    if(!txt || !p)
        return ERR_ARCH;

    fprintf(txt, "%u%c%s%c%s%c%u%c%u%c%c%c%llu\n",
            p->id, SEP_TXT,
            p->nombre, SEP_TXT,
            p->nacionalidad, SEP_TXT,
            p->id_escuderia, SEP_TXT,
            p->puntos_acumulados, SEP_TXT,
            p->estado, SEP_TXT,
            p->fechaNacimiento);

    return TODO_OK;
}

/**
 * convertirArchivoTxtABin
 * Lee linea a linea el archivo de texto rutaTxt y convierte
 * cada linea a un registro binario usando el puntero a funcion
 * txtABin del TDA correspondiente. Las lineas invalidas
 * (ERR_LINEA) se descartan sin abortar el proceso.
 * Retorna TODO_OK, ERR_ARCH si no puede abrir alguno de los
 * archivos, o SIN_MEM si falla malloc.
 */
/* Lee un .txt linea por linea, parsea cada una a registro binario con 'txtABin' y lo escribe en el .bin */
int convertirArchivoTxtABin(const char* rutaTxt, const char* rutaBin, size_t tamElem, TxtABin trozarLinea)
{
    char*  linea = NULL;
    void*  reg = NULL;
    int    resp = TODO_OK;
    FILE*  fTxt = NULL;
    FILE*  fBin = NULL;
    size_t registrosExportados = 0;

    fTxt = fopen(rutaTxt, "rt");
    if (!fTxt)
        return ERR_ARCH;

    fBin = fopen(rutaBin, "wb");
    if (!fBin)
    {
        fclose(fTxt);
        return ERR_ARCH;
    }

    reg   = malloc(tamElem);           /** Buffer para un registro del tipo que sea */
    linea = (char*)malloc(TAM_LINEA);  /** Buffer para una linea del txt*/

    if (!reg || !linea)
    {
        free(reg);
        free(linea);
        fclose(fTxt);
        fclose(fBin);
        return SIN_MEM;
    }

    while(fgets(linea, TAM_LINEA, fTxt))
    {
        resp = trozarLinea(linea, reg);/** Parsea la linea al struct del TDA */

        if (resp == TODO_OK)/** Solo escribe si el parseo fue exitoso */
        {
            fwrite(reg, tamElem, 1, fBin);
            registrosExportados++;
        }

        /** ERR_LINEA: la linea se omite silenciosamente */
    }

    free(reg);
    free(linea);
    fclose(fTxt);
    fclose(fBin);

    printf("[OK] Se exportaron %llu registros al archivo %s.\n", registrosExportados, rutaBin);

    return TODO_OK;
}

int convertirArchivoBinATxt(const char* rutaBin, const char* rutaTxt, size_t tamElem, BinATxt escribirRegistro)
{
    FILE* fBin = NULL;
    FILE* fTxt = NULL;
    void* reg = NULL;
    size_t registrosExportados = 0;

    fBin = fopen(rutaBin, "rb");

    if(!fBin)
    {
        return ERR_ARCH;
    }

    fTxt = fopen(rutaTxt, "wt");

    if(!fTxt)
    {
        fclose(fBin);
        return ERR_ARCH;
    }

    reg = malloc(tamElem);

    if(!reg)
    {
        fclose(fBin);
        fclose(fTxt);
        return ERR_MEM;
    }

    fread(reg, tamElem, 1, fBin);
    while(!feof(fBin))
    {
        if(escribirRegistro(reg, fTxt) == TODO_OK)
            registrosExportados++;

        fread(reg, tamElem, 1, fBin);
    }

    free(reg);

    fclose(fBin);
    fclose(fTxt);

    printf("[OK] Se exportaron %llu registros al archivo %s.\n", registrosExportados, rutaTxt);
    return TODO_OK;
}

/**
 * buscarRegistroPorId
 * Recorre el archivo binario desde el inicio buscando el registro
 * cuyo primer campo unsigned coincida con 'id'.
 * Retorna el offset en bytes del registro encontrado (para fseek),
 * o -1L si no existe.
 * ADVERTENCIA: hace rewind() al inicio; al retornar, el FILE*
 * queda posicionado despues del registro encontrado (o al final).
 */

long buscarRegistroPorId(FILE* fBin, const void* clave, void* reg, size_t tamElem, size_t tamClave)
{
    long offset = 0L;

    rewind(fBin); /** Posiciona al inicio para garantizar recorrido completo */

    while(fread(reg, tamElem, 1, fBin) == 1)
    {

        if(memcmp(reg, clave, tamClave) == 0)
            return offset;

        offset += (long)tamElem;
    }

    return -1L; /** No encontrado */
}

/**
 * archivoExiste
 *Si el usuario sale del programa con la opcion 0, genera un bug
 *Como no se borra los archivo que se estaban usando y tampoco se actualizan los archivos de texto
 *Hay registros basura, de operaciones anteriores que no se ven reflejados
 *Por lo tanto, si los archivos existen se usaran los datos que tiene y los que se agregen
 *Para evitar tener tantos archivos, la opcion 0 actualizara los de texto con los archivos binarios
 Nota: no me gusta mucha hacer un fopen para validar si existe o no un archivo, pero es la opcion optima
 */
int archivoExiste(const char* ruta)
{
    FILE* f = fopen(ruta, "rb");

    if(f)
    {
        fclose(f);
        return 1;
    }
    return 0;
}

