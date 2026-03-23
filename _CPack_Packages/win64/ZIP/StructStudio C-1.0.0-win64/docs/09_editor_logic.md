# 09_editor_logic.md

## 1. Propósito del documento

Este documento define la **lógica del editor** de **StructStudio C** para la primera versión (V1).

Su objetivo es describir:

- cómo se comporta el editor sobre el canvas,
- qué estados transitorios existen,
- cómo se interpretan las acciones del usuario,
- cómo se coordinan la UI, el editor, el modelo y el render,
- y qué flujo sigue cada interacción principal.

Este archivo no describe detalles internos de implementación de libui-ng ni del motor de render. Se centra en la lógica de interacción que convierte acciones de usuario en cambios sobre el documento activo.

---

## 2. Rol del editor dentro del sistema

El editor es el subsistema que conecta:

- la interfaz gráfica,
- el documento activo,
- las operaciones del dominio,
- el render,
- y parte del feedback contextual al usuario.

Su función principal es coordinar la edición del documento sin mezclar en un solo lugar:

- widgets de UI,
- reglas del dominio,
- persistencia,
- ni dibujo puro.

El editor interpreta intención de usuario y decide qué acción semántica ejecutar.

---

## 3. Responsabilidades del editor

El editor debe encargarse de:

- gestionar la selección,
- gestionar la herramienta activa,
- interpretar clics y arrastres,
- crear y eliminar entidades mediante operaciones válidas,
- conectar nodos cuando el tipo de estructura lo permita,
- coordinar edición de propiedades,
- disparar validaciones,
- solicitar auto-layout,
- mantener estados transitorios de interacción,
- y pedir redibujado cuando el documento cambie.

El editor no debe:

- dibujar directamente,
- serializar archivos,
- imponer por sí solo reglas del dominio fuera del `core`,
- ni decidir detalles visuales de bajo nivel.

---

## 4. Principios de interacción del editor

### 4.1 Un documento activo a la vez
Toda interacción ocurre sobre un único documento visible.

### 4.2 Una estructura activa a la vez
Aunque un documento pueda contener varias estructuras, la edición se centra en una estructura activa.

### 4.3 Modo estructurado
El editor no es un lienzo libre genérico: toda interacción está condicionada por el tipo de estructura activa.

### 4.4 Feedback inmediato
Toda acción relevante debe tener reflejo visual y, cuando aplique, textual.

### 4.5 Coherencia entre intención y resultado
El usuario debe sentir que la herramienta responde de forma predecible.

### 4.6 Estados simples pero claros
La V1 debe evitar una máquina de estados demasiado compleja, pero sí definir lo esencial con precisión.

---

## 5. Estado general del editor

A nivel conceptual, el editor trabajará con un `EditorState` o estructura equivalente.

Ese estado deberá incluir, como mínimo:

- documento activo,
- estructura activa,
- herramienta activa,
- selección actual,
- estado de arrastre,
- estado de conexión,
- operación contextual en curso,
- resultado de validación reciente,
- indicadores de refresco visual.

---

## 6. Componentes principales del estado del editor

### 6.1 `ToolState`
Indica qué herramienta está activa.

### 6.2 `SelectionState`
Indica qué entidad está seleccionada.

### 6.3 `DragState`
Indica si se está arrastrando una entidad y con qué datos.

### 6.4 `ConnectionState`
Indica si el usuario está en proceso de crear una relación entre nodos.

### 6.5 `OperationState`
Indica si hay una operación contextual en curso o pendiente de completar.

### 6.6 `ValidationState`
Contiene el último resultado de validación relevante para UI y render.

---

## 7. Herramientas del editor

La V1 debe operar con un conjunto controlado de herramientas.

### 7.1 Herramientas mínimas propuestas
- `select`
- `insert`
- `connect`
- `delete`
- `move` implícita mediante selección + arrastre
- `auto_layout` como acción más que como herramienta permanente

### 7.2 Regla general
No todas las herramientas aplican a todas las estructuras del mismo modo.

