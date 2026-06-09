#ifndef PUNTOS_H_INCLUDED
#define PUNTOS_H_INCLUDED

/* =========================================================
   Tabla de puntos configurable
   Cargada desde archivo o con valores F1 por defecto.
   ========================================================= */
#define MAX_POSICIONES_PUNTOS  20   /* maximo de posiciones con puntos */
#define RUTA_PUNTOS_BIN        "archivos/puntos.dat"

typedef struct
{
    int  posiciones;            /* cuantas posiciones otorgan puntos */
    int  tabla[MAX_POSICIONES_PUNTOS]; /* tabla[0] = puntos pos 1, etc. */
} Puntos;

/* Inicializa con la tabla estandar de F1 (10 posiciones) */
void  inicializarPuntosDefault(Puntos* vPuntos);

/* Guarda la tabla en disco */
int   guardarConfigPuntos(const char* ruta, const Puntos* vPuntos);

/* Carga desde disco; si no existe, usa valores por defecto */
int   cargarConfigPuntos(const char* ruta, Puntos* vPuntos);

/* Retorna los puntos para una posicion (base 1). 0 si fuera de rango. */
int   puntosParaPosicion(const Puntos* vPuntos, int posicion);

/* Muestra la tabla de puntos por pantalla */
void  mostrarConfigPuntos(const Puntos* vPuntos);

#endif // PUNTOS_H_INCLUDED
