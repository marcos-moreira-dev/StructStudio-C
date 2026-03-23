# 08_rendering.md

## 1. Propósito del documento

Este documento define las reglas de **renderizado 2D** de **StructStudio C** para la primera versión (V1).

Su objetivo es describir:

- cómo se representan visualmente las estructuras de datos,
- cómo deben dibujarse nodos, aristas, etiquetas y estados,
- qué criterios de legibilidad y organización visual se deben respetar,
- y cómo se relaciona el render con el modelo y con el editor.

Este documento no describe detalles internos de la librería gráfica ni implementación de bajo nivel. Su función es servir como contrato visual entre el modelo de datos, el editor y la interfaz.

---

## 2. Objetivo del render

El render de StructStudio C debe producir una representación visual que sea:

- clara,
- técnica,
- consistente,
- educativa,
- y compatible con una estética de software de escritorio clásico.

La representación debe ayudar a entender la estructura, no a distraer. El usuario debe poder distinguir rápidamente:

- qué entidades existen,
- cómo se relacionan,
- qué está seleccionado,
- qué tiene error,
- y qué tipo de estructura está viendo.

---

## 3. Principios generales de renderizado

### 3.1 Claridad antes que adorno
La prioridad del render es la legibilidad estructural.

### 3.2 Coherencia visual
Elementos equivalentes deben verse de forma consistente dentro del mismo tipo de estructura.

### 3.3 Diferenciación por estado
Selección, hover, error y otros estados deben verse claramente.

### 3.4 Separación entre lógica y dibujo
El render refleja el estado del modelo, pero no define sus reglas.

### 3.5 Soporte educativo
La visualización debe reforzar los conceptos de estructuras de datos.

### 3.6 Estética clásica
La representación debe convivir con una interfaz inspirada en programas de escritorio tradicionales.

---

## 4. Entradas del subsistema de render

El render deberá basarse, como mínimo, en estas entradas:

- documento activo,
- estructura activa,
- nodos y aristas de la estructura,
- estado visual de cada entidad,
- selección actual,
- validaciones activas,
- configuración visual básica,
- área visible del canvas.

---

## 5. Responsabilidad del render

El render deberá encargarse de:

- dibujar el fondo del canvas,
- dibujar grilla si está habilitada,
- dibujar nodos,
- dibujar celdas,
- dibujar aristas y flechas,
- dibujar pesos y etiquetas,
- dibujar resaltados,
- dibujar indicadores contextuales,
- y presentar el estado de error o advertencia cuando corresponda.

El render no debe:

- tomar decisiones de dominio,
- modificar relaciones lógicas por iniciativa propia,
- guardar archivos,
- reemplazar las validaciones estructurales.

---

## 6. Fondo del canvas

### 6.1 Objetivo
El fondo debe facilitar la lectura y no competir con las estructuras.

### 6.2 Reglas
- color sobrio y claro o ligeramente neutro,
- sin texturas complejas,
- sin ruido visual,
- con suficiente contraste frente a nodos y líneas.

### 6.3 Grilla opcional
La grilla puede existir como ayuda visual, pero debe ser discreta.

#### Características deseadas
- líneas finas,
- bajo contraste,
- utilidad para alineación,
- sin distraer del contenido principal.

---

## 7. Modelo visual general

Toda estructura visible se compone de elementos gráficos básicos:

- rectángulos,
- celdas,
- nodos,
- bordes,
- líneas,
- flechas,
- etiquetas,
- resaltados,
- marcadores de estado.

Estos elementos se combinan según el tipo de estructura.

---

## 8. Representación base de nodo

### 8.1 Definición visual base
Un nodo debe verse como una unidad gráfica claramente separada del resto.

### 8.2 Forma base sugerida
- rectángulo o caja con borde definido,
- fondo claro,
- texto centrado o bien alineado,
- tamaño suficiente para ser legible.

### 8.3 Componentes mínimos del nodo
- contorno,
- área de contenido,
- texto principal,
- resaltado de selección si aplica.

### 8.4 Requisitos visuales
- el texto no debe desbordarse sin control,
- el borde debe ser visible,
- debe ser fácil identificar el nodo como entidad seleccionable.

---

## 9. Representación base de arista

