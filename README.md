# TP F1 — Gestión de Temporada de Fórmula 1

**Materia:** Tópicos de Programación · Comisión 04-3600  
**Lenguaje:** C (ANSI C / C99)  
**IDE:** Code::Blocks 25.03  

---

## Descripción

Sistema de gestión de una temporada de Fórmula 1 desarrollado en C. Permite registrar pilotos, escuderías y carreras, calcular puntos acumulados, consultar estadísticas de la temporada y realizar altas, bajas lógicas y modificaciones sobre todas las entidades.

El diseño se basa en **Tipos de Datos Abstractos (TDA)** con separación estricta de módulos `.c`/`.h`, uso de **memoria dinámica** y **funciones genéricas mediante punteros a función** (`Filter`, `Reduce`, `Map`, `Comparar`, etc.).

---

## Estructura del proyecto

```
TP_F1/
├── main.c
├── piloto.c / piloto.h
├── escuderia.c / escuderia.h
├── carrera.c / carrera.h
├── resultado.h
├── puntos.c / puntos.h
├── sancion.c / sancion.h
├── vector.c / vector.h
├── utilidades.c / utilidades.h
└── archivos/
    ├── piloto.txt       ← carga inicial
    ├── piloto.dat       ← binario generado
    ├── escuderia.txt    ← carga inicial
    ├── escuderia.dat    ← binario generado
    ├── carrera.dat      ← carreras registradas
    └── puntos.dat       ← tabla de puntos configurable
```

---

## Módulos

### `utilidades` — Funciones genéricas y de archivo

Núcleo transversal del sistema. Define los **códigos de retorno** unificados y los **tipos de punteros a función** usados en todo el proyecto.

**Punteros a función definidos:**

| Tipo | Firma | Uso |
|------|-------|-----|
| `Comparar` | `int (*)(const void*, const void*)` | Ordenamiento y búsqueda binaria |
| `Mostrar` | `void (*)(const void*)` | Listados por pantalla |
| `Filter` | `int (*)(const void*)` | Filtrado de registros |
| `Reduce` | `int (*)(void*, const void*)` | Acumulación (puntos, estadísticas) |
| `Map` | `int (*)(void*, const void*)` | Transformación de estructuras |
| `TxtABin` | `int (*)(char*, void*)` | Parseo línea de texto → struct |
| `BinATxt` | `void (*)(const void*, FILE*)` | Exportación struct → texto |
| `Accion` | `int (*)(void*, const void*)` | Operación sobre un registro |

**Funciones genéricas de archivo:**

```c
int convertirArchivoTxtABin(rutaTxt, rutaBin, tamElem, TxtABin);
int convertirArchivoBinATxt(rutaBin, rutaTxt, tamElem, BinATxt);
int mostrarArchivoBinario(rutaBin, tamElem, Mostrar);
int procesarArchivoBinario(rutaBin, datos, tamElem, Filter, Accion);
```

**Códigos de retorno:**

| Código | Valor | Significado |
|--------|-------|-------------|
| `TODO_OK` | 0 | Operación exitosa |
| `ERR_ARCH` | 1 | Error de archivo |
| `ERR_LINEA` | 2 | Línea inválida |
| `SIN_MEM` | 3 | Sin memoria |
| `NO_ENCONTRADO` | 4 | Registro no encontrado |

---

### `vector` — TDA Vector dinámico genérico

Vector de capacidad fija en heap, de elementos de cualquier tipo (`void*`). Todas las operaciones trabajan con aritmética de punteros.

```c
typedef struct {
    void*  vec;      // bloque de memoria
    size_t ce;       // cantidad de elementos cargados
    size_t tamElem;  // tamaño en bytes de cada elemento
    size_t tope;     // capacidad máxima
} tVector;
```

**Operaciones disponibles:**

```c
int   crearVector(tVector* v, size_t tamElem, size_t capacidad);
void  destruirVector(tVector* v);
int   insertarVectorOrd(tVector* v, void* dato, Comparar cmp);
void* busquedaBinariaVector(tVector* v, void* clave, Comparar cmp);
void* obtenerElementoVector(tVector* v, size_t pos);
int   cargarVectorDesdeBin(const char* rutaBin, tVector* v);
int   guardarVectorEnBin(const char* rutaBin, tVector* v);
int   filtrarVector(tVector* origen, tVector* destino, Filter filtro);
int   reducirVector(tVector* v, void* acumulador, Reduce reducir);
int   mapearVector(tVector* origen, tVector* destino, size_t tamDest, Map mapear);
void  generarResultadoAleatorioVector(tVector* v);  // Fisher-Yates
```

---

### `piloto` — TDA Piloto

```c
typedef struct {
    unsigned           id;
    char               nombre[30];
    char               nacionalidad[30];
    unsigned           id_escuderia;
    unsigned           puntos_acumulados;
    char               estado;           // 'A', 'R' o 'S'
    unsigned long long fechaNacimiento;  // formato AAAAMMDD
} Piloto;
```

