# 03_architecture.md

## 1. Propósito del documento

Este documento define la **arquitectura interna** de **StructStudio C** en su primera versión (V1).

Su objetivo es describir:

- la división del sistema en módulos,
- la responsabilidad de cada módulo,
- la relación entre capas,
- el flujo general de datos,
- y las decisiones arquitectónicas principales.

Este archivo no reemplaza al modelo de datos detallado ni al documento de operaciones; su función es servir como mapa técnico de alto nivel para orientar la implementación en C.

---

## 2. Objetivo arquitectónico

La arquitectura de StructStudio C debe permitir desarrollar una aplicación de escritorio en C que sea:

- modular,
- entendible,
- mantenible,
- extensible,
- y suficientemente seria como proyecto de ingeniería de software.

El sistema no tendrá backend separado ni red, pero sí tendrá una organización interna clara para evitar que todo termine mezclado en la interfaz gráfica.

---

## 3. Principios arquitectónicos

### 3.1 Separación de responsabilidades
Cada módulo debe tener una responsabilidad clara y no mezclar innecesariamente:
- lógica del dominio,
- renderizado,
- persistencia,
- infraestructura técnica,
- e interfaz de usuario.

### 3.2 Núcleo independiente de la GUI
La lógica de estructuras de datos no debe depender de la librería gráfica.

### 3.3 Editor separado del dominio
El comportamiento del editor visual no debe contaminar el modelo puro de las estructuras.

### 3.4 Persistencia desacoplada
Guardar y cargar documentos debe ser responsabilidad de un módulo propio.

### 3.5 Base común + especialización
El sistema tendrá una base compartida, pero permitirá comportamientos especializados por tipo de estructura cuando sea necesario.

### 3.6 Extensibilidad controlada
La arquitectura debe facilitar agregar nuevas estructuras sin convertir el código en una cadena gigante de condicionales.

### 3.7 Orientación a módulos, no a clases
Como el proyecto está en C, la organización estará basada en:
- módulos `.h/.c`,
- structs,
- funciones,
- callbacks,
- tablas de operaciones,
- contratos por convención.

---

## 4. Estilo arquitectónico general

StructStudio C seguirá una arquitectura modular local con separación por capas funcionales.

No habrá arquitectura cliente-servidor.
No habrá frontend/backend como aplicaciones separadas.

La división correcta para este proyecto es:

- **app**
- **ui**
- **editor**
- **render**
- **core**
- **persistence**
- **common**

Cada una tendrá una responsabilidad propia.

---

## 5. Visión general de módulos

### 5.1 app
Punto de arranque y coordinación general.

### 5.2 ui
Interfaz gráfica de escritorio con libui-ng.

### 5.3 editor
Lógica de interacción y edición del documento activo.

### 5.4 render
Representación visual 2D de estructuras y elementos.

### 5.5 core
Modelo del dominio, reglas, tipos y operaciones semánticas.

### 5.6 persistence
Serialización y deserialización de documentos.

### 5.7 common
Infraestructura técnica compartida.

---

## 6. Responsabilidad detallada por módulo

## 6.1 Módulo `app`

### Responsabilidad principal
Inicializar el sistema, componer los módulos y controlar el ciclo de vida general de la aplicación.

### Funciones típicas
- `main`
- inicialización global
- configuración inicial
- arranque de la UI
- creación del contexto general de aplicación
- liberación ordenada de recursos al salir

### Qué no debe hacer
- no debe contener reglas de estructuras,
- no debe serializar directamente,
- no debe implementar lógica de render detallada,
- no debe mezclar toda la lógica del editor.

---

## 6.2 Módulo `ui`

### Responsabilidad principal
Construir y gestionar la interfaz gráfica de escritorio usando libui-ng.

### Elementos a cargo
- ventana principal,
- barra de menú,
- panel izquierdo,
- canvas o área de dibujo,
- panel derecho,
- barra de estado,
- diálogos de abrir/guardar,
- mensajes visuales básicos.

### Función del módulo
La UI recibe acciones del usuario y las traduce en comandos hacia el editor o hacia otros servicios del sistema.

### Qué no debe hacer
- no debe implementar reglas internas de BST, AVL, set, map, etc.,
- no debe decidir por sí sola cómo serializar,
- no debe ser dueña de la lógica del documento.

---

## 6.3 Módulo `editor`

### Responsabilidad principal
Gestionar el **estado interactivo del editor** y coordinar cómo el usuario modifica el documento actual.

### Responsabilidades específicas
- selección actual,
- herramienta activa,
- estado de arrastre,
- creación de nodos,
- conexión de elementos,
- eliminación,
- edición contextual,
- coordinación de operaciones desde la UI,
- aplicación de auto-layout,
- validación solicitada por el usuario.

### Rol clave
Este módulo es el puente entre:
- la interfaz gráfica,
- el dominio,
- y el render.

### Qué no debe hacer
- no debe contener widgets concretos,
- no debe parsear JSON,
- no debe mezclar detalles de la librería gráfica en toda su lógica.

---

## 6.4 Módulo `render`

### Responsabilidad principal
Dibujar visualmente el documento activo y sus elementos en el canvas.

