#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilidades.h"
#include "carrera.h"
#include "piloto.h"
#include "vector.h"

/**Esto hay que mejorarlo, por ahora lo dejo fijo**/
static const int puntos_f1[11] = {
    0, 25, 18, 15, 12, 10, 8, 6, 4, 2, 1
};

int registrarCarrera(const char* rutaCarrera, const char* rutaPiloto, Comparar comparar)
{
    Carrera nueva;

    FILE* fCarrera = fopen(rutaCarrera, "ab+");

    if(!fCarrera)
        return ERR_ARCH;

    memset(&nueva, 0, sizeof(Carrera));

    nueva.id = generarIdCarrera(fCarrera);
    nueva.estado = ESTADO_CARRERA_ACTIVA;

    printf("\nNombre del circuito: ");
    scanf("%19s", nueva.circuito);

    printf("\nFecha de la carrera (AAAAMMDD): ");
    scanf("%llu", &nueva.fecha);

    if(cargarResultadosCarreraAleatorios(rutaPiloto, &nueva, comparar))
    {
        fclose(fCarrera);
        return ERR_ARCH;
    }

    fseek(fCarrera, 0L, SEEK_END);
    fwrite(&nueva, sizeof(Carrera), 1, fCarrera);
    fclose(fCarrera);

    return TODO_OK;
}

int cargarResultadosCarreraAleatorios(const char* rutaPiloto, Carrera* nueva, Comparar comparar)
{
    tVector vIds;
    int i, pos;

    if(crearVector(&vIds, sizeof(unsigned), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if(cargarVectorPilotoActivos(rutaPiloto, &vIds, comparar))
    {
        destruirVector(&vIds);
        return ERR_ARCH;
    }

    if(vIds.ce == 0)
    {
        destruirVector(&vIds);
        return ERR_ARCH;
    }

    generarResultadoAleatorioVector(&vIds);

    nueva->cant_resultados = (int)vIds.ce;

    for(i = 0; i < nueva->cant_resultados; i++)
    {
        pos = *(unsigned*)obtenerElementoVector(&vIds, i);

        nueva->resultados[i][COL_ID_PILOTO] = pos;
        /** Solo los primeros 10 puestos suman puntos en F1 **/
        nueva->resultados[i][COL_PUNTOS] = (i < POS_LIMITE_PUNTOS_CARRERA)
                                        ? puntos_f1[i + 1]
                                        : 0;
    }

    destruirVector(&vIds);

    return TODO_OK;
}

int generarIdCarrera(FILE* fCarrera)
{
    Carrera ultima;

    fseek(fCarrera, 0, SEEK_END);

    /**Si esta vacio sera la primera**/
    if(ftell(fCarrera) < (long)sizeof(Carrera))
        return 1;

    /**Me posicion en la ultima carrera ingresada, para obtener el id**/
    fseek(fCarrera, -(long)sizeof(Carrera), SEEK_END);
    fread(&ultima, sizeof(Carrera), 1, fCarrera);

    return (ultima.id + 1);
}


void mostrarCarrera(const void *dato)
{
    Carrera  *c;
    int       i;
    unsigned  dia;
    unsigned  mes;
    unsigned  anio;

    c = (Carrera *)dato;

    anio = (unsigned)(c->fecha / 10000);
    mes  = (unsigned)(c->fecha / 100 % 100);
    dia  = (unsigned)(c->fecha % 100);

    printf("----------------------------------\n");
    printf("ID Carrera  : %d\n",  c->id);
    printf("Circuito    : %s\n",  c->circuito);
    printf("Fecha       : %02u/%02u/%04u\n", dia, mes, anio);
    printf("Estado      : %s\n",  c->estado == 1 ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n", c->cant_resultados);

    for (i = 0; i < c->cant_resultados; i++)
    {
        printf("  Pos %2d | Piloto ID: %3d | Puntos: %2d\n",
               i + 1,                              /* posicion = fila i  */
               c->resultados[i][COL_ID_PILOTO],    /* columna 0          */
               c->resultados[i][COL_PUNTOS]);      /* columna 1          */
    }
}