**Estados del piloto:**

| Constante | Valor | Descripción |
|-----------|-------|-------------|
| `ESTADO_ACTIVO_PILOTO` | `'A'` | Activo en la temporada |
| `ESTADO_RETIRADO_PILOTO` | `'R'` | Retirado |
| `ESTADO_SUSPENDIDO_PILOTO` | `'S'` | Suspendido |

**Funciones clave:**

```c
int  generarArchivoPilotosTxt(const char* rutaTxt);
int  cargarVectorPilotoActivos(const char* rutaBin, tVector* vIds, Comparar cmp);
int  trozarPilotoTxt(char* linea, void* reg);    // TxtABin
void mostrarPiloto(const void* dato);             // Mostrar
int  esPilotoActivo(const void* dato);            // Filter
long buscarPilotoEnBin(rutaBin, idBuscado, dest); // offset o -1L
int  darBajaPiloto(rutaBin, rutaCarrera, id);     // baja lógica
int  modificarPiloto(rutaBin, id);                // edición campo a campo
```

---

### `escuderia` — TDA Escudería

```c
typedef struct {
    unsigned id;
    char     codigo[4];
    char     nombre[30];
    char     pais[50];
    int      estado;   // 1: activa, 0: inactiva
} Escuderia;
```

Escuderías de la temporada 2026:

| ID | Código | Nombre | País |
|----|--------|--------|------|
| 1 | RBR | Red Bull Racing | Austria |
| 2 | MCL | McLaren | Reino Unido |
| 3 | FER | Ferrari | Italia |
| 4 | WLF | Williams | Reino Unido |
| 5 | AMR | Aston Martin | Reino Unido |
| 6 | SAU | Sauber | Suiza |

**Funciones clave:**

```c
int  generarArchivoEscuderiasTxt(const char* rutaTxt);
int  trozarEscuderiaTxt(char* linea, void* reg);           // TxtABin
long buscarEscuderiaEnBin(rutaBin, idBuscado, dest);       // offset o -1L
int  darBajaEscuderia(rutaBin, idEscuderia);               // baja lógica
int  modificarEscuderia(rutaBin, idEscuderia);             // edición campo a campo
```

---

### `carrera` — TDA Carrera

La carrera se serializa en `carrera.dat` como un registro variable:

```
[ CarreraHeader ][ ResultadoPiloto × cant_resultados ]
```

Esto evita matrices fijas en disco y permite un número variable de pilotos por carrera.

```c
typedef struct {
    int                id;
    char               circuito[20];
    unsigned long long fecha;          // AAAAMMDD
    int                estado;         // 1: activa, 0: cancelada
    int                cant_resultados;
} CarreraHeader;

typedef struct {
    CarreraHeader info;
    tVector       resultados;  // elementos: ResultadoPiloto
} Carrera;
```

**Tabla de puntos F1 (top 10, configurable):**

| Pos | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|-----|---|---|---|---|---|---|---|---|---|---|
| Pts | 25 | 18 | 15 | 12 | 10 | 8 | 6 | 4 | 2 | 1 |

**Modos de registro:**

- **Aleatorio:** genera posiciones con Fisher-Yates sobre los pilotos activos. Todos con estado `RES_FIN` y puntos según tabla.
- **Manual:** el usuario ingresa cada piloto y su estado. `RES_DNF`, `RES_DNS` y `RES_DSQ` asignan 0 puntos automáticamente. Los pilotos no ingresados se completan automáticamente como `DNS`.

**Actualización de puntos — dos estrategias:**

```c
// Recalcula desde cero (todas las carreras activas)
int recalcularPuntosPilotos(rutaCarrera, rutaPiloto, Filter, Reduce);

// Suma solo la última carrera (más eficiente)
int actualizarPuntosUltimaCarrera(rutaCarrera, rutaPiloto, Filter, Reduce);
```

**ABM sobre `carrera.dat`:**

```c
long buscarCarreraHeaderEnBin(rutaBin, idBuscado, dest); // recorrido secuencial (formato variable)
int  darBajaCarrera(rutaBin, rutaPiloto, idCarrera);     // baja lógica + recalculo automático
int  modificarCarrera(rutaBin, idCarrera);               // edita circuito y/o fecha
```

**Funciones auxiliares de carrera:**

```c
void ordenarResultados(tVector* vRes);                    // RES_FIN primero, insertion sort estable
void autocompletarResultados(Carrera* c, vIdsActivos);    // completa ausentes como DNS
int  listarTodasLasCarreras(const char* rutaCarrera);
```

---

### `resultado` — TDA ResultadoPiloto

```c
typedef struct {
    unsigned id_piloto;
    int      puntos;
    int      estado_resultado;  // RES_FIN, RES_DNF, RES_DNS, RES_DSQ
} ResultadoPiloto;
```

**Estados de resultado:**

| Constante | Valor | Puntos |
|-----------|-------|--------|
| `RES_FIN` | 1 | Según posición (tabla F1) |
| `RES_DNF` | 2 | 0 siempre |
| `RES_DNS` | 3 | 0 siempre |
| `RES_DSQ` | 4 | 0 siempre |