### Responsabilidades específicas
- dibujar nodos,
- dibujar celdas,
- dibujar aristas,
- dibujar flechas,
- dibujar pesos,
- dibujar pares clave-valor,
- dibujar realces de selección,
- dibujar mensajes visuales ligeros,
- dibujar estilos según tipo de estructura.

### Principio importante
Render solo debe encargarse de **cómo se ve** la información, no de las reglas del dominio.

### Qué no debe hacer
- no debe decidir reglas del BST,
- no debe mutar el documento arbitrariamente,
- no debe guardar archivos.

---

## 6.5 Módulo `core`

### Responsabilidad principal
Representar el dominio del sistema: documento, estructuras, reglas, operaciones y validaciones.

### Componentes esperados
- modelo del documento,
- tipos de estructuras,
- nodos,
- aristas,
- propiedades específicas,
- reglas estructurales,
- operaciones semánticas,
- validación por tipo,
- registro de tipos de estructura.

### Importancia
Este es el corazón lógico del proyecto.

### Qué no debe hacer
- no debe depender directamente de libui-ng,
- no debe abrir ventanas,
- no debe mostrar diálogos,
- no debe dibujar en el canvas.

---

## 6.6 Módulo `persistence`

### Responsabilidad principal
Guardar y cargar documentos desde archivos JSON.

### Responsabilidades específicas
- serializar documento,
- deserializar documento,
- validar versión de archivo,
- transformar estructuras del dominio en representación persistente,
- reconstruir el documento desde JSON,
- reportar errores de carga.

### Qué no debe hacer
- no debe decidir cómo se ve el documento,
- no debe implementar interacción de usuario,
- no debe cargar reglas visuales del editor.

---

## 6.7 Módulo `common`

### Responsabilidad principal
Contener infraestructura técnica compartida por el resto del sistema.

### Posibles componentes
- manejo de errores,
- logging,
- utilidades de cadenas,
- ids,
- helpers matemáticos 2D sencillos,
- helpers de memoria,
- constantes compartidas,
- configuración global liviana.

### Qué no debe hacer
- no debe transformarse en un cajón de sastre caótico,
- no debe absorber lógica de dominio por comodidad.

---

## 7. Flujo general de interacción

### Flujo base
1. El usuario interactúa con la UI.
2. La UI traduce la acción en una intención.
3. El editor procesa esa intención.
4. El editor invoca operaciones del core si la acción afecta la estructura.
5. Si cambia el documento, el render actualiza la representación.
6. Si se guarda o carga, el editor coordina con persistence.

### Ejemplo
**Acción:** el usuario quiere insertar un nodo en una lista.

- UI detecta clic en “Insertar nodo”.
- UI notifica al editor.
- Editor invoca operación correspondiente en core.
- Core modifica el documento.
- Editor actualiza estado de selección si aplica.
- Render redibuja la estructura.

---

## 8. Flujo general de persistencia

### Guardado
1. El usuario pulsa guardar.
2. UI dispara la acción.
3. Editor solicita a persistence serializar el documento activo.
4. Persistence produce JSON.
5. UI muestra confirmación o error.

### Carga
1. El usuario elige un archivo.
2. UI obtiene la ruta.
3. Editor solicita a persistence cargar el documento.
4. Persistence deserializa y reconstruye el modelo.
5. Editor adopta el documento cargado.
6. Render muestra el nuevo estado.

---

## 9. Flujo general de renderizado

### Principio
El render debe trabajar sobre el estado ya resuelto del documento y del editor.

### Entradas principales del render
- documento activo,
- estructura activa,
- selección actual,
- herramienta activa si afecta visualmente,
- preferencias visuales mínimas,
- estado de errores resaltables.

### Salidas
- representación visual en el canvas.

---

## 10. Modelo de extensibilidad

Dado que el sistema soportará múltiples estructuras de datos, la arquitectura debe evitar un diseño rígido y repetitivo.

### Enfoque propuesto
Usar una base común más un **registro de tipos**.

Cada tipo de estructura debería declarar, conceptualmente:

- su nombre,
- su categoría,
- sus reglas,
- sus operaciones semánticas,
- su validación,
- su estrategia de layout,
- y su render especializado si aplica.

### Beneficio
Permite agregar nuevos tipos sin romper toda la aplicación.

---

## 11. Estrategia de especialización

No todo debe forzarse a un modelo totalmente uniforme si eso vuelve el sistema artificial.

### Regla general
Se usará:
- una base común para conceptos compartidos,
- pero se permitirá especialización por tipo de estructura.

### Ejemplos
- un vértice de grafo no necesita exactamente lo mismo que una entrada de map,
- un nodo de árbol tiene relaciones distintas a un nodo de lista,
- una prioridad en priority queue requiere atributos propios.

### Decisión arquitectónica
**La especialización es válida cuando mejora la claridad y respeta la coherencia del dominio.**

---

## 12. Fronteras entre módulos

### UI ↔ Editor
La UI envía acciones; el editor coordina.

### Editor ↔ Core
El editor solicita operaciones y validaciones.

### Editor ↔ Render
El editor expone el estado necesario para dibujar.

