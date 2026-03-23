# Recorridos Guiados Paso a Paso

Este documento explica la nueva capa de `playback` educativo integrada en StructStudio C.

## 1. Objetivo

El reporte textual de un analisis sigue siendo util, pero no siempre basta para un estudiante que quiere ver:

- que nodo se visita primero,
- por que arista se llega al siguiente,
- cuando una distancia mejora en `Dijkstra`,
- y como avanza el algoritmo sin tener que imaginarlo mentalmente.

Por eso se agrego un modo de recorrido guiado manual.

Despues se extendio con una capa opcional de reproduccion automatica y animacion temporal para que el paso visible no quede como un simple cambio instantaneo de color.

## 2. Analisis que hoy soportan playback

- `Preorden`
- `Inorden`
- `Postorden`
- `Por niveles`
- `BFS`
- `DFS`
- `Dijkstra`

Analisis como `Floyd-Warshall`, `Prim` y `Kruskal` siguen disponibles como resumen textual, pero no exponen playback guiado todavia.

## 3. Como se usa desde la interfaz

1. selecciona la estructura activa
2. escoge el analisis en el bloque `Analisis`
3. si el algoritmo requiere origen, selecciona el nodo o escribe su `ID`, etiqueta o valor
4. pulsa `Preparar ...`
5. usa `Auto-reproducir`, `Paso anterior`, `Paso siguiente`, `Reiniciar` o las flechas izquierda/derecha sobre el canvas

Durante el recorrido:

- el canvas resalta el paso actual con un acento mas intenso,
- le aplica un pulso breve al nodo o arista actuales,
- deja una huella suave en nodos o aristas ya recorridos,
- puede avanzar solo cuando `Auto-reproducir` esta activo,
- y la `Guia contextual` muestra el mensaje del paso visible.

## 4. Arquitectura aplicada

La implementacion sigue una separacion intencional de responsabilidades:

- `src/core/api_analysis.c`
  - genera el resumen textual estable del analisis.
- `src/core/api_analysis_playback.c`
  - construye la secuencia de pasos para los algoritmos que vale la pena mostrar.
- `src/editor/editor.c`
  - conserva el playback activo, el paso actual y las transiciones `anterior/siguiente/reiniciar`.
- `src/editor/animation.c`
  - administra la reproduccion automatica, el temporizador y el pulso visual del paso actual.
- `src/render/render.c`
  - no ejecuta algoritmos; solo pregunta al editor que nodos o aristas debe resaltar y como interpolar el estado visible.
- `src/ui/main_window.c`
  - conecta botones, atajos y guia contextual con el estado del editor.

Esta estrategia evita un mal olor muy comun en software educativo:

> esconder la logica del algoritmo dentro del callback del boton o dentro del render.

## 5. Buenas practicas de C aplicadas aqui

### 5.1 `struct` para estado transitorio

Se agrego `SsAnalysisPlayback` dentro de `SsEditorState`.

Eso sirve para agrupar:

- tipo de analisis,
- nodo de origen efectivo,
- arreglo dinamico de pasos,
- cantidad de pasos,
- paso actual.

En C esto es mejor que repartir varias variables globales sueltas.

### 5.2 Ownership de memoria explicito

Los pasos del playback se reservan en `core` y luego pasan a ser responsabilidad del `editor`.

Cuando:

- cambia la estructura activa,
- se edita la estructura,
- se carga otro documento,
- o el usuario cambia de analisis,

el editor libera ese bloque y reinicia el playback.

Ese ownership claro evita fugas y estados colgantes.

### 5.3 Desacoplamiento por capas

El algoritmo no sabe nada de:

- botones,
- `uiArea`,
- colores,
- fuentes,
- ni eventos de mouse.

Y el render no sabe nada de:

- colas de BFS,
- recursion de DFS,
- ni relajaciones de `Dijkstra`.

Cada capa conserva una responsabilidad defendible.

## 6. Criterio UX

El playback sigue priorizando control didactico manual, pero ahora admite un autoplay opcional.

La idea es combinar dos estilos de estudio:

- manual, para quien quiere detenerse en cada paso;
- automatico, para quien primero quiere ver el recorrido completo en movimiento y luego volver a pausarlo.

La barra inferior se mantiene compacta para no repetir seleccion, validacion y mensaje en tres zonas distintas.

## 7. Prueba automatizada relacionada

Se agrego `tests/analysis_playback_smoke.c`.

Ese smoke verifica al menos:

- playback de `Inorden` sobre un `BST`,
- avance de pasos y consulta de estado visitado/actual,
- preparacion de `Dijkstra` paso a paso,
- y un autoplay basico validado por `tests/animation_smoke.c`.

## 8. Expansion natural

Las siguientes mejoras razonables son:

- playback guiado para `Prim` y `Kruskal`,
- deteccion visual de componentes y ciclos en grafos,
- y una leyenda opcional del tipo de paso actual (`visita`, `descubrimiento`, `relajacion`, `cierre`).
