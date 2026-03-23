# 25_system_capabilities_guide.md

## 1. Propósito

Esta guía describe, de forma explícita y orientada al usuario, qué puede hacer hoy StructStudio C.

No es un documento de arquitectura ni de formato interno. Su objetivo es responder preguntas prácticas como:

- qué TDAs soporta el sistema,
- qué puede editarse visualmente,
- qué puede analizarse,
- qué se puede guardar, exportar o derivar,
- y qué operaciones son automáticas o guiadas.

---

## 2. Qué es StructStudio C

StructStudio C es una aplicación de escritorio para:

- construir estructuras de datos,
- manipularlas visualmente,
- ejecutar operaciones semánticas,
- estudiar recorridos y algoritmos,
- y consultar teoría contextual sin salir de la misma ventana.

El sistema combina dos modos de trabajo:

- modo editor: crear, mover, conectar, borrar y reorganizar estructuras,
- modo didáctico: analizar, simular, reproducir y comparar resultados derivados.

---

## 3. Familias de estructuras soportadas

El sistema soporta, en distinta combinación de edición, análisis y teoría:

- Vector
- Lista simple
- Lista doble
- Lista circular
- Stack
- Queue
- Priority Queue
- Árbol binario
- BST
- AVL
- Heap
- Set
- Map
- Grafo dirigido
- Grafo no dirigido
- Grafo dirigido ponderado
- Grafo no dirigido ponderado

---

## 4. Capacidades generales del documento

### 4.1 Documento con múltiples pestañas

El documento puede contener varias estructuras al mismo tiempo.

Eso permite:

- comparar TDAs distintos,
- mantener una estructura base y otra derivada,
- y trabajar con varias vistas conceptuales en paralelo.

### 4.2 Pestañas de trabajo

Cada pestaña representa una estructura activa dentro del mismo documento.

Desde la UI puedes:

- crear una nueva pestaña,
- reiniciar la pestaña actual con otra variante,
- y cargar un ejemplo didáctico en la pestaña visible.

### 4.3 Guardado y carga

El documento completo puede:

- guardarse en JSON,
- volver a abrirse,
- y exportarse como imagen PNG.

---

## 5. Capacidades visuales del lienzo

El lienzo central es el área principal de trabajo.

Permite:

- seleccionar nodos,
- seleccionar aristas,
- arrastrar nodos cuando la familia lo admite,
- conectar entidades con previsualización,
- borrar elementos,
- recentrar la vista,
- mostrar u ocultar grilla,
- y ver resaltados temporales de simulación.

### 5.1 Paneo

La vista puede desplazarse sobre el lienzo mediante paneo.

### 5.2 Auto-layout

Cuando una estructura lo necesita, el sistema recompone automáticamente la disposición visual.

Esto se usa especialmente en:

- árboles,
- heaps,
- estructuras derivadas,
- y cambios semánticos que alteran la forma global.

### 5.3 Transiciones animadas

Los cambios de layout no aparecen como salto instantáneo.

El sistema interpola:

- inserciones,
- borrados,
- auto-layout,
- carga de ejemplos,
- rotaciones,
- y reconstrucciones derivadas.

---

## 6. Herramientas del panel izquierdo

El bloque `Herramientas` controla el modo de interacción con el lienzo.

Incluye:

- `Seleccionar`
- `Insertar`
- `Conectar`
- `Eliminar`
- `Rot. izq` o `Girar izq`
- `Rot. der` o `Girar der`
- `Deshacer`
- `Rehacer`
- `Mostrar grilla`
- `Recentrar vista`

### 6.1 Diferencia entre rotar árbol y girar grafo

En árboles balanceables o árboles binarios:

- `Rot. izq` y `Rot. der` aplican una transformación estructural real.

En grafos:

- `Girar izq` y `Girar der` rotan la geometría visual del grafo en el canvas.

### 6.2 Deshacer y rehacer

El sistema incluye historial de rotaciones para:

- deshacer rotaciones estructurales de árbol,
- rehacerlas,
- deshacer giros visuales de grafos,
- y rehacerlos.

---

## 7. Bloque de operaciones

La sección `Operaciones` adapta su interfaz al TDA activo.

Eso significa que el sistema:

- cambia las etiquetas de entrada,
- oculta campos innecesarios,
- habilita solo los botones válidos,
- y muestra una guía corta de lo que espera.

### 7.1 Tipos de acciones

Según la estructura, el bloque puede ejecutar:

- insertar,
- buscar,
- eliminar,
- reemplazar,
- validar,
- auto-layout,
- agregar hijo izquierdo,
- agregar hijo derecho,
- crear arista,
- eliminar arista,
- entre otras.

