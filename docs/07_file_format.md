# 07_file_format.md

## 1. Propósito del documento

Este documento define el **formato de archivo** de **StructStudio C** para la primera versión (V1).

Su objetivo es describir:

- cómo se persistirá un documento en JSON,
- qué entidades deben serializarse,
- qué información es obligatoria,
- qué información es opcional,
- cómo se versionará el formato,
- y qué criterios deben respetarse para guardar y cargar documentos de forma coherente.

Este documento no describe la implementación del parser o del serializador en C; define el contrato del archivo persistido.

---

## 2. Objetivo del formato

El formato JSON de StructStudio C debe permitir:

- guardar el documento completo,
- reconstruir la estructura activa,
- preservar nodos, relaciones y configuración,
- preservar la representación visual necesaria para seguir editando,
- y mantenerse extensible hacia futuras versiones.

El archivo debe ser:

- legible,
- razonablemente claro,
- consistente,
- y lo suficientemente estable para servir como base del proyecto.

---

## 3. Principios del formato

### 3.1 Formato legible
El archivo debe ser JSON legible por humanos.

### 3.2 Persistencia por IDs
Las relaciones entre entidades deben persistirse mediante IDs, nunca mediante punteros o referencias de memoria.

### 3.3 Separación entre lógica y visual
El archivo debe guardar tanto información lógica como visual cuando sea necesaria para reconstruir el editor.

### 3.4 Versionado explícito
Todo archivo debe indicar la versión del formato.

### 3.5 Extensibilidad controlada
El formato debe permitir agregar campos nuevos en el futuro sin destruir compatibilidad inmediatamente.

### 3.6 Coherencia estructural
El archivo debe permitir validar referencias, pertenencia y restricciones mínimas del documento.

---

## 4. Unidad persistida principal

La unidad persistida es el **documento**.

Un archivo `.json` de StructStudio C representa un único documento completo.

Ese documento contiene:

- metadatos del documento,
- una o varias estructuras,
- referencia a la estructura activa,
- configuración visual general,
- y el contenido completo necesario para reabrir el trabajo.

---

## 5. Nombre y extensión del archivo

### 5.1 Recomendación para V1
Durante la V1 puede usarse simplemente JSON como formato base.

### 5.2 Opciones razonables
- guardar como `.json`
- o definir una extensión propia más adelante, por ejemplo `.structstudio.json`

### 5.3 Decisión práctica para V1
El contenido será JSON. La política exacta de extensión puede mantenerse simple al inicio.

---

## 6. Estructura general del archivo

La forma general del documento será conceptualmente así:

```json
{
  "format_version": 1,
  "document": {
    "metadata": { ... },
    "active_structure_id": "...",
    "view_state": { ... },
    "structures": [ ... ]
  }
}
```

---

## 7. Nivel superior del JSON

### 7.1 Campos principales

#### `format_version`
Versión del formato persistido.

#### `document`
Objeto principal del documento.

### 7.2 Requisitos
- `format_version` es obligatorio.
- `document` es obligatorio.

---

## 8. Objeto `document`

El objeto `document` contendrá:

- `metadata`
- `active_structure_id`
- `view_state`
- `structures`

### 8.1 Ejemplo conceptual

```json
{
  "document": {
    "metadata": { ... },
    "active_structure_id": "structure_1",
    "view_state": { ... },
    "structures": [ ... ]
  }
}
```

---

## 9. `document.metadata`

### 9.1 Propósito
Guardar información descriptiva del documento.

### 9.2 Campos sugeridos
- `document_id`
- `name`
- `description`
- `created_at`
- `updated_at`

### 9.3 Ejemplo

```json
{
  "metadata": {
    "document_id": "doc_001",
    "name": "grafos_basicos",
    "description": "Documento de ejemplo para grafos",
    "created_at": "2026-03-18T20:00:00",
    "updated_at": "2026-03-18T20:45:00"
  }
}
```

