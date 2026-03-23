# 04_data_model.md

## 1. Propósito del documento

Este documento define el **modelo de datos interno** de **StructStudio C** para su primera versión (V1).

Su objetivo es describir:

- qué entidades existen en memoria,
- cómo se relacionan,
- qué parte es común a todas las estructuras,
- qué parte puede especializarse,
- y qué información debe existir para que el editor, el render y la persistencia funcionen de forma coherente.

Este archivo se centra en el modelo lógico en tiempo de ejecución. No define todavía la forma exacta del JSON final ni el layout visual detallado.

---

## 2. Objetivo del modelo

El modelo de datos debe permitir representar un **documento editable de estructuras de datos 2D** con suficiente información para:

- operar semánticamente,
- validar reglas,
- renderizar la estructura,
- editarla con interacción visual,
- guardarla y cargarla,
- y extender el sistema con nuevas estructuras en el futuro.

El modelo debe ser lo bastante general para soportar varias familias de estructuras, pero sin caer en una abstracción tan extrema que vuelva confuso el dominio.

---

## 3. Principios del modelo

### 3.1 Documento primero
La unidad principal del sistema es el **documento**.

### 3.2 Un documento visible a la vez
La aplicación trabaja con un documento activo por vez.

### 3.3 Base común + especialización
Habrá conceptos comunes compartidos, pero cada familia puede tener atributos y reglas especializadas.

### 3.4 Persistencia por identificadores
Las relaciones entre entidades se representarán mediante **IDs**, no mediante punteros persistidos.

### 3.5 Separación entre lógica y visual
El modelo debe contener tanto estado lógico como metadatos visuales, pero distinguiéndolos conceptualmente.

### 3.6 Estado editable
Todo lo necesario para reconstruir el documento y seguir editándolo debe estar representado explícitamente.

---

## 4. Vista general del modelo

A nivel alto, el modelo se organiza así:

```text
ApplicationContext
  └── ActiveDocument
        ├── DocumentMetadata
        ├── Structures[]
        ├── ActiveStructureId
        └── ViewState

Structure
  ├── StructureHeader
  ├── StructureKind
  ├── StructureConfig
  ├── Elements
  ├── Relations
  ├── VisualState
  └── SpecializedData
```

---

## 5. Entidades principales

Las entidades principales del modelo serán:

- `Document`
- `DocumentMetadata`
- `Structure`
- `StructureKind`
- `StructureConfig`
- `Node`
- `Edge`
- `NodeVisual`
- `EdgeVisual`
- `EditorSelection`
- `ViewState`
- `StructureOps`
- `StructureValidationResult`

Además, algunas familias tendrán datos especializados:

- `MapEntryData`
- `PriorityData`
- `GraphEdgeData`
- `TreeNodeData`
- `LinkedNodeData`
- `HeapData`
- etc.

---

## 6. Document

### 6.1 Definición conceptual
`Document` representa el archivo editable completo que el usuario abre, modifica, guarda y exporta.

### 6.2 Responsabilidad
Debe contener todo el estado necesario para reconstruir el trabajo del usuario.

### 6.3 Contenido esperado
Un `Document` deberá incluir, como mínimo:

- identificador interno del documento,
- nombre lógico,
- metadatos,
- lista de estructuras contenidas,
- referencia a la estructura activa,
- estado visual general de la vista,
- versión del formato de trabajo.

### 6.4 Observación
Aunque el usuario vea un documento a la vez, un documento puede contener una o varias estructuras internas si el diseño final así lo permite. En V1, la experiencia priorizará una estructura activa por vez.

---

## 7. DocumentMetadata

### 7.1 Propósito
Agrupar información descriptiva del documento.

### 7.2 Campos conceptuales sugeridos
- `document_id`
- `name`
- `description`
- `created_at`
- `updated_at`
- `format_version`

### 7.3 Nota
No todos estos campos tienen que exponerse al usuario en la V1, pero sí conviene que existan conceptualmente.

---

## 8. Structure

