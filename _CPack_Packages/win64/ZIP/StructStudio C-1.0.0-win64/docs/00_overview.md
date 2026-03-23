# 00_overview.md

## Nombre provisional del proyecto
**StructStudio C**

> Aplicación de escritorio en C para crear, editar, guardar y exportar representaciones visuales 2D de estructuras de datos con enfoque educativo.

---

## 1. Propósito del proyecto

StructStudio C es una aplicación de escritorio orientada al aprendizaje y representación de **estructuras de datos** mediante una interfaz gráfica clásica, sobria y nostálgica, inspirada en programas de escritorio tradicionales.

El objetivo principal del proyecto es permitir la creación y edición de **documentos de estructuras de datos** en formato visual 2D, de manera que el usuario pueda:

- construir estructuras de datos de forma gráfica,
- manipularlas mediante operaciones propias de cada estructura,
- guardarlas en archivos JSON,
- exportarlas como imagen PNG,
- y usarlas como apoyo educativo o material visual de estudio.

Este proyecto también tiene un objetivo técnico personal: servir como práctica seria de desarrollo de software en **C**, con **CMake**, arquitectura modular, serialización, renderizado 2D y construcción de una aplicación de escritorio nativa.

---

## 2. Objetivo general

Desarrollar una aplicación de escritorio en C que permita **crear, editar, validar, guardar y exportar estructuras de datos 2D** mediante un entorno gráfico estructurado, con soporte para múltiples tipos de estructuras enseñadas en un curso universitario típico de estructuras de datos.

---

## 3. Objetivos específicos

- Construir una aplicación de escritorio local, sin backend ni servicios remotos.
- Soportar la edición visual de múltiples estructuras de datos.
- Implementar un modelo interno desacoplado entre:
  - dominio,
  - interfaz gráfica,
  - renderizado,
  - persistencia.
- Guardar documentos en formato **JSON**.
- Exportar la representación visual de una estructura o documento en **PNG**.
- Diseñar una arquitectura extensible para agregar nuevas estructuras en el futuro.
- Mantener una estética visual clásica y funcional, inspirada en software de escritorio tipo Windows 7.
- Practicar una infraestructura profesional en C:
  - C17,
  - CMake,
  - organización modular,
  - manejo de errores,
  - build multiplataforma.

---

## 4. Naturaleza del producto

StructStudio C **no es** un videojuego, ni un motor gráfico, ni una librería de estructuras de datos para uso productivo.

StructStudio C **sí es**:

- una herramienta educativa,
- una aplicación de escritorio,
- un editor visual,
- un generador de archivos de estructuras de datos,
- y un proyecto técnico serio para practicar desarrollo en C.

El producto está pensado principalmente para uso local y personal, con enfoque académico y experimental.

---

## 5. Usuario objetivo

### Usuario principal
El propio autor del proyecto, como estudiante y desarrollador, con fines de:

- aprendizaje,
- práctica de estructuras de datos,
- generación de material visual,
- experimentación con arquitectura de software en C.

### Usuario secundario potencial
Estudiantes o docentes que quieran representar visualmente estructuras de datos de manera clara y exportable.

---

## 6. Alcance funcional de alto nivel

La aplicación permitirá:

- crear un documento de trabajo,
- elegir un tipo de estructura de datos,
- crear nodos o elementos,
- conectarlos o relacionarlos según las reglas de la estructura,
- ejecutar operaciones semánticas propias de cada estructura,
- editar propiedades visuales y lógicas,
- guardar el documento,
- volver a abrirlo,
- exportarlo como imagen.

La aplicación trabajará con **representaciones 2D** únicamente.

---

## 7. Enfoque de interacción

El sistema funcionará en **modo estructurado**.

Eso significa que el usuario no dibuja libremente cualquier diagrama arbitrario, sino que trabaja dentro de las reglas del tipo de estructura activa.

Ejemplos:

- una lista enlazada simple tendrá enlaces `next`,
- una lista doble tendrá `prev` y `next`,
- un árbol binario tendrá restricciones de hijos izquierdo y derecho,
- un set no permitirá duplicados,
- un mapa manejará pares clave-valor,
- un grafo permitirá vértices y aristas según su configuración.

El editor no será solo visual: también tendrá **inteligencia semántica** para entender operaciones y restricciones propias de cada estructura.

---

## 8. Estructuras de datos contempladas

El proyecto está orientado a incluir, como base conceptual, las estructuras de datos vistas en un entorno universitario típico.

### Familias principales previstas
- arreglos o vectores,
- listas enlazadas,
- pilas,
- colas,
- árboles,
- heaps,
- mapas,
- sets,
- grafos.

