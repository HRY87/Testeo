#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include "piloto.h"
#include "escuderia.h"
#include "utilidades.h"

static void pedirDatosPiloto(Piloto* p);
static int ingresarEstadoBaja(void);
static void mostrarCamposPiloto(const Piloto* p);

/**
 * pedirDatosPiloto  [static]
 * Solicita al usuario los datos editables de un piloto:
 * nombre, nacionalidad, ID de escuderia y fecha de nacimiento.
 * Llama a limpiarBuffer() al inicio para descartar residuos
 * del scanf anterior antes de usar leerCadena() con fgets.
 * La fecha se pide en un do-while hasta que esFechaValida()
 * confirme que el formato AAAAMMDD es coherente.
 */

/**
* ingresarEstadoBaja  [static]
* Presenta al usuario las opciones de tipo de baja y retorna
* el caracter de estado correspondiente:
*   [1] -> ESTADO_RETIRADO_PILOTO   ('R')
*   cualquier otra opcion -> ESTADO_SUSPENDIDO_PILOTO ('S')
* Se usa en bajaPiloto() una vez confirmado que el piloto esta activo.
*/
static void pedirDatosPiloto(Piloto* p)
{
    limpiarBuffer();

    printf("Nombre: ");
    leerCadena(p->nombre, TAM_NOMBRE_PILOTO);

    printf("Nacionalidad: ");
    leerCadena(p->nacionalidad, TAM_NACIONALIDAD);

    printf("ID Escuderia: ");
    scanf("%u", &p->id_escuderia);

    do
    {
        printf("Fecha nacimiento (AAAAMMDD): ");
        scanf("%llu", &p->fechaNacimiento);
    }
    while (!esFechaValida(p->fechaNacimiento));
}

/**
 * mostrarCamposPiloto  [static]
 * Muestra los campos editables del piloto con su valor actual,
 * numerados para que el usuario elija cual modificar.
 * La fecha se muestra en formato AAAAMMDD tal como esta guardada
 * (sin descomponer). El estado se muestra como char ('A', 'R' o 'S').
 * Se usa en el submenu de modificarPiloto().
 */
static int ingresarEstadoBaja(void)
{
    int op;

    printf("  Tipo de baja:\n");
    printf("  [1] Retirado    (R)\n");
    printf("  [2] Suspendido  (S)\n");
    printf("  Opcion: ");
    scanf("%d", &op);

    if (op == 1)
        return ESTADO_RETIRADO_PILOTO;

    return ESTADO_SUSPENDIDO_PILOTO;
}

static void mostrarCamposPiloto(const Piloto* p)
{
    printf("\n  Datos actuales del piloto ID %u:\n", p->id);
    printf("  [1] Nombre          : %s\n",  p->nombre);
    printf("  [2] Nacionalidad    : %s\n",  p->nacionalidad);
    printf("  [3] ID Escuderia    : %u\n",  p->id_escuderia);
    printf("  [4] Fecha nacimiento: %llu\n", p->fechaNacimiento);
    printf("  [5] Estado          : %c\n", p->estado);
    printf("  [0] Salir\n");
}

/**
 * generarArchivoPilotosTxt
 * Crea el archivo de texto pilotos.txt con el lote inicial
 * de 11 pilotos hardcodeados para prueba.
 * Usa generarArchivoTexto() con escribirPilotoTxt() como Accion.
 * Retorna TODO_OK o ERR_ARCH.
 */
