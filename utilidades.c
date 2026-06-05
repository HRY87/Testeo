#include <string.h>
#include <memory.h>
#include <ctype.h>
#include "utilidades.h"

/* =========================================================
            Funciones de cadena y utiles generales
   ========================================================= */

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

int leerCadena(char* dest, size_t n)
{
    char* pos;

    if (!dest || n == 0)
        return ERR_LINEA;

    if (!fgets(dest, (int)n, stdin))
        return ERR_LINEA;

    pos = strrchr(dest, '\n');
    if (pos)
        *pos = '\0';

    return TODO_OK;
}

void limpiarBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

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
         Lectura segura de tipos numericos desde stdin

   Estrategia unica para todos los tipos:
   1. Leer la linea completa con fgets en un buffer.
   2. Intentar convertir con sscanf.
   3. Verificar que NO queden caracteres no numericos
      despues del numero (ej: "12abc" se rechaza).
   4. Verificar rango si corresponde.
   5. Si cualquier paso falla: mensaje y reintento.

   De esta forma el buffer de stdin nunca queda sucio
   porque fgets ya consumio la linea entera.
   ========================================================= */

/*
 * esLineaNumerica
 * Retorna 1 si la cadena representa un entero sin signo
 * (solo digitos, opcionalmente precedidos de espacios).
 * Se usa para rechazar entradas como "12abc" o "3.5".
 */
static int esLineaNumerica(const char* linea)
{
    const char* p = linea;

    /* Saltear espacios iniciales */
    while (*p == ' ' || *p == '\t') p++;

    if (*p == '\0') return 0;   /* linea vacia */

    while (*p)
    {
        if (!isdigit((unsigned char)*p)) return 0;
        p++;
    }
    return 1;
}

int leerInt(int* dest, int min, int max)
{
    char buf[TAM_LINEA];
    int  val;
    int  leido;

    do
    {
        if (!fgets(buf, (int)sizeof(buf), stdin))
        {
            printf("[!] Error de lectura.\n");
            continue;
        }

        /* Eliminar salto de linea */
        {
            char* nl = strrchr(buf, '\n');
            if (nl) *nl = '\0';
        }

        leido = sscanf(buf, "%d", &val);

        if (leido != 1 || !esLineaNumerica(buf))
        {
            printf("[!] Entrada invalida. Ingrese un numero entero"
                   " entre %d y %d: ", min, max);
            continue;
        }

        if (val < min || val > max)
        {
            printf("[!] Valor fuera de rango [%d, %d]: ", min, max);
            continue;
        }

        *dest = val;
        return TODO_OK;

    } while (1);
}

int leerUnsigned(unsigned* dest, unsigned min, unsigned max)
{
    char     buf[TAM_LINEA];
    unsigned val;
    int      leido;

    do
    {
        if (!fgets(buf, (int)sizeof(buf), stdin))
        {
            printf("[!] Error de lectura.\n");
            continue;
        }

        {
            char* nl = strrchr(buf, '\n');
            if (nl) *nl = '\0';
        }

        if (!esLineaNumerica(buf))
        {
            printf("[!] Entrada invalida. Ingrese un numero positivo"
                   " entre %u y %u: ", min, max);
            continue;
        }

        leido = sscanf(buf, "%u", &val);

        if (leido != 1)
        {
            printf("[!] Entrada invalida. Ingrese un numero positivo"
                   " entre %u y %u: ", min, max);
            continue;
        }

        if (val < min || val > max)
        {
            printf("[!] Valor fuera de rango [%u, %u]: ", min, max);
            continue;
        }

        *dest = val;
        return TODO_OK;

    } while (1);
}

int leerUnsignedLongLong(unsigned long long* dest)
{
    char               buf[TAM_LINEA];
    unsigned long long val;
    int                leido;

    do
    {
        if (!fgets(buf, (int)sizeof(buf), stdin))
        {
            printf("[!] Error de lectura.\n");
            continue;
        }

        {
            char* nl = strrchr(buf, '\n');
            if (nl) *nl = '\0';
        }

        if (!esLineaNumerica(buf))
        {
            printf("[!] Entrada invalida. Ingrese solo digitos numericos: ");
            continue;
        }

        leido = sscanf(buf, "%llu", &val);

        if (leido != 1)
        {
            printf("[!] Entrada invalida. Ingrese solo digitos numericos: ");
            continue;
        }

        *dest = val;
        return TODO_OK;

    } while (1);
}

