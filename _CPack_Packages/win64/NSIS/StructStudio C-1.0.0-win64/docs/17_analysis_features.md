# 17_analysis_features.md

## 1. Proposito

Este documento explica la nueva capa de **analisis educativo** integrada en StructStudio C.

La idea no es solo "mostrar un boton mas", sino dejar claro:

- que algoritmos existen,
- donde viven en la arquitectura,
- como se ejecutan sin mezclar GUI con dominio,
- y por que se implementaron asi en C.

---

## 2. Que se agrego

Se incorporo un modulo nuevo en `core`:

- `src/core/api_analysis.c`

Ese modulo expone analisis de solo lectura sobre la estructura activa.

Hoy soporta:

- arboles: `Preorden`, `Inorden`, `Postorden`, `Por niveles`
- grafos: `BFS`, `DFS`
- grafos ponderados: `Dijkstra`
- grafos ponderados: `Floyd-Warshall`
- grafos no dirigidos ponderados: `Prim`, `Kruskal`

---

## 3. Como se usa desde la interfaz

La ventana principal ahora tiene un bloque `Analisis` en el panel izquierdo.

Su comportamiento es contextual:

- si la estructura activa no soporta analisis, el bloque se deshabilita
- si la estructura activa si los soporta, el combo muestra solo las opciones validas
- el campo `Origen` permite indicar un nodo por `ID`, etiqueta o valor
- el campo `Origen` se bloquea automaticamente cuando el algoritmo no lo usa

Resolucion del origen:

- si hay un nodo seleccionado, ese nodo tiene prioridad
- si no hay seleccion, se intenta resolver lo escrito en `Origen`
- en arboles, si no se indica nada, se usa la raiz actual
- en `Prim`, si no se indica nada, se usa el primer vertice disponible
- en `Floyd-Warshall` y `Kruskal`, el origen no aplica y la UI no lo solicita

El resultado se guarda como texto en el estado del editor y se refleja en la `Guia contextual`.

---

## 4. Donde vive cada responsabilidad

La implementacion sigue una separacion intencional:

- `core/api_analysis.c`: algoritmos y formateo del reporte
- `editor/editor.c`: decide como resolver el nodo origen y guarda el ultimo resultado
- `ui/main_window.c`: muestra controles, lanza la accion y enseña el reporte

Esta separacion es importante porque evita una mala practica comun:

> implementar BFS, DFS o Dijkstra directamente dentro de callbacks de botones

Si eso se hiciera, la GUI terminaria controlando reglas del dominio y el proyecto se volveria mas fragil.

---

## 5. Conceptos de C aplicados aqui

### 5.1 `enum` para tipos de analisis

Se creo `SsAnalysisKind` para representar cada analisis soportado.

Eso es mejor que usar strings sueltas porque:

- reduce errores,
- mejora legibilidad,
- y hace mas facil habilitar o deshabilitar opciones segun la variante activa.

### 5.2 Buffer de salida controlado

Los reportes de analisis se construyen en buffers con capacidad fija.

Aqui se usa:

- `SS_ANALYSIS_REPORT_CAPACITY`

Esto es una practica comun en C cuando quieres:

- evitar reservas innecesarias,
- mantener control explicito de memoria,
- y prevenir escrituras fuera del buffer.

### 5.3 Funciones auxiliares `static`

Dentro de `api_analysis.c` varias funciones son `static`.

Eso significa que:

- solo existen dentro de ese archivo,
- no contaminan el resto del proyecto,
- y dejan claro que son helpers internos del modulo.

---

## 6. Decision de diseño: reporte textual primero, playback guiado despues

La primera iteracion del modulo salio con reporte textual. Despues se agrego una segunda capa: playback guiado manual para los analisis donde el recorrido aporta valor didactico inmediato.

La razon tecnica para mantener ambas capas es simple:

- el reporte textual sigue siendo el resumen estable y exportable,
- el playback guiado mejora comprension sin convertir el canvas en una animacion automatica dificil de mantener,
- y el editor puede guardar el paso actual sin contaminar la API del dominio con widgets o timers.

La arquitectura final queda asi:

- `core` genera el resumen textual y, si aplica, la secuencia de pasos
- `editor` conserva el playback activo y decide que paso esta visible
- `render` solo resalta nodos/aristas segun ese estado
- `ui` ofrece botones y atajos para moverse por la secuencia

---

## 7. Pruebas automatizadas

Se agrego:

- `tests/analysis_smoke.c`
- `tests/graph_analysis_advanced_smoke.c`
- `tests/analysis_playback_smoke.c`

Ese smoke cubre dos casos representativos:

- recorrido `Inorden` sobre un `BST`
- `Dijkstra` sobre un grafo dirigido ponderado

Y el smoke avanzado cubre:

- `Floyd-Warshall` sobre un grafo dirigido ponderado
- `Prim` sobre un grafo no dirigido ponderado
- `Kruskal` sobre un grafo no dirigido ponderado

Esto no reemplaza una suite completa, pero si deja una red minima para detectar regresiones en los algoritmos y en su integracion con el editor.

---

## 8. Buenas practicas reforzadas

Con esta funcionalidad se reforzaron varias practicas sanas:

- desacoplar algoritmo, coordinacion y GUI
- evitar callbacks gigantes
- reutilizar el estado del editor en vez de repartir variables temporales
- probar primero en `core/editor` antes de depender de una verificacion manual de la ventana
- documentar el motivo de decisiones no obvias

---

## 9. Siguiente expansion natural

Las siguientes ampliaciones razonables, si se quieren seguir tomando ideas del proyecto de referencia, serian:

- recorridos o chequeos adicionales para heap y set
- deteccion de ciclos y componentes conexas
- una vista opcional de resaltado visual del resultado sin mezclar la logica del algoritmo con el render

La recomendacion tecnica es mantener el mismo criterio:

1. primero implementar el algoritmo en `core`
2. luego exponerlo mediante `editor`
3. despues integrarlo en la GUI
4. finalmente cubrirlo con pruebas y documentacion
