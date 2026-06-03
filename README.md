# TP F1 2026 — Gestión de Temporada

Sistema de gestión de temporada de Fórmula 1 desarrollado en C. Permite registrar carreras, administrar pilotos y escuderías, y calcular puntos acumulados. Implementado con **TDA (Tipos de Datos Abstractos)**, **memoria dinámica** y **funciones genéricas con puntero a función**.

---

## Funcionalidades implementadas

### 1. Listar pilotos y sus puntos
Recalcula los puntos de todos los pilotos desde cero recorriendo `carrera.dat`. Solo considera carreras con estado **Activa**. Actualiza `piloto.dat` y muestra el listado completo con nombre, escudería, nacionalidad y puntos acumulados.

### 2. Registrar carrera — Simulación aleatoria
Carga todos los pilotos activos desde `piloto.dat`, mezcla sus IDs con el algoritmo **Fisher-Yates** y asigna posiciones al azar. Distribuye puntos según la tabla oficial F1 (25-18-15-12-10-8-6-4-2-1 para el top 10). Persiste la carrera en `carrera.dat` y actualiza los puntos acumulados de cada piloto.

### 2. Registrar carrera — Ingreso manual
El usuario ingresa uno a uno los pilotos por ID, selecciona el estado del resultado (FIN / DNF / DNS / DSQ) y confirma o modifica los puntos sugeridos. Si un piloto termina **DSQ**, se solicita el tipo de sanción a aplicar. Al finalizar, persiste la carrera y actualiza los puntos.

---

## Estructura de archivos

```
TP_F1/
├── main.c              Punto de entrada, menú principal e inicialización
├── utilidades.c/h      Funciones genéricas: archivos, fechas, comparadores, punteros a función
├── vector.c/h          TDA Vector dinámico genérico (void*, tamElem, Fisher-Yates, filtrar/reducir/mapear)
├── piloto.c/h          TDA Piloto: carga, conversión txt↔bin, mostrar, filtrar activos
├── escuderia.c/h       TDA Escudería: carga, conversión txt↔bin, mostrar
├── carrera.c/h         TDA Carrera: alta manual/aleatoria, serialización variable, recálculo de puntos
├── resultado.h         Estructura ResultadoPiloto (id_piloto, puntos, estado)
├── sancion.c/h         TDA Sanción: penalizaciones por DSQ, descuento de puntos, suspensión
└── archivos/
    ├── piloto.txt / piloto.dat
    ├── escuderia.txt / escuderia.dat
    ├── carrera.dat
    └── sancion.dat
```

---

## Módulos

### `utilidades.c/h`
Base del proyecto. Define los **códigos de retorno** (`TODO_OK`, `ERR_ARCH`, `SIN_MEM`, etc.), los **tipos de puntero a función** (`Comparar`, `Mostrar`, `Filter`, `Reduce`, `Map`, `Accion`, `TxtABin`, `BinATxt`) y funciones genéricas para operar sobre archivos binarios y de texto sin conocer el tipo concreto del dato.

### `vector.c/h`
TDA Vector dinámico genérico que almacena cualquier tipo usando `void*`. Soporta inserción ordenada (insertion sort), búsqueda binaria, persistencia en `.bin`, y operaciones funcionales (`filtrarVector`, `reducirVector`, `mapearVector`). La mezcla aleatoria Fisher-Yates se usa para simular resultados de carrera.

### `piloto.c/h`
TDA Piloto. Genera el archivo `piloto.txt` con el plantel inicial de la temporada y lo convierte a binario. Provee las implementaciones concretas de los punteros a función (`trozarPilotoTxt`, `mostrarPiloto`, `esPilotoActivo`, etc.) que se pasan a las funciones genéricas de `utilidades` y `vector`.

### `escuderia.c/h`
TDA Escudería. Misma arquitectura que Piloto: generación de `.txt`, conversión a `.bin` y punteros a función propios (`trozarEscuderiaTxt`, `mostrarEscuderia`, `esEscuderiaActiva`).

### `carrera.c/h`
TDA Carrera. La serialización en `carrera.dat` usa un formato **variable**: `[CarreraHeader][ResultadoPiloto × N]`, donde N se lee del header. Esto evita desperdiciar memoria con una matriz fija. Contiene la lógica de asignación de puntos F1, el recálculo completo desde cero y la actualización incremental de solo la última carrera.

### `resultado.h`
Estructura `ResultadoPiloto` que une un piloto a su resultado en una carrera específica (puntos y estado: FIN/DNF/DNS/DSQ). Es la unidad almacenada dentro de cada Carrera.

### `sancion.c/h`
TDA Sanción. Persiste penalizaciones por DSQ en `sancion.dat`. Soporta tres tipos: descuento de puntos, saltar carreras, o ambas. Si el piloto acumula `UMBRAL_DSQ_SUSPENSION` (2) sanciones, su estado pasa a Suspendido en `piloto.dat`.

---

## Flujo de inicialización

Al arrancar, el sistema regenera siempre los archivos `.txt` y `.bin` de pilotos y escuderías desde los datos hardcodeados en memoria, garantizando un estado limpio para pruebas. El archivo `carrera.dat` y `sancion.dat` se acumulan entre ejecuciones (modo append).

---

## Formato de serialización en carrera.dat

```
[ CarreraHeader (id, circuito, fecha, estado, cant_resultados) ]
[ ResultadoPiloto #1 (id_piloto, puntos, estado_resultado)     ]
[ ResultadoPiloto #2 ...                                        ]
...
[ CarreraHeader siguiente ]
...
```

---

## Tabla de puntos F1

| Pos | 1  | 2  | 3  | 4  | 5  | 6 | 7 | 8 | 9 | 10 |
|-----|----|----|----|----|----|----|---|---|---|----|
| Pts | 25 | 18 | 15 | 12 | 10 | 8 | 6 | 4 | 2 | 1  |

Solo las posiciones 1 a 10 suman puntos. DNF, DNS y DSQ reciben 0 (DSQ puede recibir penalización adicional).
