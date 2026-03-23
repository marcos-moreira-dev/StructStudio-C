# 06_operations.md

## 1. Propósito del documento

Este documento define las **operaciones semánticas** de **StructStudio C** para la primera versión (V1).

Su objetivo es describir, de forma clara y operativa:

- qué acciones puede ejecutar el usuario sobre cada estructura,
- qué entradas requiere cada operación,
- qué validaciones deben realizarse,
- qué cambios produce en el modelo,
- y qué resultado funcional debe observarse en el editor.

Este archivo no entra en detalles de implementación en C ni en pseudocódigo profundo. Su función es servir como contrato de comportamiento para el dominio, el editor y la interfaz.

---

## 2. Criterio general de definición

Cada operación se describirá, cuando aplique, usando esta estructura:

- **Nombre**
- **Estructura(s) aplicable(s)**
- **Propósito**
- **Entradas**
- **Precondiciones / validaciones**
- **Cambios en el modelo**
- **Resultado esperado en UI/editor**
- **Errores posibles**

---

## 3. Reglas generales para toda operación

### 3.1 Documento activo
Toda operación actúa sobre el documento activo y la estructura activa.

### 3.2 Contexto estructurado
Las operaciones deben respetar las reglas de la estructura actual.

### 3.3 Consistencia
Ninguna operación debe dejar la estructura en un estado incoherente si puede evitarse.

### 3.4 Validación previa o posterior
Dependiendo del caso, el sistema puede:
- impedir la operación antes de ejecutar,
- o ejecutarla y luego informar inconsistencias detectables.

### 3.5 Actualización visual
Toda operación que modifique el modelo debe reflejarse visualmente en el editor.

### 3.6 Persistencia diferida
Las operaciones modifican el documento en memoria; el guardado en archivo es una acción aparte.

---

## 4. Operaciones globales del documento

# 4.1 Crear documento nuevo

### Propósito
Crear un documento vacío listo para comenzar a trabajar.

### Entradas
- nombre opcional,
- tipo inicial de estructura opcional.

### Validaciones
- si existe un documento actual con cambios sin guardar, la UI puede pedir confirmación antes de reemplazarlo.

### Cambios en el modelo
- crea un nuevo `Document`,
- inicializa metadatos,
- crea estructura inicial vacía si se definió un tipo,
- reinicia selección y estado transitorio del editor.

### Resultado esperado
- canvas limpio,
- estructura activa lista para editar,
- barra de estado actualizada.

### Errores posibles
- fallo interno de inicialización.

---

# 4.2 Abrir documento

### Propósito
Cargar un documento previamente guardado.

### Entradas
- ruta de archivo JSON.

### Validaciones
- el archivo debe existir,
- el formato debe ser válido,
- la versión debe ser compatible,
- el contenido debe reconstruir una estructura coherente.

### Cambios en el modelo
- reemplaza el documento activo,
- reconstruye estructuras, nodos, relaciones y configuraciones,
- reinicia selección y estados transitorios.

### Resultado esperado
- documento visible en canvas,
- estructura activa restaurada,
- paneles sincronizados con el nuevo contenido.

### Errores posibles
- archivo inexistente,
- JSON inválido,
- formato no soportado,
- referencias inconsistentes.

---

# 4.3 Guardar documento

### Propósito
Persistir el documento actual en JSON.

### Entradas
- ruta actual del documento o ruta elegida por el usuario.

### Validaciones
- el documento activo debe existir,
- la estructura debe poder serializarse.

### Cambios en el modelo
- actualiza metadato de guardado si aplica.

### Resultado esperado
- archivo JSON generado o actualizado,
- confirmación visual en barra de estado o diálogo.

### Errores posibles
- ruta inválida,
- error de escritura,
- error de serialización.

---

# 4.4 Exportar PNG

### Propósito
Generar una imagen PNG de la representación visible actual.

### Entradas
- ruta de salida.

### Validaciones
- debe existir una estructura visible,
- el área a exportar debe poder renderizarse.

### Cambios en el modelo
- no modifica el dominio,
- puede actualizar metadatos de exportación si luego se desea.

### Resultado esperado
- imagen PNG generada,
- confirmación visual.

### Errores posibles
- ruta inválida,
- error de escritura,
- fallo de renderización/exportación.

---

## 5. Operaciones globales del editor