int generarArchivoPilotosTxt(const char* rutaTxt)
{
    Piloto lote[11] =
    {
        {1,  "Max Verstappen",  "Neerlandes",  1, 0, ESTADO_ACTIVO_PILOTO, 19970930},
        {2,  "Lando Norris",    "Britanico",   2, 0, ESTADO_RETIRADO_PILOTO, 19991113},
        {3,  "Charles Leclerc", "Monegasco",   3, 0, ESTADO_ACTIVO_PILOTO, 19971016},
        {4,  "Oscar Piastri",   "Australiano", 2, 0, ESTADO_ACTIVO_PILOTO, 20010406},
        {5,  "Carlos Sainz",    "Espanol",     4, 0, ESTADO_ACTIVO_PILOTO, 19940901},
        {6,  "George Russell",  "Britanico",   1, 0, ESTADO_ACTIVO_PILOTO, 19980215},
        {7,  "Lewis Hamilton",  "Britanico",   3, 0, ESTADO_ACTIVO_PILOTO, 19850107},
        {8,  "Fernando Alonso", "Espanol",     5, 0, ESTADO_RETIRADO_PILOTO, 19810729},
        {9,  "Lance Stroll",    "Canadiense",  5, 0, ESTADO_ACTIVO_PILOTO, 19981029},
        {10, "Nico Hulkenberg", "Aleman",      6, 0, ESTADO_RETIRADO_PILOTO, 19870819},
        {11, "Juan Fernandez", "Argentino",    6, 0, ESTADO_ACTIVO_PILOTO, 19870819}
    };

    return generarArchivoTexto(rutaTxt, lote, 11, sizeof(Piloto), escribirPilotoTxt);
}

/**
 * cargarArchivoPilotos
 * Lee el archivo de texto rutaTxt con fscanf en formato CSV:
 *   id,nombre,nacionalidad,id_escuderia,puntos,estado,fechaNacimiento
 * y escribe cada registro como binario en rutaBin.
 * Usa buffers intermedios para nombre y nacionalidad porque
 * fscanf no puede leer cadenas con espacios directo al struct.
 * Retorna la cantidad de registros escritos (0 si no pudo abrir).
 */
