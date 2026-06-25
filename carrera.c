#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <string.h>
#include "utilidades.h"
#include "carrera.h"
#include "piloto.h"
#include "vector.h"

static void pedirDatosBase(Carrera* c);
static int  esPilotoDuplicado(const Carrera* c, unsigned id);
static void mostrarCamposCarrera(const Carrera* c);


/**
 * pedirDatosBase  [static]
 * Solicita al usuario los datos basicos de una carrera:
 * nombre del circuito y fecha en formato AAAAMMDD.
 * Repite el pedido de fecha hasta que esFechaValida() apruebe.
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
 * si el ID recibido ya fue registrado.
 * Retorna 1 si el piloto esta duplicado, 0 si no.
 * Se usa en el ingreso manual para evitar cargar el mismo
 * piloto dos veces en la misma carrera.
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
 */
static void mostrarCamposCarrera(const Carrera* c)
{
    unsigned dia, mes, anio;

    anio = (unsigned)(c->fecha / 10000);
    mes  = (unsigned)(c->fecha / 100 % 100);
    dia  = (unsigned)(c->fecha % 100);

    printf("\n  Datos actuales de la carrera ID %d:\n", c->id);
    printf("  [1] Circuito : %s\n",              c->circuito);
    printf("  [2] Fecha    : %02u/%02u/%04u\n",  dia, mes, anio);
    printf("  [3] Estado   : %s\n",
           c->estado == ESTADO_CARRERA_ACTIVA ? "Activa" : "Cancelada");
    printf("  [0] Salir\n");
}


/**
 * registrarCarreraAleatoria
 * Crea una nueva carrera con resultados generados por Fisher-Yates
 * y la guarda al final de carreras.dat.
 * Paso a paso:
 *   1. Abre el .bin en modo "ab+" (crea si no existe).
 *   2. Genera el ID autoincremental con generarIdCarrera().
 *   3. Pide circuito y fecha al usuario con pedirDatosBase().
 *   4. Llama a cargarResultadosCarreraAleatorios() para generar
 *      el orden aleatorio y asignar los puntos.
 *   5. Si todo salio bien, escribe la Carrera al final del .bin.
 * Retorna TODO_OK o ERR_ARCH.
 */
int registrarCarreraAleatoria(const char* rutaCarrera, const char* rutaPiloto, Comparar comparar, const Puntos* pts)
{
    Carrera nueva;
    FILE*   fCarrera;
    int     resp;

    fCarrera = fopen(rutaCarrera, "ab+"); // ab+ crea el archivo si no existe y no borra el contenido
    if (!fCarrera)
        return ERR_ARCH;

    nueva.id     = generarIdCarrera(fCarrera); // id = ultimo id + 1
    nueva.estado = ESTADO_CARRERA_ACTIVA;
    nueva.cant_resultados = 0;

    pedirDatosBase(&nueva); // pide circuito y fecha al usuario

    resp = cargarResultadosCarreraAleatorios(rutaPiloto, &nueva, comparar, pts); // genera el orden aleatorio

    if (resp == TODO_OK)
    {
        fseek(fCarrera, 0L, SEEK_END);              // asegura escribir al final
        fwrite(&nueva, sizeof(Carrera), 1, fCarrera);
    }

    fclose(fCarrera);
    return resp;
}

/**
 * registrarCarreraManual
 * Crea una nueva carrera con posiciones y puntos ingresados
 * uno a uno por el usuario.
 * Paso a paso:
 *   1. Abre el .bin, genera ID y pide datos base.
 *   2. Ciclo: pide ID de piloto por posicion (0 = fin).
 *      - Verifica duplicados con esPilotoDuplicado().
 *      - Sugiere los puntos oficiales F1 para esa posicion.
 *      - Si el usuario ingresa un valor negativo, usa el sugerido.
 *      - Guarda el par {idPiloto, puntos} en resultados[][].
 *   3. Escribe la Carrera completa al final del .bin.
 * Retorna TODO_OK o ERR_ARCH.
 */
