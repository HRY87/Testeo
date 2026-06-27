#ifndef CARRERA_H_INCLUDED
#define CARRERA_H_INCLUDED

#include "utilidades.h"
#include "puntos.h"

#define RUTA_CARRERA_BIN        "carreras.dat"
#define RUTA_CARRERA_TXT        "carreras.txt"
#define RUTA_CARRERA_EXP_TXT    "carrera_exportada.txt"
#define RUTA_BAJAS_CARRERA_TXT  "bajas_carreras.txt"


#define ESTADO_CARRERA_INACTIVA     0
#define ESTADO_CARRERA_ACTIVA       1

#define TAM_NOMBRE_CIRCUITO         20
#define MAX_COL_RESULTADOS          2
#define MAX_PILOTOS_CARRERA         20
#define MAX_CARRERAS_TEMPORADA      30
#define POS_LIMITE_PUNTOS_CARRERA   10


typedef struct
{
    int                id;
    char               circuito[TAM_NOMBRE_CIRCUITO];
    unsigned long long fecha;           /* formato AAAAMMDD          */
    int                estado;          /* ESTADO_CARRERA_ACTIVA / INACTIVA */
    int                cant_resultados;
    int                resultados[MAX_PILOTOS_CARRERA][MAX_COL_RESULTADOS];

} Carrera;

    /* resultados[i][COL_ID_PILOTO] = id del piloto en posicion i+1  */
    /* resultados[i][COL_PUNTOS]    = puntos obtenidos en esa posicion */

/*Generar una carrera*/
int registrarCarreraAleatoria(const char* rutaCarrera, const char* rutaPiloto,Comparar comparar, const Puntos* pts);
int registrarCarreraManual(const char* rutaCarrera, const char* rutaPiloto,Comparar comparar, const Puntos* pts);

/*Funciones que se encargar de los puntos*/
int cargarResultadosCarreraAleatorios(const char* rutaPiloto, Carrera* nueva, Comparar comparar, const Puntos* pts);
int recalcularPuntosPilotos(const char* rutaCarrera, const char* rutaPiloto);
int recalcularPuntosPilotos_Old(const char* rutaCarrera, const char* rutaPiloto);
int aplicarPuntosUltimaCarrera(const char* rutaCarrera, const char* rutaPiloto);

/*Generacion de id automatico*/
int generarIdCarrera(FILE* fCarrera);

/*Punteros a funcion del TDA Carrera*/
int carreraBinATxt(const void* dato, FILE* archTxt);
void mostrarCarrera(const void* dato);


int exportarCarrerasTxt(const char* rutaBin, const char* rutaTxtExportado);

/*ABM*/
int bajaCarrera(const char* rutaBin, const char* rutaBajasTxt);
int modificarCarrera(const char* rutaBin);

#endif /* CARRERA_H_INCLUDED */