---

### `puntos` — Tabla de puntos configurable

Permite personalizar cuántas posiciones suman puntos y cuántos puntos otorga cada una. Se persiste en `archivos/puntos.dat` y se carga al iniciar el sistema.

```c
typedef struct {
    int posiciones;                    // cuántas posiciones otorgan puntos
    int tabla[MAX_POSICIONES_PUNTOS];  // tabla[0] = puntos pos 1, etc.
} Puntos;

void inicializarPuntosDefault(Puntos* vPuntos);      // valores F1 estándar
int  guardarConfigPuntos(ruta, vPuntos);
int  cargarConfigPuntos(ruta, vPuntos);              // genera defaults si no existe
int  puntosParaPosicion(vPuntos, posicion);          // 0 si fuera de rango
void mostrarConfigPuntos(vPuntos);
```

---

## Flujo de datos

```
piloto.txt  ──────────────► piloto.dat
escuderia.txt ────────────► escuderia.dat
puntos.dat (config) ──────► Puntos (en memoria)
                                │
                         [en memoria]
                           tVector
                                │
          Registrar carrera ────┤
          (aleatorio / manual)  │
                                ▼
                          carrera.dat
                                │
          Recalcular puntos ────┤
                                ▼
                     piloto.dat (actualizado)
```

---

## Menú principal

```
+=====================================+
|    GESTION TEMPORADA F1 2026        |
|=====================================|
|  1. Listar pilotos y sus puntos     |
|  2. Registrar carrera               |
|     └─ 1. Simulación aleatoria      |
|     └─ 2. Ingreso manual            |
|  3. Listar escuderías               |
|  4. Ver todas las carreras          |
|-------------------------------------|
|  5. ABM Pilotos                     |
|  6. ABM Escuderías                  |
|  7. ABM Carreras                    |
|  8. Configurar tabla de puntos      |
|-------------------------------------|
|  0. Salir                           |
+=====================================+
```

---

## Validaciones implementadas

- Fecha de carrera con formato AAAAMMDD y validación de rango (incluyendo años bisiestos).
- Advertencia si la fecha ingresada es anterior a la última carrera registrada.
- Verificación de que el ID de piloto exista **y** esté activo antes de aceptarlo en carrera manual.
- Detección de piloto duplicado dentro de la misma carrera.
- Reintento ante entrada no numérica en los `scanf` de estado y puntos.
- Vector vacío de pilotos activos retorna `VEC_VACIO` con mensaje descriptivo.

---

## Compilación

El proyecto usa **Code::Blocks 25.03** con compilador `gcc`.

Abrí `TP_F1.cbp` y compilá con **Build → Build** (o `Ctrl+F9`).

Flags utilizados:

- Debug: `-g -Wall`
- Release: `-O2 -Wall -s`

El ejecutable necesita que exista la carpeta `archivos/` en el directorio de trabajo. Los archivos `.txt` y `.dat` se generan automáticamente al iniciar el sistema.

---

## Pilotos de prueba — Temporada 2026

| ID | Nombre | Escudería | Nac. |
|----|--------|-----------|------|
| 1 | Max Verstappen | Red Bull Racing | NED |
| 2 | Lando Norris | McLaren | GBR |
| 3 | Charles Leclerc | Ferrari | MON |
| 4 | Oscar Piastri | McLaren | AUS |
| 5 | Carlos Sainz | Williams | ESP |
| 6 | George Russell | Red Bull Racing | GBR |
| 7 | Lewis Hamilton | Ferrari | GBR |
| 8 | Fernando Alonso | Aston Martin | ESP |
| 9 | Lance Stroll | Aston Martin | CAN |
| 10 | Nico Hülkenberg | Sauber | GER |

---

## Lote de prueba sugerido

| # | Caso | Qué verificar |
|---|------|---------------|
| 1 | 0 pilotos activos | Todos en estado `R` o `S`, intentar carrera aleatoria → `VEC_VACIO` |
| 2 | 1 solo piloto activo | Carrera aleatoria con un único participante |
| 3 | ID inexistente en manual | Ingresar ID `999` → rechazo con mensaje |
| 4 | Mismo piloto dos veces | Ingresar el mismo ID dos veces → rechazo por duplicado |
| 5 | Fecha inválida | Probar `20261340`, `0`, y letras → reintento |
| 6 | Piloto con DNF/DNS/DSQ | Verificar que aparece en resultados con 0 puntos |
| 7 | Carrera sin resultados | Terminar ingreso manual con ID `0` sin cargar ningún piloto |
| 8 | Input no numérico en menú | Ingresar `abc` en cualquier `scanf` numérico → reintento |
| 9 | Baja de carrera | Dar de baja una carrera y verificar que los puntos se recalculan |
| 10 | Tabla de puntos custom | Cambiar tabla y registrar carrera → verificar puntos asignados |