# 5.1 Seleccionar elemento

### Estructuras aplicables
Todas.

### Propósito
Marcar un elemento como foco de edición actual.

### Entradas
- referencia a nodo, arista o estructura.

### Validaciones
- el elemento debe existir en la estructura activa.

### Cambios en el modelo
- actualiza `SelectionState` o equivalente.

### Resultado esperado
- resaltado visual del elemento,
- panel derecho mostrando propiedades.

### Errores posibles
- selección de entidad inexistente.

---

# 5.2 Deseleccionar

### Estructuras aplicables
Todas.

### Propósito
Limpiar la selección actual.

### Entradas
No requiere.

### Cambios en el modelo
- limpia la selección.

### Resultado esperado
- ningún elemento resaltado,
- panel derecho vuelve al contexto general.

---

# 5.3 Mover nodo

### Estructuras aplicables
Todas las que usen nodos o elementos visuales movibles.

### Propósito
Cambiar la posición visual de un nodo en el canvas.

### Entradas
- nodo seleccionado,
- nueva posición.

### Validaciones
- el nodo debe existir,
- la nueva posición debe estar dentro de un rango razonable del canvas si se aplican límites.

### Cambios en el modelo
- actualiza `NodeVisual`.

### Resultado esperado
- el nodo cambia de lugar,
- relaciones conectadas se redibujan.

### Errores posibles
- nodo inexistente,
- operación bloqueada por estado del editor.

---

# 5.4 Eliminar selección

### Estructuras aplicables
Todas.

### Propósito
Eliminar el elemento seleccionado.

### Entradas
- selección actual.

### Validaciones
- debe existir selección válida,
- la eliminación no debe dejar referencias huérfanas sin tratamiento.

### Cambios en el modelo
- elimina nodo o arista,
- elimina o reajusta relaciones dependientes,
- limpia selección si corresponde.

### Resultado esperado
- el elemento desaparece del canvas,
- la estructura sigue coherente o se informa error.

### Errores posibles
- no hay selección,
- operación no permitida,
- estructura quedaría inconsistente.

---

# 5.5 Aplicar auto-layout

### Estructuras aplicables
Todas.

### Propósito
Reorganizar la disposición visual según las reglas del tipo de estructura.

### Entradas
- estructura activa.

### Validaciones
- debe existir estructura activa.

### Cambios en el modelo
- actualiza posiciones visuales de nodos y etiquetas.

### Resultado esperado
- disposición más clara y ordenada,
- sin cambiar el significado lógico de la estructura.

### Errores posibles
- estructura inválida o insuficiente para aplicar layout útil.

---

# 5.6 Validar estructura

### Estructuras aplicables
Todas.

### Propósito
Comprobar que la estructura cumple las reglas del tipo activo.

### Entradas
- estructura activa.

### Validaciones
No aplica; esta operación es la validación.

### Cambios en el modelo
- genera `StructureValidationResult`,
- puede actualizar overlays o mensajes visuales.

### Resultado esperado
- mensajes de errores o advertencias,
- resaltado opcional de entidades problemáticas.

### Errores posibles
- fallo interno de validación.

---

## 6. Operaciones comunes sobre nodos y relaciones

# 6.1 Crear nodo genérico

### Estructuras aplicables
Listas, árboles, grafos, sets, maps, heaps, stacks, queues y otras que lo permitan.

### Propósito
Crear una nueva entidad base dentro de la estructura activa.

### Entradas
- valor inicial opcional,
- tipo o subtipo de nodo,
- posición inicial opcional.

### Validaciones
- la estructura debe admitir creación manual de nodos,
- si el tipo requiere propiedades obligatorias, deben completarse.

### Cambios en el modelo
- crea `Node`,
- asigna ID,
- inicializa estado visual,
- registra en la estructura activa.

### Resultado esperado
- nuevo nodo visible y seleccionable.

### Errores posibles
- tipo inválido,
- datos insuficientes,
- restricción de estructura.

---

# 6.2 Editar valor de nodo

### Estructuras aplicables
Todas las que usen nodos editables.

### Propósito
Modificar el valor o etiqueta principal de un nodo.

### Entradas
- nodo,
- nuevo valor.

### Validaciones
- nodo existente,
- formato de valor compatible,
- no romper unicidad o reglas específicas.