### 8.1 Definición conceptual
`Structure` representa una estructura de datos concreta dentro de un documento.

### 8.2 Responsabilidad
Contener el estado lógico, relacional y visual de una instancia de una estructura.

### 8.3 Contenido esperado
Una `Structure` deberá incluir:

- identificador de estructura,
- tipo o familia,
- variante,
- configuración específica,
- colección de nodos o elementos,
- colección de relaciones,
- metadatos visuales,
- estado especializado si aplica.

### 8.4 Ejemplos
- una lista enlazada doble,
- un BST,
- un heap,
- un set,
- un grafo dirigido ponderado.

---

## 9. StructureKind

### 9.1 Propósito
Identificar la categoría y variante exacta de una estructura.

### 9.2 Nivel de clasificación
Conviene separar:

#### Familia
- vector
- list
- stack
- queue
- tree
- heap
- set
- map
- graph

#### Variante
- singly_linked_list
- doubly_linked_list
- circular_singly_linked_list
- circular_doubly_linked_list
- bst
- avl
- priority_queue
- directed_weighted_graph
- etc.

### 9.3 Beneficio
Esto evita usar un único texto ambiguo para todo y facilita reglas por familia y por variante.

---

## 10. StructureConfig

### 10.1 Propósito
Guardar configuración particular de la estructura activa.

### 10.2 Ejemplos de configuración
- si el grafo es dirigido,
- si el grafo es ponderado,
- capacidad lógica del vector si aplica,
- orientación visual preferida,
- si una lista circular está cerrada,
- tipo de heap,
- reglas de presentación.

### 10.3 Importancia
No todo debe inferirse del tipo. Algunas opciones conviene guardarlas explícitamente en configuración.

---

## 11. Node

### 11.1 Definición conceptual
`Node` representa una unidad básica de información visible o estructural dentro de una `Structure`.

### 11.2 Responsabilidad
Contener:
- identidad,
- valor o contenido,
- atributos lógicos,
- y referencia a su estado visual.

### 11.3 Campos comunes sugeridos
- `node_id`
- `kind`
- `label`
- `value`
- `value_type`
- `flags`
- `visual`
- `specialized_data`

### 11.4 Observación importante
No todos los elementos visuales serán nodos idénticos en significado.

Ejemplos:
- un nodo de lista,
- una celda de vector,
- una entrada de map,
- un vértice de grafo,
- un nodo de árbol.

Por eso el modelo permite datos especializados.

---

## 12. Edge

### 12.1 Definición conceptual
`Edge` representa una relación entre dos nodos dentro de una misma estructura.

### 12.2 Responsabilidad
Describir la conexión lógica entre nodos.

### 12.3 Campos comunes sugeridos
- `edge_id`
- `source_node_id`
- `target_node_id`
- `relation_kind`
- `is_directed`
- `weight`
- `flags`
- `visual`
- `specialized_data`

### 12.4 Ejemplos de `relation_kind`
- `next`
- `prev`
- `left`
- `right`
- `parent`
- `graph_link`
- `ownership`
- `index_binding`

### 12.5 Observación
No todas las estructuras requerirán edges explícitos en la misma forma, pero el modelo base los contempla porque son útiles en listas, árboles y grafos.

---

## 13. NodeVisual

### 13.1 Propósito
Contener el estado visual del nodo dentro del canvas.

### 13.2 Campos conceptuales sugeridos
- `x`
- `y`
- `width`
- `height`
- `is_selected`
- `is_highlighted`
- `is_hidden`
- `style_variant`

### 13.3 Función
Permitir que el render y el editor trabajen sin mezclar esta información con la lógica pura de la estructura.

---

## 14. EdgeVisual

### 14.1 Propósito
Contener metadatos visuales de una relación.

### 14.2 Campos conceptuales sugeridos
- `label_x`
- `label_y`
- `curve_kind`
- `is_selected`
- `show_arrow`
- `show_weight`
- `style_variant`

### 14.3 Importancia
En grafos, listas circulares y relaciones dirigidas, esto cobra bastante valor.