### 9.4 Requisitos
- `document_id` debe existir.
- `name` debe existir al menos como cadena no vacía o nombre por defecto.

---

## 10. `document.active_structure_id`

### 10.1 Propósito
Indicar qué estructura estaba activa al guardar el documento.

### 10.2 Requisitos
- debe apuntar a una estructura existente,
- si solo existe una estructura, debe referenciarla claramente.

---

## 11. `document.view_state`

### 11.1 Propósito
Guardar parte del estado visual general del documento.

### 11.2 Campos sugeridos para V1
- `show_grid`
- `left_panel_visible`
- `right_panel_visible`
- `canvas_offset_x` opcional
- `canvas_offset_y` opcional

### 11.3 Ejemplo

```json
{
  "view_state": {
    "show_grid": true,
    "left_panel_visible": true,
    "right_panel_visible": true
  }
}
```

### 11.4 Nota
No conviene persistir demasiados detalles efímeros en V1.

---

## 12. `document.structures`

### 12.1 Propósito
Guardar todas las estructuras del documento.

### 12.2 Tipo
Arreglo de objetos `structure`.

### 12.3 Requisitos
- debe existir,
- puede estar vacío solo si el sistema lo permite explícitamente,
- cada estructura debe tener ID único dentro del documento.

---

## 13. Objeto `structure`

Cada estructura debe incluir, como mínimo:

- `structure_id`
- `family`
- `variant`
- `config`
- `visual_state`
- `nodes`
- `edges`

### 13.1 Ejemplo base

```json
{
  "structure_id": "structure_1",
  "family": "graph",
  "variant": "directed_weighted_graph",
  "config": { ... },
  "visual_state": { ... },
  "nodes": [ ... ],
  "edges": [ ... ]
}
```

---

## 14. `structure.structure_id`

### Propósito
Identificar la estructura dentro del documento.

### Requisito
Debe ser única dentro del archivo.

---

## 15. `structure.family`

### Propósito
Indicar la familia general de la estructura.

### Valores previstos
- `vector`
- `list`
- `stack`
- `queue`
- `tree`
- `heap`
- `set`
- `map`
- `graph`

### Requisito
Debe ser obligatorio.

---

## 16. `structure.variant`

### Propósito
Indicar la variante específica de la estructura.

### Valores previstos de ejemplo
- `vector`
- `singly_linked_list`
- `doubly_linked_list`
- `circular_singly_linked_list`
- `circular_doubly_linked_list`
- `stack`
- `queue`
- `circular_queue`
- `priority_queue`
- `binary_tree`
- `bst`
- `avl`
- `heap`
- `set`
- `map`
- `directed_graph`
- `undirected_graph`
- `directed_weighted_graph`
- `undirected_weighted_graph`

### Requisito
Debe ser obligatorio.

---

## 17. `structure.config`

### Propósito
Guardar configuración específica de la estructura.

### Ejemplos según estructura
- grafo dirigido o no,
- grafo ponderado o no,
- tipo de heap,
- capacidad del vector,
- bandera de circularidad,
- reglas de presentación relevantes.

### Ejemplo

```json
{
  "config": {
    "is_directed": true,
    "is_weighted": true
  }
}
```

### Requisito
Debe existir aunque sea un objeto vacío.

---

## 18. `structure.visual_state`

### Propósito
Guardar configuración visual general de la estructura.

### Campos sugeridos
- `layout_mode`
- `show_labels`
- `show_weights`
- `show_indices`

### Ejemplo

```json
{
  "visual_state": {
    "layout_mode": "manual",
    "show_labels": true,
    "show_weights": true
  }
}
```

### Nota
No confundir con el estado visual individual de nodos y aristas.

---

## 19. `structure.nodes`

### 19.1 Propósito
Guardar todas las entidades principales tipo nodo o equivalente de la estructura.

