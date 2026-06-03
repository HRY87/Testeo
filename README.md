# TP F1 — Gestión de Temporada de Fórmula 1

**Materia:** Tópicos de Programación · Comisión 04-3600  
**Lenguaje:** C (ANSI C / C99)  
**IDE:** Code::Blocks 25.03  

---

## Descripción

Sistema de gestión de una temporada de Fórmula 1 desarrollado en C. Permite registrar pilotos, escuderías y carreras, calcular puntos acumulados, aplicar sanciones por descalificación y consultar estadísticas de la temporada.

El diseño se basa en **Tipos de Datos Abstractos (TDA)** con separación estricta de módulos `.c`/`.h`, uso de **memoria dinámica** y **funciones genéricas mediante punteros a función** (`Filter`, `Reduce`, `Map`, `Comparar`, etc.).

---

## Estructura del proyecto

```
TP_F1/
├── main.c
├── piloto.c / piloto.h
├── escuderia.c / escuderia.h
├── carrera.c / carrera.h
├── sancion.c / sancion.h
├── vector.c / vector.h
├── resultado.h
├── utilidades.c / utilidades.h
└── archivos/
    ├── piloto.txt       ← carga inicial
    ├── piloto.dat       ← binario generado
    ├── escuderia.txt    ← carga inicial
    ├── escuderia.dat    ← binario generado
    ├── carrera.dat      ← carreras registradas
    └── sancion.dat      ← sanciones aplicadas
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
| `ESTADO_SUSPENDIDO_PILOTO` | `'S'` | Suspendido por sanciones |

**Funciones clave:**

```c
int generarArchivoPilotosTxt(const char* rutaTxt);
int cargarVectorPilotoActivos(const char* rutaBin, tVector* vIds, Comparar cmp);
int trozarPilotoTxt(char* linea, void* reg);   // TxtABin
void mostrarPiloto(const void* dato);           // Mostrar
int  esPilotoActivo(const void* dato);          // Filter
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

**Tabla de puntos F1 (top 10):**

| Pos | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|-----|---|---|---|---|---|---|---|---|---|---|
| Pts | 25 | 18 | 15 | 12 | 10 | 8 | 6 | 4 | 2 | 1 |

**Modos de registro:**

- **Aleatorio:** genera posiciones con Fisher-Yates sobre los pilotos activos.
- **Manual:** el usuario ingresa cada piloto, estado (FIN/DNF/DNS/DSQ) y puntos.

**Actualización de puntos — dos estrategias:**

```c
// Recalcula desde cero (todas las carreras activas)
int recalcularPuntosPilotos(rutaCarrera, rutaPiloto, Filter, Reduce);

// Suma solo la última carrera (más eficiente)
int actualizarPuntosUltimaCarrera(rutaCarrera, rutaPiloto, Filter, Reduce);
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

---

### `sancion` — TDA Sancion

Se aplica automáticamente cuando un piloto termina con estado `DSQ` en una carrera manual.

```c
typedef struct {
    unsigned           id_piloto;
    unsigned           id_carrera;
    int                tipo;      // SANCION_PUNTOS / SANCION_CARRERA / SANCION_AMBAS
    int                puntos;    // puntos a descontar
    int                carreras;  // carreras a saltear
    unsigned long long fecha;
} Sancion;
```

**Tipos de sanción:**

| Constante | Valor | Efecto |
|-----------|-------|--------|
| `SANCION_PUNTOS` | 1 | Descuenta `PUNTOS_PENALIZACION_DSQ` (10 pts) |
| `SANCION_CARRERA` | 2 | El piloto saltea N carreras |
| `SANCION_AMBAS` | 3 | Descuento de puntos + saltear carreras |

Si un piloto acumula `UMBRAL_DSQ_SUSPENSION` (2) sanciones, su estado cambia automáticamente a `'S'` (suspendido).

---

## Flujo de datos

```
piloto.txt  ──────────────► piloto.dat
escuderia.txt ────────────► escuderia.dat
                                │
                         [en memoria]
                           tVector
                                │
          Registrar carrera ────┤
          (aleatorio / manual)  │
                                ▼
                          carrera.dat
                                │
          DSQ detectado ────────┤
                                ▼
                          sancion.dat
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
|  0. Salir                           |
+=====================================+
```

---

## Compilación

El proyecto usa **Code::Blocks 25.03** con compilador `gcc`.

Abrí `TP_F1.cbp` y compilá con **Build → Build** (o `Ctrl+F9`).

Flags utilizados:
- Debug: `-g -Wall`
- Release: `-O2 -Wall -s`

El ejecutable necesita que exista la carpeta `archivos/` en el directorio de trabajo. Los archivos `.txt` y `.dat` se generan automáticamente al iniciar el sistema.

---

## Requisitos técnicos cumplidos

- Aritmética de punteros en todos los recorridos de vectores y cadenas.
- Sin VLA (Variable Length Arrays); toda la memoria dinámica es `malloc`/`free`.
- Variables declaradas al inicio de cada bloque.
- Archivos binarios y de texto nunca mezclados (`fopen` con `"rb"`/`"wb"` vs `"rt"`/`"wt"`).
- Ningún archivo queda abierto ante error (recursos liberados en todos los caminos).
- Cero accesos a memoria no propia.
- Funciones genéricas con punteros a función en lugar de lógica duplicada.
- Separación completa de TDA en archivos `.c`/`.h` independientes.

---

## Puntos avanzados implementados

- **Motor de consultas genérico (B):** `Filter`, `Reduce`, `Map` y `Comparar` permiten filtrar, acumular y transformar cualquier TDA sin reescribir lógica.
- **Simulación de carrera (C):** `generarResultadoAleatorioVector` con Fisher-Yates genera posiciones aleatorias reproducibles tras `srand(time(NULL))`.
- **Sistema incremental de estadísticas (D):** `actualizarPuntosUltimaCarrera` evita recorrer todo `carrera.dat` en el flujo normal; `recalcularPuntosPilotos` solo se invoca al listar o ante cancelaciones.

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