### Editor ↔ Persistence
El editor coordina guardar y cargar.

### Core ↔ Persistence
Persistence necesita conocer la forma serializable del dominio, pero sin contaminar la lógica del dominio con JSON crudo.

### Common ↔ Todos
Common puede ser usado por los demás módulos de forma controlada.

---

## 13. Dependencias deseadas entre módulos

### Dependencias permitidas
- `app` depende de todos los módulos principales para componer la aplicación.
- `ui` puede depender de `editor`, `render` y `common`.
- `editor` puede depender de `core`, `persistence` y `common`.
- `render` puede depender de `core`, del estado del editor y de `common`.
- `persistence` puede depender de `core` y `common`.
- `core` puede depender de `common`.

### Dependencias a evitar
- `core` no debe depender de `ui`.
- `core` no debe depender de `render`.
- `persistence` no debe depender de `ui`.
- `render` no debe contener lógica de persistencia.

---

## 14. Contexto global de aplicación

Es conveniente contar con un contexto de aplicación que agrupe las piezas principales en tiempo de ejecución.

### Posibles contenidos del contexto
- documento activo,
- estado del editor,
- referencias a módulos principales,
- configuración visual básica,
- servicios compartidos.

### Objetivo
Evitar variables globales desordenadas y facilitar el cableado entre módulos.

---

## 15. Organización del repositorio

Una estructura razonable para V1 sería:

```text
project/
  CMakeLists.txt
  cmake/
  external/
  assets/
  docs/
  include/
    app/
    ui/
    editor/
    render/
    core/
    persistence/
    common/
  src/
    app/
    ui/
    editor/
    render/
    core/
    persistence/
    common/
  tests/
  samples/
```

### Idea base
- `include/` contiene headers públicos por módulo.
- `src/` contiene implementación.
- `docs/` contiene los Markdown del proyecto.
- `tests/` contiene pruebas.
- `samples/` contiene archivos JSON de ejemplo.

---

## 16. Convención modular en C

Cada módulo debe intentar mantener una interfaz clara entre `.h` y `.c`.

### Convención sugerida
- headers públicos pequeños y claros,
- tipos opacos cuando sea conveniente,
- funciones nombradas por prefijo de módulo,
- evitar exponer detalles internos sin necesidad.

### Ejemplo conceptual
- `editor_state.h` / `editor_state.c`
- `graph_ops.h` / `graph_ops.c`
- `document_io.h` / `document_io.c`

---

## 17. Estrategia de errores

La arquitectura debe prever manejo ordenado de errores.

### Posibles tipos de error
- error de validación,
- error de archivo,
- error de estructura,
- error de operación inválida,
- error interno.

### Recomendación
Usar códigos de error y mensajes claros, evitando depender únicamente de texto suelto.

---

## 18. Estrategia de pruebas

Aunque la V1 sea un proyecto visual, algunas partes deben poder probarse sin la GUI.

### Áreas ideales para pruebas
- validación del dominio,
- operaciones de estructuras,
- serialización y deserialización,
- restricciones por tipo,
- consistencia del documento.

### Principio
La arquitectura debe facilitar pruebas del `core` y `persistence` de forma aislada.

---

## 19. Decisiones arquitectónicas ya tomadas

### Decisiones cerradas
- lenguaje principal: C
- build system: CMake
- GUI: libui-ng
- persistencia: JSON
- exportación visual: PNG
- solo 2D
- un documento visible a la vez
- modo estructurado
- sin backend
- sin animaciones automaticas complejas en V1; los recorridos guiados manuales si forman parte del alcance

### Implicación
La arquitectura debe ser local, modular y enfocada en documento + editor + render + persistencia.

---

## 20. Riesgos arquitectónicos a evitar

### 20.1 Mezclar toda la lógica en la GUI
Error típico en proyectos pequeños que luego crecen.

### 20.2 Convertir `common` en basurero universal
Debe mantenerse controlado.

### 20.3 Forzar una abstracción demasiado genérica
No conviene si eso hace más difícil representar estructuras reales.

### 20.4 Duplicar lógica por cada estructura
Debe haber reutilización razonable.

### 20.5 Hacer que render gobierne el dominio
El dibujo debe reflejar el estado, no gobernarlo.

---

## 21. Resultado esperado de la arquitectura

Al finalizar la V1, la arquitectura debe permitir:

- trabajar con múltiples tipos de estructuras,
- mantener reglas claras por tipo,
- editar visualmente un documento,
- guardar y cargar archivos,
- exportar a PNG,
- y seguir creciendo sin tener que reescribir todo desde cero.

---

## 22. Relación con otros documentos

Este archivo se complementa con:

- `00_overview.md` → visión general
- `01_product_spec.md` → comportamiento funcional
- `02_ui_ux.md` → diseño de interfaz y experiencia
- `04_data_model.md` → entidades y relaciones del modelo en memoria
- `05_structure_definitions.md` → reglas específicas por estructura
- `07_file_format.md` → persistencia JSON
- `09_editor_logic.md` → lógica detallada del editor
- `11_build_system.md` → CMake e infraestructura técnica