int cargarArchivoPilotos(const char* rutaTxt, const char* rutaBin)
{
    Piloto piloto;
    int camposLeidos = 0;
    int total = 0;

    char bufferNombre[TAM_NOMBRE_PILOTO];           // fscanf no puede leer directo a struct con espacios
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

    // formato CSV: id,nombre,nacionalidad,id_escuderia,puntos,estado,fechaNacimiento
    camposLeidos = fscanf(fTxt,
                          "%u,%29[^,],%29[^,],%u,%u ,%c,%llu\n",
                          &piloto.id, bufferNombre, bufferNacionalidad,
                          &piloto.id_escuderia, &piloto.puntos_acumulados,
                          &piloto.estado,&piloto.fechaNacimiento);

    while(camposLeidos == 7) // si no leyó los 7 campos, la linea es invalida o llego al fin
    {
        copiarCadena(piloto.nombre, bufferNombre, TAM_NOMBRE_PILOTO);
        copiarCadena(piloto.nacionalidad, bufferNacionalidad, TAM_NACIONALIDAD);

        fwrite(&piloto, sizeof(Piloto), 1, fBin); // escribe el registro binario
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

/**
 * listarPilotos
 * Abre el .bin y muestra todos los pilotos en formato tabla
 * (ID | Nombre | Estado | Puntos) sin filtrar por estado.
 * Retorna la cantidad de registros listados.
 */
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

    while(fread(&piloto, sizeof(Piloto), 1, fBin) == 1) // lee un registro por iteracion
    {
        printf("%-4u  %-28s  %-10c  %u\n",
               piloto.id, piloto.nombre, piloto.estado, piloto.puntos_acumulados);

        listados++;
    }
    printf("-------------------------------------------------------------\n");

    fclose(fBin);

    return listados;
}

/**
 * cmp_desc  [Comparar]
 * Compara dos PilotoRef por puntos en orden DESCENDENTE.
 * Retorna 1 si b > a, -1 si b < a, 0 si iguales.
 * Compatible con qsort para ordenar el ranking de mayor a menor.
 */
int cmp_desc(const void* a, const void* b)
{
    const PilotoRef* ra = (const PilotoRef*)a;
    const PilotoRef* rb = (const PilotoRef*)b;
    if (rb->puntos > ra->puntos) return  1;
    if (rb->puntos < ra->puntos) return -1;
    return 0;
}

/**
 * RankingPiloto
 * Genera y muestra el ranking de pilotos activos ordenado
 * por puntos de mayor a menor.
 * Paso a paso:
 *   1. Recorre el .bin construyendo un arreglo dinamico de
 *      PilotoRef (puntos + offset en archivo) solo para
 *      pilotos activos. Duplica la capacidad con realloc si
 *      se llena.
 *   2. Ordena el arreglo con qsort usando cmp_desc.
 *   3. Para cada referencia, hace fseek al offset y lee el
 *      Piloto completo para mostrarlo.
 * Retorna TODO_OK, ERR_ARCH o ERR_MEM.
 */
int RankingPiloto(const char* rutaBin)
{
    FILE* fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCH;

    PilotoRef* p;
    Piloto pil;
    long offset = 0;
    size_t cap = CAP_MAX, n = 0;
    PilotoRef* refs = malloc(cap * sizeof(PilotoRef)); // arreglo dinamico de referencias

    if (!refs)
    {
        fclose(fBin);
        return ERR_MEM;
    }

    /** Fase 1: construir indice en memoria */
    while (fread(&pil, sizeof(Piloto), 1, fBin) == 1)
    {
        if (n == cap)
        {
            cap *= 2;  //duplica capacidad si se llena
            PilotoRef* tmp = realloc(refs, cap * sizeof(PilotoRef));

            if (!tmp)
            {
                free(refs);
                fclose(fBin);
                return ERR_MEM;
            }

            refs = tmp;
        }
        if(esPilotoActivos(&pil))//cargar el indicie pregunta si el piloto es activo
        {
            (refs + n)->puntos = pil.puntos_acumulados;
            (refs + n)->offset = offset; //La direccion donde debera buscar en el archivo
            n++;
        }

        offset += sizeof(Piloto);
    }

    qsort(refs, n, sizeof(PilotoRef), cmp_desc);/** Fase 2: ordena el indice por puntos*/

    printf("\n");
    printf("=============================================================\n");
    printf("  RANKING DE PILOTOS - TEMPORADA\n");
    printf("=============================================================\n");
    printf("%-4s  %-28s  %-10s  %s\n", "ID", "Nombre", "Estado", "Puntos");
    printf("-------------------------------------------------------------\n");

    /** Fase 3: muestra cada piloto accediendo por offset */
    for (p = refs; p < refs + n; p++)
    {
        fseek(fBin, p->offset, SEEK_SET);           //va al registro original en el .bin
        fread(&pil, sizeof(Piloto), 1, fBin);        //lee el piloto completo
        printf("%-4u  %-28s  %-10c  %u\n",
               pil.id, pil.nombre, pil.estado, pil.puntos_acumulados);
    }

    free(refs);
    fclose(fBin);
    return TODO_OK;
}

/**Funciones para manejo de datos TDA vector**/
/**
 * esPilotoActivos  [Filter]
 * Retorna 1 si el piloto tiene estado ACTIVO ('A'), 0 si no.
 * Se usa con filtrarVector() para excluir retirados/suspendidos.
 */
int esPilotoActivos(const void* dato)
{
    Piloto* p = (Piloto*)dato;

    return(p->estado == ESTADO_ACTIVO_PILOTO);
}

/**
 * sumarPuntos  [Reduce]
 * Acumula en *(unsigned*)acumulador los puntos del piloto.
 * Se usa con reducirVector() para calcular el total de puntos.
 */
int sumarPuntos(void* acumulador, const void* dato)
{
    Piloto* p = (Piloto*)dato;

    *(unsigned*)acumulador += p->puntos_acumulados;

    return TODO_OK;
}

/**
 * extraerIdPuntos  [Map]
 * Transforma un Piloto en un par {id, puntos} almacenado
 * en un arreglo unsigned[2]. Columna 0 = id, columna 1 = puntos.
 * Compatible con mapearVector().
 */
int extraerIdPuntos(void* dest, const void* orig)
{
    unsigned* resultado = (unsigned*)dest;
    Piloto* p = (Piloto*)orig;

    resultado[COL_ID_PILOTO] = p->id;
    resultado[COL_PUNTOS] = p->puntos_acumulados;

    return TODO_OK;
}


/**
 * cargarVectorPilotoActivos
 * Recorre el .bin de pilotos e inserta en vIds (ordenado)
 * solo el ID de cada piloto con estado ACTIVO.
 * El vector resultante se usa como base para la simulacion
 * aleatoria de carreras.
 * Retorna TODO_OK o ERR_ARCH.
 */
int cargarVectorPilotoActivos(const char* rutaBin, tVector* vIds, Comparar comparar)
{
    Piloto piloto;

    FILE* fPiloto = fopen(rutaBin, "rb");

    if(!fPiloto)
        return ERR_ARCH;

    while(fread(&piloto, sizeof(Piloto), 1, fPiloto) == 1)
    {
        if(piloto.estado == ESTADO_ACTIVO_PILOTO)
            insertarVectorOrd(vIds, &piloto.id, comparar); // inserta el ID manteniendo orden
    }

    fclose(fPiloto);

    return TODO_OK;
}

/**
 * listarPilotosPorEscuderia
 * Valida que la escuderia con idEscuderia exista y este activa,
 * luego recorre el .bin de pilotos y muestra en tabla solo los
 * que pertenecen a esa escuderia.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int listarPilotosPorEscuderia(const char* rutaPiloto, const char* rutaEscuderia, unsigned idEscuderia)
{
    Escuderia esc;
    Piloto    piloto;
    FILE*     fEsc;
    FILE*     fPil;
    int       encontrado = 0;
    int       cont = 0;

    fEsc = fopen(rutaEscuderia, "rb");
    if (!fEsc)
        return ERR_ARCH;

    // busqueda lineal de la escuderia en el .bin
    while (fread(&esc, sizeof(Escuderia), 1, fEsc) == 1)
    {
        if (esc.id == idEscuderia)
        {
            encontrado = 1;
            break; // encontrada, esc queda cargada con los datos
        }
    }
    fclose(fEsc);

    if (!encontrado)
    {
        printf("[!] Escuderia con ID %u no encontrada.\n", idEscuderia);
        return NO_ENCONTRADO;
    }

    /* No se listan pilotos de escuderias inactivas */
    if (esc.estado != ESTADO_ESCUDERIA_ACTIVA)
    {
        printf("[!] La escuderia [%s] %s esta inactiva.\n", esc.codigo, esc.nombre);
        return TODO_OK;
    }

    fPil = fopen(rutaPiloto, "rb");
    if (!fPil)
        return ERR_ARCH;

    printf("\n");
    printf("=============================================================\n");
    printf("  PILOTOS DE: [%s] %s\n", esc.codigo, esc.nombre);
    printf("=============================================================\n");
    printf("%-4s  %-28s  %-12s  %-6s  %s\n",
           "ID", "Nombre", "Nacionalidad", "Estado", "Puntos");
    printf("-------------------------------------------------------------\n");

    while (fread(&piloto, sizeof(Piloto), 1, fPil) == 1)
    {
        if (piloto.id_escuderia == idEscuderia) // filtra solo los pilotos de esa escuderia
        {
            printf("%-4u  %-28s  %-12s  %-6c  %u\n",
                   piloto.id, piloto.nombre, piloto.nacionalidad,
                   piloto.estado, piloto.puntos_acumulados);
            cont++;
        }
    }

    if (cont == 0)
        printf("  (Sin pilotos registrados para esta escuderia)\n");

    printf("-------------------------------------------------------------\n");
    printf("  Total: %d piloto(s)\n\n", cont);

    fclose(fPil);
    return TODO_OK;
}

