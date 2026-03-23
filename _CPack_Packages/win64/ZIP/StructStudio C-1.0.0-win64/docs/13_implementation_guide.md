# 13_implementation_guide.md

## 1. Proposito

Este documento explica **como quedo implementado realmente** StructStudio C y por que la base esta organizada de esa manera.

Esta guia esta escrita pensando en alguien que:

- sabe programar un poco,
- todavia no domina bien C,
- y quiere entender decisiones de arquitectura y buenas practicas sin perderse en teoria vacia.

---

## 2. Idea general

Aunque el proyecto esta hecho en C y **C no es orientado a objetos**, eso no significa que todo deba ir mezclado en un solo archivo.

La estrategia usada aqui es:

- separar el sistema por modulos,
- definir responsabilidades claras por capa,
- y mantener los datos y las operaciones agrupados con un criterio estable.

Eso permite que el proyecto siga siendo:

- idiomatico para C,
- mantenible,
- y entendible para estudio.

---

## 3. Arquitectura implementada

La implementacion sigue esta separacion:

- `common/`
- `core/`
- `editor/`
- `persistence/`
- `render/`
- `ui/`
- `app/`

### 3.1 `common`

Contiene piezas basicas reutilizables:

- manejo de errores,
- helpers de strings,
- timestamps,
- crecimiento de arrays dinamicos,
- ids.

La idea es que estas utilidades no sepan nada de listas, arboles, UI ni JSON.

### 3.2 `core`

Es el nucleo del dominio.

Aqui viven:

- `Document`
- `Structure`
- `Node`
- `Edge`
- variantes soportadas
- operaciones semanticas
- auto-layout
- validacion

Este modulo es el corazon del proyecto. La UI no deberia inventar reglas que ya pertenecen al dominio.

Ademas, la API del dominio ya no esta concentrada en un solo archivo grande. Se dividio asi:

- `api_base.c`: primitivas compartidas y rebuilds internos
- `api_analysis.c`: recorridos y algoritmos educativos desacoplados de la GUI
- `api_analysis_playback.c`: construccion de pasos guiados para visualizacion secuencial
- `api_analysis_graph_advanced.c`: analisis avanzados de grafos como Floyd-Warshall, Prim y Kruskal
- `api_examples.c`: ejemplos didacticos reutilizables por variante para poblar el canvas sin codificar recetas dentro de la UI
- `api_theory.c`: catalogo de teoria contextual para estructuras y algoritmos
- `api_tree.c`: helpers de arboles, BST, AVL y heap
- `api_primary.c`: accion primaria por variante
- `api_secondary.c`: accion secundaria por variante
- `api_tertiary.c`: accion terciaria por variante
- `api_mutation.c`: conectar, actualizar y eliminar
- `api_layout.c`: auto-layout
- `api_validation.c`: reglas de validacion

### 3.3 `editor`

Es la capa que coordina la interaccion.

Aqui viven:

- herramienta activa,
- seleccion actual,
- arrastre,
- conexion en curso,
- previsualizacion de conexion,
- reporte del ultimo analisis ejecutado,
- playback guiado del analisis actual,
- transiciones temporales de layout,
- pulso del paso actual,
- autoplay del playback,
- mensajes de estado,
- coordinacion entre UI, core, persistencia y render.

En otras palabras:

la UI detecta eventos, pero el **editor decide que significan**.

Ademas, la parte temporal ya no quedo mezclada dentro de `main_window.c` ni del render:

- `src/editor/editor.c`
  - sigue siendo el coordinador de acciones y estado semantico.
- `src/editor/animation.c`
  - encapsula snapshots de layout, interpolacion temporal, pulso visual y reproduccion automatica.

Eso importa mucho en C porque, si no separas estas dos responsabilidades, terminas con funciones gigantes que mezclan:

- mutacion del modelo,
- temporizadores,
- y reglas de dibujo.

### 3.4 `persistence`

Se encarga de:

- guardar JSON,
- cargar JSON,
- reconstruir el documento.

Esto evita que la logica de archivo termine metida dentro de la ventana o del canvas.

### 3.5 `render`

Tiene dos responsabilidades concretas:

- dibujar la estructura dentro del `uiArea` de `libui-ng`,
- exportar la vista a PNG usando un render software separado.

Separar estas dos rutas ayuda a no depender de screenshots ni trucos del sistema operativo.

Ademas, el render ya no dibuja todo de forma generica:

- arboles, heap, set y grafos usan nodos redondos,
- listas usan compartimentos visuales para punteros,
- map usa tarjetas partidas clave/valor,
- y las conexiones usan flechas o lineas segun la semantica de la estructura.

### 3.6 `ui`

Contiene la construccion de la ventana principal con `libui-ng`:

- menus,
- panel izquierdo,
- canvas,
- panel derecho,
- barra de estado,
- y wrappers de dialogos nativos que cargan iconos locales del proyecto para no depender de los del sistema.