int registrarCarreraManual(const char* rutaCarrera, const char* rutaPiloto, Comparar comparar, const Puntos* pts)
{
    Carrera  nueva;
    FILE*    fCarrera;
    unsigned idPiloto = 1;
    int      pInput, puntosSug, pos, esDuplicado;

    fCarrera = fopen(rutaCarrera, "ab+");

    if (!fCarrera)
        return ERR_ARCH;

    nueva.id              = generarIdCarrera(fCarrera);
    nueva.estado          = ESTADO_CARRERA_ACTIVA;
    nueva.cant_resultados = 0;

    pedirDatosBase(&nueva); // pide circuito y fecha al usuario

    printf("\nIngreso de resultados (ID piloto = 0 para terminar)\n");
    printf("--------------------------------------------\n");

    pos = 1; // posicion en la carrera, arranca en 1

    while(nueva.cant_resultados < MAX_PILOTOS_CARRERA && idPiloto != 0)
    {
        printf("Posicion %2d - ID piloto (0 = fin): ", pos);
        scanf("%u", &idPiloto);
        limpiarBuffer();

        esDuplicado = esPilotoDuplicado(&nueva, idPiloto); // evita cargar el mismo piloto dos veces

        if(idPiloto != 0 && !esDuplicado)
        {
            // sugiere los puntos segun la tabla oficial de F1 para esa posicion
           puntosSug = puntosParaPosicion(pts, pos);   /* ya retorna 0 si está fuera de rango */

            printf("Puntos (sugerido %2d): ", puntosSug);
            scanf("%d", &pInput);
            limpiarBuffer();

            nueva.resultados[nueva.cant_resultados][COL_ID_PILOTO] = idPiloto;
            nueva.resultados[nueva.cant_resultados][COL_PUNTOS]    = (pInput < 0) ? puntosSug : pInput; // usa sugerido si el input es invalido
            nueva.cant_resultados++;
            pos++;
        }

        if(esDuplicado)
            printf("[!] Piloto ID %u ya fue registrado en esta carrera.\n", idPiloto);
    }

    if (nueva.cant_resultados >= MAX_PILOTOS_CARRERA)
        printf("[!] Capacidad maxima de pilotos alcanzada (%d).\n", MAX_PILOTOS_CARRERA);

    fseek(fCarrera, 0L, SEEK_END);
    fwrite(&nueva, sizeof(Carrera), 1, fCarrera); // escribe la carrera completa al final del .bin
    fclose(fCarrera);

    return TODO_OK;
}


/**
 * cargarResultadosCarreraAleatorios
 * Genera el orden de llegada aleatorio para una carrera.
 * Paso a paso:
 *   1. Crea un vector de IDs de pilotos activos con
 *      cargarVectorPilotoActivos().
 *   2. Mezcla el vector con Fisher-Yates via
 *      generarResultadoAleatorioVector().
 *   3. Recorre el vector mezclado y para cada posicion i:
 *      - Copia el ID del piloto en resultados[i][COL_ID_PILOTO].
 *      - Asigna los puntos F1 para esa posicion con
 *        puntosParaPosicion(); si esta fuera de rango usa
 *        puntosParaPosicion(pts, 0) que retorna 0.
 *   4. Libera el vector.
 * Retorna TODO_OK, SIN_MEM o ERR_ARCH.
 */
int cargarResultadosCarreraAleatorios(const char* rutaPiloto, Carrera* nueva, Comparar comparar, const Puntos* pts)
{
    tVector vIds;
    int i, pos;

    if(crearVector(&vIds, sizeof(unsigned), MAX_PILOTOS_CARRERA))
        return SIN_MEM;

    if(cargarVectorPilotoActivos(rutaPiloto, &vIds, comparar)) // llena vIds con IDs activos
    {
        destruirVector(&vIds);
        return ERR_ARCH;
    }

    if(vIds.ce == 0) // no hay pilotos activos, no se puede simular
    {
        destruirVector(&vIds);
        return ERR_ARCH;
    }

    generarResultadoAleatorioVector(&vIds); // mezcla los IDs (shuffle)

    nueva->cant_resultados = (int)vIds.ce;

    for(i = 0; i < nueva->cant_resultados; i++)
    {
        pos = *(unsigned*)obtenerElementoVector(&vIds, i); // obtiene el ID en la posicion i

        nueva->resultados[i][COL_ID_PILOTO] = pos;
        nueva->resultados[i][COL_PUNTOS] = puntosParaPosicion(pts, i + 1);
    }

    destruirVector(&vIds);

    return TODO_OK;
}