### 9.1 Definición visual base
Una arista debe comunicar claramente una relación entre dos entidades.

### 9.2 Forma base sugerida
- línea recta o suavemente curva cuando sea necesario,
- grosor moderado,
- contraste suficiente con el fondo.

### 9.3 Componentes opcionales
- punta de flecha,
- etiqueta,
- peso,
- resaltado de selección.

### 9.4 Requisitos visuales
- debe conectar visualmente las entidades correctas,
- no debe ocultar innecesariamente etiquetas,
- debe permitir identificar dirección si aplica.

---

## 10. Etiquetas y texto

### 10.1 Tipos de texto visibles
- valor del nodo,
- etiqueta del nodo,
- índice,
- peso,
- clave,
- valor de map,
- marcadores como `top`, `front`, `rear`, `root`.

### 10.2 Reglas generales
- texto legible,
- tamaño consistente,
- no abusar de múltiples tamaños distintos,
- contraste suficiente,
- evitar superposición con bordes o flechas.

### 10.3 Tratamiento de textos largos
- truncado visual moderado o ajuste controlado,
- tooltip futuro opcional, pero no necesario en V1,
- el render debe preservar claridad antes que mostrar texto excesivo.

---

## 11. Estados visuales comunes

El render debe contemplar al menos estos estados:

### 11.1 Estado normal
Elemento visible sin interacción activa.

### 11.2 Hover
Elemento bajo el puntero.

### 11.3 Seleccionado
Elemento actualmente seleccionado.

### 11.4 Resaltado contextual
Elemento destacado por búsqueda, validación o consulta.

### 11.5 Error o inconsistencia
Elemento relacionado con una violación estructural.

### 11.6 Deshabilitado o no aplicable
Elemento que no debe percibirse como activo si aparece en una operación contextual.

---

## 12. Convenciones visuales por estado

### 12.1 Normal
- borde estándar,
- fondo estándar,
- texto normal.

### 12.2 Hover
- leve realce de borde o fondo,
- sin exageración.

### 12.3 Seleccionado
- borde más visible,
- color de realce sobrio,
- posible sombreado o marco adicional.

### 12.4 Error
- realce claro pero no escandaloso,
- puede usar borde o fondo con advertencia,
- la legibilidad del contenido debe mantenerse.

### 12.5 Resultado de búsqueda
- resaltado temporal moderado,
- visible sin confundirse con error.

---

## 13. Layout general

### 13.1 Principio
El layout debe organizar los elementos para que la estructura se entienda visualmente.

### 13.2 Fuentes del layout
El sistema puede combinar:

- posiciones manuales,
- auto-layout por tipo,
- pequeños reajustes visuales.

### 13.3 Regla de convivencia
El auto-layout no debe destruir innecesariamente la edición manual del usuario sin su consentimiento explícito.

---

## 14. Reglas visuales del vector

### 14.1 Forma general
- secuencia horizontal,
- celdas alineadas,
- estructura lineal y ordenada.

### 14.2 Elementos visibles
- contenedor opcional del vector,
- celdas rectangulares,
- valor dentro de cada celda,
- índice encima o debajo si se decide mostrarlo.

### 14.3 Criterios de legibilidad
- separación uniforme,
- índices claros,
- orden de izquierda a derecha.

---

## 15. Reglas visuales de listas enlazadas

### 15.1 Lista simple
- nodos en línea,
- flechas `next` visibles,
- lectura de izquierda a derecha o en una orientación consistente.

### 15.2 Lista doble
- nodos en línea,
- relaciones `prev` y `next` claramente distinguibles,
- posibilidad de usar dos flechas o una convención visual estable.

### 15.3 Lista circular simple y doble
- debe existir una indicación de cierre circular,
- la circularidad debe verse explícitamente,
- se debe evitar ambigüedad sobre dónde cierra el ciclo.

### 15.4 Criterios comunes
- nodos equidistantes o casi equidistantes,
- relaciones claras,
- cabeza y cola distinguibles cuando sea útil.

---

## 16. Reglas visuales del stack

### 16.1 Forma general
- disposición vertical,
- elementos apilados.

### 16.2 Elementos visibles
- cajas apiladas,
- marcador de `top`,
- valor en cada elemento.

