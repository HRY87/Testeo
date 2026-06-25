#ifndef CARRERA_H_INCLUDED
#define CARRERA_H_INCLUDED


#include "utilidades.h"
#include "puntos.h"

#define RUTA_CARRERA_BIN            "carreras.dat"
#define RUTA_CARRERA_TXT            "carreras.txt"
#define RUTA_CARRERA_EXP_TXT        "carrera_exportada.txt"
#define RUTA_BAJAS_CARRERA_TXT      "bajas_carreras.txt"

#define ESTADO_CARRERA_INACTIVA     0
#define ESTADO_CARRERA_ACTIVA       1
#define TAM_NOMBRE_CIRCUITO         20


#define MAX_COL_RESULTADOS          2
#define MAX_PILOTOS_CARRERA         20
#define MAX_CARRERAS_TEMPORADA      30
#define POS_LIMITE_PUNTOS_CARRERA   10

typedef struct
{
    int id;
    char circuito[TAM_NOMBRE_CIRCUITO];
    unsigned long long fecha;
    int estado;
    int cant_resultados;
    int resultados[MAX_PILOTOS_CARRERA][2]; /**Aun no definido**/
}Carrera;


int registrarCarreraAleatoria(const char* rutaCarrera, const char* rutaPiloto, Comparar comparar, const Puntos* pts);
int registrarCarreraManual(const char* rutaCarrera, const char* rutaPiloto, Comparar comparar, const Puntos* pts);
int cargarResultadosCarreraAleatorios(const char* rutaPiloto, Carrera* nueva, Comparar comparar, const Puntos* pts);
int generarIdCarrera(FILE* fCarrera);
int recalcularPuntosPilotos(const char* rutaCarrera, const char* rutaPiloto);
int recalcularPuntosPilotos_old(const char* rutaCarrera, const char* rutaPiloto);
int exportarCarrerasTxt(const char* rutaBin, const char* rutaTxtExportado);

/**Puntero a funcion**/
void mostrarCarrera(const void *dato);

/* --- Baja y modificacion --- */
int bajaCarrera      (const char* rutaBin, const char* rutaBajasTxt);
int modificarCarrera (const char* rutaBin);
#endif // CARRERA_H_INCLUDED