/**
 * recalcularPuntosPilotos_old
 * Version anterior de recalcularPuntosPilotos (conservada como referencia).
 * Carga TODOS los pilotos y TODAS las carreras en vectores en memoria,
 * resetea los puntos a 0, recorre las carreras acumulando puntos con
 * busquedaBinariaVector() y guarda el vector actualizado en el .bin.
 * Limitacion: requiere que ambos vectores quepan en memoria simultaneamente.
 */
//int recalcularPuntosPilotos_old(const char* rutaCarrera, const char* rutaPiloto)
//{
//    tVector  vPilotos, vCarreras;
//    Carrera* carrera; //Puntero auxiliar
//    Piloto*  piloto; //Puntero auxiliar
//    char    *act, *fin; //Lectura byte a byte
//    unsigned idBuscar;
//    size_t      i;
//
//    /** 1. Cargar pilotos en memoria **/
//    if(crearVector(&vPilotos, sizeof(Piloto), MAX_PILOTOS_CARRERA))
//        return SIN_MEM;
//
//    if(cargarVectorDesdeBin(rutaPiloto, &vPilotos))
//    {
//        destruirVector(&vPilotos);
//        return ERR_ARCH;
//    }
//
//    /** 2. Seteo su valor a 0, para evitar basura **/
//    act = (char*)vPilotos.vec;
//    fin = act + (vPilotos.ce * vPilotos.tamElem);
//
//    while(act < fin)
//    {
//        ((Piloto*)act)->puntos_acumulados = 0;
//        act += vPilotos.tamElem;
//    }
//
//    /** 3. Cargar carreras en memoria **/
//    if(crearVector(&vCarreras, sizeof(Carrera), MAX_CARRERAS_TEMPORADA))
//    {
//        destruirVector(&vPilotos);
//        return SIN_MEM;
//    }
//
//    /** Si no hay carreras aun, guardamos pilotos con puntos en 0 y salimos **/
//    if(cargarVectorDesdeBin(rutaCarrera, &vCarreras))
//    {
//        guardarVectorEnBin(rutaPiloto, &vPilotos);
//        destruirVector(&vPilotos);
//        destruirVector(&vCarreras);
//        return TODO_OK;
//    }
//
//    /** 4. Recorrer carreras y acumulamos sus puntos **/
//    act = (char*)vCarreras.vec;
//    fin = act + (vCarreras.ce * vCarreras.tamElem);
//
//    /**5. Actualizamos sus puntos acumulados**/
//    while(act < fin)
//    {
//        carrera = (Carrera*)act;
//
//        //Demasiadas condiciones, buscar una alternativa
//        if(carrera->estado == ESTADO_CARRERA_ACTIVA)
//        {
//            for(i = 0; i < carrera->cant_resultados; i++)
//            {
//                if(carrera->resultados[i][COL_PUNTOS] > 0)
//                {
//                    idBuscar = (unsigned)carrera->resultados[i][COL_ID_PILOTO];
//
//                    /** El vector debe estar ordenado por id **/
//                    piloto = (Piloto*)busquedaBinariaVector(&vPilotos,
//                                                            &idBuscar,
//                                                            compararUnsigned);
//                    if(piloto)
//                        piloto->puntos_acumulados += (unsigned)carrera->resultados[i][COL_PUNTOS];
//                }
//            }
//        }
//        act += vCarreras.tamElem;
//    }
//
//    /** 6. Guardar y liberar memoria **/
//    guardarVectorEnBin(rutaPiloto, &vPilotos);
//    destruirVector(&vPilotos);
//    destruirVector(&vCarreras);
//
//    return TODO_OK;
//}