### 16.3 Criterios de legibilidad
- el tope debe distinguirse con claridad,
- la dirección de crecimiento debe entenderse visualmente.

---

## 17. Reglas visuales de queue

### 17.1 Queue clásica
- disposición lineal,
- frente y final marcados.

### 17.2 Circular queue
- continuidad circular indicada,
- frente y final distinguidos,
- el cierre debe ser claro sin hacer el diagrama confuso.

### 17.3 Priority queue
- prioridad visible,
- el orden lógico debe ser comprensible,
- el elemento dominante debe destacarse cuando convenga.

---

## 18. Reglas visuales de árboles

### 18.1 Forma general
- raíz arriba,
- hijos debajo,
- jerarquía vertical.

### 18.2 Elementos visibles
- nodos,
- líneas padre-hijo,
- posible marcador de `root`.

### 18.3 Criterios de legibilidad
- separación horizontal suficiente entre ramas,
- ramas sin cruces innecesarios,
- hijo izquierdo y derecho claramente distinguibles.

### 18.4 BST y AVL
- se dibujan como árboles binarios,
- si hay indicadores de balance o altura, deben ser discretos,
- el árbol resultante tras operaciones debe verse ordenado.

---

## 19. Reglas visuales del heap

### 19.1 Forma general
- árbol casi completo,
- raíz arriba,
- niveles claramente alineados.

### 19.2 Elementos visibles
- nodos,
- líneas padre-hijo,
- posible marca visual del elemento principal.

### 19.3 Criterios de legibilidad
- jerarquía clara,
- estructura compacta,
- lectura sencilla de niveles.

---

## 20. Reglas visuales del set

### 20.1 Forma general
- colección de elementos únicos,
- sin sugerir necesariamente una secuencia estricta.

### 20.2 Elementos visibles
- nodos o celdas,
- posible agrupación visual simple.

### 20.3 Criterios de legibilidad
- elementos bien separados,
- ausencia de duplicados visualmente evidente,
- composición limpia.

---

## 21. Reglas visuales del map

### 21.1 Forma general
- pares clave-valor visibles,
- presentación técnica y clara.

### 21.2 Elementos visibles
- fila o tarjeta por entrada,
- zona de clave,
- zona de valor,
- separación entre ambas.

### 21.3 Criterios de legibilidad
- clave y valor no deben confundirse,
- el formato debe invitar a lectura rápida,
- claves repetidas deben poder resaltarse como error si ocurrieran.

---

## 22. Reglas visuales del graph

### 22.1 Forma general
- vértices distribuidos libremente en 2D,
- aristas conectando vértices.

### 22.2 Elementos visibles
- vértices,
- aristas,
- flechas si es dirigido,
- pesos si es ponderado,
- etiquetas opcionales.

### 22.3 Criterios de legibilidad
- minimizar cruces innecesarios cuando sea posible,
- mantener pesos cercanos a su arista,
- asegurar que la dirección se lea con claridad,
- permitir mover vértices manualmente.

### 22.4 Grafos dirigidos
- deben usar flechas visibles,
- la orientación no debe ser ambigua.

### 22.5 Grafos no dirigidos
- no deben usar flechas,
- la conexión debe verse neutra.

### 22.6 Grafos ponderados
- el peso debe verse junto a la arista,
- sin tapar nodos ni líneas importantes.

---

## 23. Elementos de apoyo visual

El render puede mostrar algunos marcadores funcionales:

- `root` en árboles,
- `top` en stack,
- `front` y `rear` en queue,
- `head` y `tail` en listas cuando sea útil,
- índice en vectores,
- peso en grafos,
- clave en maps.

Estos marcadores deben ser discretos y útiles.

---

## 24. Selección y foco

### 24.1 Nodo seleccionado
Debe tener un borde o realce claramente visible.

### 24.2 Arista seleccionada
Debe poder distinguirse del resto sin perder legibilidad.

### 24.3 Estructura activa
La estructura en edición debe percibirse como el foco principal del canvas.

### 24.4 Resultado de operación
El render puede resaltar brevemente entidades afectadas por una operación reciente si el editor lo solicita.

---

## 25. Visualización de errores

### 25.1 Objetivo
Ayudar al usuario a detectar rápidamente el problema sin destruir la legibilidad del diagrama.