Ejemplo:
- `connect` tiene mucho sentido en listas, árboles y grafos,
- pero no es central en un vector,
- y en stack/queue puede quedar más subordinada a operaciones semánticas.

---

## 8. `ToolState`

### 8.1 Propósito
Representar la herramienta actualmente activa en el editor.

### 8.2 Posibles valores en V1
- `TOOL_SELECT`
- `TOOL_INSERT`
- `TOOL_CONNECT`
- `TOOL_DELETE`
- `TOOL_NONE` en estados especiales si hace falta

### 8.3 Reglas
- debe existir siempre una herramienta activa razonable,
- `select` puede ser la predeterminada,
- al cambiar de estructura, puede mantenerse la herramienta o reiniciarse según la política elegida.

---

## 9. `SelectionState`

### 9.1 Propósito
Representar el foco actual de edición.

### 9.2 Tipos de selección
- ninguna,
- nodo,
- arista,
- estructura.

### 9.3 Campos conceptuales
- `selection_type`
- `selected_structure_id`
- `selected_node_id`
- `selected_edge_id`

### 9.4 Reglas
- en V1 se prioriza selección simple,
- la selección múltiple no es necesaria inicialmente,
- cambiar selección debe reflejarse inmediatamente en la UI.

---

## 10. `DragState`

### 10.1 Propósito
Representar un arrastre de entidad visual.

### 10.2 Campos conceptuales sugeridos
- `is_dragging`
- `dragged_node_id`
- `start_mouse_x`
- `start_mouse_y`
- `start_node_x`
- `start_node_y`

### 10.3 Reglas
- el arrastre aplica principalmente a nodos,
- una arista no se arrastra como entidad principal en V1 salvo casos especiales,
- si el usuario arrastra un nodo, el editor actualiza su estado visual y solicita redibujado.

---

## 11. `ConnectionState`

### 11.1 Propósito
Representar una conexión en proceso entre dos nodos.

### 11.2 Campos conceptuales sugeridos
- `is_connecting`
- `source_node_id`
- `relation_kind_candidate`
- `connection_preview_position`

### 11.3 Flujo típico
1. el usuario activa herramienta `connect`,
2. selecciona nodo origen,
3. el editor entra en estado de conexión,
4. el usuario elige nodo destino,
5. el editor intenta crear la relación.

### 11.4 Reglas
- solo aplica a estructuras que admiten conexiones explícitas,
- si la operación es inválida, debe cancelarse con mensaje claro.

---

## 12. `OperationState`

### 12.1 Propósito
Representar una operación semántica pendiente o contextual.

### 12.2 Uso típico
Algunas acciones no se resuelven con solo un clic, por ejemplo:
- insertar después de un nodo,
- conectar origen y destino,
- editar peso de arista,
- insertar hijo izquierdo o derecho,
- cambiar clave de map.

### 12.3 Beneficio
Permite que el editor gestione mini flujos sin volver todo caótico.

### 12.4 Política recomendada
En V1, este estado debe mantenerse simple y explícito.

---

## 13. `ValidationState`

### 13.1 Propósito
Guardar el resultado más reciente de validación relevante.

### 13.2 Contenido posible
- último resultado completo,
- lista resumida de errores,
- entidades asociadas al error,
- si hay overlay activo de advertencia.

### 13.3 Uso
- mostrar mensajes,
- resaltar errores,
- decidir si ciertas operaciones quedan bloqueadas.

---

## 14. Flujo general de interacción del editor

El flujo base del editor debe ser:

1. la UI detecta una acción,
2. esa acción llega al editor,
3. el editor la interpreta según el contexto actual,
4. si hace falta, invoca operaciones del dominio,
5. actualiza sus estados transitorios,
6. solicita revalidación si corresponde,
7. solicita redibujado,
8. la UI y el render muestran el resultado.

---

## 15. Interacciones principales del canvas

Las interacciones del canvas deben mapearse a acciones del editor.

### 15.1 Clic sobre nodo
Puede significar:
- seleccionar,
- iniciar conexión,
- iniciar una operación contextual,
- o confirmar una relación.

