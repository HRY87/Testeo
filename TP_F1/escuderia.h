#ifndef ESCUDERIA_H_INCLUDED
#define ESCUDERIA_H_INCLUDED

#define TAM_CODIGO                  4
#define TAM_NOMBRE_ESCUDERIA        30
#define TAM_PAIS                    50
#define ESTADO_ESCUDERIA_INACTIVA   0
#define ESTADO_ESCUDERIA_ACTIVA     1
typedef struct
{
    unsigned id;
    char codigo[TAM_CODIGO];
    char nombre[TAM_NOMBRE_ESCUDERIA];
    char pais[TAM_PAIS];
    int estado; /** 1 o 0**/
}Escuderia;

#endif // ESCUDERIA_H_INCLUDED