### Cambios en el modelo
- actualiza valor,
- puede disparar revalidación.

### Resultado esperado
- texto actualizado en UI,
- posibles advertencias si la edición rompe reglas.

### Errores posibles
- valor inválido,
- duplicado no permitido,
- violación de reglas estructurales.

---

# 6.3 Crear relación

### Estructuras aplicables
Listas, árboles, grafos y otras que usen conexiones explícitas.

### Propósito
Conectar dos nodos según un tipo de relación permitido.

### Entradas
- nodo origen,
- nodo destino,
- tipo de relación,
- dirección,
- peso opcional.

### Validaciones
- ambos nodos existen,
- pertenecen a la misma estructura,
- la relación es compatible con el tipo,
- no se excede la cantidad máxima permitida,
- peso presente si es obligatorio.

### Cambios en el modelo
- crea `Edge`,
- actualiza datos especializados si corresponde.

### Resultado esperado
- conexión visible en canvas,
- estructura revalidada si aplica.

### Errores posibles
- tipo de relación inválido,
- nodo de destino inválido,
- límite de hijos o conexiones excedido,
- peso faltante en grafo ponderado.

---

# 6.4 Editar relación

### Estructuras aplicables
Listas, árboles, grafos.

### Propósito
Modificar propiedades de una relación existente.

### Entradas
- arista,
- nuevo peso,
- nueva dirección,
- nuevo tipo si se permite.

### Validaciones
- la arista existe,
- el cambio es compatible con la estructura,
- no rompe coherencia.

### Cambios en el modelo
- actualiza `Edge` y/o datos especializados.

### Resultado esperado
- representación visual actualizada.

### Errores posibles
- arista inexistente,
- edición incompatible.

---

# 6.5 Eliminar relación

### Estructuras aplicables
Listas, árboles, grafos.

### Propósito
Eliminar una conexión entre dos nodos.

### Entradas
- arista seleccionada o referencia a relación.

### Validaciones
- la relación debe existir,
- su eliminación no debe producir un estado imposible si la estructura exige conectividad mínima.

### Cambios en el modelo
- elimina la arista,
- limpia referencias derivadas si aplica.

### Resultado esperado
- desaparición visual de la relación,
- revalidación opcional de la estructura.

### Errores posibles
- relación inexistente,
- relación obligatoria para el tipo en el estado actual.

---

## 7. Operaciones del vector

# 7.1 Insertar en posición

### Propósito
Asignar un valor a una posición del vector.

### Entradas
- índice,
- valor.

### Validaciones
- índice válido,
- valor compatible con el tipo si se restringe.

### Cambios en el modelo
- crea o actualiza la celda correspondiente.

### Resultado esperado
- la celda muestra el nuevo valor.

### Errores posibles
- índice fuera de rango,
- tipo incompatible.

---

# 7.2 Reemplazar en posición

### Propósito
Cambiar el valor existente en una posición.

### Entradas
- índice,
- nuevo valor.

### Validaciones
- la posición debe existir.

### Cambios en el modelo
- actualiza la celda.

### Resultado esperado
- valor reemplazado visualmente.

### Errores posibles
- posición inexistente,
- valor inválido.

---

# 7.3 Eliminar en posición

### Propósito
Eliminar el contenido de una posición o marcarla vacía.

### Entradas
- índice.

### Validaciones
- índice válido.

### Cambios en el modelo
- vacía o elimina la entrada de la celda según diseño final.

### Resultado esperado
- posición sin contenido.

### Errores posibles
- índice inválido.

---

## 8. Operaciones de listas

# 8.1 Insertar al inicio

### Estructuras aplicables
Lista simple, doble, circular simple, circular doble.

### Propósito
Agregar un nuevo nodo al comienzo lógico de la lista.

### Entradas
- valor.

### Validaciones
- el valor debe ser válido,
- la lista debe existir.

### Cambios en el modelo
- crea nodo,
- reajusta cabeza,
- actualiza enlaces `next/prev` según variante,
- reajusta circularidad si aplica.

### Resultado esperado
- nuevo primer nodo visible.

### Errores posibles
- datos inválidos,
- fallo de reconexión.

---

# 8.2 Insertar al final

### Estructuras aplicables
Todas las variantes de lista.

### Propósito
Agregar un nuevo nodo al final lógico.

### Entradas
- valor.