/**
 * recalcularPuntosPilotos
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
int recalcularPuntosPilotos(const char* rutaCarrera, const char* rutaPiloto)
{
    unsigned ids[CAP_MAX];   /* IDs de pilotos en orden de archivo */
    unsigned acum[CAP_MAX];  /* acumulador de puntos por piloto    */
    Piloto   pil;            /* buffer de lectura/escritura, stack */
    Carrera  car;            /* buffer de lectura, stack           */
    FILE*    fPil;
    FILE*    fCar;
    size_t   n = 0;
    size_t   i;
    int      j;

    /* Paso 1: leer piloto.bin una sola vez y construir el indice ids[]/acum[] */
    fPil = fopen(rutaPiloto, "rb");
    if (!fPil)
        return ERR_ARCH;

    while (n < CAP_MAX && fread(&pil, sizeof(Piloto), 1, fPil) == 1)
    {
        ids[n]  = pil.id; // guarda el ID para buscarlo en las carreras
        acum[n] = 0;      // inicia el acumulador en 0 antes de sumar
        n++;
    }
    fclose(fPil);

    /* Paso 2: recorrer carrera.bin acumulando puntos por piloto */
    fCar = fopen(rutaCarrera, "rb");
    if (!fCar)
    {
        /* Sin carreras todavia: pone puntos en 0 en el .bin y sale */
        fPil = fopen(rutaPiloto, "r+b");
        if (!fPil)
            return ERR_ARCH;

        for (i = 0; i < n; i++)
        {
            fseek(fPil, (long)(i * sizeof(Piloto)), SEEK_SET); // va al registro i
            fread(&pil, sizeof(Piloto), 1, fPil);
            pil.puntos_acumulados = 0;
            fseek(fPil, (long)(i * sizeof(Piloto)), SEEK_SET); // vuelve para sobreescribir
            fwrite(&pil, sizeof(Piloto), 1, fPil);
        }
        fclose(fPil);
        return TODO_OK;
    }

    /**Archivo carrera**/
    while (fread(&car, sizeof(Carrera), 1, fCar) == 1)
    {
        if (car.estado != ESTADO_CARRERA_ACTIVA)
            continue; // ignora carreras canceladas

        for (i = 0; i < (size_t)car.cant_resultados; i++)
        {
            if (car.resultados[i][COL_PUNTOS] <= 0)
                continue; // no suma si el piloto no hizo puntos

            // busca el piloto en el indice y suma sus puntos
            for (j = 0; j < (int)n; j++)
            {
                if (ids[j] == (unsigned)car.resultados[i][COL_ID_PILOTO])
                {
                    acum[j] += (unsigned)car.resultados[i][COL_PUNTOS];
                    break;
                }
            }
        }
    }
    fclose(fCar);

    /* Paso 3: actualizar puntos_acumulados en piloto.bin (read-modify-write por registro) */
    fPil = fopen(rutaPiloto, "r+b");
    if (!fPil)
        return ERR_ARCH;

    for (i = 0; i < n; i++)
    {
        fseek(fPil, (long)(i * sizeof(Piloto)), SEEK_SET);
        fread(&pil, sizeof(Piloto), 1, fPil);
        pil.puntos_acumulados = acum[i];           // asigna el total acumulado
        fseek(fPil, (long)(i * sizeof(Piloto)), SEEK_SET);
        fwrite(&pil, sizeof(Piloto), 1, fPil);     // sobreescribe solo ese campo
    }
    fclose(fPil);

    return TODO_OK;
}

/**
 * generarIdCarrera
 * Determina el ID para la proxima carrera de forma autoincremental.
 * Si el archivo esta vacio retorna 1 (primera carrera).
 * Si ya tiene registros, hace fseek al ultimo, lee su ID y retorna ID+1.
 * Retorna el nuevo ID como int.
 */