---

## 15. ViewState

### 15.1 Propósito
Guardar estado general de visualización del documento o de la estructura activa.

### 15.2 Ejemplos
- visibilidad de grilla,
- desplazamiento del canvas si existe,
- paneles abiertos o cerrados,
- estructura visible,
- filtros mínimos de visualización.

### 15.3 Nota
En V1 debe mantenerse moderado. No conviene llenarlo de complejidad prematura.

---

## 16. EditorSelection

### 16.1 Propósito
Representar la selección actual del editor.

### 16.2 Posibilidades de selección
- ninguna selección,
- un nodo,
- una arista,
- una estructura,
- eventualmente selección múltiple en el futuro.

### 16.3 Campos sugeridos
- `selection_type`
- `selected_structure_id`
- `selected_node_id`
- `selected_edge_id`

### 16.4 Observación
Aunque la selección pertenece más al editor que al dominio puro, es útil documentarla aquí por su fuerte relación con el modelo visible.

---

## 17. StructureOps

### 17.1 Propósito
Definir el conjunto de operaciones que una estructura concreta soporta.

### 17.2 Idea conceptual
Cada tipo de estructura puede registrar:

- validación,
- inserción,
- eliminación,
- búsqueda,
- auto-layout,
- render especializado,
- operaciones contextuales.

### 17.3 Beneficio
Permite una arquitectura más desacoplada y extensible.

### 17.4 Importante
No es orientación a objetos formal; es polimorfismo funcional por módulo o tabla de operaciones.

---

## 18. StructureValidationResult

### 18.1 Propósito
Representar el resultado de validar una estructura.

### 18.2 Campos conceptuales sugeridos
- `is_valid`
- `error_count`
- `warning_count`
- `messages[]`
- `related_node_ids[]`
- `related_edge_ids[]`

### 18.3 Uso
Permite mostrar errores coherentes en editor y UI sin mezclar validación con render.

---

## 19. Base común de estructura

Toda `Structure` debería compartir una base común mínima:

- `structure_id`
- `family`
- `variant`
- `config`
- `nodes`
- `edges`
- `visual_state`
- `ops`

Esto permite una columna vertebral uniforme.

---

## 20. Especialización por familia

A partir de esa base común, algunas familias requerirán datos propios.

---

## 20.1 Vector

### Elemento principal
Celdas indexadas.

### Necesidades especiales
- índice lógico,
- capacidad o tamaño,
- orden secuencial fijo,
- representación por posición.

### Observación
Puede modelarse como nodos indexados o como una colección especializada de celdas.

---

## 20.2 Listas enlazadas

### Elemento principal
Nodos con relaciones `next` y opcionalmente `prev`.

### Necesidades especiales
- distinguir si es simple o doble,
- distinguir si es circular,
- mantener consistencia de enlaces.

### Posible dato especializado
`LinkedNodeData` con referencias lógicas a `next` y `prev` por ID.

---

## 20.3 Stack

### Elemento principal
Secuencia ordenada LIFO.

### Necesidades especiales
- identificar el tope,
- mantener orden de apilamiento,
- permitir operaciones semánticas `push`, `pop`, `peek`.

### Modelo sugerido
Colección ordenada de nodos más un indicador de tope.

---

## 20.4 Queue

### Elemento principal
Secuencia ordenada FIFO.

### Necesidades especiales
- identificar frente y final,
- distinguir variantes circular y prioridad.

### Modelo sugerido
Colección ordenada de nodos y metadatos de frente/final.

---

## 20.5 Priority Queue

### Elemento principal
Elemento con valor y prioridad.

### Necesidades especiales
Cada nodo debe tener una prioridad explícita.

### Posible dato especializado
`PriorityData`:
- `priority`
- `priority_label`

---

## 20.6 Tree

### Elemento principal
Nodo jerárquico.

### Necesidades especiales
- identificar hijos izquierdo y derecho,
- identificar raíz,
- soportar padre opcional,
- verificar restricciones por variante.