### 19.2 Tipo
Arreglo de objetos `node`.

### 19.3 Requisitos
- cada nodo debe tener ID único dentro de la estructura,
- los datos visuales mínimos deben persistirse,
- los datos especializados deben poder reconstruirse.

---

## 20. Objeto `node`

Cada nodo deberá incluir, como mínimo:

- `node_id`
- `kind`
- `label`
- `value`
- `value_type`
- `visual`
- `data`

### 20.1 Ejemplo base

```json
{
  "node_id": "node_1",
  "kind": "graph_vertex",
  "label": "A",
  "value": "A",
  "value_type": "string",
  "visual": {
    "x": 120,
    "y": 80,
    "width": 80,
    "height": 40
  },
  "data": {}
}
```

---

## 21. `node.kind`

### Propósito
Indicar la subcategoría del nodo.

### Valores posibles de ejemplo
- `vector_cell`
- `list_node`
- `stack_node`
- `queue_node`
- `tree_node`
- `heap_node`
- `set_node`
- `map_entry`
- `graph_vertex`

### Requisito
Debe ser obligatorio.

---

## 22. `node.value` y `node.value_type`

### Propósito
Representar el contenido principal del nodo.

### Tipos base sugeridos para V1
- `string`
- `int`
- `float`
- `bool`
- `null`

### Observación
En `map`, parte del contenido real puede vivir dentro de `data`, porque ahí hay clave y valor separados.

---

## 23. `node.visual`

### Propósito
Persistir posición y tamaño del nodo para reconstruir la edición visual.

### Campos sugeridos
- `x`
- `y`
- `width`
- `height`

### Ejemplo

```json
{
  "visual": {
    "x": 220,
    "y": 160,
    "width": 100,
    "height": 48
  }
}
```

### Requisito
Debe existir para cualquier entidad visible en canvas.

---

## 24. `node.data`

### Propósito
Guardar metadatos especializados por tipo de nodo.

### Ejemplos por estructura

#### Lista
```json
{
  "data": {
    "next_node_id": "node_2",
    "prev_node_id": null
  }
}
```

#### Árbol
```json
{
  "data": {
    "parent_id": null,
    "left_child_id": "node_2",
    "right_child_id": "node_3"
  }
}
```

#### Map
```json
{
  "data": {
    "key": "usuario",
    "key_type": "string",
    "map_value": "Marcos",
    "map_value_type": "string"
  }
}
```

#### Priority queue
```json
{
  "data": {
    "priority": 10
  }
}
```

### Nota
`data` permite especialización sin ensuciar el nivel superior del nodo.

---

## 25. `structure.edges`

### 25.1 Propósito
Guardar relaciones explícitas entre nodos.

### 25.2 Tipo
Arreglo de objetos `edge`.

### 25.3 Uso
Especialmente relevante en:
- listas,
- árboles,
- grafos.

Puede mantenerse vacío en estructuras que no requieran relaciones explícitas.

---

## 26. Objeto `edge`

Cada arista deberá incluir, como mínimo:

- `edge_id`
- `source_node_id`
- `target_node_id`
- `relation_kind`
- `is_directed`
- `weight`
- `visual`
- `data`

### 26.1 Ejemplo base

```json
{
  "edge_id": "edge_1",
  "source_node_id": "node_1",
  "target_node_id": "node_2",
  "relation_kind": "graph_link",
  "is_directed": true,
  "weight": 5,
  "visual": {
    "show_arrow": true,
    "show_weight": true
  },
  "data": {}
}
```

---

## 27. `edge.relation_kind`

### Propósito
Indicar el significado lógico de la relación.

### Valores de ejemplo
- `next`
- `prev`
- `left`
- `right`
- `graph_link`
- `tree_parent_child`

### Requisito
Debe ser obligatorio cuando existe una arista.

---

## 28. `edge.weight`

### Propósito
Persistir el peso de una arista cuando aplique.

