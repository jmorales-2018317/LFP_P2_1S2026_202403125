# Casos de prueba TaskScript

Este directorio contiene 10 archivos `.task` que cubren entradas válidas,
errores léxicos, errores sintácticos, casos con múltiples errores y casos
borde. Cada caso se documenta abajo con la entrada, el resultado esperado y
el resultado obtenido al ejecutarlo en la aplicación.

> Cómo reproducirlos: abrir `TaskScriptApp` → **Cargar archivo** → seleccionar
> el `.task` → **Analizar**. Comparar las pestañas **Tokens** y **Errores**
> con el resultado esperado. Para los casos válidos, pulsar **Generar reportes
> HTML** y **Generar árbol DOT** y verificar la salida en la carpeta
> `salida_taskscript/` (creada junto al ejecutable).

## Resumen

| # | Archivo | Tipo | Descripción | Errores esperados |
|---|---------|------|-------------|-------------------|
| 1 | `01_valido_completo.task` | Válido | Tablero con 3 columnas y 6 tareas. | 0 |
| 2 | `02_valido_minimo.task` | Válido | Tablero con 1 columna y 1 tarea. | 0 |
| 3 | `03_error_lexico_caracter.task` | Léxico | Carácter `$` dentro de un valor. | 1 léxico |
| 4 | `04_error_lexico_cadena_sin_cerrar.task` | Léxico | Cadena que no cierra antes de fin de línea. | 1 léxico CRITICO + sintácticos en cascada |
| 5 | `05_error_lexico_fecha_invalida.task` | Léxico | Fecha `2026-13-40` fuera de rango. | 1 léxico + 1 sintáctico |
| 6 | `06_error_sintactico_falta_dospuntos.task` | Sintáctico | Falta `:` después de `prioridad`. | 1 sintáctico |
| 7 | `07_error_sintactico_llave_sin_cerrar.task` | Sintáctico | Falta `}` al cerrar la columna. | >=1 sintáctico |
| 8 | `08_error_sintactico_atributo_desconocido.task` | Sintáctico | Atributo `etiqueta` no existe en la GLC. | >=1 sintáctico |
| 9 | `09_multiples_errores.task` | Mixto | Combina los casos 3, 4, 5 y 6. | >=10 errores |
| 10 | `10_caso_borde_columna_vacia.task` | Borde | Columna sin tareas + columna con 1 tarea. | 0 |

## Detalle por caso

### Caso 1 — Válido completo

- **Entrada:** [`01_valido_completo.task`](01_valido_completo.task) — el ejemplo
  del enunciado con 3 columnas (`Por Hacer`, `En Progreso`, `Completado`) y 6 tareas.
- **Esperado:** 0 errores. Tabla de tokens completa. Reportes HTML con todas
  las tareas y árbol DOT con la raíz `<programa>`.
- **Obtenido:** ✅ Sin errores. La GUI muestra los tokens en orden y los dos
  reportes se abren mostrando el tablero completo y la carga por responsable
  (Jorge: 2, Maria: 1, Carlos: 1, Ana: 1, Pedro: 1).

### Caso 2 — Válido mínimo

- **Entrada:** [`02_valido_minimo.task`](02_valido_minimo.task) — un tablero
  con una sola columna (`Backlog`) y una sola tarea, sin coma colgante.
- **Esperado:** 0 errores. Verifica que la gramática acepta listas de un
  único elemento sin coma final.
- **Obtenido:** ✅ Sin errores. Reportes generados correctamente.

### Caso 3 — Error léxico: carácter inválido

- **Entrada:** [`03_error_lexico_caracter.task`](03_error_lexico_caracter.task)
  — usa `$Marta` como responsable. El `$` no está en el alfabeto.
- **Esperado:** Un error léxico tipo `Caracter no reconocido '$'` en la
  línea/columna donde aparece. El análisis continúa.
- **Obtenido:** ✅ El analizador reporta el error léxico y el parser registra
  un error sintáctico secundario porque tras el `$` la cadena `Marta` se
  rechaza como identificador. Es el comportamiento documentado.

### Caso 4 — Error léxico: cadena sin cerrar

- **Entrada:** [`04_error_lexico_cadena_sin_cerrar.task`](04_error_lexico_cadena_sin_cerrar.task)
  — la tarea `"Tarea sin cerrar` nunca cierra la comilla; la siguiente comilla
  pertenece a otro literal.
- **Esperado:** Un error léxico **CRITICO** "Cadena no cerrada antes de fin
  de línea/archivo." y errores sintácticos en cascada.
- **Obtenido:** ✅ Se reporta el error CRITICO con la línea correcta y se
  continúa el análisis hasta el final.

### Caso 5 — Error léxico: fecha inválida

- **Entrada:** [`05_error_lexico_fecha_invalida.task`](05_error_lexico_fecha_invalida.task)
  — la fecha `2026-13-40` viola los rangos (mes 13, día 40).
- **Esperado:** Un error léxico "Fecha invalida '2026-13-40'..." y un error
  sintáctico secundario porque el parser esperaba un token `FECHA`.
- **Obtenido:** ✅ Errores reportados con posición exacta.

### Caso 6 — Error sintáctico: falta `:` después de `prioridad`

- **Entrada:** [`06_error_sintactico_falta_dospuntos.task`](06_error_sintactico_falta_dospuntos.task)
  — `prioridad ALTA` (sin los dos puntos).
- **Esperado:** Un error sintáctico "Se esperaba ':' después de 'prioridad',
  se encontro 'ALTA'." y el resto del archivo se analiza correctamente.
- **Obtenido:** ✅ El parser entra en modo pánico, sincroniza con `,` y
  continúa.

### Caso 7 — Error sintáctico: llave sin cerrar

- **Entrada:** [`07_error_sintactico_llave_sin_cerrar.task`](07_error_sintactico_llave_sin_cerrar.task)
  — falta `}` al cerrar la columna.
- **Esperado:** Errores sintácticos "Se esperaba '}'..." y "Se esperaba
  ';'..." con posición.
- **Obtenido:** ✅ El parser detecta los `}` faltantes y los reporta.

### Caso 8 — Error sintáctico: atributo desconocido

- **Entrada:** [`08_error_sintactico_atributo_desconocido.task`](08_error_sintactico_atributo_desconocido.task)
  — usa `etiqueta`, atributo no definido en la GLC.
- **Esperado:** Error sintáctico "Se esperaba un atributo
  (prioridad/responsable/fecha_limite)..." y un error léxico "Identificador
  'etiqueta' no reconocido por el lenguaje".
- **Obtenido:** ✅ Ambos errores reportados.

### Caso 9 — Múltiples errores

- **Entrada:** [`09_multiples_errores.task`](09_multiples_errores.task) —
  combina caracter inválido, fecha inválida, falta `:`, identificador no
  reconocido y cadena sin cerrar.
- **Esperado:** Más de 10 errores acumulados (léxicos + sintácticos) sin que
  el analizador se detenga.
- **Obtenido:** ✅ El analizador reporta 15 errores con números, lexema, tipo,
  descripción, línea, columna y gravedad.

### Caso 10 — Caso borde: columna vacía

- **Entrada:** [`10_caso_borde_columna_vacia.task`](10_caso_borde_columna_vacia.task)
  — primera columna sin tareas; segunda columna con una tarea.
- **Esperado:** 0 errores; la columna vacía debe aparecer en el reporte
  Kanban con un mensaje de "Sin tareas en esta columna".
- **Obtenido:** ✅ La GUI muestra ambos casos sin errores y el reporte HTML
  pinta la columna vacía con la nota correspondiente.
