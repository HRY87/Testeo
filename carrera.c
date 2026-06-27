#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <string.h>
#include "utilidades.h"
#include "carrera.h"
#include "piloto.h"
#include "vector.h"
#include "indice.h"

/* =========================================================
        Prototipos de funciones estaticas (uso interno)
   ========================================================= */

static void pedirDatosBase     (Carrera* c);
static int  esPilotoDuplicado  (const Carrera* c, unsigned id);
static void mostrarCamposCarrera(const Carrera* c);

/* =========================================================
                    Auxiliares estaticas
   ========================================================= */

/**
 * pedirDatosBase  [static]
 * Solicita al usuario los datos basicos de una carrera:
 * nombre del circuito y fecha en formato AAAAMMDD.
 * Repite el pedido de fecha hasta que esFechaValida() apruebe.
 * No retorna valor.
 */
static void pedirDatosBase(Carrera* c)
{
    limpiarBuffer();

    printf("\nNombre del circuito: ");
    leerCadena(c->circuito, TAM_NOMBRE_CIRCUITO);

    do
    {
        printf("Fecha de la carrera (AAAAMMDD): ");
        scanf("%llu", &c->fecha);
    }
    while (!esFechaValida(c->fecha));
}

/**
 * esPilotoDuplicado  [static]
 * Recorre los resultados ya cargados en la carrera y verifica
 * si el ID recibido ya fue registrado en alguna posicion.
 * Se usa en el ingreso manual para evitar cargar el mismo
 * piloto dos veces en la misma carrera.
 * Retorna 1 si el piloto esta duplicado, 0 si no.
 */
static int esPilotoDuplicado(const Carrera* c, unsigned id)
{
    int i;

    for (i = 0; i < c->cant_resultados; i++)
    {
        if ((unsigned)c->resultados[i][COL_ID_PILOTO] == id)
            return 1;
    }

    return 0;
}

/**
 * mostrarCamposCarrera  [static]
 * Descompone la fecha AAAAMMDD en dia/mes/anio y muestra
 * los campos editables de la carrera con su valor actual:
 * [1] Circuito, [2] Fecha, [3] Estado.
 * Se usa en el submenu de modificarCarrera().
 * No retorna valor.
 */
static void mostrarCamposCarrera(const Carrera* c)
{
    unsigned dia;
    unsigned mes;
    unsigned anio;

    anio = (unsigned)(c->fecha / 10000);
    mes  = (unsigned)(c->fecha / 100 % 100);
    dia  = (unsigned)(c->fecha % 100);

    printf("\n  Datos actuales de la carrera ID %d:\n", c->id);
    printf("  [1] Circuito : %s\n",             c->circuito);
    printf("  [2] Fecha    : %02u/%02u/%04u\n", dia, mes, anio);
    printf("  [3] Estado   : %s\n",
           c->estado == ESTADO_CARRERA_ACTIVA ? "Activa" : "Cancelada");
    printf("  [0] Salir\n");
}

/* =========================================================
   Registro de carreras (Funcionalidad C — Simulacion)
   ========================================================= */

/**
 * registrarCarreraAleatoria
 * Crea una nueva carrera con resultados generados aleatoriamente
 * mediante el algoritmo Fisher-Yates y la guarda al final de
 * carreras.dat. El archivo se crea si no existe (modo "ab+").
 * Paso a paso:
 *   1. Abre carreras.dat en modo "ab+" (append + lectura).
 *   2. Genera el ID autoincremental con generarIdCarrera().
 *   3. Pide circuito y fecha al usuario con pedirDatosBase().
 *   4. Llama a cargarResultadosCarreraAleatorios() para generar
 *      el orden aleatorio y asignar los puntos F1.
 *   5. Si todo salio bien, escribe la Carrera al final del .bin.
 * Retorna TODO_OK o ERR_ARCH.
 */
