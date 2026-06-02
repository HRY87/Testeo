#ifndef SANCION_H_INCLUDED
#define SANCION_H_INCLUDED

/* =========================================================
   Rutas de archivos del TDA Sancion
   ========================================================= */
#define RUTA_SANCION_BIN  "archivos/sancion.dat"
#define RUTA_SANCION_TXT  "archivos/sancion.txt"

/* =========================================================
   Parametros configurables de sanciones
   Modificar estos #define para ajustar las reglas de juego.
   ========================================================= */

/* Cantidad de DSQ acumulados que provocan suspension del piloto */
#define UMBRAL_DSQ_SUSPENSION    2

/* Puntos que se descuentan al piloto cuando el tipo incluye puntos */
#define PUNTOS_PENALIZACION_DSQ  10

/* =========================================================
   Tipos de sancion (campo 'tipo' en Sancion)
   Se pueden combinar con OR si se extiende a bitmask,
   pero por ahora son valores mutuamente excluyentes.
   ========================================================= */
#define SANCION_PUNTOS   1   /* Solo descuento de puntos          */
#define SANCION_CARRERA  2   /* Solo saltar N carreras            */
#define SANCION_AMBAS    3   /* Descuento de puntos + saltear carrera */

/* =========================================================
   Estructura Sancion
   Registro persistido en sancion.bin cuando se aplica
   una penalizacion a un piloto (tipicamente por DSQ).
   ========================================================= */
typedef struct
{
    unsigned           id_piloto;   /* piloto sancionado               */
    unsigned           id_carrera;  /* carrera que origino la sancion  */
    int                tipo;        /* SANCION_PUNTOS/CARRERA/AMBAS    */
    int                puntos;      /* puntos a descontar (si aplica)  */
    int                carreras;    /* carreras a saltear (si aplica)  */
    unsigned long long fecha;       /* fecha de la carrera (AAAAMMDD)  */
} Sancion;

/* =========================================================
   Funciones del TDA Sancion
   ========================================================= */

/**
 * registrarSancion
 * Persiste una Sancion en sancion.bin (modo append).
 * Retorna TODO_OK o ERR_ARCH.
 */
int registrarSancion(const char* rutaSancion, const Sancion* s);

/**
 * contarDSQPiloto
 * Cuenta cuantas sanciones tiene el piloto en sancion.bin.
 * Se usa para decidir si supera UMBRAL_DSQ_SUSPENSION.
 * Retorna la cantidad (>= 0) o -1 si no se puede leer el archivo.
 */
int contarDSQPiloto(const char* rutaSancion, unsigned idPiloto);

/**
 * aplicarSancionDSQ
 * Orquesta la penalizacion completa cuando un piloto es DSQ:
 *   1. Crea y persiste la Sancion segun el tipo elegido.
 *   2. Si tipo incluye SANCION_PUNTOS: descuenta puntos en piloto.bin.
 *   3. Cuenta DSQ acumulados; si >= UMBRAL_DSQ_SUSPENSION
 *      cambia el estado del piloto a 'S' en piloto.bin.
 *
 * Parametros:
 *   rutaSancion  ruta a sancion.bin
 *   rutaPiloto   ruta a piloto.bin
 *   idPiloto     id del piloto a sancionar
 *   idCarrera    id de la carrera origen
 *   fechaCarrera fecha de la carrera (AAAAMMDD)
 *   tipo         SANCION_PUNTOS, SANCION_CARRERA o SANCION_AMBAS
 *   carreras     carreras a saltear (0 si tipo == SANCION_PUNTOS)
 *
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int aplicarSancionDSQ(const char*        rutaSancion,
                      const char*        rutaPiloto,
                      unsigned           idPiloto,
                      unsigned           idCarrera,
                      unsigned long long fechaCarrera,
                      int                tipo,
                      int                carreras);

/**
 * mostrarSancion  [Mostrar]
 * Imprime una Sancion formateada por pantalla.
 */
void mostrarSancion(const void* dato);

/**
 * escribirSancionTxt  [Accion / BinATxt wrapper]
 * Escribe una Sancion en formato CSV al archivo texto.
 */
void sancionBinATxt(const void* dato, FILE* archTxt);

#endif // SANCION_H_INCLUDED
