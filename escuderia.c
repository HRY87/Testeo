#include <stdio.h>
#include <string.h>
#include "escuderia.h"
#include "utilidades.h"

#define TAM_LOTE_ESCUDERIA      6

int generarArchivoEscuderiasTxt(const char* rutaTxt)
{
    Escuderia lote[TAM_LOTE_ESCUDERIA] = {
        {1, "RBR", "Red Bull Racing", "Austria",      ESTADO_ESCUDERIA_ACTIVA},
        {2, "MCL", "McLaren",         "Reino Unido",  ESTADO_ESCUDERIA_ACTIVA},
        {3, "FER", "Ferrari",         "Italia",       ESTADO_ESCUDERIA_ACTIVA},
        {4, "WLF", "Williams",        "Reino Unido",  ESTADO_ESCUDERIA_ACTIVA},
        {5, "AMR", "Aston Martin",    "Reino Unido",  ESTADO_ESCUDERIA_ACTIVA},
        {6, "SAU", "Sauber",          "Suiza",        ESTADO_ESCUDERIA_ACTIVA}
    };

    return generarArchivoTexto(rutaTxt, lote, TAM_LOTE_ESCUDERIA, sizeof(Escuderia), escribirEscuderiaTxt);
}

/** Punteros a funcion **/
int escribirEscuderiaTxt(void* accion, const void* dato)
{
    FILE* txt = (FILE*)accion;
    const Escuderia* e   = (const Escuderia*)dato;

    if(!txt || !e)
        return ERR_ARCH;

    fprintf(txt, "%u,%s,%s,%s,%d\n",
            e->id,
            e->codigo,
            e->nombre,
            e->pais,
            e->estado);

    return TODO_OK;
}

void mostrarEscuderia(const void* dato)
{
    const Escuderia* e = (const Escuderia*)dato;

    printf("----------------------------------\n");
    printf("ID      : %u\n",  e->id);
    printf("Codigo  : %s\n",  e->codigo);
    printf("Nombre  : %s\n",  e->nombre);
    printf("Pais    : %s\n",  e->pais);
    printf("Estado  : %s\n",  e->estado == ESTADO_ESCUDERIA_ACTIVA ? "Activa" : "Inactiva");
}

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