/**
 * exportarPilotosTxt
 * Lee el .bin de pilotos y escribe cada registro en formato CSV
 * en el archivo de texto de exportacion:
 *   id,nombre,estado,id_escuderia,puntos
 * Retorna TODO_OK o ERR_ARCH.
 */
int exportarPilotosTxt(const char* rutaBin, const char* rutaTxtExportado)
{
    FILE* fBin = fopen(rutaBin, "rb");

    if(!fBin)
    {
        printf("[!] No se encontro la base de datos de pilotos.\n");
        return ERR_ARCH;
    }

    FILE* fTxt = fopen(rutaTxtExportado, "wt");

    if(!fTxt)
    {
        printf("[!] No se pudo crear el archivo de exportacion.\n");
        fclose(fBin);
        return 0;
    }

    Piloto p;
    int registrosExportados = 0;

    while(fread(&p, sizeof(Piloto), 1, fBin))
    {
        // formato: id,nombre,estado,id_escuderia,puntos
        fprintf(fTxt, "%u,%s,%c,%u,%u\n",
                p.id, p.nombre, p.estado, p.id_escuderia, p.puntos_acumulados);
        registrosExportados++;
    }

    fclose(fBin);
    fclose(fTxt);

    printf("[OK] Se exportaron %d pilotos al archivo de texto.\n", registrosExportados);

    return TODO_OK;
}


