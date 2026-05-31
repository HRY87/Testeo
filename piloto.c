#include <stdio.h>
#include <string.h>
#include "piloto.h"
#include "utilidades.h"

int generarArchivoPilotosTxt(const char* rutaTxt)
{
    Piloto lote[10] = {
        {1,  "Max Verstappen",  "Neerlandes",  1, 0, ESTADO_ACTIVO_PILOTO, 19970930},
        {2,  "Lando Norris",    "Britanico",   2, 0, ESTADO_ACTIVO_PILOTO, 19991113},
        {3,  "Charles Leclerc", "Monegasco",   3, 0, ESTADO_ACTIVO_PILOTO, 19971016},
        {4,  "Oscar Piastri",   "Australiano", 2, 0, ESTADO_ACTIVO_PILOTO, 20010406},
        {5,  "Carlos Sainz",    "Espanol",     4, 0, ESTADO_ACTIVO_PILOTO, 19940901},
        {6,  "George Russell",  "Britanico",   1, 0, ESTADO_ACTIVO_PILOTO, 19980215},
        {7,  "Lewis Hamilton",  "Britanico",   3, 0, ESTADO_ACTIVO_PILOTO, 19850107},
        {8,  "Fernando Alonso", "Espanol",     5, 0, ESTADO_ACTIVO_PILOTO, 19810729},
        {9,  "Lance Stroll",    "Canadiense",  5, 0, ESTADO_ACTIVO_PILOTO, 19981029},
        {10, "Nico Hulkenberg", "Aleman",      6, 0, ESTADO_ACTIVO_PILOTO, 19870819}
    };

    return generarArchivoTexto(rutaTxt, lote, 10, sizeof(Piloto), escribirPilotoTxt);
}



/**Funciones para manejo de datos TDA vector**/
//Filter
int esPilotoActivos(const void* dato)
{
    Piloto* p = (Piloto*)dato;

    return(p->estado == ESTADO_ACTIVO_PILOTO);
}

//Reduce
int sumarPuntos(void* acumulador, const void* dato)
{
    Piloto* p = (Piloto*)dato;

    *(unsigned*)acumulador += p->puntos_acumulados;

    return TODO_OK;
}

//Map
int extraerIdPuntos(void* dest, const void* orig)
{
    unsigned* resultado = (unsigned*)dest;
    Piloto* p = (Piloto*)orig;

    resultado[COL_ID_PILOTO] = p->id;
    resultado[COL_PUNTOS] = p->puntos_acumulados;

    return TODO_OK;
}


int cargarVectorPilotoActivos(const char* rutaBin, tVector* vIds, Comparar comparar)
{
    Piloto piloto;

    FILE* fPiloto = fopen(rutaBin, "rb");

    if(!fPiloto)
        return ERR_ARCH;

    while(fread(&piloto, sizeof(Piloto), 1, fPiloto) == 1)
    {
        if(piloto.estado == ESTADO_ACTIVO_PILOTO)
            insertarVectorOrd(vIds, & piloto.id, comparar);
    }

    fclose(fPiloto);

    return TODO_OK;
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

void mostrarPiloto(const void* dato)
{
    const Piloto* p = (const Piloto*)dato;

    printf("----------------------------------\n");
    printf("ID                 : %u\n",  p->id);
    printf("Nombre             : %s\n",  p->nombre);
    printf("Nacionalidad       : %s\n",  p->nacionalidad);
    printf("ID Escuderia       : %u\n",  p->id_escuderia);
    printf("Puntos Acumulados  : %u\n",  p->puntos_acumulados);
    printf("Estado             : %s\n",  (p->estado == ESTADO_ACTIVO_PILOTO ? "Activo" :
                                          (p->estado == ESTADO_RETIRADO_PILOTO ? "Retirado" : "Suspendido")));
    printf("Fecha Nacimiento   : %llu\n", p->fechaNacimiento);

}

int trozarPilotoTxt(char* linea, void* reg)
{
    Piloto* p   = (Piloto*)reg;
    char*   act = strchr(linea, '\n');

    if(!act)
        return ERR_LINEA;

    *act = '\0';

    /* fechaNacimiento */
    act = strrchr(linea, ',');
    sscanf(act + 1, "%llu", &p->fechaNacimiento);
    *act = '\0';

    /* estado */
    act = strrchr(linea, ',');
    p->estado = *(act + 1);
    *act = '\0';

    /* puntos_acumulados */
    act = strrchr(linea, ',');
    sscanf(act + 1, "%u", &p->puntos_acumulados);
    *act = '\0';

    /* id_escuderia */
    act = strrchr(linea, ',');
    sscanf(act + 1, "%u", &p->id_escuderia);
    *act = '\0';

    /* nacionalidad */
    act = strrchr(linea, ',');
    copiarCadena(p->nacionalidad, act + 1, TAM_NACIONALIDAD);
    *act = '\0';

    /* nombre */
    act = strrchr(linea, ',');
    copiarCadena(p->nombre, act + 1, TAM_NOMBRE_PILOTO);
    *act = '\0';

    /* id — lo que queda al inicio */
    sscanf(linea, "%u", &p->id);

    return TODO_OK;
}