### Validaciones
- valor válido.

### Cambios en el modelo
- crea nodo,
- actualiza cola o final lógico,
- reajusta enlaces.

### Resultado esperado
- nuevo último nodo visible.

### Errores posibles
- fallo de enlace,
- circularidad inconsistente.

---

# 8.3 Insertar después de nodo

### Estructuras aplicables
Lista simple, doble y variantes circulares.

### Propósito
Insertar un nodo después de un nodo existente.

### Entradas
- nodo de referencia,
- valor.

### Validaciones
- nodo de referencia existente.

### Cambios en el modelo
- crea nodo,
- reconfigura conexiones adyacentes.

### Resultado esperado
- nodo intermedio correctamente insertado.

### Errores posibles
- nodo inexistente,
- enlaces inconsistentes.

---

# 8.4 Insertar antes de nodo

### Estructuras aplicables
Lista doble y circular doble.

### Propósito
Insertar un nodo antes de otro nodo.

### Entradas
- nodo de referencia,
- valor.

### Validaciones
- nodo de referencia existente,
- la variante soporta `prev`.

### Cambios en el modelo
- crea nodo,
- actualiza `prev/next` adyacentes.

### Resultado esperado
- nodo visible en la posición lógica correcta.

### Errores posibles
- nodo inexistente,
- variante no compatible.

---

# 8.5 Eliminar nodo de lista

### Estructuras aplicables
Todas las variantes de lista.

### Propósito
Quitar un nodo y recomponer la continuidad.

### Entradas
- nodo objetivo.

### Validaciones
- nodo existente,
- la lista no queda en estado incoherente.

### Cambios en el modelo
- elimina nodo,
- recompone `next/prev`,
- actualiza cabeza/cola si aplica,
- mantiene o reajusta circularidad.

### Resultado esperado
- lista sigue siendo legible y coherente.

### Errores posibles
- nodo inexistente,
- fallo de recomposición.

---

# 8.6 Cerrar circularidad

### Estructuras aplicables
Listas circulares.

### Propósito
Conectar el final lógico con la cabeza.

### Entradas
No requiere o usa cabeza y final detectados.

### Validaciones
- deben existir al menos nodos suficientes,
- la variante debe ser circular.

### Cambios en el modelo
- crea o ajusta el enlace de cierre.

### Resultado esperado
- circularidad visible.

### Errores posibles
- lista vacía,
- variante incompatible.

---

# 8.7 Abrir circularidad

### Estructuras aplicables
Listas circulares.

### Propósito
Romper el cierre circular.

### Entradas
No requiere.

### Cambios en el modelo
- elimina la relación de cierre.

### Resultado esperado
- lista abierta.

### Errores posibles
- no existía cierre,
- variante incompatible.

---

## 9. Operaciones de stack

# 9.1 Push

### Propósito
Agregar un nuevo elemento al tope de la pila.

### Entradas
- valor.

### Validaciones
- valor válido.

### Cambios en el modelo
- crea nodo o entrada,
- actualiza tope,
- aumenta altura lógica.

### Resultado esperado
- nuevo nodo arriba,
- tope actualizado.

### Errores posibles
- valor inválido.

---

# 9.2 Pop

### Propósito
Retirar el elemento del tope.

### Entradas
No requiere.

### Validaciones
- la pila no debe estar vacía.

### Cambios en el modelo
- elimina el tope,
- actualiza referencia al nuevo tope.

### Resultado esperado
- pila reducida,
- nuevo tope visible.

### Errores posibles
- pila vacía.

---

# 9.3 Peek

### Propósito
Consultar el elemento del tope sin eliminarlo.

### Entradas
No requiere.

### Validaciones
- la pila no debe estar vacía.

### Cambios en el modelo
- no modifica el dominio.

### Resultado esperado
- el tope puede resaltarse o mostrarse en panel de propiedades.

### Errores posibles
- pila vacía.

---

## 10. Operaciones de queue

# 10.1 Enqueue

### Propósito
Agregar un elemento al final de la cola.

### Entradas
- valor.

### Validaciones
- valor válido.

### Cambios en el modelo
- crea elemento,
- actualiza final.

### Resultado esperado
- nuevo elemento al final.

### Errores posibles
- valor inválido.

---

# 10.2 Dequeue

### Propósito
Retirar el elemento del frente.