int leerChar(char* dest, const char* validos)
{
    char        buf[TAM_LINEA];
    const char* p;
    char        c;

    do
    {
        if (!fgets(buf, (int)sizeof(buf), stdin))
        {
            printf("[!] Error de lectura.\n");
            continue;
        }

        {
            char* nl = strrchr(buf, '\n');
            if (nl) *nl = '\0';
        }

        /* Solo aceptar exactamente un caracter (sin espacios extra) */
        if (buf[0] == '\0' || buf[1] != '\0')
        {
            printf("[!] Ingrese exactamente un caracter (%s): ", validos);
            continue;
        }

        c = buf[0];

        /* Buscar en la cadena de validos */
        p = validos;
        while (*p)
        {
            if (*p == c)
            {
                *dest = c;
                return TODO_OK;
            }
            p++;
        }

        printf("[!] Caracter invalido. Opciones validas: %s: ", validos);

    } while (1);
}

/* =========================================================
            Funciones genericas para archivos
   ========================================================= */

int generarArchivoTexto(const char* rutaTxt,
                        const void* datos,
                        size_t      cantElem,
                        size_t      tamElem,
                        Accion      escribir)
{
    size_t      i;
    const char* pLec;
    FILE*       fTxt;

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

int convertirArchivoTxtABin(const char* rutaTxt,
                            const char* rutaBin,
                            size_t      tamElem,
                            TxtABin     txtABin)
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
    }

    free(reg);
    free(linea);
    fclose(fTxt);
    fclose(fBin);

    return TODO_OK;
}

int convertirArchivoBinATxt(const char* rutaBin,
                            const char* rutaTxt,
                            size_t      tamElem,
                            BinATxt     binATxt)
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

int mostrarArchivoBinario(const char* rutaBin,
                          size_t      tamElem,
                          Mostrar     mostrar)
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

int procesarArchivoBinario(const char* rutaBin,
                           void*       datos,
                           size_t      tamElem,
                           Filter      filtrar,
                           Accion      procesar)
{
    void*  reg;
    FILE*  fBin;

    if (!filtrar || !procesar)
        return ERR_LINEA;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    reg = malloc(tamElem);
    if (!reg)
    {
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
   modificarRegistroEnBin
   Busca el registro por clave y lo sobreescribe en disco.
   No mueve otros registros: solo hace fseek + fwrite sobre
   el registro encontrado.
   ========================================================= */
int modificarRegistroEnBin(const char*   rutaBin,
                           size_t        tamElem,
                           const void*   clave,
                           Comparar      comparar,
                           ObtenerClave  obtenerClave,
                           ModificarReg  modificar)
{
    FILE*  fBin;
    void*  reg;
    void*  claveReg;
    long   pos;
    int    encontrado;

    fBin = fopen(rutaBin, "r+b");   /* lectura/escritura sin truncar */
    if (!fBin)
        return ERR_ARCH;

    reg      = malloc(tamElem);
    claveReg = malloc(tamElem);     /* tamanio maximo posible para la clave */

    if (!reg || !claveReg)
    {
        free(reg);
        free(claveReg);
        fclose(fBin);
        return SIN_MEM;
    }

    encontrado = 0;

    while (!encontrado && fread(reg, tamElem, 1, fBin) == 1)
    {
        obtenerClave(reg, claveReg);

        if (comparar(clave, claveReg) == 0)
        {
            /* Retroceder exactamente un registro para sobreescribir */
            pos = ftell(fBin);
            fseek(fBin, pos - (long)tamElem, SEEK_SET);

            modificar(reg);
            fwrite(reg, tamElem, 1, fBin);
            encontrado = 1;
        }
    }

    free(reg);
    free(claveReg);
    fclose(fBin);

    return encontrado ? TODO_OK : NO_ENCONTRADO;
}

/* =========================================================
            Funciones para fechas (formato AAAAMMDD)
   ========================================================= */

int diasPorMes(unsigned mes, unsigned anio)
{
    static const int dias[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

    if (mes == 2)
        return ES_ANIO_BISIESTO(anio) ? 29 : 28;

    return dias[mes];
}

int esFechaValida(unsigned long long fecha)
{
    unsigned anio = (unsigned)(fecha / 10000);
    unsigned mes  = (unsigned)(fecha / 100 % 100);
    unsigned dia  = (unsigned)(fecha % 100);

    if (anio > ANIO_BASE             &&
        mes  >= 1 && mes  <= 12      &&
        dia  >= 1 && dia  <= (unsigned)diasPorMes(mes, anio))
        return 1;

    return 0;
}

/* =========================================================
                Comparadores genericos
   ========================================================= */

int compararUnsigned(const void* a, const void* b)
{
    unsigned ua = *(const unsigned*)a;
    unsigned ub = *(const unsigned*)b;
    return (ua > ub) - (ua < ub);
}

int compararInt(const void* a, const void* b)
{
    return (*(const int*)a - *(const int*)b);
}
