## Guía de estudio del código

Esta guía complementa los comentarios embebidos en el código fuente. La idea no es solo decir "qué archivo existe", sino en qué orden conviene leerlo para entender mejor un sistema grande en C.

### 1. Orden recomendado de lectura

1. `include/core/model.h`
   Explica los tipos fundamentales: documento, estructura, nodo, arista y validación.

2. `include/core/api.h`
   Explica las operaciones semánticas que el sistema sabe ejecutar sobre el modelo.

3. `include/editor/editor.h`
   Explica el estado transitorio de interacción: selección, arrastre, paneo, playback y transiciones.

4. `src/core/model.c`
   Explica el catálogo de variantes y la gestión básica de memoria del modelo.

5. `src/core/*.c`
   Aquí viven las reglas de negocio reales por estructura, análisis, teoría y transformaciones.

6. `src/editor/editor.c`
   Coordina intenciones del usuario con la API del núcleo.

7. `src/editor/animation.c`
   Explica cómo se interpolan movimientos y playback sin mezclar animación con reglas de negocio.

8. `src/render/render.c`
   Explica cómo el estado actual se convierte en el lienzo visible y en la exportación PNG.

9. `src/ui/main_window.c`
   Explica cómo los widgets nativos de la ventana invocan acciones del editor.

10. `src/persistence/document_io.c`
    Explica cómo se guarda y carga el documento completo como JSON.

### 2. Idea arquitectónica central

La arquitectura sigue una separación práctica de responsabilidades:

- `core/`
  Aquí están los algoritmos, reglas del TDA, teoría, validación y transformaciones.

- `editor/`
  Aquí está el estado transitorio de interacción. Por ejemplo:
  selección actual, paneo, arrastre, playback y undo/redo de rotaciones.

- `render/`
  Aquí solo se dibuja. No decide semántica.

- `ui/`
  Aquí se conectan botones, campos y eventos del lienzo con funciones del editor.

- `persistence/`
  Aquí se serializa y deserializa el documento.

### 3. Cómo pensar este proyecto al estudiar C

En proyectos grandes en C, una dificultad común es que no existe encapsulación automática como en lenguajes orientados a objetos. Por eso aquí conviene pensar por capas:

- tipos compartidos
- contratos públicos
- implementación del núcleo
- coordinación del editor
- presentación visual

En otras palabras:

- primero entender las estructuras `struct`
- luego entender las funciones públicas
- y solo al final leer helpers privados

### 4. Patrones aplicados en el proyecto

#### 4.1. Modelo + editor + render

No es MVC puro de libro, pero sí una separación similar:

- el modelo representa datos persistentes
- el editor representa estado de interacción
- el render transforma ese estado en dibujo

#### 4.2. API semántica

La UI no manipula nodos y aristas “a mano” siempre que puede. En vez de eso llama funciones del núcleo como:

- insertar
- conectar
- validar
- analizar
- rotar

Eso reduce duplicación y facilita pruebas.

#### 4.3. Transiciones por snapshot

Las animaciones no recalculan la semántica. El sistema:

1. captura la geometría anterior
2. ejecuta la operación real
3. obtiene la geometría nueva
4. interpola entre ambas

Este patrón es útil porque desacopla:

- lógica del TDA
- presentación animada

#### 4.4. Historial específico en vez de undo universal

Por ahora el proyecto tiene historial de rotaciones, no un undo universal de todo.

Eso es una decisión de ingeniería válida porque:

- era una necesidad didáctica clara
- evita introducir una deuda técnica muy grande antes de tiempo

### 5. Dónde buscar según el tipo de duda

Si la duda es...

- "¿qué datos tiene una estructura?"
  leer `include/core/model.h`

- "¿qué operación ejecuta un botón?"
  leer `src/ui/main_window.c`

- "¿dónde está la lógica real de insertar/eliminar/rotar?"
  leer `src/core/` y `src/editor/editor.c`

- "¿por qué se anima un nodo?"
  leer `src/editor/animation.c`

- "¿por qué se dibuja así?"
  leer `src/render/render.c`

- "¿por qué al guardar aparece tal campo JSON?"
  leer `src/persistence/document_io.c`

### 6. Consejos prácticos para estudiar este código

- Leer primero comentarios de archivo y de tipo.
- Seguir una sola operación de punta a punta, por ejemplo `Insertar`.
- No intentar comprender todo `main_window.c` de una sola vez.
- Empezar por las funciones públicas del editor.
- Usar el nombre de un botón o acción como hilo conductor.

### 7. Ejemplo de ruta de ejecución: insertar desde la UI

1. El usuario pulsa `Insertar` en la ventana.
2. `src/ui/main_window.c` recoge los parámetros visibles.
3. La UI llama al editor.
4. `src/editor/editor.c` prepara la operación y captura layout si hace falta.
5. El editor invoca la API semántica del núcleo.
6. `src/core/` modifica la estructura.
7. El editor actualiza validación, estado y transición.
8. `src/render/render.c` dibuja la nueva escena.

### 8. Idea final

Para estudiar bien este proyecto, no conviene memorizar funciones aisladas. Conviene entender qué responsabilidad tiene cada capa y por qué esa capa existe.

Ese enfoque reduce mucho la dificultad inicial de C en proyectos medianos o grandes.
