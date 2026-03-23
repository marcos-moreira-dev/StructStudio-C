# 05_structure_definitions.md

## 1. Propósito del documento

Este documento define, para la V1 de **StructStudio C**, las reglas específicas de cada estructura de datos contemplada por el proyecto.

Su objetivo es responder, por cada tipo de estructura:

- qué representa,
- qué entidades usa,
- qué restricciones debe cumplir,
- qué operaciones semánticas están permitidas,
- qué metadatos o especializaciones necesita,
- y cómo debe entenderse dentro del editor.

Este archivo complementa al modelo de datos general y a la especificación funcional. Aquí se aterriza la identidad de cada estructura de manera más precisa.

---

## 2. Criterio general de definición

Cada estructura se documentará con esta lógica:

- **Familia**
- **Variante**
- **Propósito conceptual**
- **Entidades principales**
- **Relaciones principales**
- **Restricciones estructurales**
- **Operaciones semánticas mínimas**
- **Metadatos especializados**
- **Consideraciones visuales**

---

## 3. Reglas generales compartidas

Antes de definir cada estructura, se asumen estas reglas generales:

### 3.1 Identidad
Toda entidad importante debe tener un ID estable dentro del documento.

### 3.2 Integridad
Ninguna referencia entre entidades debe apuntar a un nodo o arista inexistente.

### 3.3 Pertenencia
Una relación solo puede conectar entidades dentro de la misma estructura.

### 3.4 Persistencia
Toda estructura debe poder reconstruirse desde JSON sin depender de punteros persistidos.

### 3.5 Visualización
Toda estructura debe tener una representación 2D clara dentro del canvas.

### 3.6 Modo estructurado
El usuario no puede romper arbitrariamente las reglas centrales del tipo activo sin que el sistema lo detecte o lo impida.

---

# 4. Vector

## 4.1 Familia
`vector`

## 4.2 Variante
`vector`

## 4.3 Propósito conceptual
Representar una secuencia indexada de elementos en una disposición lineal.

## 4.4 Entidades principales
- celdas o nodos indexados,
- valor por posición,
- configuración de longitud o capacidad lógica si aplica.

## 4.5 Relaciones principales
No requiere aristas lógicas obligatorias entre celdas.
La relación principal es el **índice** y la **posición ordinal**.

## 4.6 Restricciones estructurales
- cada elemento pertenece a una posición,
- no debe haber dos elementos con el mismo índice activo,
- el orden visual debe corresponder al orden lógico,
- la estructura es secuencial.

## 4.7 Operaciones semánticas mínimas
- insertar en posición,
- reemplazar en posición,
- eliminar en posición,
- leer valor en posición,
- limpiar vector.

## 4.8 Metadatos especializados
- índice,
- longitud,
- capacidad opcional,
- tipo de valor.

## 4.9 Consideraciones visuales
- disposición horizontal,
- celdas visibles y separadas,
- índice visible o disponible,
- sensación de arreglo ordenado.

---

# 5. Lista enlazada simple

## 5.1 Familia
`list`

## 5.2 Variante
`singly_linked_list`

## 5.3 Propósito conceptual
Representar una secuencia de nodos donde cada nodo apunta al siguiente.

## 5.4 Entidades principales
- nodos,
- enlace `next`.

## 5.5 Relaciones principales
- un nodo puede apuntar a otro mediante `next`.

## 5.6 Restricciones estructurales
- cada nodo tiene como máximo un `next`,
- no debe haber ambigüedad de siguiente,
- el editor debe poder identificar inicio y continuidad,
- si la lista no es circular, el final no apunta a otro nodo.

## 5.7 Operaciones semánticas mínimas
- insertar al inicio,
- insertar al final,
- insertar después de un nodo,
- eliminar nodo,
- reconectar `next`,
- editar valor.

## 5.8 Metadatos especializados
- `next_node_id`,
- marcador opcional de cabeza.

## 5.9 Consideraciones visuales
- nodos en línea,
- flechas `next` visibles,
- lectura de izquierda a derecha,
- cabeza distinguible si aplica.

---

# 6. Lista enlazada doble

## 6.1 Familia
`list`

## 6.2 Variante
`doubly_linked_list`