int registrarCarreraAleatoria(const char* rutaCarrera, const char* rutaPiloto,
                               Comparar comparar, const Puntos* pts)
{
    Carrera nueva;
    FILE*   fCarrera;
    int     resp;

    /* "ab+" crea el archivo si no existe y no borra el contenido previo */
    fCarrera = fopen(rutaCarrera, "ab+");
    if (!fCarrera)
        return ERR_ARCHIVO;

    nueva.id            = generarIdCarrera(fCarrera); /* ID = ultimo ID + 1 */
    nueva.estado        = ESTADO_CARRERA_ACTIVA;
    nueva.cant_resultados = 0;

    pedirDatosBase(&nueva); /* solicita circuito y fecha */

    resp = cargarResultadosCarreraAleatorios(rutaPiloto, &nueva, comparar, pts);

    if (resp == TODO_OK)
    {
        fseek(fCarrera, 0L, SEEK_END); /* garantiza escritura al final */
        fwrite(&nueva, sizeof(Carrera), 1, fCarrera);
    }

    fclose(fCarrera);
    return resp;
}

/**
 * registrarCarreraManual
 * Crea una nueva carrera con posiciones y puntos ingresados
 * uno a uno por el usuario. Valida duplicados en tiempo real.
 * Paso a paso:
 *   1. Abre carreras.dat, genera ID y solicita datos base.
 *   2. Ciclo: pide ID de piloto por posicion (0 = fin).
 *      - Verifica duplicados con esPilotoDuplicado().
 *      - Sugiere los puntos oficiales F1 para esa posicion.
 *      - Si el usuario ingresa un valor negativo, usa el sugerido.
 *      - Guarda el par {idPiloto, puntos} en resultados[][].
 *   3. Escribe la Carrera completa al final del .bin.
 * Retorna TODO_OK o ERR_ARCH.
 */
int registrarCarreraManual(const char* rutaCarrera, const char* rutaPiloto,
                            Comparar comparar, const Puntos* pts)
{
    Carrera  nueva;
    FILE*    fCarrera;
    unsigned idPiloto;
    int      pInput;
    int      puntosSug;
    int      pos;
    int      esDuplicado;

    /* parametro rutaPiloto no se usa aqui pero mantiene firma uniforme */
    (void)rutaPiloto;
    (void)comparar;

    fCarrera = fopen(rutaCarrera, "ab+");
    if (!fCarrera)
        return ERR_ARCHIVO;

    nueva.id              = generarIdCarrera(fCarrera);
    nueva.estado          = ESTADO_CARRERA_ACTIVA;
    nueva.cant_resultados = 0;

    pedirDatosBase(&nueva); /* solicita circuito y fecha */

    printf("\nIngreso de resultados (ID piloto = 0 para terminar)\n");
    printf("--------------------------------------------\n");

    pos      = 1;
    idPiloto = 1; /* valor distinto de 0 para entrar al ciclo */

    while (nueva.cant_resultados < MAX_PILOTOS_CARRERA && idPiloto != 0)
    {
        printf("Posicion %2d - ID piloto (0 = fin): ", pos);
        scanf("%u", &idPiloto);
        limpiarBuffer();

        esDuplicado = esPilotoDuplicado(&nueva, idPiloto);

        if (idPiloto != 0 && !esDuplicado)
        {
            /* sugiere puntos oficiales F1 para la posicion actual */
            puntosSug = puntosParaPosicion(pts, pos);

            printf("Puntos (sugerido %2d): ", puntosSug);
            scanf("%d", &pInput);
            limpiarBuffer();

            nueva.resultados[nueva.cant_resultados][COL_ID_PILOTO] = (int)idPiloto;
            /* usa el sugerido si el usuario ingresa un valor negativo */
            nueva.resultados[nueva.cant_resultados][COL_PUNTOS] =
                (pInput < 0) ? puntosSug : pInput;

            nueva.cant_resultados++;
            pos++;
        }

        if (esDuplicado && idPiloto != 0)
            printf("[!] Piloto ID %u ya fue registrado en esta carrera.\n", idPiloto);
    }

    if (nueva.cant_resultados >= MAX_PILOTOS_CARRERA)
        printf("[!] Capacidad maxima de pilotos alcanzada (%d).\n", MAX_PILOTOS_CARRERA);

    fseek(fCarrera, 0L, SEEK_END);
    fwrite(&nueva, sizeof(Carrera), 1, fCarrera);
    fclose(fCarrera);

    return TODO_OK;
}