/**
 * altaPiloto
 * Pide ID y datos al usuario, asigna estado Activo y puntos=0,
 * y agrega el nuevo registro al final del .bin en modo append.
 * Retorna TODO_OK o ERR_ARCH.
 */
int altaPiloto(const char* rutaBin)
{
    Piloto  nuevo;
    FILE*   fBin;

    printf("Ingresar ID:");
    scanf("%u", &nuevo.id);

    nuevo.puntos_acumulados  = 0;
    nuevo.estado             = ESTADO_ACTIVO_PILOTO;

    printf("\n--- ALTA DE PILOTO (ID asignado: %u) ---\n", nuevo.id);

    pedirDatosPiloto(&nuevo);

    fBin = fopen(rutaBin, "ab");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de pilotos.\n");
        return ERR_ARCH;
    }

    fwrite(&nuevo, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    printf("[OK] Piloto '%s' registrado con ID %u.\n", nuevo.nombre, nuevo.id);
    return TODO_OK;
}

/**
 * bajaPiloto
 * Busca el piloto por ID con buscarRegistroPorId().
 * Si esta Activo, cambia su estado a Retirado o Suspendido
 * segun eleccion del usuario y sobreescribe el registro con fseek.
 * Registra la baja en el archivo de texto de bajas.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int bajaPiloto(const char* rutaBin, const char* rutaBajasTxt)
{
    Piloto  piloto;
    FILE*   fBin;
    FILE*   fBajas;
    long    offset;
    unsigned id;

    fBin = fopen(rutaBin, "rb+");

    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de pilotos.\n");
        return ERR_ARCH;
    }

    printf("\n--- BAJA DE PILOTO ---\n");
    printf("ID del piloto: ");
    scanf("%u", &id);

    offset = buscarRegistroPorId(fBin, id, &piloto, sizeof(Piloto));

    if (offset == -1L)
    {
        printf("[!] Piloto con ID %u no encontrado.\n", id);
        return NO_ENCONTRADO;
    }

    if (piloto.estado != ESTADO_ACTIVO_PILOTO)
    {
        printf("[!] El piloto '%s' ya tiene estado '%c'. No se realizo cambio.\n",
               piloto.nombre, piloto.estado);
        return TODO_OK;
    }

    printf("  Piloto encontrado: %s\n", piloto.nombre);

    piloto.estado = (char)ingresarEstadoBaja();

    fseek(fBin, offset, SEEK_SET);
    fwrite(&piloto, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    /* Registrar la baja en el archivo de texto */
    fBajas = fopen(rutaBajasTxt, "at");
    if (fBajas)
    {
        fprintf(fBajas, "PILOTO|%u|%s|%c\n", piloto.id, piloto.nombre, piloto.estado);
        fclose(fBajas);
    }

    printf("[OK] Estado del piloto '%s' actualizado a '%c'.\n",
           piloto.nombre, piloto.estado);
    return TODO_OK;
}