## 6.3 Propósito conceptual
Representar una secuencia donde cada nodo conoce el anterior y el siguiente.

## 6.4 Entidades principales
- nodos,
- enlaces `prev` y `next`.

## 6.5 Relaciones principales
- un nodo puede tener un `prev`,
- un nodo puede tener un `next`.

## 6.6 Restricciones estructurales
- cada nodo tiene como máximo un `prev` y un `next`,
- las relaciones deben mantener coherencia bidireccional,
- si A es `next` de B, entonces B debe poder reflejar la relación inversa cuando corresponda.

## 6.7 Operaciones semánticas mínimas
- insertar al inicio,
- insertar al final,
- insertar antes,
- insertar después,
- eliminar nodo,
- editar valor,
- reconectar `prev/next`.

## 6.8 Metadatos especializados
- `prev_node_id`,
- `next_node_id`,
- cabeza opcional,
- cola opcional.

## 6.9 Consideraciones visuales
- nodos en línea,
- flechas o indicadores de ida y vuelta,
- lectura clara de ambas direcciones.

---

# 7. Lista circular simple

## 7.1 Familia
`list`

## 7.2 Variante
`circular_singly_linked_list`

## 7.3 Propósito conceptual
Lista simple donde el último nodo apunta al primero.

## 7.4 Entidades principales
- nodos,
- enlace `next`,
- cierre circular.

## 7.5 Relaciones principales
- cada nodo tiene como máximo un `next`,
- el último puede apuntar al primero.

## 7.6 Restricciones estructurales
- debe existir un único ciclo principal cuando la circularidad está activa,
- no debe haber múltiples cierres conflictivos,
- el sistema debe reconocer la continuidad circular.

## 7.7 Operaciones semánticas mínimas
- insertar nodo,
- eliminar nodo,
- editar valor,
- cerrar circularidad,
- abrir circularidad.

## 7.8 Metadatos especializados
- `is_circular`,
- referencia lógica de cabeza.

## 7.9 Consideraciones visuales
- forma lineal o curva cerrada,
- flecha de cierre claramente visible.

---

# 8. Lista circular doble

## 8.1 Familia
`list`

## 8.2 Variante
`circular_doubly_linked_list`

## 8.3 Propósito conceptual
Lista doble donde la cabeza y la cola se conectan circularmente.

## 8.4 Entidades principales
- nodos,
- enlaces `prev` y `next`,
- cierre circular.

## 8.5 Relaciones principales
- un nodo puede tener `prev` y `next`,
- el primer y último nodo pueden cerrar el ciclo.

## 8.6 Restricciones estructurales
- máximo un `prev` y un `next` por nodo,
- la circularidad debe mantenerse coherente en ambos sentidos,
- no deben existir cierres incompatibles.

## 8.7 Operaciones semánticas mínimas
- insertar nodo,
- eliminar nodo,
- editar valor,
- cerrar circularidad,
- abrir circularidad,
- reconectar ambos sentidos.

## 8.8 Metadatos especializados
- `prev_node_id`,
- `next_node_id`,
- `is_circular`.

## 8.9 Consideraciones visuales
- continuidad circular visible,
- claridad en las dos direcciones.

---

# 9. Stack

## 9.1 Familia
`stack`

## 9.2 Variante
`stack`

## 9.3 Propósito conceptual
Representar una secuencia con política LIFO (Last In, First Out).

## 9.4 Entidades principales
- elementos apilados,
- referencia al tope.

## 9.5 Relaciones principales
Puede modelarse como secuencia ordenada o como nodos enlazados, según implementación interna.

## 9.6 Restricciones estructurales
- debe existir un tope bien definido,
- el orden semántico debe respetar LIFO,
- `pop` solo remueve el tope.

## 9.7 Operaciones semánticas mínimas
- `push`,
- `pop`,
- `peek`,
- limpiar,
- editar valor.

## 9.8 Metadatos especializados
- índice o referencia de tope,
- altura lógica.

## 9.9 Consideraciones visuales
- disposición vertical,
- tope destacado,
- sensación clara de apilamiento.

---

# 10. Queue

## 10.1 Familia
`queue`

## 10.2 Variante
`queue`

## 10.3 Propósito conceptual
Representar una secuencia con política FIFO (First In, First Out).