### Entradas
No requiere.

### Validaciones
- la cola no debe estar vacía.

### Cambios en el modelo
- elimina el frente,
- actualiza referencia de frente.

### Resultado esperado
- cola reducida,
- nuevo frente visible.

### Errores posibles
- cola vacía.

---

# 10.3 Front

### Propósito
Consultar el elemento del frente.

### Entradas
No requiere.

### Validaciones
- cola no vacía.

### Cambios en el modelo
- no modifica el dominio.

### Resultado esperado
- frente resaltado o mostrado.

### Errores posibles
- cola vacía.

---

# 10.4 Rear

### Propósito
Consultar el elemento del final.

### Entradas
No requiere.

### Validaciones
- cola no vacía.

### Cambios en el modelo
- no modifica el dominio.

### Resultado esperado
- final resaltado o mostrado.

### Errores posibles
- cola vacía.

---

## 11. Operaciones de priority queue

# 11.1 Insertar con prioridad

### Propósito
Agregar un elemento con un valor de prioridad.

### Entradas
- valor,
- prioridad.

### Validaciones
- prioridad presente y válida,
- valor válido.

### Cambios en el modelo
- crea elemento,
- registra prioridad,
- reordena o recalcula posición lógica.

### Resultado esperado
- el elemento aparece con prioridad visible.

### Errores posibles
- prioridad inválida,
- valor inválido.

---

# 11.2 Extraer prioritario

### Propósito
Retirar el elemento con mayor prioridad efectiva según la regla definida.

### Entradas
No requiere.

### Validaciones
- la estructura no debe estar vacía.

### Cambios en el modelo
- elimina el elemento prioritario,
- recalcula el siguiente prioritario.

### Resultado esperado
- nuevo prioritario visible.

### Errores posibles
- estructura vacía.

---

# 11.3 Editar prioridad

### Propósito
Cambiar la prioridad de un elemento existente.

### Entradas
- elemento,
- nueva prioridad.

### Validaciones
- elemento existente,
- prioridad válida.

### Cambios en el modelo
- actualiza prioridad,
- reordena la estructura si es necesario.

### Resultado esperado
- la posición lógica o visual puede cambiar.

### Errores posibles
- elemento inexistente,
- prioridad inválida.

---

## 12. Operaciones de árboles

# 12.1 Crear raíz

### Estructuras aplicables
Árbol binario, BST, AVL, heap cuando tenga sentido inicializarlo así.

### Propósito
Crear el nodo raíz de una estructura vacía.

### Entradas
- valor.

### Validaciones
- la estructura no debe tener raíz aún, o la operación debe redefinirse como reemplazo explícito.

### Cambios en el modelo
- crea nodo raíz,
- lo marca como tal.

### Resultado esperado
- nodo visible en la parte superior del árbol.

### Errores posibles
- raíz ya existente,
- valor inválido.

---

# 12.2 Insertar hijo izquierdo

### Estructuras aplicables
Árbol binario y variantes.

### Propósito
Agregar un hijo izquierdo a un nodo.

### Entradas
- nodo padre,
- valor.

### Validaciones
- padre existente,
- el padre no debe tener ya hijo izquierdo,
- la operación debe respetar reglas del tipo si es BST/AVL.

### Cambios en el modelo
- crea nodo hijo,
- actualiza relación `left`.

### Resultado esperado
- nuevo hijo izquierdo visible.

### Errores posibles
- padre inexistente,
- hijo izquierdo ya ocupado,
- violación de regla BST.

---

# 12.3 Insertar hijo derecho

### Estructuras aplicables
Árbol binario y variantes.

### Propósito
Agregar un hijo derecho a un nodo.

### Entradas
- nodo padre,
- valor.

### Validaciones
- padre existente,
- el padre no debe tener ya hijo derecho,
- debe respetarse la variante.

### Cambios en el modelo
- crea nodo hijo,
- actualiza relación `right`.

### Resultado esperado
- nuevo hijo derecho visible.

### Errores posibles
- padre inexistente,
- hijo derecho ya ocupado,
- violación de regla BST.

---

# 12.4 Eliminar nodo de árbol

### Estructuras aplicables
Árbol binario, BST, AVL, heap cuando sea compatible con la política elegida.

### Propósito
Eliminar un nodo y reajustar la estructura según reglas del tipo.

