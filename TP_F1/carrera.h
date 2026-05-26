#ifndef CARRERA_H_INCLUDED
#define CARRERA_H_INCLUDED

#define RUTA_CARRERA_BIN            "carrera.bin"
#define RUTA_CARRERA_TXT            "carrera.txt"

#define ESTADO_CARRERA_INACTIVA     0
#define ESTADO_CARRERA_ACTIVA       1
#define TAM_NOMBRE_CIRCUITO         20
#define MAX_PILOTOS_CARRERA         20

#define COL_ID_PILOTO               0
#define COL_PUNTOS                  1
#define MAX_COL_RESULTADOS          2
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

int registrarCarrera(const char* rutaCarrera, const char* rutaPiloto, Comparar comparar);
int cargarResultadosCarreraAleatorios(const char* rutaPiloto, Carrera* nueva, Comparar comparar);
int generarIdCarrera(FILE* fCarrera);

/**Puntero a funcion**/
void mostrarCarrera(const void *dato);
#endif // CARRERA_H_INCLUDED