## 10.4 Entidades principales
- elementos en orden,
- referencia a frente,
- referencia a final.

## 10.5 Relaciones principales
Secuencia lógica ordenada.

## 10.6 Restricciones estructurales
- debe identificarse frente y final,
- `enqueue` agrega al final,
- `dequeue` retira del frente,
- el orden lógico debe ser coherente.

## 10.7 Operaciones semánticas mínimas
- `enqueue`,
- `dequeue`,
- `front`,
- `rear`,
- limpiar,
- editar valor.

## 10.8 Metadatos especializados
- `front_index` o referencia,
- `rear_index` o referencia.

## 10.9 Consideraciones visuales
- alineación lineal,
- frente y final claramente marcados.

---

# 11. Circular Queue

## 11.1 Familia
`queue`

## 11.2 Variante
`circular_queue`

## 11.3 Propósito conceptual
Cola FIFO con continuidad circular lógica.

## 11.4 Entidades principales
- elementos,
- frente,
- final,
- continuidad circular.

## 11.5 Relaciones principales
Orden secuencial con cierre lógico circular.

## 11.6 Restricciones estructurales
- debe existir continuidad circular consistente,
- frente y final deben mantenerse válidos,
- la estructura no debe perder coherencia al envolver posiciones.

## 11.7 Operaciones semánticas mínimas
- `enqueue`,
- `dequeue`,
- `front`,
- `rear`,
- editar valor.

## 11.8 Metadatos especializados
- `front_index`,
- `rear_index`,
- `is_circular`.

## 11.9 Consideraciones visuales
- puede verse lineal con indicación de circularidad,
- o con una marca de continuidad explícita.

---

# 12. Priority Queue

## 12.1 Familia
`queue`

## 12.2 Variante
`priority_queue`

## 12.3 Propósito conceptual
Cola donde cada elemento tiene prioridad y la extracción depende de ella.

## 12.4 Entidades principales
- elementos,
- prioridad asociada,
- orden lógico según prioridad.

## 12.5 Relaciones principales
Secuencia con criterio de prioridad.

## 12.6 Restricciones estructurales
- cada elemento debe tener prioridad,
- la estructura debe poder ordenar o identificar el elemento prioritario,
- el criterio de prioridad debe ser consistente.

## 12.7 Operaciones semánticas mínimas
- insertar con prioridad,
- extraer prioritario,
- consultar prioritario,
- editar prioridad,
- editar valor.

## 12.8 Metadatos especializados
- `priority`,
- criterio de orden (si se documenta más adelante).

## 12.9 Consideraciones visuales
- prioridad visible,
- orden comprensible,
- el elemento dominante debe poder distinguirse.

---

# 13. Árbol binario

## 13.1 Familia
`tree`

## 13.2 Variante
`binary_tree`

## 13.3 Propósito conceptual
Representar una estructura jerárquica donde cada nodo puede tener hasta dos hijos.

## 13.4 Entidades principales
- nodos,
- raíz,
- hijo izquierdo,
- hijo derecho,
- padre opcional.

## 13.5 Relaciones principales
- `left`,
- `right`,
- `parent` opcionalmente derivable.

## 13.6 Restricciones estructurales
- cada nodo puede tener máximo dos hijos,
- debe distinguirse entre izquierdo y derecho,
- la raíz es única,
- un nodo no debe tener múltiples padres.

## 13.7 Operaciones semánticas mínimas
- crear raíz,
- insertar hijo izquierdo,
- insertar hijo derecho,
- eliminar nodo,
- editar valor,
- cambiar relación padre-hijo.

## 13.8 Metadatos especializados
- `parent_id`,
- `left_child_id`,
- `right_child_id`,
- marca de raíz si aplica.

## 13.9 Consideraciones visuales
- disposición jerárquica vertical,
- raíz arriba,
- hijos abajo,
- izquierda y derecha claramente diferenciadas.

---

# 14. BST

## 14.1 Familia
`tree`

## 14.2 Variante
`bst`

## 14.3 Propósito conceptual
Árbol binario de búsqueda donde el orden de los valores define la posición de cada nodo.

## 14.4 Entidades principales
- mismas que el árbol binario,
- con regla de orden adicional.

## 14.5 Relaciones principales
- `left`,
- `right`.