/**
 * cargarResultadosCarreraAleatorios
 * Genera el orden de llegada aleatorio para una carrera usando
 * el algoritmo Fisher-Yates sobre los pilotos activos.
 * Paso a paso:
 *   1. Crea un tVector de IDs de pilotos activos con
 *      cargarVectorPilotoActivos().
 *   2. Mezcla el vector con Fisher-Yates via
 *      generarResultadoAleatorioVector().
 *   3. Recorre el vector mezclado y para cada posicion i:
 *      - Copia el ID del piloto en resultados[i][COL_ID_PILOTO].
 *      - Asigna los puntos F1 con puntosParaPosicion() (0 si
 *        la posicion supera el limite de puntuacion).
 *   4. Libera el vector.
 * Retorna TODO_OK, SIN_MEM o ERR_ARCH.
 */
int cargarResultadosCarreraAleatorios(const char* rutaPiloto, Carrera* nueva,
                                       Comparar comparar, const Puntos* pts)
{
    tVector  vIds;
    int      i;
    unsigned idPiloto;

    if (crearVector(&vIds, sizeof(unsigned), MAX_PILOTOS_CARRERA) != TODO_OK)
        return ERR_MEMORIA;

    if (cargarVectorPilotoActivos(rutaPiloto, &vIds, comparar) != TODO_OK)
    {
        destruirVector(&vIds);
        return ERR_ARCHIVO;
    }

    if (vIds.ce == 0) /* sin pilotos activos no hay carrera posible */
    {
        destruirVector(&vIds);
        return ERR_ARCHIVO;
    }

    generarResultadoAleatorioVector(&vIds); /* shuffle Fisher-Yates */

    nueva->cant_resultados = (int)vIds.ce;

    for (i = 0; i < nueva->cant_resultados; i++)
    {
        /* obtiene el ID del piloto en la posicion i del vector mezclado */
        idPiloto = *(unsigned*)obtenerElementoVector(&vIds, (size_t)i);

        nueva->resultados[i][COL_ID_PILOTO] = (int)idPiloto;
        nueva->resultados[i][COL_PUNTOS]    = puntosParaPosicion(pts, i + 1);
    }

    destruirVector(&vIds);
    return TODO_OK;
}

/* =========================================================
   Sistema de puntos (Funcionalidades A y D)
   ========================================================= */

/**
 * recalcularPuntosPilotos
 * Recomputa DESDE CERO los puntos acumulados de todos los pilotos
 * recorriendo el historial completo de carreras.dat.
 * Usar solo ante baja o modificacion de estado de una carrera;
 * para carreras nuevas usar aplicarPuntosUltimaCarrera() (mas eficiente).
 * Paso a paso:
 *   1. Abre pilotos.dat en "rb+" y resetea puntos_acumulados = 0
 *      en cada registro (read-modify-write con fseek).
 *   2. Abre carreras.dat; si no existe los pilotos ya quedaron en 0.
 *   3. Por cada carrera activa, por cada resultado con puntos > 0,
 *      localiza al piloto con buscarPilotoPorIndice() (busqueda
 *      binaria sobre pilotos.idx) y suma los puntos.
 * Retorna TODO_OK o ERR_ARCH.
 */
