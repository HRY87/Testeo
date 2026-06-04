# Mejoras pendientes — TP F1 2026

---

## Robustez ante lote de prueba vacío / datos erróneos

**1. `cargarResultadosAleatorios` no valida lote vacío correctamente**
Si todos los pilotos están suspendidos o retirados, `vIds.ce == 0` y el Fisher-Yates opera sobre un vector vacío. El retorno actual es `ERR_ARCH`, que es engañoso. Debería retornar `VEC_VACIO` y mostrar un mensaje claro al usuario.

**2. `registrarCarreraManual` no verifica que el ID ingresado exista**
El usuario puede ingresar cualquier número y el sistema lo acepta sin verificar que ese ID corresponda a un piloto real y activo. Hay que buscar el ID en `piloto.dat` antes de aceptarlo.

**3. `pedirEstadoResultado` no controla entrada no numérica**
Si el usuario ingresa letras, `scanf("%d")` falla silenciosamente y el loop puede volverse infinito porque la variable nunca cambia. Hay que verificar el valor de retorno de `scanf` y limpiar el buffer ante entrada inválida.

**4. `scanf("%llu")` para la fecha no controla entrada no numérica**
Mismo problema que el punto anterior. El retorno de `scanf` debe chequearse antes de validar la fecha.

**5. `generarIdCarrera` usa el último ID leído, no el máximo**
Si por algún motivo las carreras en el `.dat` no están en orden, el nuevo ID puede colisionar con uno existente. Hay que buscar el máximo, no simplemente el último.

---

## Manejo de memoria y recursos

**6. `intercambiar` hace `malloc`/`free` en cada llamada**
El Fisher-Yates llama `intercambiar` N veces seguidas. Cada llamada reserva y libera memoria. Para un vector de 20 elementos son 20 allocations innecesarias. La solución es usar un buffer de tamaño fijo en stack o pasarlo como parámetro externo.

**7. Swap manual de `tVector` en `actualizarPuntosUltimaCarrera` es frágil**
El intercambio de `tmpH`/`tmpV` por campos sueltos se rompe si `tVector` cambia de estructura. Es mejor leer todas las carreras, quedarse con la última y procesarla directamente sin copiar structs manualmente.

**8. `leerCarrera` no verifica capacidad del vector antes de escribir**
Si `cant_resultados` en el header supera el `tope` del vector, el `memcpy` escribe fuera de los límites. Falta un guard que corte el loop si `c->resultados.ce >= c->resultados.tope`.

---

## Validaciones de negocio

**9. No se valida que la fecha de la carrera sea coherente con las anteriores**
Se puede registrar una carrera con fecha `20200101` sin ningún aviso. Una validación mínima sería advertir si la fecha es anterior a la última carrera ya registrada.

**10. Un piloto suspendido puede ingresarse en una carrera manual**
En el ingreso manual solo se verifica que el ID no esté duplicado, no que el piloto esté activo. Hay que agregar la validación de estado `'A'` al aceptar un ID en `registrarCarreraManual`.

---

## Calidad de código

**11. `COL_ID_PILOTO` y `COL_PUNTOS` están definidos en dos headers**
Aparecen tanto en `resultado.h` como en `piloto.h`. Alcanza con definirlos en uno solo y que el otro lo incluya, eliminando la duplicación.

**12. `reduceAcumularPuntosCarrera` no hace nada**
Recibe los parámetros, los castea a `void` y retorna `TODO_OK`. La lógica real está en `acumularPuntosDesdeCarrera`. El parámetro `Reduce reducir` en `recalcularPuntosPilotos` y `actualizarPuntosUltimaCarrera` nunca se usa; debería unificarse o eliminarse.

**13. Dependencia de `piloto.h` está oculta en `carrera.c`**
`carrera.c` incluye `piloto.h` pero `carrera.h` no lo declara. Si otro módulo incluye solo `carrera.h` puede fallar en compilación. La dependencia debe ser explícita en el `.h`.

**14. `SEP_TXT` no se refleja en las funciones `trozar*`**
El separador está definido como `SEP_TXT` en `utilidades.h`, pero las funciones de parseo usan `strtok` con el literal `"|"` hardcodeado. Si se cambia `SEP_TXT`, el parseo no lo refleja. Hay que construir el string desde `SEP_TXT`:
```c
char sep[2] = {SEP_TXT, '\0'};
strtok(linea, sep);
```

---

## Lote de prueba sugerido

| # | Caso | Qué verificar |
|---|------|---------------|
| 1 | 0 pilotos activos | Todos en estado `R` o `S`, intentar carrera aleatoria |
| 2 | 1 solo piloto activo | Carrera aleatoria con un único participante |
| 3 | ID inexistente en manual | Ingresar ID `999` que no existe en `piloto.dat` |
| 4 | Mismo piloto dos veces | Ingresar el mismo ID dos veces en la misma carrera |
| 5 | Fecha inválida | Probar `20261340`, `0`, y letras |
| 6 | Piloto con DNF/DNS/DSQ | Verificar que aparece en resultados con 0 puntos |
| 7 | Carrera sin resultados | Terminar ingreso manual con ID `0` sin cargar ningún piloto |
| 8 | Input no numérico en menú | Ingresar `abc` en cualquier `scanf` numérico |
