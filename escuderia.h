#ifndef ESCUDERIA_H_INCLUDED
#define ESCUDERIA_H_INCLUDED

#define TAM_CODIGO                  4
#define TAM_NOMBRE_ESCUDERIA        30
#define TAM_PAIS                    50

#define ESTADO_ESCUDERIA_INACTIVA   0
#define ESTADO_ESCUDERIA_ACTIVA     1

#define RUTA_ESCUDERIA_BIN          "escuderia.bin"
#define RUTA_ESCUDERIA_TXT          "escuderia.txt"

typedef struct
{
    unsigned id;
    char codigo[TAM_CODIGO];
    char nombre[TAM_NOMBRE_ESCUDERIA];
    char pais[TAM_PAIS];
    int estado; /** 1 o 0**/
}Escuderia;

int generarArchivoEscuderiasTxt(const char* rutaTxt);

/** Punteros a funcion **/
int escribirEscuderiaTxt(void* accion, const void* dato);
void mostrarEscuderia(const void* dato);
int trozarEscuderiaTxt(char* linea, void* reg);

#endif // ESCUDERIA_H_INCLUDED