int generarIdCarrera(FILE* fCarrera)
{
    Carrera ultima;

    fseek(fCarrera, 0, SEEK_END);

    /**Si esta vacio sera la primera**/
    if(ftell(fCarrera) < (long)sizeof(Carrera))
        return 1;

    /**Me posicion en la ultima carrera ingresada, para obtener el id**/
    fseek(fCarrera, -(long)sizeof(Carrera), SEEK_END);
    fread(&ultima, sizeof(Carrera), 1, fCarrera);

    return (ultima.id + 1);
}


/**
 * mostrarCarrera  [Mostrar]
 * Imprime el encabezado de la carrera (ID, circuito, fecha, estado)
 * y la lista de resultados con posicion, ID de piloto y puntos.
 * Descompone la fecha AAAAMMDD en dia/mes/anio para mostrarla
 * en formato DD/MM/AAAA.
 */
void mostrarCarrera(const void *dato)
{
    Carrera *c;
    int i;
    unsigned dia, mes, anio;

    c = (Carrera *)dato;

    // descompone el entero AAAAMMDD en sus partes
    anio = (unsigned)(c->fecha / 10000);
    mes  = (unsigned)(c->fecha / 100 % 100);
    dia  = (unsigned)(c->fecha % 100);

    printf("----------------------------------\n");
    printf("ID Carrera  : %d\n",  c->id);
    printf("Circuito    : %s\n",  c->circuito);
    printf("Fecha       : %02u/%02u/%04u\n", dia, mes, anio);
    printf("Estado      : %s\n",  c->estado == 1 ? "Activa" : "Cancelada");
    printf("Resultados  : %d pilotos\n", c->cant_resultados);

    for (i = 0; i < c->cant_resultados; i++)
    {
        printf("  Pos %2d | Piloto ID: %3d | Puntos: %2d\n",
               i + 1,
               c->resultados[i][COL_ID_PILOTO],
               c->resultados[i][COL_PUNTOS]);
    }
}

/**
 * exportarCarrerasTxt
 * Lee carreras.dat y escribe cada registro en formato CSV al
 * archivo de exportacion.
 * Formato por linea:
 *   id,circuito,fecha,estado,cant_resultados,idPiloto:puntos-idPiloto:puntos-...
 * Los resultados se escriben con aritmetica de punteros sobre
 * la matriz: *(*(c.resultados + i) + 0) = id, + 1 = puntos.
 * El separador entre pares es "-"; no se escribe despues del ultimo.
 * Retorna TODO_OK o 0 si no pudo abrir alguno de los archivos.
 */
int exportarCarrerasTxt(const char* rutaBin, const char* rutaTxtExportado)
{
    FILE* fBin = fopen(rutaBin, "rb");
    if(!fBin) return 0;

    FILE* fTxt = fopen(rutaTxtExportado, "wt");
    if(!fTxt)
    {
        fclose(fBin);
        return 0;
    }

    Carrera c;
    int registrosExportados = 0;

    while(fread(&c, sizeof(Carrera), 1, fBin))
    {
        // cabecera de la carrera: id,circuito,fecha,estado,cant_resultados,
        fprintf(fTxt, "%d,%s,%llu,%d,%d,",
                c.id, c.circuito, c.fecha, c.estado, c.cant_resultados);

        for(int i = 0; i < c.cant_resultados; i++)
        {
            fprintf(fTxt, "%d:%d",
                    *(*(c.resultados + i) + 0),  // id del piloto
                    *(*(c.resultados + i) + 1));  // puntos obtenidos

            if(i < c.cant_resultados - 1)
                fprintf(fTxt, "-"); // separador entre resultados
        }

        fprintf(fTxt, "\n");
        registrosExportados++;
    }

    fclose(fBin);
    fclose(fTxt);
    return TODO_OK;
}