int recalcularPuntosPilotos(const char* rutaCarrera, const char* rutaPiloto)
{
    Piloto  pil;
    Carrera car;
    FILE*   fPil;
    FILE*   fCar;
    long    offset;
    int     i;

    /* Paso 1: resetear puntos de todos los pilotos a 0 */
    fPil = fopen(rutaPiloto, "rb+");
    if (!fPil)
        return ERR_ARCHIVO;

    offset = 0L;
    while (fread(&pil, sizeof(Piloto), 1, fPil) == 1)
    {
        pil.puntos_acumulados = 0;
        fseek(fPil, offset, SEEK_SET);
        fwrite(&pil, sizeof(Piloto), 1, fPil);
        fseek(fPil, 0L, SEEK_CUR); /* adelanta el puntero tras fwrite */
        offset += (long)sizeof(Piloto);
    }

    /* Paso 2: abrir historial de carreras */
    fCar = fopen(rutaCarrera, "rb");
    if (!fCar)
    {
        /* Sin historial todavia: pilotos ya quedaron en 0, correcto */
        fclose(fPil);
        return TODO_OK;
    }

    /* Paso 3: acumular puntos de cada carrera activa */
    while (fread(&car, sizeof(Carrera), 1, fCar) == 1)
    {
        if (car.estado == ESTADO_CARRERA_ACTIVA)
        {
            for (i = 0; i < car.cant_resultados; i++)
            {
                if (car.resultados[i][COL_PUNTOS] > 0)
                {
                    offset = buscarPilotoPorIndice(
                                 RUTA_INDICE_PILOTO, fPil,
                                 (unsigned)car.resultados[i][COL_ID_PILOTO],
                                 &pil);

                    if (offset != -1L)
                    {
                        pil.puntos_acumulados += (unsigned)car.resultados[i][COL_PUNTOS];
                        fseek(fPil, offset, SEEK_SET);
                        fwrite(&pil, sizeof(Piloto), 1, fPil);
                    }
                }
            }
        }
    }

    fclose(fCar);
    fclose(fPil);
    return TODO_OK;
}

/**
 * aplicarPuntosUltimaCarrera  [Funcionalidad D — Sistema incremental]
 * Aplica de forma incremental solo los puntos de la ultima carrera
 * guardada en carreras.dat, sin recorrer el historial completo.
 * Se posiciona directamente en el ultimo registro con fseek(SEEK_END)
 * y usa buscarPilotoPorIndice() (busqueda binaria) para cada piloto.
 * Llamar inmediatamente despues de registrar una carrera nueva.
 * Retorna TODO_OK o ERR_ARCH.
 */
int aplicarPuntosUltimaCarrera(const char* rutaCarrera, const char* rutaPiloto)
{
    Carrera car;
    Piloto  pil;
    FILE*   fCar;
    FILE*   fPil;
    long    offset;
    int     i;

    fCar = fopen(rutaCarrera, "rb");
    if (!fCar)
        return ERR_ARCHIVO;

    /* acceso directo al ultimo registro sin recorrer el archivo */
    fseek(fCar, -(long)sizeof(Carrera), SEEK_END);
    fread(&car, sizeof(Carrera), 1, fCar);
    fclose(fCar);

    if (car.estado != ESTADO_CARRERA_ACTIVA)
        return TODO_OK; /* carrera cancelada: nada que sumar */

    fPil = fopen(rutaPiloto, "rb+");
    if (!fPil)
        return ERR_ARCHIVO;

    for (i = 0; i < car.cant_resultados; i++)
    {
        if (car.resultados[i][COL_PUNTOS] > 0)
        {
            offset = buscarPilotoPorIndice(
                         RUTA_INDICE_PILOTO, fPil,
                         (unsigned)car.resultados[i][COL_ID_PILOTO],
                         &pil);

            if (offset != -1L)
            {
                pil.puntos_acumulados += (unsigned)car.resultados[i][COL_PUNTOS];
                fseek(fPil, offset, SEEK_SET);
                fwrite(&pil, sizeof(Piloto), 1, fPil);
            }
        }
    }

    fclose(fPil);
    return TODO_OK;
}