### Regla
- en grafos ponderados debe poder existir,
- en grafos no ponderados puede ser `null` o ausente,
- en otras estructuras normalmente será `null`.

---

## 29. `edge.visual`

### Propósito
Persistir metadatos visuales de la arista.

### Campos sugeridos
- `show_arrow`
- `show_weight`
- `label_x` opcional
- `label_y` opcional
- `curve_kind` opcional

### Ejemplo

```json
{
  "visual": {
    "show_arrow": true,
    "show_weight": true,
    "curve_kind": "straight"
  }
}
```

---

## 30. `edge.data`

### Propósito
Persistir datos especializados de la relación cuando sea necesario.

### Ejemplos
- tipo especial de relación de grafo,
- metadatos de layout futuro,
- datos auxiliares por variante.

### Regla
En V1 puede ser objeto vacío si no se necesita más detalle.

---

## 31. Campos transitorios que no deben persistirse

No conviene persistir, al menos en V1:

- hover actual,
- selección actual del editor,
- estados temporales de arrastre,
- advertencias efímeras,
- overlays de validación temporales,
- mensajes de UI.

### Motivo
Son estados de ejecución, no parte esencial del documento persistido.

---

## 32. Reglas de integridad del archivo

Al cargar un documento, el sistema debe poder verificar al menos:

- unicidad de `structure_id`,
- unicidad de `node_id` dentro de cada estructura,
- unicidad de `edge_id` dentro de cada estructura,
- que `active_structure_id` exista,
- que cada arista apunte a nodos existentes,
- que la familia y variante sean compatibles,
- que los campos obligatorios existan,
- que el JSON tenga estructura válida.

---

## 33. Estrategia de compatibilidad futura

### 33.1 Versionado
El campo `format_version` permitirá detectar cambios de formato.

### 33.2 Evolución sugerida
- agregar campos nuevos sin romper los existentes,
- tolerar campos opcionales desconocidos si el cargador lo soporta,
- reservar la posibilidad de migración futura entre versiones.

### 33.3 Política práctica en V1
La V1 puede ser estricta con su formato, pero ya debe incluir el campo de versión.

---

## 34. Ejemplo completo mínimo: lista simple

```json
{
  "format_version": 1,
  "document": {
    "metadata": {
      "document_id": "doc_001",
      "name": "lista_simple_demo",
      "description": "Ejemplo de lista enlazada simple",
      "created_at": "2026-03-18T20:00:00",
      "updated_at": "2026-03-18T20:10:00"
    },
    "active_structure_id": "structure_1",
    "view_state": {
      "show_grid": true,
      "left_panel_visible": true,
      "right_panel_visible": true
    },
    "structures": [
      {
        "structure_id": "structure_1",
        "family": "list",
        "variant": "singly_linked_list",
        "config": {},
        "visual_state": {
          "layout_mode": "manual",
          "show_labels": true
        },
        "nodes": [
          {
            "node_id": "node_1",
            "kind": "list_node",
            "label": "N1",
            "value": 10,
            "value_type": "int",
            "visual": {
              "x": 100,
              "y": 120,
              "width": 90,
              "height": 40
            },
            "data": {
              "next_node_id": "node_2"
            }
          },
          {
            "node_id": "node_2",
            "kind": "list_node",
            "label": "N2",
            "value": 20,
            "value_type": "int",
            "visual": {
              "x": 260,
              "y": 120,
              "width": 90,
              "height": 40
            },
            "data": {
              "next_node_id": null
            }
          }
        ],
        "edges": [
          {
            "edge_id": "edge_1",
            "source_node_id": "node_1",
            "target_node_id": "node_2",
            "relation_kind": "next",
            "is_directed": true,
            "weight": null,
            "visual": {
              "show_arrow": true,
              "show_weight": false,
              "curve_kind": "straight"
            },
            "data": {}
          }
        ]
      }
    ]
  }
}
```