El panel derecho ya no es solo de propiedades:

- arriba edita el elemento seleccionado,
- abajo mantiene un panel persistente de teoria con scroll,
- el menu `Ver` puede ocultar o mostrar ambos paneles laterales para priorizar canvas o teoria,
- y esa teoria se recalcula desde `core/api_theory.c` cada vez que cambia la estructura o el analisis activo.

La UI no implementa reglas de BST, AVL o Map. Solo recoge acciones y refleja estado.

### 3.7 `app`

Solo arranca la aplicacion.

Esto parece pequeno, pero es una buena practica:

- `main()` no debe convertirse en un archivo gigante.

---

## 4. Conceptos de C que se aplican aqui

## 4.1 `struct`

En C una `struct` agrupa datos relacionados.

Ejemplos:

- `SsNode`
- `SsEdge`
- `SsStructure`
- `SsEditorState`

Piensalo como una forma de decir:

> "estos campos pertenecen a la misma cosa"

---

## 4.2 `enum`

Se usan `enum` para representar opciones cerradas.

Ejemplos:

- familia de estructura,
- variante,
- herramienta activa,
- tipo de seleccion,
- severidad de validacion.

Esto es mejor que usar strings o enteros magicos por todos lados.

---

## 4.3 `.h` y `.c`

Cada modulo tiene:

- un header `.h` con la interfaz publica,
- y un `.c` con la implementacion.

Eso permite:

- ocultar detalles internos,
- exponer solo lo necesario,
- y reducir acoplamiento.

---

## 4.4 Funciones `static`

Muchas funciones internas son `static`.

Eso significa:

- solo existen dentro de ese archivo,
- no contaminan el resto del proyecto,
- y ayudan a marcar que son helpers privados del modulo.

Esta es una practica muy buena en C.

---

## 4.5 Punteros

En C casi todo se pasa por puntero cuando quieres modificar una estructura grande.

Ejemplo mental:

- `SsDocument *document`

significa:

> "trabaja sobre este documento real, no sobre una copia"

Esto es normal y necesario en C.

---

## 4.6 `const`

Cuando una funcion solo debe leer, se usa `const`.

Ejemplo:

- `const SsStructure *`

Eso comunica intencion y evita modificaciones accidentales.

---

## 5. Estrategias de software aplicadas

## 5.1 Separacion de responsabilidades

Cada modulo debe responder a una pregunta clara:

- `core`: que significa la estructura
- `editor`: como se interpreta la accion del usuario
- `render`: como se ve
- `persistence`: como se guarda
- `ui`: como se presenta la ventana

Cuando esta separacion se rompe, el proyecto se vuelve dificil de mantener.

---

## 5.2 Datos primero, GUI despues

Primero existe el modelo en memoria.

Despues se dibuja.

Eso es importante porque:

- permite probar logica sin abrir ventanas,
- facilita persistencia,
- y evita depender de la GUI para "saber" el estado real.

---

## 5.3 Estado centralizado del editor

En lugar de repartir variables globales por toda la UI, se agrupo el estado interactivo en `SsEditorState`.

Eso mejora:

- trazabilidad,
- depuracion,
- y legibilidad.

Tambien permite algo importante:

- que el canvas tenga interacciones ricas sin repartir estado temporal en varios callbacks,
- por ejemplo arrastre, seleccion, origen de conexion, previsualizacion del trazado y paso actual de un recorrido guiado.

---

## 5.4 Prefijo comun de nombres

Las funciones del proyecto usan prefijo `ss_`.

Ejemplos:

- `ss_editor_apply_primary`
- `ss_structure_validate`
- `ss_document_save_json`

En C esto es importante porque no hay namespaces reales.

---

## 5.5 Error handling explicito

En vez de lanzar excepciones, aqui se usa `SsError`.

Esto es tipico en C.

Ventajas:

- el flujo es mas explicito,
- el llamador decide que hacer,
- y se evitan fallos silenciosos.

---

## 6. Buenas practicas visibles en el codigo

## 6.1 Evitar logica importante en callbacks de UI

Los callbacks de `libui-ng` son pequenos y delegan al editor.

Eso evita ventanas gigantes llenas de logica de negocio.

Un ejemplo practico:

- la ventana detecta un clic,
- el editor decide si eso significa seleccionar, insertar, conectar o eliminar,
- y luego el render solo refleja el nuevo estado.

---

## 6.2 Arrays dinamicos controlados

En vez de usar cantidades fijas por comodidad, el proyecto usa `realloc` de forma controlada mediante helpers.

Beneficios:

- crecimiento progresivo,
- menos desperdicio,
- mejor base para documentos reales.

---

## 6.3 Build serio

El proyecto no compila "a mano".

Usa:

- `CMake`
- dependencia oficial `libui-ng`
- targets separados
- prueba automatizada base con `ctest`

Eso lo acerca mucho mas a un proyecto profesional real.