/**
 * recalcularPuntosPilotosOld-Version anterior-No usa indice
 * Version optimizada: usa dos arreglos paralelos en stack
 * (ids[] y acum[]) en lugar de cargar vectores completos,
 * reduciendo el uso de memoria heap.
 * Paso a paso:
 *   1. Lee pilotos.dat una sola vez y construye ids[] con los IDs
 *      y acum[] inicializado en 0.
 *   2. Recorre carreras.dat; por cada resultado con puntos > 0
 *      busca el ID en ids[] con busqueda lineal y acumula en acum[].
 *      Las carreras canceladas se saltan.
 *   3. Actualiza puntos_acumulados en pilotos.dat con fseek/fread/
 *      fwrite registro por registro (read-modify-write).
 * Si no existe carreras.dat aun, pone todos los puntos en 0 y sale.
 * Retorna TODO_OK o ERR_ARCH.
 */
//int recalcularPuntosPilotosOld(const char* rutaCarrera, const char* rutaPiloto)
//{
//    unsigned ids[CAP_MAX];   /* IDs de pilotos en orden de archivo */
//    unsigned acum[CAP_MAX];  /* acumulador de puntos por piloto    */
//    Piloto   pil;            /* buffer de lectura/escritura, stack */
//    Carrera  car;            /* buffer de lectura, stack           */
//    FILE*    fPil;
//    FILE*    fCar;
//    size_t   n = 0;
//    size_t   i;
//    int      j;
//
//    /* Paso 1: leer piloto.bin una sola vez y construir el indice ids[]/acum[] */
//    fPil = fopen(rutaPiloto, "rb");
//    if (!fPil)
//        return ERR_ARCH;
//
//    while (n < CAP_MAX && fread(&pil, sizeof(Piloto), 1, fPil) == 1)
//    {
//        ids[n]  = pil.id; // guarda el ID para buscarlo en las carreras
//        acum[n] = 0;      // inicia el acumulador en 0 antes de sumar
//        n++;
//    }
//    fclose(fPil);
//
//    /* Paso 2: recorrer carrera.bin acumulando puntos por piloto */
//    fCar = fopen(rutaCarrera, "rb");
//    if (!fCar)
//    {
//        /* Sin carreras todavia: pone puntos en 0 en el .bin y sale */
//        fPil = fopen(rutaPiloto, "r+b");
//        if (!fPil)
//            return ERR_ARCH;
//
//        for (i = 0; i < n; i++)
//        {
//            fseek(fPil, (long)(i * sizeof(Piloto)), SEEK_SET); // va al registro i
//            fread(&pil, sizeof(Piloto), 1, fPil);
//            pil.puntos_acumulados = 0;
//            fseek(fPil, (long)(i * sizeof(Piloto)), SEEK_SET); // vuelve para sobreescribir
//            fwrite(&pil, sizeof(Piloto), 1, fPil);
//        }
//        fclose(fPil);
//        return TODO_OK;
//    }
//
//    /**Archivo carrera**/
//    while (fread(&car, sizeof(Carrera), 1, fCar) == 1)
//    {
//        if (car.estado != ESTADO_CARRERA_ACTIVA)
//            continue; // ignora carreras canceladas
//
//        for (i = 0; i < (size_t)car.cant_resultados; i++)
//        {
//            if (car.resultados[i][COL_PUNTOS] <= 0)
//                continue; // no suma si el piloto no hizo puntos
//
//            // busca el piloto en el indice y suma sus puntos
//            for (j = 0; j < (int)n; j++)
//            {
//                if (ids[j] == (unsigned)car.resultados[i][COL_ID_PILOTO])
//                {
//                    acum[j] += (unsigned)car.resultados[i][COL_PUNTOS];
//                    break;
//                }
//            }
//        }
//    }
//    fclose(fCar);
//
//    /* Paso 3: actualizar puntos_acumulados en piloto.bin (read-modify-write por registro) */
//    fPil = fopen(rutaPiloto, "r+b");
//    if (!fPil)
//        return ERR_ARCH;
//
//    for (i = 0; i < n; i++)
//    {
//        fseek(fPil, (long)(i * sizeof(Piloto)), SEEK_SET);
//        fread(&pil, sizeof(Piloto), 1, fPil);
//        pil.puntos_acumulados = acum[i];           // asigna el total acumulado
//        fseek(fPil, (long)(i * sizeof(Piloto)), SEEK_SET);
//        fwrite(&pil, sizeof(Piloto), 1, fPil);     // sobreescribe solo ese campo
//    }
//    fclose(fPil);
//
//    return TODO_OK;
//}