### 15.2 Clic sobre arista
Puede significar:
- seleccionar arista,
- editar propiedades,
- eliminar si la herramienta activa es `delete`.

### 15.3 Clic sobre espacio vacío
Puede significar:
- deseleccionar,
- insertar nuevo nodo si la herramienta activa lo indica,
- cancelar una operación en curso.

### 15.4 Arrastre sobre nodo
Significa mover el nodo.

---

## 16. Lógica de selección

### 16.1 Selección de nodo
Al hacer clic sobre un nodo con herramienta normal:
- se selecciona ese nodo,
- se deselecciona cualquier otro elemento,
- se actualiza el panel derecho.

### 16.2 Selección de arista
Al hacer clic sobre una arista:
- se selecciona la arista,
- se limpian selecciones incompatibles,
- se muestran propiedades de la relación.

### 16.3 Deselección
Al hacer clic en área vacía:
- se limpia la selección,
- salvo que la herramienta activa implique otra intención explícita.

---

## 17. Lógica de arrastre

### 17.1 Inicio del arrastre
Se activa cuando:
- existe un nodo seleccionable,
- el usuario hace clic y mantiene,
- el movimiento supera el umbral mínimo si decides manejarlo así.

### 17.2 Durante el arrastre
- se calcula nueva posición,
- se actualiza el estado visual del nodo,
- se solicita redibujado,
- las aristas conectadas deben redibujarse con la nueva geometría.

### 17.3 Fin del arrastre
- el nodo conserva la nueva posición,
- se limpia `DragState`,
- la selección permanece activa.

---

## 18. Lógica de inserción

La inserción debe depender del tipo de estructura y del contexto.

### 18.1 Inserción genérica
En algunos tipos, `insert` puede significar:
- crear nodo en posición del clic,
- luego completar propiedades.

### 18.2 Inserción estructurada
En otros tipos, la inserción se resuelve por operación semántica:
- `push` en stack,
- `enqueue` en queue,
- insertar BST,
- insertar par clave-valor en map,
- agregar vértice en grafo.

### 18.3 Regla recomendada
En V1, las inserciones deben priorizar claridad semántica por encima de libertad total de dibujo.

---

## 19. Lógica de conexión

### 19.1 Inicio
El usuario activa `connect` o inicia una operación que requiere relación explícita.

### 19.2 Selección de origen
El editor registra nodo origen.

### 19.3 Previsualización opcional
Puede mostrarse una línea temporal desde el origen hasta el puntero.

### 19.4 Selección de destino
El editor recibe nodo destino e intenta crear la relación.

### 19.5 Validación
Antes de confirmar:
- verifica compatibilidad del tipo,
- cantidad máxima de conexiones,
- peso o dirección si aplica,
- consistencia estructural.

### 19.6 Resultado
- si es válido, se crea la relación,
- si no, se cancela con mensaje claro.

---

## 20. Lógica de eliminación

### 20.1 Eliminación por herramienta
Si la herramienta activa es `delete`, un clic sobre un elemento intenta eliminarlo.

### 20.2 Eliminación por tecla
Si existe selección y el usuario pulsa `Delete`, el editor intenta eliminar la selección.

### 20.3 Reglas
- si se elimina un nodo, deben tratarse sus relaciones dependientes,
- si se elimina una arista, no deben quedar referencias colgantes,
- si la operación es delicada, puede requerir validación posterior.

---

## 21. Lógica de edición de propiedades

### 21.1 Origen de la edición
La edición de propiedades normalmente ocurre desde el panel derecho.

### 21.2 Flujo
1. hay una entidad seleccionada,
2. el usuario cambia una propiedad,
3. la UI pasa el cambio al editor,
4. el editor valida o deriva la acción apropiada,
5. actualiza el modelo,
6. solicita revalidación si hace falta,
7. actualiza el render.

### 21.3 Ejemplos
- cambiar valor de nodo,
- cambiar prioridad,
- cambiar peso de arista,
- cambiar clave en map,
- editar etiqueta de vértice.