/**
 * modificarPiloto
 * Busca el piloto por ID, presenta un submenu de campos
 * (nombre, nacionalidad, id_escuderia, fecha, estado) y
 * repite hasta que el usuario elija 0.
 * Sobreescribe el registro completo con fseek al finalizar.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int modificarPiloto(const char* rutaBin)
{
    Piloto   piloto;
    FILE*    fBin;
    long     offset;
    unsigned id;
    int      campo;
    unsigned long long fechaNueva;
    int nuevoEstado;

    fBin = fopen(rutaBin, "rb+");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de pilotos.\n");
        return ERR_ARCH;
    }

    printf("\n--- MODIFICAR PILOTO ---\n");
    printf("ID del piloto: ");
    scanf("%u", &id);

    /**Busco el offset del registro para usar con el fseek*/
    offset = buscarRegistroPorId(fBin, id, &piloto, sizeof(Piloto));

    if (offset == -1L)
    {
        printf("[!] Piloto con ID %u no encontrado.\n", id);
        return NO_ENCONTRADO;
    }

    mostrarCamposPiloto(&piloto);
    printf("Campo a modificar: ");
    scanf("%d", &campo);

    while (campo != 0)
    {
        limpiarBuffer();

        switch (campo)
        {
        case 1:
            printf("Nuevo nombre: ");
            leerCadena(piloto.nombre, TAM_NOMBRE_PILOTO);
            break;

        case 2:
            printf("Nueva nacionalidad: ");
            leerCadena(piloto.nacionalidad, TAM_NACIONALIDAD);
            break;

        case 3:
            {
                int okEsc = 0;
                do
                {
                    printf("Nuevo ID Escuderia: ");
                    if (scanf("%u", &piloto.id_escuderia) != 1) // <-- Usa 'piloto', no 'p'
                    {
                        printf("[!] Entrada invalida. Ingrese un numero.\n");
                        limpiarBuffer();
                    }
                    else
                    {
                        limpiarBuffer();
                        if (esEscuderiaValida(RUTA_ESCUDERIA_BIN, piloto.id_escuderia))
                        {
                            okEsc = 1;
                        }
                        else
                        {
                            printf("[!] Escuderia con ID %u no existe.\n", piloto.id_escuderia);
                        }
                    }
                }
                while (!okEsc);
            }
            break;

                break;

        case 4:
            do
            {
                printf("Nueva fecha nacimiento (AAAAMMDD): ");
                scanf("%llu", &fechaNueva);
            }
            while (!esFechaValida(fechaNueva));
            piloto.fechaNacimiento = fechaNueva;
            break;

        case 5:
            do
            {
                printf("  [1] Activo      (A)\n");
                printf("  [2] Retirado    (R)\n");
                printf("  [3] Suspendido  (S)\n");
                printf("Nuevo Estado: ");
                if(scanf("%d", &nuevoEstado) != 1)
                {
                    printf("[!] Entrada invalida. Ingrese un numero entre 1 y 3.\n");

                    limpiarBuffer();
                    nuevoEstado = -1;
                }

            }
            while (nuevoEstado < 1 || nuevoEstado > 3);

            /**Dependiendo el valor de nuevoEstado (1/2/3) sera el estado del piloto (A/R/S)**/
            piloto.estado = (nuevoEstado == 1 ? ESTADO_ACTIVO_PILOTO : (nuevoEstado == 2 ? ESTADO_RETIRADO_PILOTO : ESTADO_SUSPENDIDO_PILOTO));

            break;
        default:
            printf("[!] Campo invalido.\n");
            break;
        }

        mostrarCamposPiloto(&piloto);
        printf("Campo a modificar: ");
        scanf("%d", &campo);
    }

    /**Realizo la modificacion**/
    fseek(fBin, offset, SEEK_SET);
    fwrite(&piloto, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    printf("[OK] Piloto ID %u modificado correctamente.\n", piloto.id);
    return TODO_OK;
}
