#include <stdio.h>
#include <string.h>
#include "escuderia.h"
#include "utilidades.h"

/* =========================================================
                    Lote de prueba inicial
   ========================================================= */

/**
 * generarArchivoEscuderiasTxt
 * Crea el archivo escuderia.txt con el lote inicial.
 * Usa generarArchivoTexto() + escribirEscuderiaTxt().
 * Retorna TODO_OK o ERR_ARCH.
 */
int generarArchivoEscuderiasTxt(const char* rutaTxt)
{
    Escuderia lote[6] = {
        {1, "RBR", "Red Bull Racing", "Austria",    ESTADO_ESCUDERIA_ACTIVA},
        {2, "MCL", "McLaren",         "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
        {3, "FER", "Ferrari",         "Italia",     ESTADO_ESCUDERIA_ACTIVA},
        {4, "WLF", "Williams",        "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
        {5, "AMR", "Aston Martin",    "Reino Unido", ESTADO_ESCUDERIA_ACTIVA},
        {6, "SAU", "Sauber",          "Suiza",      ESTADO_ESCUDERIA_ACTIVA}
    };

    return generarArchivoTexto(rutaTxt, lote, 6, sizeof(Escuderia), escribirEscuderiaTxt);
}

/* =========================================================
            Punteros a funcion del TDA Escuderia
   ========================================================= */

/**
 * trozarEscuderiaTxt  [TxtABin]
 * Parsea una linea CSV con el formato:
 *   id,codigo,nombre,pais,estado
 * Retorna TODO_OK si se leyeron los 5 campos, ERR_LINEA si no.
 */
int trozarEscuderiaTxt(char* linea, void* reg)
{
    Escuderia* e   = (Escuderia*)reg;
    char*   act = strchr(linea, '\n');

    if(!act)
        return ERR_LINEA;

    *act = '\0';

    /* Estado */
    act = strrchr(linea, ',');
    e->estado = *(act + 1);
    *act = '\0';

    /* Pais */
    act = strrchr(linea, ',');
    copiarCadena(e->pais, act + 1, TAM_PAIS);
    *act = '\0';

    /* Nombre */
    act = strrchr(linea, ',');
    copiarCadena(e->nombre, act + 1, TAM_NOMBRE_ESCUDERIA);
    *act = '\0';

    /* Codigo */
    act = strrchr(linea, ',');
    copiarCadena(e->codigo, act + 1, TAM_CODIGO);
    *act = '\0';

    /* Id — lo que queda al inicio */
    sscanf(linea, "%u", &e->id);

    return TODO_OK;
}

/**
 * escuderiaBinATxt  [BinATxt]
 * Escribe una Escuderia en formato CSV en el archivo de texto.
 */
void escuderiaBinATxt(const void* dato, FILE* archTxt)
{
    const Escuderia* e = (const Escuderia*)dato;

    fprintf(archTxt, "%u,%s,%s,%s,%d\n",
            e->id,
            e->codigo,
            e->nombre,
            e->pais,
            e->estado);
}

/**
 * escribirEscuderiaTxt  [Accion]
 * Misma logica que escuderiaBinATxt pero con firma Accion.
 * Se usa con generarArchivoTexto() para el lote inicial.
 */
int escribirEscuderiaTxt(void* archTxt, const void* dato)
{
    escuderiaBinATxt(dato, (FILE*)archTxt);
    return TODO_OK;
}

/**
 * mostrarEscuderia  [Mostrar]
 * Imprime una Escuderia formateada por pantalla.
 */
void mostrarEscuderia(const void* dato)
{
    const Escuderia* e = (const Escuderia*)dato;

    printf("%-4u  [%s]  %-25s  %-15s  %s\n",
           e->id,
           e->codigo,
           e->nombre,
           e->pais,
           e->estado == ESTADO_ESCUDERIA_ACTIVA ? "Activa" : "Inactiva");
}

/**
 * esEscuderiaActiva  [Filter]
 * Retorna 1 si la escuderia tiene estado ACTIVA, 0 si no.
 */
int esEscuderiaActiva(const void* dato)
{
    return (((const Escuderia*)dato)->estado == ESTADO_ESCUDERIA_ACTIVA);
}

