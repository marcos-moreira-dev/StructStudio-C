# 01_product_spec.md

## 1. Propósito funcional

Este documento define el comportamiento funcional de **StructStudio C** en su primera versión (V1).

La aplicación permitirá crear, editar, validar, guardar y exportar **documentos de estructuras de datos 2D** con enfoque educativo, mediante una interfaz gráfica de escritorio clásica y un modo de trabajo estructurado.

Este archivo no describe detalles internos de implementación ni decisiones profundas de arquitectura; su objetivo es especificar **qué hace el producto**, **qué puede hacer el usuario** y **qué operaciones estarán disponibles** por tipo de estructura.

---

## 2. Resumen funcional del producto

StructStudio C es una aplicación de escritorio en C que permite:

- crear un documento nuevo,
- seleccionar un tipo de estructura de datos,
- agregar, editar y eliminar elementos de esa estructura,
- aplicar operaciones semánticas propias de cada tipo,
- visualizar la estructura en 2D,
- guardar el documento en JSON,
- volver a abrirlo posteriormente,
- y exportar la representación visual a PNG.

La aplicación trabaja con **un solo documento visible a la vez**.

---

## 3. Principios funcionales

### 3.1 Modo estructurado
El sistema no será un editor libre de diagramas. Cada documento o estructura activa estará sujeta a reglas propias del tipo seleccionado.

### 3.2 Representación 2D
Todas las estructuras se representarán en un plano 2D.

### 3.3 Inteligencia semántica
La aplicación no solo dibuja nodos y conexiones: entiende las restricciones y operaciones propias de cada estructura.

### 3.4 Persistencia local
Todo el trabajo se guarda localmente en archivos JSON.

### 3.5 Exportación visual
La estructura visible podrá exportarse como imagen PNG.

---

## 4. Flujo principal de uso

El flujo funcional esperado en V1 es el siguiente:

1. El usuario abre la aplicación.
2. Crea un documento nuevo o abre uno existente.
3. Selecciona el tipo de estructura a trabajar.
4. Inserta elementos o aplica operaciones propias de la estructura.
5. Edita propiedades del nodo, arista o estructura si corresponde.
6. Reorganiza visualmente la representación si es necesario.
7. Guarda el documento en JSON.
8. Exporta la representación como PNG si lo desea.

---

## 5. Catálogo funcional de V1

### 5.1 Funciones globales de documento

La V1 deberá permitir:

- crear documento nuevo,
- abrir documento existente,
- guardar documento,
- guardar como,
- cerrar documento actual,
- exportar a PNG,
- limpiar el área de trabajo,
- cambiar la estructura activa dentro del documento,
- mostrar información básica del documento.

### 5.2 Funciones globales del editor

La V1 deberá permitir:

- seleccionar un elemento,
- deseleccionar,
- mover elementos visuales,
- eliminar elementos,
- editar propiedades,
- aplicar auto-layout,
- mostrar errores o restricciones violadas,
- visualizar el tipo de estructura activa,
- visualizar el estado actual del editor.

---

## 6. Tipos de estructuras contempladas en V1

La primera versión contemplará las siguientes familias y variantes:

### 6.1 Arreglos / vectores
- vector

### 6.2 Listas enlazadas
- lista enlazada simple
- lista enlazada doble
- lista circular simple
- lista circular doble

### 6.3 Pilas
- stack

### 6.4 Colas
- queue
- circular queue
- priority queue

### 6.5 Árboles
- árbol binario
- BST
- AVL

### 6.6 Heaps
- heap

### 6.7 Sets
- set

### 6.8 Maps
- map

### 6.9 Grafos
- grafo dirigido
- grafo no dirigido
- grafo ponderado
- grafo no ponderado

> Nota: algunas variantes podrán compartir base funcional y diferir por reglas, restricciones y operaciones disponibles.

---

## 7. Operaciones funcionales por familia

Este apartado describe el conjunto funcional esperado. El detalle fino de reglas y validaciones se documentará en archivos especializados posteriores.

---

## 7.1 Vector

### Objetivo funcional
Representar una secuencia indexada de elementos en disposición lineal.