## 14.6 Restricciones estructurales
- toda rama izquierda debe respetar el orden del BST,
- toda rama derecha debe respetar el orden del BST,
- la raíz es única,
- los hijos izquierdo y derecho no se intercambian arbitrariamente.

## 14.7 Operaciones semánticas mínimas
- insertar valor,
- buscar valor,
- eliminar valor,
- editar valor con validación,
- revalidar estructura.

## 14.8 Metadatos especializados
No requiere un nodo radicalmente distinto, pero sí validación especializada.

## 14.9 Consideraciones visuales
- debe verse como árbol binario,
- pero la UI puede ayudar a remarcar que existe una regla de orden.

---

# 15. AVL

## 15.1 Familia
`tree`

## 15.2 Variante
`avl`

## 15.3 Propósito conceptual
Árbol binario de búsqueda balanceado.

## 15.4 Entidades principales
- mismas que BST,
- con balance adicional.

## 15.5 Relaciones principales
- `left`,
- `right`,
- estructura de árbol binario.

## 15.6 Restricciones estructurales
- debe cumplir la propiedad BST,
- debe cumplir restricción de balance AVL,
- las rotaciones lógicas deben conservar la coherencia estructural.

## 15.7 Operaciones semánticas mínimas
- insertar valor,
- eliminar valor,
- buscar valor,
- rebalancear,
- editar valor con validación.

## 15.8 Metadatos especializados
Posiblemente:
- `height`,
- `balance_factor`,
según la estrategia final de implementación.

## 15.9 Consideraciones visuales
- similar a BST,
- el sistema debe reflejar el árbol resultante tras balanceo,
- sin necesidad de animación paso a paso en V1.

---

# 16. Heap

## 16.1 Familia
`heap`

## 16.2 Variante
`heap`

## 16.3 Propósito conceptual
Estructura jerárquica con propiedad de heap y forma de árbol casi completo.

## 16.4 Entidades principales
- nodos,
- raíz,
- relaciones padre-hijo,
- configuración de tipo de heap.

## 16.5 Relaciones principales
- `left`,
- `right`,
- orden jerárquico.

## 16.6 Restricciones estructurales
- debe mantenerse la propiedad del heap,
- la forma debe ser compatible con árbol casi completo,
- debe existir un elemento principal en la raíz.

## 16.7 Operaciones semánticas mínimas
- insertar valor,
- extraer principal,
- consultar principal,
- heapify,
- editar valor con reajuste.

## 16.8 Metadatos especializados
- `heap_kind` (por ejemplo, min o max si se adopta esa distinción),
- posibles datos derivados de posición lógica.

## 16.9 Consideraciones visuales
- representación arbórea,
- raíz destacada,
- jerarquía visible.

---

# 17. Set

## 17.1 Familia
`set`

## 17.2 Variante
`set`

## 17.3 Propósito conceptual
Representar una colección de elementos únicos.

## 17.4 Entidades principales
- elementos únicos.

## 17.5 Relaciones principales
No requiere aristas obligatorias entre elementos.

## 17.6 Restricciones estructurales
- no se permiten duplicados,
- la pertenencia de un elemento al conjunto debe ser clara,
- el orden visual no implica necesariamente orden semántico.

## 17.7 Operaciones semánticas mínimas
- agregar elemento,
- eliminar elemento,
- verificar existencia,
- editar valor con verificación de unicidad,
- limpiar conjunto.

## 17.8 Metadatos especializados
- criterio de unicidad,
- tipo de valor.

## 17.9 Consideraciones visuales
- disposición limpia,
- sensación de colección y no de secuencia obligatoria,
- claridad en ausencia de duplicados.

---

# 18. Map

## 18.1 Familia
`map`

## 18.2 Variante
`map`

## 18.3 Propósito conceptual
Representar pares clave-valor con clave única.

## 18.4 Entidades principales
- entradas clave-valor,
- clave,
- valor.

## 18.5 Relaciones principales
Cada entrada asocia una clave con un valor.
No requiere aristas del mismo tipo que listas o grafos.

## 18.6 Restricciones estructurales
- cada clave debe ser única,
- toda entrada debe tener clave y valor,
- editar la clave requiere revalidación de unicidad.