### 25.2 Posibles casos
- duplicado en set,
- clave repetida en map,
- nodo con demasiados hijos,
- relación inválida,
- peso faltante en grafo ponderado,
- violación BST,
- inconsistencia de circularidad.

### 25.3 Reglas visuales
- resaltar la entidad problemática,
- opcionalmente resaltar la relación problemática,
- no cubrir el contenido principal con mensajes gigantes,
- usar la barra de estado o panel derecho para el detalle textual.

---

## 26. Reglas de espaciado

### 26.1 Separación mínima
Debe existir separación suficiente entre entidades para evitar confusión visual.

### 26.2 Relación con el tipo de estructura
- vectores: separación uniforme,
- listas: distancia suficiente para flechas,
- árboles: separación creciente según profundidad si hace falta,
- grafos: distancia suficiente para arrastre y lectura.

### 26.3 Principio
El espaciado debe ayudar a entender la estructura, no solo a decorarla.

---

## 27. Reglas de tamaño

### 27.1 Tamaño de nodo
Debe permitir:
- leer el texto,
- seleccionar el elemento con comodidad,
- y distinguirlo del resto.

### 27.2 Consistencia
No conviene tener tamaños radicalmente distintos sin motivo.

### 27.3 Excepciones razonables
- maps pueden requerir nodos más anchos,
- pesos o etiquetas pueden ser más pequeños,
- árboles muy anchos pueden requerir compactación moderada.

---

## 28. Reglas de color

### 28.1 Enfoque general
Paleta sobria, técnica y compatible con estética clásica.

### 28.2 Usos del color
El color debe servir para:
- diferenciar estado,
- remarcar selección,
- indicar error,
- distinguir tipos secundarios cuando haga falta.

### 28.3 Lo que debe evitarse
- colores chillones sin motivo,
- saturación excesiva,
- demasiados códigos de color simultáneos.

---

## 29. Reglas de exportación visual

### 29.1 Objetivo
La exportación a PNG debe preservar la claridad del documento visible.

### 29.2 Requisitos mínimos
- el contenido debe verse nítido,
- no deben perderse etiquetas importantes,
- selección y errores deben exportarse según la política elegida,
- el resultado debe ser útil para estudio o presentación.

### 29.3 Política recomendada para V1
Exportar la estructura tal como se ve en el canvas, con una limpieza razonable.

---

## 30. Relación entre render y auto-layout

### 30.1 Distinción conceptual
- el **layout** decide dónde va cada entidad,
- el **render** decide cómo se dibuja.

### 30.2 Regla importante
El render no debe inventar posiciones lógicas. Debe usar las ya calculadas por el modelo o el editor.

---

## 31. Relación entre render y selección

El render debe poder leer el estado de selección sin convertirse en el dueño de esa lógica.

### Principio
- el editor decide la selección,
- el render solo la representa.

---

## 32. Relación entre render y estructura activa

La representación visual puede depender del tipo activo.

### Ejemplos
- `graph` requiere aristas libres y pesos,
- `stack` requiere disposición vertical,
- `vector` requiere celdas indexadas,
- `map` requiere pares clave-valor.

### Implicación
Debe existir soporte para render especializado por familia o variante.

---

## 33. Elementos que no entran en V1

La V1 no necesita incluir:

- animaciones paso a paso,
- efectos visuales complejos,
- transiciones sofisticadas,
- sombreado decorativo innecesario,
- zoom avanzado si aún no es indispensable,
- mini mapas,
- modo presentación avanzado.

---

## 34. Resultado esperado del subsistema de render

Al finalizar la V1, el render debe permitir:

- visualizar todas las estructuras contempladas,
- distinguir claramente nodos, relaciones y estados,
- reflejar operaciones del editor,
- y generar una imagen PNG clara y útil.

---

## 35. Relación con otros documentos

Este archivo se complementa con:

- `02_ui_ux.md` → distribución de la interfaz y experiencia visual general,
- `03_architecture.md` → separación entre render, editor y core,
- `04_data_model.md` → entidades visuales y lógicas,
- `05_structure_definitions.md` → reglas por estructura,
- `06_operations.md` → operaciones que modifican la representación,
- `09_editor_logic.md` → interacción del editor sobre el canvas.