/* =========================================================
                        Generacion de ID
   ========================================================= */

/**
 * generarIdCarrera
 * Determina el ID para la proxima carrera de forma autoincremental.
 * Si el archivo esta vacio retorna 1 (primera carrera de la temporada).
 * Si ya tiene registros, hace fseek al ultimo y retorna su ID + 1.
 * Recibe fCarrera ya abierto (el llamador lo abre y cierra).
 * Retorna el nuevo ID como int.
 */
int generarIdCarrera(FILE* fCarrera)
{
    Carrera ultima;

    fseek(fCarrera, 0L, SEEK_END);

    if (ftell(fCarrera) < (long)sizeof(Carrera))
        return 1; /* archivo vacio: primera carrera */

    fseek(fCarrera, -(long)sizeof(Carrera), SEEK_END);
    fread(&ultima, sizeof(Carrera), 1, fCarrera);

    return (ultima.id + 1);
}

/* =========================================================
   Punteros a funcion del TDA Carrera
   ========================================================= */

/**
 * escribirCarreraBinATxt  [BinATxt]
 * Escribe una Carrera en formato texto separado por SEP_TXT.
 * Formato: id|circuito|fecha|estado|cant_resultados|id:pts-id:pts-...
 * El separador entre pares de resultados es '-'; no se escribe
 * despues del ultimo par.
 * Retorna TODO_OK o ERR_ARCH.
 */
int carreraBinATxt(const void* dato, FILE* archTxt)
{
    const Carrera* c = (const Carrera*)dato;
    int i;

    if (!dato || !archTxt)
        return ERR_ARCHIVO;

    fprintf(archTxt, "%d%c%s%c%llu%c%d%c%d%c",
            c->id,              SEP_TXT,
            c->circuito,        SEP_TXT,
            c->fecha,           SEP_TXT,
            c->estado,          SEP_TXT,
            c->cant_resultados, SEP_TXT);

    for (i = 0; i < c->cant_resultados; i++)
    {
        fprintf(archTxt, "%d:%d",
                c->resultados[i][COL_ID_PILOTO],
                c->resultados[i][COL_PUNTOS]);

        if (i < c->cant_resultados - 1)
            fprintf(archTxt, "-"); /* separador entre pares */
    }

    fprintf(archTxt, "\n");
    return TODO_OK;
}

/**
 * mostrarCarrera  [Mostrar]
 * Imprime el encabezado de la carrera (ID, circuito, fecha, estado)
 * y la tabla de resultados con posicion, ID de piloto y puntos.
 * Descompone la fecha AAAAMMDD en dia/mes/anio para mostrarla
 * en formato DD/MM/AAAA.
 * No retorna valor.
 */