---

## 22. Lógica de auto-layout

### 22.1 Activación
Se ejecuta por acción explícita del usuario.

### 22.2 Flujo
1. el usuario solicita auto-layout,
2. el editor identifica el tipo de estructura,
3. invoca la estrategia adecuada,
4. actualiza posiciones visuales,
5. redibuja la estructura.

### 22.3 Regla importante
El auto-layout no debe cambiar la semántica del documento; solo la disposición visual.

---

## 23. Lógica de validación

### 23.1 Momentos de validación
La validación puede ocurrir:
- tras una operación importante,
- bajo demanda del usuario,
- al cargar archivo,
- antes de ciertas confirmaciones.

### 23.2 Resultado
El editor recibe un resultado de validación y decide:
- mostrar errores,
- resaltar entidades,
- impedir ciertas acciones futuras si hace falta,
- o continuar normalmente.

### 23.3 Política recomendada
No todo debe bloquearse. Algunas inconsistencias pueden informarse sin impedir edición si eso ayuda al flujo.

---

## 24. Interacción con operaciones semánticas

Muchas acciones del usuario no se traducen directamente en dibujo libre, sino en operaciones del dominio.

### Ejemplos
- `push`
- `pop`
- `enqueue`
- `dequeue`
- insertar BST
- insertar AVL
- insertar par clave-valor
- agregar vértice
- agregar arista

### Regla
El editor debe servir como coordinador entre el gesto de usuario y la operación del dominio.

---

## 25. Comandos contextuales por estructura

El editor debe poder exponer operaciones distintas según la estructura activa.

### Ejemplos
#### Stack
- `push`
- `pop`
- `peek`

#### Queue
- `enqueue`
- `dequeue`
- `front`
- `rear`

#### BST
- insertar valor
- buscar valor
- eliminar valor

#### AVL
- insertar valor
- eliminar valor
- rebalancear

#### Map
- insertar par
- buscar clave
- actualizar valor
- eliminar clave

#### Graph
- agregar vértice
- agregar arista
- editar arista
- eliminar arista

### Implicación
El editor debe conocer el conjunto contextual de acciones disponibles para cada tipo.

---

## 26. Cambio de estructura activa

### 26.1 Propósito
Permitir que el usuario cambie el foco de edición a otra estructura del documento si existe más de una.

### 26.2 Efectos esperados
- cambia la estructura visible o activa,
- limpia o reajusta selección,
- actualiza herramientas y paneles contextuales,
- redibuja el canvas.

### 26.3 Regla
El cambio no debe corromper el documento ni perder el estado persistente de la otra estructura.

---

## 27. Cambio de herramienta activa

### 27.1 Flujo
1. el usuario selecciona una herramienta,
2. el editor actualiza `ToolState`,
3. si una operación estaba a medio completar, se decide si se cancela,
4. se actualiza feedback visual.

### 27.2 Regla
El editor debe evitar estados ambiguos al cambiar de herramienta en medio de una acción parcial.

---

## 28. Cancelación de acciones

### 28.1 Casos típicos
- conexión iniciada pero no completada,
- inserción pendiente,
- selección transitoria,
- operación contextual no confirmada.

### 28.2 Mecanismos
- clic en vacío,
- tecla `Esc`,
- cambio de herramienta.

### 28.3 Resultado esperado
El editor vuelve a un estado limpio y predecible.

---

## 29. Estados que deben reiniciarse bajo ciertas condiciones

Deben limpiarse o reiniciarse cuando corresponda:

- `DragState` al soltar el mouse,
- `ConnectionState` al completar o cancelar conexión,
- `SelectionState` al cerrar documento o cambiar estructura si es incompatible,
- `ValidationOverlay` al cambiar el contexto si ya no aplica,
- `OperationState` al abortar una operación contextual.

---

## 30. Comunicación entre UI y editor

### 30.1 Dirección principal
La UI envía eventos; el editor responde con cambios de estado y solicitudes al dominio.