### Variantes previstas
- lista simple,
- lista doble,
- lista circular simple,
- lista circular doble,
- pila,
- cola,
- cola circular,
- cola de prioridad,
- árbol binario,
- BST,
- AVL,
- heap,
- map,
- set,
- grafo dirigido,
- grafo no dirigido,
- grafo ponderado,
- grafo no ponderado.

La definición exacta de reglas y operaciones por estructura se documentará en archivos especializados del proyecto.

---

## 9. Decisiones clave ya tomadas

### Interfaz gráfica
- Aplicación de escritorio nativa.
- Estilo visual clásico y nostálgico.
- Inspiración estética tipo Windows 7.
- Uso de **libui-ng** como base de la GUI.

### Persistencia
- Formato de documento: **JSON**
- Formato de exportación visual: **PNG**

### Documento
- Se trabajará con **un solo documento visible a la vez** para simplificar la experiencia de usuario y la implementación inicial.

### Representación
- Solo **2D**
- No habrá soporte 3D.

### Animación
- No habrá animaciones paso a paso en la primera versión.

### Modo de trabajo
- El sistema será **estructurado**, no un editor libre genérico.

---

## 10. Filosofía técnica del proyecto

El proyecto busca equilibrar tres ideas:

### 1. Claridad educativa
La estructura debe verse y entenderse con claridad.

### 2. Arquitectura seria
El proyecto debe estar organizado como software real y mantenible, no como un prototipo improvisado.

### 3. Extensibilidad
Cada estructura debe poder integrarse como un componente especializado dentro de una base común, sin forzar generalizaciones excesivas que perjudiquen la claridad.

En otras palabras, el sistema tendrá:
- una base común compartida,
- pero permitirá especialización por tipo de estructura cuando sea necesario.

---

## 11. Qué sí incluye la V1

La primera versión del proyecto deberá incluir, como mínimo:

- ventana principal funcional,
- creación y edición de estructuras de datos 2D,
- representación visual estructurada,
- selección y manipulación básica de elementos,
- operaciones semánticas por tipo de estructura,
- guardado y carga en JSON,
- exportación a PNG,
- soporte para múltiples familias de estructuras,
- interfaz clásica y estable.

---

## 12. Qué no incluye la V1

La primera versión **no** incluirá:

- animaciones paso a paso de algoritmos,
- visualización 3D,
- colaboración en red,
- backend,
- sincronización en la nube,
- múltiples documentos abiertos simultáneamente,
- scripting avanzado,
- sistema de plugins externo,
- optimización extrema,
- enfoque comercial o empresarial.

---

## 13. Valor del proyecto

Este proyecto tiene valor en varios niveles:

### Valor educativo
Permite representar estructuras de datos de forma clara y exportable.

### Valor técnico
Sirve como práctica seria de:
- C,
- GUI nativa,
- CMake,
- arquitectura modular,
- serialización,
- renderizado,
- diseño de software.

### Valor de portafolio
Puede convertirse en una pieza demostrativa sólida de desarrollo de software en C con interfaz gráfica.

---

## 14. Visión general de arquitectura

A nivel alto, el sistema se dividirá en módulos internos, sin backend separado.

Las áreas principales serán:

- **core**: dominio y reglas de las estructuras,
- **editor**: lógica de edición e interacción,
- **ui**: interfaz de escritorio,
- **render**: representación visual 2D,
- **persistence**: guardado y carga de documentos,
- **common**: infraestructura técnica compartida,
- **app**: inicialización y coordinación general.

La arquitectura detallada se definirá en documentos específicos.

---

## 15. Estado actual del proyecto

En la etapa actual, el proyecto se encuentra en fase de:

- definición conceptual,
- delimitación del alcance,
- selección de tecnologías,
- y preparación de la arquitectura documental y técnica.

Todavía no se considera cerrado el detalle fino de:
- operaciones exactas por estructura,
- modelo de datos completo,
- formato JSON final,
- layout visual específico de cada estructura,
- y distribución final de carpetas y módulos.

---

## 16. Resumen ejecutivo

StructStudio C será una aplicación de escritorio en C, con interfaz nativa de estilo clásico, orientada a crear y manipular visualmente estructuras de datos 2D con enfoque educativo.

El sistema trabajará con documentos locales, guardados en JSON y exportables a PNG. Tendrá un modo estructurado, una sola vista de documento a la vez, soporte para múltiples familias de estructuras de datos y un diseño interno modular que permita crecimiento futuro.

El proyecto no busca ser un producto comercial ni una librería productiva, sino una herramienta educativa y un ejercicio serio de ingeniería de software en C.