void mostrarCarrera(const void* dato)
{
    const Carrera* c = (const Carrera*)dato;
    int      i;
    unsigned dia;
    unsigned mes;
    unsigned anio;

    anio = (unsigned)(c->fecha / 10000);
    mes  = (unsigned)(c->fecha / 100 % 100);
    dia  = (unsigned)(c->fecha % 100);

    printf("----------------------------------\n");
    printf("ID Carrera  : %d\n",             c->id);
    printf("Circuito    : %s\n",             c->circuito);
    printf("Fecha       : %02u/%02u/%04u\n", dia, mes, anio);
    printf("Estado      : %s\n",  c->estado == ESTADO_CARRERA_ACTIVA ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n",     c->cant_resultados);

    for (i = 0; i < c->cant_resultados; i++)
    {
        printf("  Pos %2d | Piloto ID: %3d | Puntos: %2d\n",
               i + 1,
               c->resultados[i][COL_ID_PILOTO],
               c->resultados[i][COL_PUNTOS]);
    }
}

/* =========================================================
                    Exportacion
   ========================================================= */

/**
 * exportarCarrerasTxt
 * Lee carreras.dat registro a registro y escribe cada uno en
 * formato texto al archivo de exportacion usando aritmetica de
 * punteros sobre la matriz de resultados.
 * Formato por linea:
 *   id|circuito|fecha|estado|cant_resultados|id:pts-id:pts-...
 * El separador entre pares es '-'; no se escribe tras el ultimo.
 * Retorna TODO_OK o ERR_ARCH.
 */
int exportarCarrerasTxt(const char* rutaBin, const char* rutaTxtExportado)
{
    FILE*   fBin;
    FILE*   fTxt;
    Carrera c;
    int     i;

    fBin = fopen(rutaBin, "rb");
    if (!fBin)
        return ERR_ARCHIVO;

    fTxt = fopen(rutaTxtExportado, "wt");
    if (!fTxt)
    {
        fclose(fBin);
        return ERR_ARCHIVO;
    }

    while (fread(&c, sizeof(Carrera), 1, fBin) == 1)
    {
        fprintf(fTxt, "%d%c%s%c%llu%c%d%c%d%c",
                c.id, SEP_TXT,
                c.circuito, SEP_TXT,
                c.fecha, SEP_TXT,
                c.estado, SEP_TXT,
                c.cant_resultados,SEP_TXT);

        for (i = 0; i < c.cant_resultados; i++)
        {
            /* aritmetica de punteros sobre la matriz de resultados */
            fprintf(fTxt, "%d:%d",
                    *(*(c.resultados + i) + COL_ID_PILOTO),
                    *(*(c.resultados + i) + COL_PUNTOS));

            if (i < c.cant_resultados - 1)
                fprintf(fTxt, "-");
        }

        fprintf(fTxt, "\n");
    }

    fclose(fBin);
    fclose(fTxt);
    return TODO_OK;
}

/* =========================================================
                                ABM
   ========================================================= */

/**
 * bajaCarrera
 * Busca la carrera por ID con buscarRegistroPorId().
 * Si esta activa y el usuario confirma, cambia su estado a
 * ESTADO_CARRERA_INACTIVA y sobreescribe el registro con fseek.
 * Registra la baja en el archivo de texto de bajas con formato:
 *   CARRERA|id|circuito|fecha|CANCELADA
 * Como la carrera quedo inactiva, sus puntos ya no cuentan:
 * llama a recalcularPuntosPilotos() para corregir los acumulados.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int bajaCarrera(const char* rutaBin, const char* rutaBajasTxt)
{
    Carrera  carrera;
    FILE*    fBin;
    FILE*    fBajas;
    long     offset;
    int      id;
    int      confirmacion;
    unsigned dia;
    unsigned mes;
    unsigned anio;

    fBin = fopen(rutaBin, "rb+");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de carreras.\n");
        return ERR_ARCHIVO;
    }

    printf("\n--- BAJA DE CARRERA ---\n");
    printf("ID de la carrera: ");
    scanf("%d", &id);

    offset = buscarRegistroPorId(fBin, &id, &carrera, sizeof(Carrera), sizeof(int));

    if (offset == -1L)
    {
        fclose(fBin);
        printf("[!] Carrera con ID %d no encontrada.\n", id);
        return ERR_NO_ENCONTRADO;
    }

    if (carrera.estado == ESTADO_CARRERA_INACTIVA)
    {
        fclose(fBin);
        printf("[!] La carrera '%s' ya se encuentra cancelada.\n", carrera.circuito);
        return TODO_OK;
    }

    dia  = (unsigned)(carrera.fecha % 100);
    mes  = (unsigned)(carrera.fecha / 100 % 100);
    anio = (unsigned)(carrera.fecha / 10000);

    printf("  Carrera encontrada: %s  (%02u/%02u/%04u)\n",
           carrera.circuito, dia, mes, anio);
    printf("  Confirma la baja? [1] Si  [0] No: ");
    scanf("%d", &confirmacion);

    if (confirmacion != 1)
    {
        fclose(fBin);
        printf("[!] Baja cancelada.\n");
        return TODO_OK;
    }

    carrera.estado = ESTADO_CARRERA_INACTIVA;

    fseek(fBin, offset, SEEK_SET);
    fwrite(&carrera, sizeof(Carrera), 1, fBin);
    fclose(fBin);

    /* registra la baja en el log de texto */
    fBajas = fopen(rutaBajasTxt, "at");
    if (fBajas)
    {
        fprintf(fBajas, "CARRERA|%d|%s|%llu|CANCELADA\n",
                carrera.id, carrera.circuito, carrera.fecha);
        fclose(fBajas);
    }

    /* la carrera quedo inactiva: sus puntos ya no cuentan */
    recalcularPuntosPilotos(rutaBin, RUTA_PILOTO_BIN);

    printf("[OK] Carrera '%s' dada de baja.\n", carrera.circuito);
    return TODO_OK;
}