### Posible dato especializado
`TreeNodeData`:
- `parent_id`
- `left_child_id`
- `right_child_id`

---

## 20.7 BST

### Hereda de árbol
Además requiere:
- propiedad de orden,
- operaciones de inserción y búsqueda acordes.

### Observación
El nodo puede ser el mismo `TreeNode`, pero la estructura necesita validación y operaciones especializadas.

---

## 20.8 AVL

### Hereda de BST
Además requiere:
- balance,
- información derivable o explícita del factor de balance,
- operaciones de rotación lógicas.

### Posible dato especializado
No necesariamente por nodo en V1, pero puede contemplarse:
- `balance_factor`
- `height`

---

## 20.9 Heap

### Hereda parte del modelo de árbol
Además requiere:
- propiedad de heap,
- variante min/max si aplica,
- orden estructural de árbol casi completo.

### Posible dato especializado
`HeapConfig`:
- `heap_kind`

---

## 20.10 Set

### Elemento principal
Colección de elementos únicos.

### Necesidades especiales
- impedir duplicados,
- representar pertenencia,
- no obligar una relación de orden visual fuerte.

### Modelo sugerido
Colección de nodos con regla de unicidad.

---

## 20.11 Map

### Elemento principal
Par clave-valor.

### Necesidades especiales
- unicidad de clave,
- representación dual clave/valor,
- búsqueda por clave,
- actualización por clave.

### Posible dato especializado
`MapEntryData`:
- `key`
- `key_type`
- `value`
- `value_type`

### Observación
Aquí tiene mucho sentido que el nodo esté especializado en vez de intentar fingir que es un nodo genérico simple.

---

## 20.12 Graph

### Elemento principal
Vértices y aristas.

### Necesidades especiales
- dirigido o no dirigido,
- ponderado o no ponderado,
- múltiples aristas según reglas que se definan,
- peso opcional,
- posición libre de vértices.

### Posible dato especializado
`GraphEdgeData`:
- `weight`
- `is_directed`
- `graph_relation_type`

### Observación
En grafos, las aristas tienen un papel más central que en otras estructuras.

---

## 21. Valores y tipos de dato

El sistema debe contemplar que un nodo o entrada pueda contener distintos tipos de valor.

### En V1, recomendación
Trabajar internamente con una representación simple y controlada, por ejemplo:
- entero,
- decimal,
- texto,
- booleano,
- o valor serializado simple.

### Beneficio
Evita sobrecomplicar demasiado el modelo desde el inicio.

---

## 22. Identidad de las entidades

Toda entidad importante debe tener un identificador estable dentro del documento.

### IDs mínimos esperados
- `document_id`
- `structure_id`
- `node_id`
- `edge_id`

### Requisito
Los IDs deben ser suficientes para:
- persistir,
- reconstruir relaciones,
- y manipular selección y edición.

---

## 23. Relaciones entre entidades

### Relaciones principales
- un documento contiene muchas estructuras,
- una estructura contiene muchos nodos,
- una estructura contiene muchas aristas,
- una arista conecta dos nodos,
- un nodo pertenece a una estructura,
- una selección apunta a una entidad del documento activo.

### Regla importante
Una arista no debe unir nodos de estructuras distintas en V1.

---

## 24. Estado lógico vs estado visual

Conviene distinguir dos dimensiones:

### Estado lógico
- tipo,
- valor,
- relaciones,
- reglas,
- propiedades estructurales.

### Estado visual
- posición,
- tamaño,
- selección,
- estilo,
- visibilidad.

### Decisión
Ambos viven dentro del modelo del documento editable, pero deben distinguirse conceptualmente para no confundir reglas con presentación.

---

## 25. Estado derivado vs estado persistido

No todo necesita persistirse si puede recalcularse.

### Puede ser derivado
- advertencias temporales,
- selección actual,
- algunos resaltados,
- algunos factores visuales transitorios.

### Debe persistirse
- estructuras,
- nodos,
- relaciones,
- configuración,
- posiciones visuales importantes,
- tipo y variante,
- pesos,
- claves y valores,
- metadatos del documento.