## 18.7 Operaciones semánticas mínimas
- insertar par clave-valor,
- actualizar valor por clave,
- buscar por clave,
- eliminar por clave,
- editar clave,
- editar valor.

## 18.8 Metadatos especializados
- `key`,
- `key_type`,
- `value`,
- `value_type`.

## 18.9 Consideraciones visuales
- cada entrada debe mostrar claramente su clave y su valor,
- puede verse como fila, tarjeta o celda doble,
- debe ser legible y técnica.

---

# 19. Graph

## 19.1 Familia
`graph`

## 19.2 Variantes
- `directed_graph`
- `undirected_graph`
- `directed_weighted_graph`
- `undirected_weighted_graph`

## 19.3 Propósito conceptual
Representar una colección de vértices conectados por aristas, con posibilidad de dirección y peso.

## 19.4 Entidades principales
- vértices,
- aristas,
- peso opcional,
- dirección opcional.

## 19.5 Relaciones principales
- una arista conecta dos vértices,
- puede ser dirigida o no,
- puede ser ponderada o no.

## 19.6 Restricciones estructurales
- toda arista debe apuntar a vértices válidos,
- un grafo dirigido debe mostrar orientación,
- un grafo no dirigido no debe comportarse como dirigido,
- un grafo ponderado debe mostrar o almacenar peso,
- un grafo no ponderado no requiere peso obligatorio.

## 19.7 Operaciones semánticas mínimas
- agregar vértice,
- eliminar vértice,
- editar vértice,
- agregar arista,
- eliminar arista,
- editar arista,
- cambiar dirección cuando aplique,
- asignar peso cuando aplique.

## 19.8 Metadatos especializados
- `is_directed`,
- `is_weighted`,
- `weight`,
- etiqueta de arista,
- posibles restricciones futuras sobre multiaristas o bucles.

## 19.9 Consideraciones visuales
- vértices libres en el plano,
- aristas claramente conectadas,
- flechas visibles en dirigidos,
- peso visible en ponderados,
- buena legibilidad incluso con varias conexiones.

---

## 20. Relación entre familia y variante

No todas las reglas viven en el mismo nivel.

### Reglas de familia
Ejemplo:
- toda lista usa relaciones de continuidad,
- todo árbol usa jerarquía,
- todo grafo usa vértices y aristas.

### Reglas de variante
Ejemplo:
- lista doble añade `prev`,
- AVL añade balance,
- grafo ponderado añade peso,
- grafo dirigido añade dirección.

### Criterio arquitectónico
El sistema debe permitir reutilización por familia y especialización por variante.

---

## 21. Compatibilidad con operaciones del editor

Cada estructura debe poder responder, al menos conceptualmente, a estas preguntas:

- ¿qué tipo de nodo usa?
- ¿qué tipo de relación usa?
- ¿qué operaciones admite?
- ¿qué restricciones valida?
- ¿qué layout base le corresponde?
- ¿qué datos especializados necesita?

Este criterio será clave para el registro de tipos y para `StructureOps`.

---

## 22. Compatibilidad con persistencia

Toda estructura definida aquí debe poder serializarse sin ambigüedad.

### Requisito mínimo
El JSON debe poder reconstruir:
- tipo,
- variante,
- nodos,
- aristas si aplica,
- datos especializados,
- posiciones visuales relevantes,
- configuración general.

---

## 23. Compatibilidad con render

Toda estructura debe poder dibujarse de forma clara con reglas visuales mínimas.

### Requisitos comunes
- texto legible,
- nodos distinguibles,
- relaciones claras,
- selección visible,
- metadatos visibles cuando sean importantes.

---

## 24. Resultado esperado del documento

Al finalizar la V1, estas definiciones deben servir como base para:

- implementar operaciones por estructura,
- construir validación por tipo,
- diseñar el JSON,
- organizar módulos especializados,
- y mantener coherencia entre dominio, editor y render.

---

## 25. Relación con otros documentos

Este archivo se complementa con:

- `01_product_spec.md` → funciones y operaciones esperadas
- `03_architecture.md` → arquitectura general
- `04_data_model.md` → entidades del modelo
- `06_operations.md` → detalle de operaciones por estructura
- `07_file_format.md` → persistencia JSON
- `08_rendering.md` → representación visual por tipo
