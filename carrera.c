#include <stdio.h>
#include <string.h>
#include "utilidades.h"
#include "carrera.h"
#include "piloto.h"
#include "vector.h"

//Esto hay que mejorarlo, por ahora lo dejo fijo
static const int puntos_f1[11] =
{
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

    /**Limpiar el \n que dejo el scanf del menu**/
    limpiarBuffer();

    printf("\nNombre del circuito: ");
    leerCadena(nueva.circuito, TAM_NOMBRE_CIRCUITO);

    do
    {
        printf("\nFecha de la carrera (AAAAMMDD): ");
        scanf("%llu", &nueva.fecha);
    }
    while(!esFechaValida(nueva.fecha));


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

int recalcularPuntosPilotos(const char* rutaCarrera, const char* rutaPiloto)
{
    tVector  vPilotos, vCarreras;
    Carrera* carrera; //Puntero auxiliar
    Piloto*  piloto; //Puntero auxiliar
    char    *act, *fin; //Lectura byte a byte
    unsigned idBuscar;
    size_t      i;

    /** 1. Cargar pilotos en memoria **/
    if(crearVector(&vPilotos, sizeof(Piloto), 20))
        return SIN_MEM;

    if(cargarVectorDesdeBin(rutaPiloto, &vPilotos))
    {
        destruirVector(&vPilotos);
        return ERR_ARCH;
    }

    /** 2. Seteo su valor a 0, para evitar basura **/
    act = (char*)vPilotos.vec;
    fin = act + (vPilotos.ce * vPilotos.tamElem);

    while(act < fin)
    {
        ((Piloto*)act)->puntos_acumulados = 0;
        act += vPilotos.tamElem;
    }

    /** 3. Cargar carreras en memoria **/
    if(crearVector(&vCarreras, sizeof(Carrera), MAX_CARRERAS_TEMPORADA))
    {
        destruirVector(&vPilotos);
        return SIN_MEM;
    }

    /** Si no hay carreras aun, guardamos pilotos con puntos en 0 y salimos **/
    if(cargarVectorDesdeBin(rutaCarrera, &vCarreras))
    {
        guardarVectorEnBin(rutaPiloto, &vPilotos);
        destruirVector(&vPilotos);
        destruirVector(&vCarreras);
        return TODO_OK;
    }

    /** 4. Recorrer carreras y acumulamos sus puntos **/
    act = (char*)vCarreras.vec;
    fin = act + (vCarreras.ce * vCarreras.tamElem);

    /**5. Actualizamos sus puntos acumulados**/
    while(act < fin)
    {
        carrera = (Carrera*)act;

        //Demasiadas condiciones, buscar una alternativa
        if(carrera->estado == ESTADO_CARRERA_ACTIVA)
        {
            for(i = 0; i < carrera->cant_resultados; i++)
            {
                if(carrera->resultados[i][COL_PUNTOS] > 0)
                {
                    idBuscar = (unsigned)carrera->resultados[i][COL_ID_PILOTO];

                    /** El vector debe estar ordenado por id **/
                    piloto = (Piloto*)busquedaBinariaVector(&vPilotos,
                                                            &idBuscar,
                                                            compararUnsigned);
                    if(piloto)
                        piloto->puntos_acumulados += (unsigned)carrera->resultados[i][COL_PUNTOS];
                }
            }
        }
        act += vCarreras.tamElem;
    }

    /** 6. Guardar y liberar memoria **/
    guardarVectorEnBin(rutaPiloto, &vPilotos);
    destruirVector(&vPilotos);
    destruirVector(&vCarreras);

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
    Carrera *c;
    int i;
    unsigned dia, mes, anio;

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