---

## 35. Ejemplo completo mínimo: grafo dirigido ponderado

```json
{
  "format_version": 1,
  "document": {
    "metadata": {
      "document_id": "doc_002",
      "name": "grafo_dirigido_ponderado",
      "description": "Ejemplo de grafo para pruebas",
      "created_at": "2026-03-18T21:00:00",
      "updated_at": "2026-03-18T21:10:00"
    },
    "active_structure_id": "graph_1",
    "view_state": {
      "show_grid": false,
      "left_panel_visible": true,
      "right_panel_visible": true
    },
    "structures": [
      {
        "structure_id": "graph_1",
        "family": "graph",
        "variant": "directed_weighted_graph",
        "config": {
          "is_directed": true,
          "is_weighted": true
        },
        "visual_state": {
          "layout_mode": "manual",
          "show_labels": true,
          "show_weights": true
        },
        "nodes": [
          {
            "node_id": "v1",
            "kind": "graph_vertex",
            "label": "A",
            "value": "A",
            "value_type": "string",
            "visual": {
              "x": 120,
              "y": 80,
              "width": 80,
              "height": 40
            },
            "data": {}
          },
          {
            "node_id": "v2",
            "kind": "graph_vertex",
            "label": "B",
            "value": "B",
            "value_type": "string",
            "visual": {
              "x": 320,
              "y": 180,
              "width": 80,
              "height": 40
            },
            "data": {}
          }
        ],
        "edges": [
          {
            "edge_id": "e1",
            "source_node_id": "v1",
            "target_node_id": "v2",
            "relation_kind": "graph_link",
            "is_directed": true,
            "weight": 5,
            "visual": {
              "show_arrow": true,
              "show_weight": true,
              "curve_kind": "straight"
            },
            "data": {}
          }
        ]
      }
    ]
  }
}
```

---

## 36. Estrategia de serialización por estructura

### 36.1 Base común
Toda estructura usa el mismo esqueleto general:
- `structure_id`
- `family`
- `variant`
- `config`
- `visual_state`
- `nodes`
- `edges`

### 36.2 Especialización
Las diferencias por estructura viven principalmente en:
- `variant`
- `config`
- `node.kind`
- `node.data`
- `edge.relation_kind`
- `edge.data`

### 36.3 Beneficio
Esto mantiene el formato uniforme sin impedir especialización.

---

## 37. Política de campos obligatorios y opcionales

### 37.1 Obligatorios
- `format_version`
- `document`
- `document.metadata`
- `document.active_structure_id`
- `document.structures`
- `structure_id`
- `family`
- `variant`
- `nodes`
- `edges`
- `node_id`
- `kind`
- `visual`

### 37.2 Opcionales o contextuales
- `description`
- `weight`
- `data` con contenido específico
- `show_weights`
- `curve_kind`
- timestamps detallados si luego decides simplificarlos

---

## 38. Reglas de carga tolerante

Para V1, el cargador puede ser conservador, pero idealmente debería:

- aceptar campos opcionales ausentes,
- rechazar campos obligatorios faltantes,
- rechazar referencias rotas,
- informar con claridad la causa del fallo,
- permitir evolución futura razonable del formato.

---

## 39. Resultado esperado del formato

Al finalizar la V1, el formato de archivo debe permitir:

- guardar cualquier documento soportado por el sistema,
- recargarlo sin perder coherencia,
- preservar la edición visual,
- y servir como base estable para el crecimiento futuro del proyecto.

---

## 40. Relación con otros documentos

Este archivo se complementa con:

- `03_architecture.md` → módulo de persistencia y responsabilidades
- `04_data_model.md` → entidades internas del documento
- `05_structure_definitions.md` → reglas específicas por estructura
- `06_operations.md` → operaciones que afectan el contenido persistido
- `08_rendering.md` → metadatos visuales necesarios para reconstrucción