---

## 6.4 Prueba automatizada minima

Existen pruebas `core_smoke` y `core_variants_smoke` para asegurar que:

- el editor puede crear estructuras,
- insertar datos,
- mantener comportamiento basico,
- respetar orden semantico en priority queue,
- y rechazar duplicados en set.

Tambien existe `analysis_smoke` para comprobar que:

- los recorridos de arbol siguen disponibles desde la capa `editor`,
- Dijkstra produce un reporte coherente sobre un grafo ponderado,
- y la GUI puede reutilizar ese texto sin duplicar logica del algoritmo.

Tambien existe `graph_analysis_advanced_smoke` para comprobar que:

- Floyd-Warshall produce distancias entre todos los pares,
- Prim construye un MST coherente desde un origen opcional,
- y Kruskal mantiene el costo total esperado sobre un grafo ponderado no dirigido.

No es una suite completa, pero ya marca una direccion correcta.

---

## 6.5 Comentarios tecnicos utiles

Los archivos nuevos o refactorizados incluyen comentarios con enfoque profesional:

- responsabilidad del archivo
- contrato del modulo
- motivo de decisiones no obvias

La idea no es comentar cada linea. La idea es dejar contexto donde realmente ayuda a alguien que aun esta aprendiendo C o la arquitectura del proyecto.

---

## 6.6 Layout automatico donde conviene

No todas las estructuras deben comportarse igual.

En esta implementacion se aplico este criterio:

- grafos permiten edicion mas libre en canvas,
- listas, arboles, heap, set y map se reordenan automaticamente cuando una operacion cambia su forma.

Esa decision evita un problema comun en software educativo:

- que el modelo interno cambie,
- pero la vista quede desordenada hasta que el usuario recuerde pulsar otro boton.

En otras palabras:

se permite libertad donde aporta valor,
pero se conserva consistencia visual donde la estructura tiene una geometria mas semantica.

---

## 6.7 UX guiada por estado

En una GUI tecnica, no basta con "permitir acciones".

Tambien conviene:

- indicar que herramienta esta activa,
- explicar que espera cada campo,
- bloquear acciones que en ese contexto no aplican,
- y dejar visible una guia corta para no obligar al usuario a memorizar reglas.

Por eso la ventana principal ahora incluye:

- una guia contextual persistente,
- botones de herramienta que muestran el estado activo,
- entradas y acciones que se habilitan o deshabilitan segun seleccion y variante,
- un bloque de `Analisis` que cambia segun la estructura activa,
- un bloque de `Teoria` con una sola accion clara para consultar conceptos,
- controles de playback solo cuando el analisis elegido realmente los soporta,
- una barra inferior compacta para evitar duplicar seleccion, estado y validacion en varios lugares,
- y menues rapidos para tareas frecuentes como deseleccionar, limpiar estructura o mostrar grilla.

Este enfoque es una buena practica porque baja carga cognitiva sin meter logica del dominio dentro del render.

---

## 7. Limitaciones actuales y criterio tecnico

La base ya es funcional y el nucleo principal ya fue particionado en modulos mas pequenos.

Las siguientes mejoras naturales ahora son otras:

- ampliar la suite de pruebas del `core`
- seguir documentando ownership de memoria
- dividir `render.c` si en futuras iteraciones vuelve a crecer demasiado

La regla correcta sigue siendo esta:

> primero mantener la base coherente, luego crecer con refactors pequenos y medibles.

---

## 8. Como leer el proyecto si sabes poco de C

Orden recomendado:

1. `include/core/model.h`
2. `src/core/model.c`
3. `src/core/api_base.c`
4. `src/core/api_primary.c`
5. `src/core/api_tree.c`
6. `include/editor/editor.h`
7. `src/editor/editor.c`
8. `src/ui/main_window.c`
9. `src/render/render.c`
10. `src/persistence/document_io.c`

No conviene empezar leyendo el archivo mas grande sin contexto.

---

## 9. Como extender el proyecto sin romperlo

Si quieres agregar una nueva estructura:

1. agrega la variante al enum y al descriptor
2. define operaciones semanticas
3. define layout
4. define validacion
5. actualiza render si requiere dibujo especial
6. actualiza persistencia si necesita metadatos nuevos

Ese orden es sano porque respeta las capas.

---

## 10. Veredicto tecnico

La implementacion actual ya cumple una base seria para estudio porque:

- usa GUI nativa real,
- tiene dominio separado,
- persiste JSON real,
- exporta PNG,
- mantiene una estructura modular,
- incorpora comentarios tecnicos utiles en los archivos clave,
- y ya incorpora una mentalidad de software mantenible.

La siguiente mejora natural no es "meter mas features a ciegas".

La siguiente mejora natural es:

- **profundizar pruebas y documentacion interna**,
- seguir ampliando algoritmos academicos donde aporten valor real,
- y hacerlo sin perder claridad ni reintroducir duplicacion.