---

## 26. Reglas de coherencia del modelo

El modelo debe poder expresar y comprobar, como mínimo:

- unicidad de IDs,
- pertenencia correcta de nodos y aristas a una estructura,
- validez de referencias por ID,
- reglas de conexión por tipo,
- unicidad de clave en map,
- ausencia de duplicados en set,
- restricciones de hijos en árboles,
- restricciones de dirección y peso en grafos,
- consistencia de circularidad en listas.

---

## 27. Modelo del documento activo en ejecución

En tiempo de ejecución, el editor probablemente trabajará con algo como:

```text
AppContext
  ├── ActiveDocument
  ├── EditorState
  ├── Selection
  ├── UIState
  └── Services
```

### Nota
`AppContext` no es parte del documento persistido, pero sí del modelo de ejecución de la aplicación.

---

## 28. Estructuras auxiliares del editor

Aunque no pertenecen al dominio puro, conviene contemplar estas estructuras de soporte:

- `EditorState`
- `ToolState`
- `SelectionState`
- `DragState`
- `ValidationOverlay`

Estas se documentarán con más detalle en `09_editor_logic.md`, pero se conectan directamente con el modelo de datos.

---

## 29. Estrategia de representación interna recomendada

### Recomendación general
Usar:

- arrays dinámicos o listas internas de entidades,
- referencias por ID,
- acceso controlado por funciones del módulo,
- especialización moderada por familia.

### Evitar
- mezclar punteros crudos persistibles con lógica de archivo,
- poner toda la semántica en un único struct gigante,
- depender del layout visual para definir reglas lógicas.

---

## 30. Ejemplo conceptual simplificado

```text
Document
  name = "grafos_basicos"
  structures = [
    Structure {
      family = graph,
      variant = directed_weighted_graph,
      nodes = [
        Node { id = 1, label = "A", visual = (...) },
        Node { id = 2, label = "B", visual = (...) }
      ],
      edges = [
        Edge {
          id = 10,
          source_node_id = 1,
          target_node_id = 2,
          is_directed = true,
          weight = 5
        }
      ]
    }
  ]
```

Este ejemplo solo busca ilustrar la relación entre documento, estructura, nodos y aristas.

---

## 31. Decisiones del modelo ya tomadas

### Decisiones cerradas
- el sistema trabaja con documentos,
- se verá un documento a la vez,
- las estructuras son 2D,
- el modelo debe soportar especialización,
- el documento se guardará en JSON,
- la vista visual puede exportarse a PNG,
- el sistema tendrá modo estructurado,
- grafos sí forman parte de V1.

---

## 32. Riesgos de modelado a evitar

### 32.1 Generalización excesiva
No todo debe reducirse a “nodo genérico universal” si eso destruye claridad.

### 32.2 Especialización caótica
Tampoco conviene que cada tipo invente su propio mundo sin base común.

### 32.3 Duplicación de información sin control
Hay que evitar inconsistencias entre relaciones explícitas y datos derivados.

### 32.4 Mezclar estado transitorio con persistencia definitiva
La selección o un hover no deben contaminar el archivo persistido salvo que tenga sentido real.

---

## 33. Resultado esperado del modelo

Al final de la V1, el modelo de datos debe permitir:

- representar todas las estructuras objetivo del proyecto,
- editarlas visualmente,
- operar semánticamente sobre ellas,
- validarlas,
- persistirlas,
- y recuperarlas sin perder coherencia.

---

## 34. Relación con otros documentos

Este archivo se complementa con:

- `00_overview.md` → visión general del proyecto
- `01_product_spec.md` → comportamiento funcional
- `03_architecture.md` → arquitectura general
- `05_structure_definitions.md` → reglas específicas por estructura
- `06_operations.md` → operaciones detalladas
- `07_file_format.md` → persistencia JSON
- `08_rendering.md` → representación visual
- `09_editor_logic.md` → estado y lógica del ed