/**
 * bajaCarrera
 * Busca la carrera por ID con buscarRegistroPorId().
 * Si esta activa y el usuario confirma, cambia su estado a
 * ESTADO_CARRERA_INACTIVA y sobreescribe el registro con fseek.
 * Registra la baja en el archivo de texto de bajas con formato:
 *   CARRERA|id|circuito|fecha|CANCELADA
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int bajaCarrera(const char* rutaBin, const char* rutaBajasTxt)
{
    Carrera  carrera;
    FILE*    fBin;
    FILE*    fBajas;
    long     offset;
    unsigned id;
    unsigned dia, mes, anio;

    fBin = fopen(rutaBin, "rb+");

    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de carreras.\n");
        return ERR_ARCH;
    }

    printf("\n--- BAJA DE CARRERA ---\n");
    printf("ID de la carrera: ");
    scanf("%u", &id);

    offset = buscarRegistroPorId(fBin, (unsigned)id, &carrera, sizeof(Carrera));

    if (offset == -1L)
    {
        printf("[!] Carrera con ID %u no encontrada.\n", id);
        return NO_ENCONTRADO;
    }

    if (carrera.estado == ESTADO_CARRERA_INACTIVA)
    {
        printf("[!] La carrera '%s' ya se encuentra cancelada.\n", carrera.circuito);
        return TODO_OK;
    }

    anio = (unsigned)(carrera.fecha / 10000);
    mes  = (unsigned)(carrera.fecha / 100 % 100);
    dia  = (unsigned)(carrera.fecha % 100);

    printf("  Carrera encontrada: %s  (%02u/%02u/%04u)\n",carrera.circuito, dia, mes, anio);
    printf("  Confirma la baja? [1] Si  [0] No: ");

    {
        int confirmacion;
        scanf("%d", &confirmacion);

        if (confirmacion != 1)
        {
            printf("[!] Baja cancelada.\n");
            return TODO_OK;
        }
    }

    carrera.estado = ESTADO_CARRERA_INACTIVA;


    fseek(fBin, offset, SEEK_SET);
    fwrite(&carrera, sizeof(Carrera), 1, fBin);
    fclose(fBin);

    fBajas = fopen(rutaBajasTxt, "at");
    if (fBajas)
    {
        fprintf(fBajas, "CARRERA|%d|%s|%llu|CANCELADA\n",
                carrera.id, carrera.circuito, carrera.fecha);
        fclose(fBajas);
    }

    printf("[OK] Carrera '%s' dada de baja.\n", carrera.circuito);
    return TODO_OK;
}

/**
 * modificarCarrera
 * Busca la carrera por ID, presenta un submenu de campos
 * editables: [1] circuito, [2] fecha, [3] estado.
 * El ciclo repite hasta que el usuario elija 0.
 * Nota: los resultados y cant_resultados no son modificables aqui.
 * Sobreescribe el registro completo con fseek al finalizar.
 * Retorna TODO_OK, ERR_ARCH o NO_ENCONTRADO.
 */
int modificarCarrera(const char* rutaBin)
{
    Carrera            carrera;
    FILE*              fBin;
    long               offset;
    unsigned           id;
    int                campo;
    unsigned long long fechaNueva;
    int nuevoEstado;

    fBin = fopen(rutaBin, "rb+");

    if (!fBin)
    {
        printf("[!] No se pudo abrir el archivo de carreras.\n");
        return ERR_ARCH;
    }

    printf("\n--- MODIFICAR CARRERA ---\n");
    printf("ID de la carrera: ");
    scanf("%u", &id);

    offset = buscarRegistroPorId(fBin, id, &carrera, sizeof(Carrera));

    if (offset == -1L)
    {
        printf("[!] Carrera con ID %u no encontrada.\n", id);
        return NO_ENCONTRADO;
    }

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
        {

            printf("  [1] Activa  [0] Cancelada: ");
            scanf("%d", &nuevoEstado);
            carrera.estado = (nuevoEstado == 1)
                             ? ESTADO_CARRERA_ACTIVA
                             : ESTADO_CARRERA_INACTIVA;
            break;
        }

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

    printf("[OK] Carrera ID %d modificada correctamente.\n", carrera.id);
    return TODO_OK;
}
