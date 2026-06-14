#include <stdio.h>
#include "piloto.h"
#include "escuderia.h"
#include "utilidades.h"

static void pedirDatosPiloto(Piloto* p);
static int ingresarEstadoBaja(void);
static void mostrarCamposPiloto(const Piloto* p);

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
/* Lee el .txt de pilotos con fscanf y genera el .bin con un registro por piloto */
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
/* Lee el .bin y muestra todos los pilotos en formato tabla con ID, nombre, estado y puntos */
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

int cmp_desc(const void* a, const void* b)
{
    const PilotoRef* ra = (const PilotoRef*)a;
    const PilotoRef* rb = (const PilotoRef*)b;
    if (rb->puntos > ra->puntos) return  1;
    if (rb->puntos < ra->puntos) return -1;
    return 0;
}

/* Carga referencias (puntos + offset en archivo) de todos los pilotos,
   las ordena por puntos descendente con qsort y los imprime en ese orden */
int RankingPiloto(const char* rutaBin) {
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

    while (fread(&pil, sizeof(Piloto), 1, fBin) == 1) {
        if (n == cap) {
            cap *= 2;                                        // duplica capacidad si se llena
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

    qsort(refs, n, sizeof(PilotoRef), cmp_desc);//Ordena el indice por por puntos

    printf("\n");
    printf("=============================================================\n");
    printf("  RANKING DE PILOTOS - TEMPORADA\n");
    printf("=============================================================\n");
    printf("%-4s  %-28s  %-10s  %s\n", "ID", "Nombre", "Estado", "Puntos");
    printf("-------------------------------------------------------------\n");

    for (p = refs; p < refs + n; p++) {
        fseek(fBin, p->offset, SEEK_SET);           // va al registro original en el .bin
        fread(&pil, sizeof(Piloto), 1, fBin);        // lee el piloto completo
        printf("%-4u  %-28s  %-10c  %u\n",
               pil.id, pil.nombre, pil.estado, pil.puntos_acumulados);
    }

    free(refs);
    fclose(fBin);
    return TODO_OK;
}

/**Funciones para manejo de datos TDA vector**/
//Filter
int esPilotoActivos(const void* dato)
{
    Piloto* p = (Piloto*)dato;

    return(p->estado == ESTADO_ACTIVO_PILOTO);
}

//Reduce
int sumarPuntos(void* acumulador, const void* dato)
{
    Piloto* p = (Piloto*)dato;

    *(unsigned*)acumulador += p->puntos_acumulados;

    return TODO_OK;
}

//Map
int extraerIdPuntos(void* dest, const void* orig)
{
    unsigned* resultado = (unsigned*)dest;
    Piloto* p = (Piloto*)orig;

    resultado[COL_ID_PILOTO] = p->id;
    resultado[COL_PUNTOS] = p->puntos_acumulados;

    return TODO_OK;
}


/* Recorre el .bin de pilotos e inserta ordenado en el vector solo los IDs de pilotos activos */
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

/* Busca la escuderia por ID, valida que este activa y lista todos sus pilotos desde el .bin */
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

/* Lee el .bin de pilotos y exporta cada registro al .txt en formato CSV */
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


/*
 * altaPiloto
 * Genera un nuevo piloto con ID autoincremental y lo agrega
 * al final del binario. El estado inicial es siempre Activo.
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

/*
 * bajaPiloto
 * Busca el piloto por ID. Si esta activo, cambia su estado a
 * Retirado o Suspendido segun elija el usuario y sobreescribe
 * solo ese registro en disco. Ademas registra la baja en el
 * archivo de texto de bajas.
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

/*
 * modificarPiloto
 * Busca el piloto por ID y presenta un submenu de campos.
 * El usuario elige que campo modificar; el ciclo repite hasta
 * que elija 0. Al finalizar sobreescribe el registro completo.
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
            printf("Nuevo ID Escuderia: ");
            scanf("%u", &piloto.id_escuderia);
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
        do{
            printf("  [1] Activo      (A)\n");
            printf("  [2] Retirado    (R)\n");
            printf("  [3] Suspendido  (S)\n");
            printf("Nuevo Estado:");
            scanf("%d", &nuevoEstado);

        }while (nuevoEstado < 1 || nuevoEstado > 3);

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


    fseek(fBin, offset, SEEK_SET);
    fwrite(&piloto, sizeof(Piloto), 1, fBin);
    fclose(fBin);

    printf("[OK] Piloto ID %u modificado correctamente.\n", piloto.id);
    return TODO_OK;
}