/**
 * modificarCarrera
 * Busca la carrera por ID con buscarRegistroPorId() y presenta
 * un submenu de campos editables: [1] circuito, [2] fecha, [3] estado.
 * El ciclo repite hasta que el usuario elija 0.
 * Los resultados y cant_resultados no son modificables aqui.
 * Si el estado cambio, llama a recalcularPuntosPilotos() para
 * mantener la consistencia del historial de puntos.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int modificarCarrera(const char* rutaBin)
{
    Carrera            carrera;
    FILE*              fBin;
    long               offset;
    int                id;
    int                campo;
    unsigned long long fechaNueva;
    int                nuevoEstado;
    int                estadoAnterior;

    fBin = fopen(rutaBin, "rb+");
    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de carreras.\n");
        return ERR_ARCHIVO;
    }

    printf("\n--- MODIFICAR CARRERA ---\n");
    printf("ID de la carrera: ");
    scanf("%d", &id);

    offset = buscarRegistroPorId(fBin, &id, &carrera, sizeof(Carrera), sizeof(int));

    if (offset == -1L)
    {
        fclose(fBin);
        printf("[!] Carrera con ID %d no encontrada.\n", id);
        return ERR_NO_ENCONTRADO;
    }

    estadoAnterior = carrera.estado; /* para detectar si el estado cambio al final */

    mostrarCamposCarrera(&carrera);
    printf("Campo a modificar: ");
    scanf("%d", &campo);

    while (campo != 0)
    {
        limpiarBuffer();

        switch (campo)
        {
        case 1:
            printf("Nuevo circuito: ");
            leerCadena(carrera.circuito, TAM_NOMBRE_CIRCUITO);
            break;

        case 2:
            do
            {
                printf("Nueva fecha (AAAAMMDD): ");
                scanf("%llu", &fechaNueva);
            }
            while (!esFechaValida(fechaNueva));
            carrera.fecha = fechaNueva;
            break;

        case 3:
            printf("  [1] Activa  [0] Cancelada: ");
            scanf("%d", &nuevoEstado);
            carrera.estado = (nuevoEstado == 1)
                             ? ESTADO_CARRERA_ACTIVA
                             : ESTADO_CARRERA_INACTIVA;
            break;

        default:
            printf("[!] Campo invalido.\n");
            break;
        }

        mostrarCamposCarrera(&carrera);
        printf("Campo a modificar: ");
        scanf("%d", &campo);
    }

    fseek(fBin, offset, SEEK_SET);
    fwrite(&carrera, sizeof(Carrera), 1, fBin);
    fclose(fBin);

    /* si el estado cambio, los puntos acumulados ya no son correctos */
    if (carrera.estado != estadoAnterior)
        recalcularPuntosPilotos(rutaBin, RUTA_PILOTO_BIN);

    printf("[OK] Carrera ID %d modificada correctamente.\n", carrera.id);
    return TODO_OK;
}