### 7.2 Parámetros

El sistema usa tres tipos de entrada:

- principal,
- secundaria,
- numérica.

Pero no siempre muestra las tres.

Ejemplos:

- en grafos ponderados, el parámetro numérico representa el peso,
- en BST/AVL puede representar un valor entero,
- en vectores puede representar índice o índice alterno según la operación.

---

## 8. Operaciones visuales por familia

### 8.1 Estructuras lineales

En listas, stack y queue puedes:

- insertar elementos,
- eliminar,
- validar,
- cargar ejemplos,
- y ver la representación lineal correspondiente.

### 8.2 Árboles

En árboles puedes:

- crear raíz,
- insertar nodos,
- conectar padres e hijos cuando la variante lo permite,
- rotar izquierda o derecha,
- validar la estructura,
- ejecutar auto-layout,
- y simular recorridos.

### 8.3 BST y AVL

En BST y AVL el sistema permite además:

- búsquedas,
- rotaciones didácticas,
- rebalanceos visibles cuando el algoritmo lo exige,
- y explicación contextual desde teoría.

### 8.4 Grafos

En grafos puedes:

- agregar vértices,
- conectar con aristas,
- usar pesos en variantes ponderadas,
- seleccionar nodos origen,
- generar árboles derivados desde algoritmos,
- y girar la geometría del dibujo para estudiar la disposición visual.

---

## 9. Análisis y algoritmos

La sección `Análisis` permite ejecutar algoritmos sin salir del documento.

Actualmente soporta:

- preorden,
- inorden,
- postorden,
- por niveles,
- BFS,
- DFS,
- Dijkstra,
- Floyd-Warshall,
- Prim,
- Kruskal.

### 9.1 Resumen textual

`Resumen` ejecuta el algoritmo y deja un reporte textual.

Úsalo cuando:

- no necesitas animación,
- o quieres revisar solo el resultado final.

### 9.2 Simulación paso a paso

`Simular` prepara el playback guiado y ahora inicia la reproducción automática si el algoritmo genera pasos.

La simulación puede:

- resaltar nodos,
- resaltar aristas,
- mostrar el paso actual,
- dejar huella de lo ya recorrido,
- y avanzar automáticamente.

### 9.3 Control del playback

Durante una simulación puedes:

- pausar o reactivar autoplay,
- retroceder,
- avanzar,
- reiniciar,
- usar teclas izquierda/derecha,
- y usar `Space` para alternar la reproducción automática.

### 9.4 Árbol derivado desde grafos

Cuando el algoritmo lo permite, el sistema puede generar una estructura derivada.

Esa estructura:

- se abre como nueva pestaña,
- conserva el documento actual,
- y sirve para comparar origen y resultado.

---

## 10. Teoría contextual

El panel derecho combina propiedades y teoría.

La parte de teoría explica:

- qué es el TDA activo,
- a qué familia pertenece,
- qué variantes relacionadas existen,
- qué algoritmos o recorridos aplican,
- y qué significado didáctico tiene el análisis actual.

No es una ayuda modal aislada: forma parte de la vista principal para estudiar mientras editas.

---

## 11. Propiedades editables

Cuando seleccionas un nodo o una arista, el panel derecho permite modificar:

- ID técnico cuando aplica como lectura,
- etiqueta,
- valor,
- campo extra,
- valor numérico,
- y propiedades específicas de aristas o nodos según el caso.

---

## 12. Capacidades didácticas

StructStudio C no solo dibuja estructuras; también sirve para enseñar.

Las funciones más didácticas hoy son:

- ejemplos precargados por variante,
- teoría contextual persistente,
- pasos guiados,
- autoplay en simulaciones,
- rotaciones con feedback textual,
- árboles derivados desde grafos,
- y resaltado visual del paso actual.

---

## 13. Limitaciones actuales importantes

El sistema ya es funcional, pero todavía existen límites que conviene conocer:

- no todos los algoritmos avanzados tienen el mismo nivel de playback visual,
- la UI sigue dependiendo de controles nativos de libui-ng,
- el panel izquierdo todavía requiere trabajo adicional si se quisiera un contenedor con scroll nativo completo,
- y el rendimiento visual depende de cuánto trabajo de sincronización se haga en el hilo principal de UI.

---

## 14. Lectura recomendada

Para complementar esta guía:

- `02_ui_ux.md`
- `03_architecture.md`
- `06_operations.md`
- `17_analysis_features.md`
- `19_guided_playback.md`
- `22_animation_system.md`
- `26_technical_debt_audit.md`