### Operaciones funcionales mínimas
- crear vector
- insertar valor en posición
- reemplazar valor en posición
- eliminar valor en posición
- leer valor en posición
- redimensionar visualmente el vector si aplica

### Restricciones funcionales
- los índices deben ser válidos,
- cada posición representa una celda,
- no existen enlaces tipo puntero entre elementos.

---

## 7.2 Lista enlazada simple

### Operaciones funcionales mínimas
- crear nodo
- insertar al inicio
- insertar al final
- insertar después de un nodo
- eliminar nodo
- editar valor
- conectar siguiente
- desconectar siguiente

### Restricciones funcionales
- cada nodo puede tener como máximo un enlace `next`,
- no debe haber ambigüedad de siguiente dentro del modo estructurado,
- la lista debe poder representarse linealmente.

---

## 7.3 Lista enlazada doble

### Operaciones funcionales mínimas
- crear nodo
- insertar al inicio
- insertar al final
- insertar antes o después de un nodo
- eliminar nodo
- editar valor
- conectar `prev`
- conectar `next`
- desconectar enlaces

### Restricciones funcionales
- cada nodo puede tener como máximo un `prev` y un `next`,
- la relación entre nodos debe mantener coherencia bidireccional.

---

## 7.4 Lista circular simple

### Operaciones funcionales mínimas
- crear nodo
- insertar nodo
- eliminar nodo
- editar valor
- cerrar circularidad
- abrir circularidad

### Restricciones funcionales
- la lista mantiene un único ciclo principal,
- cada nodo tiene como máximo un `next`.

---

## 7.5 Lista circular doble

### Operaciones funcionales mínimas
- crear nodo
- insertar nodo
- eliminar nodo
- editar valor
- cerrar circularidad
- abrir circularidad

### Restricciones funcionales
- cada nodo tiene como máximo un `prev` y un `next`,
- la circularidad debe mantenerse consistente.

---

## 7.6 Stack

### Operaciones funcionales mínimas
- push
- pop
- peek
- editar valor de nodo
- limpiar stack

### Restricciones funcionales
- la estructura debe mantener orden LIFO,
- el nodo superior debe identificarse claramente.

---

## 7.7 Queue

### Operaciones funcionales mínimas
- enqueue
- dequeue
- front
- rear
- editar valor
- limpiar cola

### Restricciones funcionales
- la estructura debe mantener orden FIFO,
- deben identificarse frente y final.

---

## 7.8 Circular Queue

### Operaciones funcionales mínimas
- enqueue
- dequeue
- front
- rear
- marcar continuidad circular
- editar valor

### Restricciones funcionales
- el modelo debe representar continuidad circular,
- deben mantenerse referencias coherentes de frente y final.

---

## 7.9 Priority Queue

### Operaciones funcionales mínimas
- insertar elemento con prioridad
- extraer elemento prioritario
- consultar elemento prioritario
- editar valor
- editar prioridad

### Restricciones funcionales
- cada elemento debe tener prioridad,
- la representación debe dejar clara la prioridad u orden resultante.

---

## 7.10 Árbol binario

### Operaciones funcionales mínimas
- crear nodo raíz
- insertar hijo izquierdo
- insertar hijo derecho
- eliminar nodo
- editar valor
- cambiar relación padre-hijo

### Restricciones funcionales
- cada nodo puede tener como máximo dos hijos,
- debe distinguirse entre hijo izquierdo y derecho.

---

## 7.11 BST

### Operaciones funcionales mínimas
- insertar valor
- buscar valor
- eliminar valor
- editar valor
- reorganizar si la edición rompe reglas

### Restricciones funcionales
- debe cumplirse la propiedad de orden del BST,
- las operaciones deben respetar la semántica de búsqueda binaria.

---

## 7.12 AVL

### Operaciones funcionales mínimas
- insertar valor
- eliminar valor
- buscar valor
- rebalancear
- editar valor con verificación posterior

### Restricciones funcionales
- debe cumplirse la propiedad BST,
- debe controlarse el balance del árbol,
- la estructura debe poder reflejar rotaciones internas sin animación paso a paso.

---

## 7.13 Heap