### Entradas
- nodo objetivo.

### Validaciones
- nodo existente,
- la operación debe tener una política definida si el nodo tiene hijos.

### Cambios en el modelo
- elimina nodo,
- reajusta relaciones,
- puede disparar rebalanceo o revalidación.

### Resultado esperado
- árbol coherente tras eliminación.

### Errores posibles
- nodo inexistente,
- eliminación no soportada aún para cierto caso complejo.

---

## 13. Operaciones de BST

# 13.1 Insertar valor BST

### Propósito
Insertar un valor respetando la propiedad de orden del BST.

### Entradas
- valor.

### Validaciones
- valor válido,
- política de duplicados definida.

### Cambios en el modelo
- crea nodo,
- lo ubica según comparaciones,
- actualiza relaciones.

### Resultado esperado
- árbol BST visualmente actualizado.

### Errores posibles
- valor incompatible,
- duplicado no permitido.

---

# 13.2 Buscar valor BST

### Propósito
Encontrar un valor dentro del BST.

### Entradas
- valor buscado.

### Validaciones
- árbol existente.

### Cambios en el modelo
- no modifica el dominio,
- puede actualizar estado de selección o resaltado.

### Resultado esperado
- nodo encontrado resaltado,
- mensaje si no existe.

### Errores posibles
- árbol vacío.

---

# 13.3 Eliminar valor BST

### Propósito
Eliminar un valor respetando la semántica del BST.

### Entradas
- valor o nodo objetivo.

### Validaciones
- el nodo debe existir,
- debe aplicarse la política de eliminación escogida.

### Cambios en el modelo
- elimina o reemplaza nodo,
- reacomoda relaciones,
- revalida el BST.

### Resultado esperado
- BST consistente.

### Errores posibles
- valor inexistente,
- caso de eliminación no resuelto aún.

---

## 14. Operaciones de AVL

# 14.1 Insertar valor AVL

### Propósito
Insertar un valor y mantener el balance AVL.

### Entradas
- valor.

### Validaciones
- valor válido,
- duplicados según política.

### Cambios en el modelo
- inserta como BST,
- recalcula balance,
- aplica rotaciones lógicas si corresponde.

### Resultado esperado
- árbol AVL válido y actualizado.

### Errores posibles
- valor inválido,
- duplicado no permitido,
- fallo al recalcular balance.

---

# 14.2 Eliminar valor AVL

### Propósito
Eliminar un valor y mantener balance AVL.

### Entradas
- valor o nodo.

### Validaciones
- nodo existente.

### Cambios en el modelo
- elimina según política,
- rebalancea si es necesario.

### Resultado esperado
- árbol AVL consistente.

### Errores posibles
- valor inexistente,
- error de reestructuración.

---

# 14.3 Rebalancear AVL

### Propósito
Reaplicar balanceo sobre la estructura AVL actual.

### Entradas
No requiere o usa la estructura activa.

### Validaciones
- la estructura debe ser AVL.

### Cambios en el modelo
- recalcula balance,
- aplica rotaciones necesarias.

### Resultado esperado
- árbol balanceado visualmente.

### Errores posibles
- estructura no AVL,
- fallo interno de reestructuración.

---

## 15. Operaciones de heap

# 15.1 Insertar valor en heap

### Propósito
Agregar un valor manteniendo la propiedad del heap.

### Entradas
- valor.

### Validaciones
- valor válido.

### Cambios en el modelo
- agrega nodo,
- reajusta posiciones lógicas,
- aplica `heapify` local si corresponde.

### Resultado esperado
- heap coherente con nuevo valor.

### Errores posibles
- valor inválido.

---

# 15.2 Extraer principal

### Propósito
Retirar el valor principal del heap.

### Entradas
No requiere.

### Validaciones
- heap no vacío.

### Cambios en el modelo
- elimina raíz lógica,
- reestructura el heap,
- aplica `heapify`.

### Resultado esperado
- nuevo principal visible en la raíz.

### Errores posibles
- heap vacío.

---

# 15.3 Consultar principal

### Propósito
Consultar el elemento dominante del heap.

### Entradas
No requiere.

### Validaciones
- heap no vacío.

### Cambios en el modelo
- no modifica el dominio.

### Resultado esperado
- raíz resaltada o mostrada.

### Errores posibles
- heap vacío.

---

