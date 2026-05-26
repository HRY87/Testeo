#include <stdio.h>
#include <stdlib.h>
#include "piloto.h"
#include "utilidades.h"

int generarArchivoTxt(const char* rutaTxt)
{
    FILE* fTxt = fopen(rutaTxt, "wt");

    if(!fTxt)
        return ERR_ARCH;

    fprintf(fTxt, "1,Max Verstappen,Neerlandes,1,0,A,20020930\n");
    fprintf(fTxt, "2,Lewis Hamilton,Britanico,2,0,A,19850107\n");
    fprintf(fTxt, "3,Charles Leclerc,Monegasco,3,0,A,19971016\n");
    fprintf(fTxt, "4,Carlos Sainz,Espanol,3,0,A,19940901\n");
    fprintf(fTxt, "5,Fernando Alonso,Espanol,4,0,A,19810729\n");

    fclose(fTxt);
    return TODO_OK;
}

int cargarArchivoPilotos(const char* rutaTxt, const char* rutaBin)
{
    Piloto piloto;
    int camposLeidos = 0;
    int total = 0;

    /**Necesario para obtener las cadenas con fscanf**/
    char bufferNombre[TAM_NOMBRE_PILOTO];
    char bufferNacionalidad[TAM_NACIONALIDAD];

    FILE* fTxt = fopen(rutaTxt, "rt");
    FILE* fBin = fopen(rutaBin, "wb");

    if(!fTxt)
        return total;

    if(!fBin)
    {
        fclose(fTxt);
        return total;
    }

    camposLeidos = fscanf(fTxt,
                          "%u,%29[^,],%29[^,],%u,%u ,%c,%llu\n",
                          &piloto.id, bufferNombre, bufferNacionalidad,
                          &piloto.id_escuderia, &piloto.puntos_acumulados,
                          &piloto.estado,&piloto.fechaNacimiento);

    /**Si estan todos los campos, ahi solo escribe**/
    while(camposLeidos == 7)
    {
        copiarCadena(piloto.nombre, bufferNombre, TAM_NOMBRE_PILOTO);
        copiarCadena(piloto.nacionalidad, bufferNacionalidad, TAM_NACIONALIDAD);

        fwrite(&piloto, sizeof(Piloto), 1, fBin);
        total++;

        camposLeidos = fscanf(fTxt,
                              "%u,%29[^,],%29[^,],%u,%u ,%c,%llu\n",
                              &piloto.id, bufferNombre, bufferNacionalidad,
                              &piloto.id_escuderia, &piloto.puntos_acumulados,
                              &piloto.estado,&piloto.fechaNacimiento);
    }

    fclose(fTxt);
    fclose(fBin);

    return total;
}
size_t listarPilotos(const char* rutaBin)
{
    Piloto piloto;
    size_t listados = 0;

    FILE* fBin = fopen(rutaBin, "rb");

    if(!fBin)
        return listados;

    printf("\n");
    printf("=============================================================\n");
    printf("  LISTADO DE PILOTOS - TEMPORADA\n");
    printf("=============================================================\n");
    printf("%-4s  %-28s  %-10s  %s\n",
           "ID", "Nombre", "Estado", "Puntos");
    printf("-------------------------------------------------------------\n");

    while(fread(&piloto, sizeof(Piloto), 1, fBin) == 1)
    {
        printf("%-4u  %-28s  %-10c  %u\n",
               piloto.id, piloto.nombre, piloto.estado, piloto.puntos_acumulados);

        listados++;
    }
    printf("-------------------------------------------------------------\n");

    fclose(fBin);

    return listados;
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