### 30.2 Tipos de eventos esperados
- clic en botón,
- clic en canvas,
- doble clic si luego lo decides usar,
- arrastre,
- tecla,
- edición de propiedad,
- acción de menú.

### 30.3 Resultado
El editor devuelve:
- cambio de estado,
- cambios en el documento,
- mensajes,
- necesidad de redibujo,
- necesidad de validación.

---

## 31. Comunicación entre editor y core

### 31.1 Regla
El editor no debe reimplementar el dominio por comodidad.

### 31.2 Ejemplo
Si el usuario quiere insertar en BST:
- la UI recoge la intención,
- el editor interpreta,
- el `core` ejecuta la operación semántica,
- el editor recibe el resultado,
- el render refleja el nuevo árbol.

---

## 32. Comunicación entre editor y render

### 32.1 Regla
El editor no dibuja; el render no decide lógica.

### 32.2 Interacción
El editor informa:
- qué estructura está activa,
- qué está seleccionado,
- qué estados transitorios merecen representación,
- y cuándo debe actualizarse el canvas.

---

## 33. Comunicación entre editor y persistence

### 33.1 Guardado
El editor coordina la acción de guardar usando el documento actual.

### 33.2 Carga
El editor recibe un documento reconstruido y reemplaza el contexto activo.

### 33.3 Regla
Persistence no debe gestionar estados transitorios del editor.

---

## 34. Estrategia de mensajes al usuario

El editor debe producir mensajes funcionales claros cuando una acción:

- se completa,
- falla,
- es inválida,
- o produce una advertencia.

### Ejemplos
- Nodo insertado correctamente
- No se permite clave duplicada
- El nodo ya tiene hijo izquierdo
- Peso requerido para grafo ponderado
- Conexión inválida para esta estructura

### Reglas
- mensajes cortos,
- directos,
- útiles,
- sin dramatismo.

---

## 35. Estrategia de refresco visual

### 35.1 Regla general
Toda operación que afecte el modelo o un estado visual relevante debe producir solicitud de redibujado.

### 35.2 Casos típicos
- selección,
- arrastre,
- inserción,
- eliminación,
- edición de propiedades,
- auto-layout,
- validación con errores resaltables.

### 35.3 Nota
El editor no debe forzar redibujos inútiles si nada cambió.

---

## 36. Simplificaciones intencionales de la V1

Para mantener el proyecto controlado, la V1 puede evitar:

- selección múltiple,
- historial de deshacer/rehacer avanzado,
- edición colaborativa,
- múltiples operaciones encadenadas complejas,
- macros,
- scripting,
- herramientas gráficas excesivas.

---

## 37. Riesgos lógicos a evitar

### 37.1 Mezclar interacción con reglas del dominio
El editor coordina, pero el `core` valida la semántica profunda.

### 37.2 Estados huérfanos
No deben quedar arrastres, conexiones u operaciones pendientes sin cerrar correctamente.

### 37.3 Ambigüedad de herramienta
El usuario debe saber qué modo está activo.

### 37.4 Operaciones silenciosas
Las acciones inválidas no deben fallar sin explicación.

### 37.5 Exceso de rigidez
No todo error debe bloquear por completo la edición si puede tratarse como advertencia útil.

---

## 38. Resultado esperado del editor

Al finalizar la V1, la lógica del editor debe permitir:

- seleccionar elementos,
- mover nodos,
- crear y eliminar entidades,
- conectar nodos según reglas,
- editar propiedades,
- ejecutar operaciones semánticas por estructura,
- validar el documento,
- aplicar auto-layout,
- y coordinar una experiencia de edición estable y clara.

---

## 39. Relación con otros documentos

Este archivo se complementa con:

- `02_ui_ux.md` → interfaz y comportamiento visible,
- `03_architecture.md` → rol del módulo `editor`,
- `04_data_model.md` → entidades y estados base,
- `05_structure_definitions.md` → reglas de cada estructura,
- `06_operations.md` → operaciones semánticas,
- `08_rendering.md` → representación visual del estado del editor.