# 15.4 Heapify

### Propósito
Reordenar la estructura para restaurar la propiedad del heap.

### Entradas
- estructura activa.

### Validaciones
- la estructura debe ser heap.

### Cambios en el modelo
- reajusta orden de nodos y/o relaciones lógicas.

### Resultado esperado
- heap válido.

### Errores posibles
- estructura incompatible.

---

## 16. Operaciones de set

# 16.1 Agregar elemento

### Propósito
Añadir un nuevo valor al conjunto.

### Entradas
- valor.

### Validaciones
- el valor no debe existir ya.

### Cambios en el modelo
- crea elemento nuevo.

### Resultado esperado
- elemento visible en el set.

### Errores posibles
- duplicado no permitido.

---

# 16.2 Eliminar elemento

### Propósito
Quitar un valor del conjunto.

### Entradas
- valor o nodo objetivo.

### Validaciones
- el elemento debe existir.

### Cambios en el modelo
- elimina el elemento.

### Resultado esperado
- set actualizado.

### Errores posibles
- elemento inexistente.

---

# 16.3 Verificar existencia

### Propósito
Consultar si un valor pertenece al conjunto.

### Entradas
- valor.

### Validaciones
No requiere especiales.

### Cambios en el modelo
- no modifica el dominio,
- puede actualizar resaltado o mensaje.

### Resultado esperado
- confirmación visual o textual.

### Errores posibles
- ninguno relevante fuera de formato inválido.

---

## 17. Operaciones de map

# 17.1 Insertar par clave-valor

### Propósito
Agregar una nueva entrada al map.

### Entradas
- clave,
- valor.

### Validaciones
- la clave no debe existir,
- la clave y el valor deben ser válidos.

### Cambios en el modelo
- crea entrada especializada,
- la agrega a la estructura.

### Resultado esperado
- nueva entrada visible.

### Errores posibles
- clave duplicada,
- datos inválidos.

---

# 17.2 Actualizar valor por clave

### Propósito
Cambiar el valor asociado a una clave existente.

### Entradas
- clave,
- nuevo valor.

### Validaciones
- la clave debe existir.

### Cambios en el modelo
- actualiza la entrada correspondiente.

### Resultado esperado
- valor actualizado en UI.

### Errores posibles
- clave inexistente,
- valor inválido.

---

# 17.3 Buscar por clave

### Propósito
Encontrar una entrada usando su clave.

### Entradas
- clave.

### Validaciones
No requiere especiales además del formato.

### Cambios en el modelo
- no modifica el dominio,
- puede resaltar la entrada encontrada.

### Resultado esperado
- entrada encontrada resaltada o mensaje de ausencia.

### Errores posibles
- clave con formato inválido.

---

# 17.4 Eliminar por clave

### Propósito
Eliminar una entrada del map.

### Entradas
- clave.

### Validaciones
- la clave debe existir.

### Cambios en el modelo
- elimina la entrada.

### Resultado esperado
- map actualizado.

### Errores posibles
- clave inexistente.

---

# 17.5 Editar clave

### Propósito
Cambiar la clave de una entrada.

### Entradas
- entrada o clave actual,
- nueva clave.

### Validaciones
- la nueva clave no debe repetirse,
- la entrada debe existir.

### Cambios en el modelo
- actualiza la clave.

### Resultado esperado
- entrada visible con nueva clave.

### Errores posibles
- clave duplicada,
- entrada inexistente.

---

## 18. Operaciones de graph

# 18.1 Agregar vértice

### Propósito
Crear un nuevo vértice en el grafo.

### Entradas
- etiqueta o valor,
- posición inicial opcional.

### Validaciones
- datos válidos,
- unicidad de etiqueta si se decide exigirla en cierto contexto.

### Cambios en el modelo
- crea nodo tipo vértice,
- asigna posición visual.

### Resultado esperado
- vértice visible y seleccionable.

### Errores posibles
- datos inválidos.

---

# 18.2 Eliminar vértice

### Propósito
Eliminar un vértice y sus aristas asociadas.

### Entradas
- vértice objetivo.

### Validaciones
- vértice existente.

### Cambios en el modelo
- elimina el vértice,
- elimina aristas incidentes.

### Resultado esperado
- grafo actualizado sin conexiones huérfanas.

### Errores posibles
- vértice inexistente.

---