### Operaciones funcionales mínimas
- insertar valor
- extraer elemento principal
- consultar elemento principal
- heapify / reordenar
- editar valor con reajuste

### Restricciones funcionales
- debe mantenerse la propiedad del heap,
- la representación será tipo árbol.

---

## 7.14 Set

### Operaciones funcionales mínimas
- agregar elemento
- eliminar elemento
- verificar existencia
- editar valor
- limpiar set

### Restricciones funcionales
- no se permiten duplicados,
- la visualización debe dejar claro que se trata de una colección sin repetición.

---

## 7.15 Map

### Operaciones funcionales mínimas
- insertar par clave-valor
- actualizar valor por clave
- buscar por clave
- eliminar por clave
- editar clave o valor con validación

### Restricciones funcionales
- cada clave debe ser única,
- la representación debe mostrar asociación clave-valor.

---

## 7.16 Grafos

### Variantes funcionales
- dirigido
- no dirigido
- ponderado
- no ponderado

### Operaciones funcionales mínimas
- agregar vértice
- eliminar vértice
- editar vértice
- agregar arista
- eliminar arista
- editar arista
- cambiar dirección de arista cuando aplique
- asignar o editar peso cuando aplique

### Restricciones funcionales
- la dirección debe ser visible en grafos dirigidos,
- el peso debe ser visible en grafos ponderados,
- un grafo no dirigido no debe mostrar dirección,
- el sistema debe permitir una representación 2D clara de vértices y aristas.

---

## 8. Funciones comunes de edición visual

La aplicación deberá permitir, según corresponda al tipo de estructura:

- seleccionar nodo,
- seleccionar arista,
- mover nodo,
- editar etiqueta o valor,
- eliminar elemento,
- reorganizar automáticamente,
- mostrar relaciones entre elementos,
- mostrar errores estructurales básicos,
- refrescar la vista del documento.

---

## 9. Funciones de validación

La V1 deberá validar, como mínimo:

- tipos de relación inválidos,
- cantidad máxima de conexiones permitidas por nodo,
- duplicados no permitidos,
- claves repetidas,
- ruptura de reglas de BST,
- ruptura de balance AVL cuando corresponda,
- incoherencias de circularidad,
- inconsistencias de dirección o peso en grafos.

La validación puede ser:
- inmediata al editar,
- o invocada mediante acciones del usuario,
según lo que resulte más estable en la implementación.

---

## 10. Funciones de persistencia

### Guardado
El usuario podrá guardar el documento actual en formato JSON.

### Carga
El usuario podrá abrir un documento JSON previamente guardado.

### Requisitos funcionales
- preservar tipo de estructura,
- preservar nodos y relaciones,
- preservar metadatos visuales necesarios,
- preservar configuración particular del tipo activo.

---

## 11. Funciones de exportación

La V1 permitirá exportar la representación visual actual a PNG.

### Requisitos funcionales
- exportar lo visible del documento o estructura activa,
- mantener claridad visual,
- incluir texto, nodos y relaciones visibles.

---

## 12. Restricciones de V1

La V1 no incluirá:

- animaciones paso a paso de algoritmos,
- reproducción automática de recorridos,
- múltiples documentos abiertos simultáneamente,
- colaboración en red,
- backend,
- persistencia remota,
- scripting,
- plugins externos,
- soporte 3D.

---

## 13. Criterios de éxito funcional

La V1 se considerará funcionalmente exitosa si permite:

- trabajar con un documento a la vez,
- crear y editar estructuras 2D con reglas claras,
- ejecutar operaciones semánticas básicas por tipo,
- guardar y cargar documentos,
- exportar a PNG,
- y mantener una experiencia coherente de aplicación de escritorio clásica.

---

## 14. Relación con otros documentos

Este archivo se complementará con los siguientes documentos del proyecto:

- `00_overview.md` → visión general
- `02_ui_ux.md` → diseño de interfaz y experiencia de usuario
- `03_architecture.md` → arquitectura del sistema
- `04_data_model.md` → modelo de datos
- `05_structure_definitions.md` → reglas detalladas por estructura
- `06_operations.md` → definición detallada de operaciones
- `07_file_format.md` → formato JSON
- `12_roadmap.md` → fases de desarrollo

