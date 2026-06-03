#ifndef RESULTADO_H_INCLUDED
#define RESULTADO_H_INCLUDED

/* =========================================================
   TDA ResultadoPiloto
   Representa el resultado de UN piloto en UNA carrera.
   Es la unidad que se almacena en el tVector de Carrera
   y tambien el registro serializado en carrera.bin tras
   cada CarreraHeader.
   ========================================================= */

/* ---------------------------------------------------------
   Estados de resultado de carrera (campo estado_resultado)
   --------------------------------------------------------- */
#define RES_FIN  1   /* Termino la carrera normalmente     */
#define RES_DNF  2   /* Did Not Finish  - 0 puntos         */
#define RES_DNS  3   /* Did Not Start   - 0 puntos         */
#define RES_DSQ  4   /* Descalificado   - puntos segun regla */

/* ---------------------------------------------------------
   Indices de columna (compatibilidad con codigo existente)
   Se mantienen aqui porque ResultadoPiloto los define.
   --------------------------------------------------------- */
#define COL_ID_PILOTO        0
#define COL_PUNTOS           1
#define COL_ESTADO_RESULTADO 2

/* ---------------------------------------------------------
   Estructura
   --------------------------------------------------------- */
typedef struct
{
    unsigned id_piloto;
    int      puntos;           /* puntos asignados (puede ser 0 o negativo si DSQ) */
    int      estado_resultado; /* RES_FIN, RES_DNF, RES_DNS, RES_DSQ  */
} ResultadoPiloto;

#endif // RESULTADO_H_INCLUDED