# 18.3 Editar vértice

### Propósito
Modificar etiqueta, valor o propiedades del vértice.

### Entradas
- vértice,
- nuevos datos.

### Validaciones
- vértice existente,
- formato válido.

### Cambios en el modelo
- actualiza el nodo.

### Resultado esperado
- vértice actualizado visualmente.

### Errores posibles
- vértice inexistente,
- datos inválidos.

---

# 18.4 Agregar arista

### Propósito
Conectar dos vértices.

### Entradas
- vértice origen,
- vértice destino,
- peso opcional,
- tipo de relación si aplica.

### Validaciones
- ambos vértices existen,
- el tipo de grafo admite esa arista,
- el peso está presente si el grafo es ponderado.

### Cambios en el modelo
- crea arista,
- registra dirección y peso según configuración.

### Resultado esperado
- conexión visible entre vértices.

### Errores posibles
- vértices inválidos,
- peso faltante,
- relación incompatible.

---

# 18.5 Eliminar arista

### Propósito
Quitar una conexión entre vértices.

### Entradas
- arista objetivo.

### Validaciones
- la arista existe.

### Cambios en el modelo
- elimina la arista.

### Resultado esperado
- conexión removida del canvas.

### Errores posibles
- arista inexistente.

---

# 18.6 Editar arista

### Propósito
Cambiar peso, etiqueta u otras propiedades de una arista.

### Entradas
- arista,
- nuevos datos.

### Validaciones
- la arista debe existir,
- el tipo de grafo debe admitir el cambio.

### Cambios en el modelo
- actualiza la arista.

### Resultado esperado
- arista actualizada visualmente.

### Errores posibles
- arista inexistente,
- peso inválido,
- operación incompatible.

---

# 18.7 Cambiar dirección de arista

### Propósito
Modificar orientación de una arista cuando el grafo sea dirigido.

### Entradas
- arista,
- nueva orientación.

### Validaciones
- el grafo debe ser dirigido,
- la arista debe existir.

### Cambios en el modelo
- actualiza `source/target` o bandera de dirección según diseño.

### Resultado esperado
- flecha visual actualizada.

### Errores posibles
- grafo no dirigido,
- arista inexistente.

---

# 18.8 Asignar o editar peso

### Propósito
Establecer o modificar el peso de una arista en un grafo ponderado.

### Entradas
- arista,
- peso.

### Validaciones
- el grafo debe ser ponderado,
- el peso debe tener formato válido.

### Cambios en el modelo
- actualiza el peso.

### Resultado esperado
- etiqueta de peso visible y actualizada.

### Errores posibles
- grafo no ponderado,
- peso inválido,
- arista inexistente.

---

## 19. Operaciones de mantenimiento y soporte

# 19.1 Limpiar estructura

### Estructuras aplicables
Todas.

### Propósito
Vaciar el contenido lógico de la estructura activa sin cambiar necesariamente su tipo.

### Entradas
No requiere.

### Validaciones
- confirmación del usuario si se considera necesario.

### Cambios en el modelo
- elimina nodos y aristas,
- conserva configuración del tipo si corresponde.

### Resultado esperado
- estructura vacía lista para reutilizar.

### Errores posibles
- fallo interno de limpieza.

---

# 19.2 Reinicializar estructura actual

### Estructuras aplicables
Todas.

### Propósito
Restablecer la estructura activa a un estado base limpio.

### Entradas
No requiere.

### Cambios en el modelo
- limpia contenido,
- restablece configuración por defecto de la variante.

### Resultado esperado
- estructura en estado inicial.

### Errores posibles
- fallo interno de reinicialización.

---

## 20. Resultado esperado del documento

Este documento debe servir como base para:

- implementar operaciones del dominio,
- diseñar menús y acciones de UI,
- definir comandos del editor,
- establecer validaciones previas y posteriores,
- y preparar pruebas funcionales sobre el comportamiento del sistema.

---

## 21. Relación con otros documentos

Este archivo se complementa con:

- `01_product_spec.md` → catálogo funcional general
- `03_architecture.md` → módulos y responsabilidades
- `04_data_model.md` → entidades y relaciones en memoria
- `05_structure_definitions.md` → reglas por estructura
- `07_file_format.md` → persistencia JSON
- `09_editor_logic.md` → interacción del editor y estados transitorios

