# Sistema de Animacion del Canvas

Este documento resume la capa de animacion agregada para que StructStudio C no solo muestre el estado final, sino tambien el **proceso visual** de una operacion o de un recorrido.

## 1. Que problemas resuelve

Antes el usuario podia notar dos limitaciones:

- un recorrido paso a paso cambiaba el resaltado de forma muy brusca,
- y varias operaciones reacomodaban nodos de golpe, sin dejar claro que habia pasado en medio.

Eso afectaba especialmente:

- rotaciones de `AVL`,
- rebuilds de `heap`,
- auto-layouts,
- inserciones o eliminaciones con corrimiento visual,
- y recorridos como `BFS`, `DFS` o `Dijkstra`.

## 2. Estrategia aplicada

La solucion no se implemento metiendo logica temporal dentro del render.

Se separo en tres responsabilidades:

- `src/editor/editor.c`
  - decide cuando una accion semantica cambia realmente la estructura.
- `src/editor/animation.c`
  - captura posiciones previas, calcula transiciones, administra autoplay y ticking temporal.
- `src/render/render.c`
  - consulta la posicion visible interpolada de cada nodo y dibuja el pulso del paso actual.

Esta separacion evita un mal olor comun:

> mezclar reglas del dominio, temporizadores y dibujo en una sola funcion enorme.

## 3. Dos tipos de animacion

### 3.1 Transicion de layout

Cuando una operacion modifica posiciones finales de nodos:

- se toma un snapshot previo,
- se ejecuta la mutacion real,
- se calcula el layout final,
- y el canvas interpola entre ambos estados durante unos milisegundos.

Eso hace visibles:

- corrimientos en estructuras lineales,
- reacomodos de arboles,
- rotaciones y rebuilds,
- y cambios por `Auto-layout`.

### 3.2 Pulso de playback

Cuando existe un recorrido guiado preparado:

- el paso actual se dibuja con color mas fuerte,
- y, al cambiar de paso, recibe un pulso breve.

Si el usuario activa `Auto-reproducir`, un temporizador avanza solo el playback y reinicia ese pulso en cada nuevo paso.

## 4. Detalles tecnicos utiles para aprender C

### 4.1 Snapshot explicito

En vez de intentar reconstruir posiciones previas despues de mutar la estructura, se usa un snapshot previo:

- `SsLayoutSnapshot`

Eso es una buena practica porque hace explicito el estado anterior que necesita la animacion.

### 4.2 Estado transitorio fuera del modelo

El documento y las estructuras no guardan tiempos ni banderas de animacion.

Todo eso vive en `SsEditorState`, dentro de:

- `SsLayoutTransition`
- `SsPlaybackFx`

Asi el JSON y el dominio siguen limpios y deterministas.

### 4.3 Temporizador pequeño y desacoplado

La UI registra un `uiTimer(16, ...)` para avanzar la capa temporal.

El callback no ejecuta algoritmos ni muta la estructura por su cuenta.

Solo:

- avanza interpolaciones,
- refresca el canvas,
- o mueve el playback si el autoplay esta activo.

## 5. Cobertura automatica

Se agrego `tests/animation_smoke.c`.

Ese smoke verifica:

- que cargar un ejemplo puede activar una transicion de layout,
- que la posicion visible interpolada difiere del estado final mientras dura la animacion,
- y que el autoplay del playback realmente avanza pasos con el tiempo.

## 6. Limites actuales

La capa actual prioriza claridad y mantenibilidad.

Todavia no anima de forma especializada:

- desaparicion gradual de nodos eliminados,
- aparicion gradual de aristas nuevas,
- ni playback guiado para `Prim`, `Kruskal` o `Floyd-Warshall`.

Pero la base ya quedo preparada para crecer sin volver a mezclar editor, temporizador y render.
