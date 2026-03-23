# 12_roadmap.md

## 1. Propósito del documento

Este documento define la **hoja de ruta de desarrollo** de **StructStudio C**.

Su objetivo es organizar la construcción del proyecto de manera realista, ordenada y técnica, pero con una decisión importante ya tomada:

> el proyecto se planificará como **una sola V1 completa**, no como varias V1, V2 o V3 de producto.

Eso significa que aquí no se dividirá el producto en versiones funcionales independientes, sino en **fases internas de construcción** que conduzcan a una única **V1 completa y coherente**.

---

## 2. Decisión principal del roadmap

### Decisión cerrada
El proyecto tendrá:

- **una sola versión objetivo: V1**
- y esa V1 se construirá por etapas internas.

### Qué significa eso
No habrá una lógica de:
- “V1 mínima”
- “V2 mejorada”
- “V3 avanzada”

sino una lógica de:

- **fase 1 de implementación**
- **fase 2 de implementación**
- **fase 3 de implementación**
- ...
- hasta completar la **V1 final**.

### Beneficio
Esto evita fragmentar demasiado la visión del producto y mantiene el foco en un solo objetivo completo.

---

## 3. Objetivo final de la V1

La V1 final de StructStudio C deberá ser una aplicación de escritorio en C que permita:

- crear y abrir documentos de estructuras de datos 2D,
- trabajar con una estructura activa a la vez,
- editar múltiples tipos de estructuras enseñadas en un entorno universitario,
- ejecutar operaciones semánticas por tipo,
- validar restricciones estructurales,
- guardar y cargar documentos en JSON,
- exportar la vista actual a PNG,
- usar una interfaz clásica de escritorio,
- y correr como proyecto serio con CMake y organización modular.

---

## 4. Alcance final esperado de la V1

La V1 se considerará completa cuando incluya, en conjunto:

### 4.1 Infraestructura técnica
- proyecto en C17
- CMake funcional
- estructura de carpetas modular
- build Debug y Release
- compilación razonable en Windows y Linux

### 4.2 Interfaz de escritorio
- ventana principal
- menú superior
- panel izquierdo
- canvas central
- panel derecho
- barra de estado
- estética sobria y clásica

### 4.3 Núcleo del dominio
- documento
- estructuras
- nodos
- aristas
- configuración por tipo
- validación básica y especializada
- operaciones por estructura

### 4.4 Editor
- selección
- arrastre
- inserción
- conexión
- eliminación
- edición de propiedades
- auto-layout
- mensajes claros

### 4.5 Persistencia
- guardar JSON
- cargar JSON
- validar integridad básica al cargar
- ejemplos de documentos

### 4.6 Exportación visual
- exportar PNG

### 4.7 Estructuras soportadas
- vector
- listas enlazadas
- stack
- queue
- priority queue
- árboles
- BST
- AVL
- heap
- set
- map
- grafos dirigidos y no dirigidos, ponderados y no ponderados

---

## 5. Estrategia general de construcción

Como se trata de una sola V1 completa, la estrategia correcta es construir el proyecto por **capas acumulativas**.

Cada fase debe dejar algo usable internamente, pero sin venderse como “versión pública separada”.

La lógica es:

- primero asegurar base técnica,
- luego asegurar modelo,
- luego asegurar editor,
- luego asegurar persistencia,
- luego completar familias de estructuras,
- luego cerrar estabilidad y acabado.

---

## 6. Fase 0 — Preparación del proyecto

### Objetivo
Dejar listo el terreno técnico y documental antes de entrar a implementación fuerte.

### Resultados esperados
- estructura del repositorio creada
- CMake raíz funcionando
- módulos base creados
- documento de arquitectura disponible
- documentos Markdown principales listos
- carpeta `samples/` creada
- carpeta `tests/` creada

### Entregables
- árbol base del proyecto
- CMake inicial
- ejecutable mínimo que arranque una ventana o base de app

### Criterio de cierre
El proyecto compila y tiene una base limpia para crecer.

---

## 7. Fase 1 — Núcleo mínimo del dominio

### Objetivo
Construir el modelo central del sistema antes de depender demasiado de la interfaz.

### Alcance
- `Document`
- `Structure`
- `StructureKind`
- `StructureConfig`
- `Node`
- `Edge`
- IDs
- estado visual mínimo
- estructuras base de validación

### Resultados esperados
- el sistema puede representar en memoria una estructura simple
- se puede crear un documento y una estructura activa
- existen entidades serializables y navegables desde código

### Entregables
- módulos `core/` base
- creación programática de documentos y estructuras
- pruebas unitarias mínimas del modelo

### Criterio de cierre
El dominio ya existe como base real, aunque la GUI todavía sea limitada.

---

## 8. Fase 2 — Ventana principal y esqueleto de interfaz

### Objetivo
Levantar la aplicación de escritorio con su carcasa principal.

### Alcance
- ventana principal
- menú superior
- panel izquierdo
- canvas central
- panel derecho
- barra de estado
- estructura básica de eventos

### Resultados esperados
- la aplicación ya parece una herramienta de escritorio
- la UI puede mostrar estado básico del documento
- existe integración inicial con el editor

### Entregables
- módulo `ui/` inicial funcional
- interacción básica entre menú, paneles y contexto activo

### Criterio de cierre
La app abre, muestra su layout principal y permite navegar un estado básico del documento.

---

## 9. Fase 3 — Render base del canvas

### Objetivo
Hacer visible el modelo sobre el área de trabajo.

### Alcance
- render del fondo
- render de nodos
- render de aristas
- render de etiquetas
- render de selección
- render de estados simples

### Resultados esperados
- una estructura simple ya puede verse en pantalla
- mover o cambiar elementos actualiza el canvas

### Entregables
- módulo `render/` base
- primeros estilos visuales consistentes

### Criterio de cierre
El documento puede visualizarse de forma clara aunque todavía no estén todas las herramientas terminadas.

---

## 10. Fase 4 — Lógica básica del editor

### Objetivo
Hacer que el usuario pueda interactuar de verdad con el canvas.

### Alcance
- `SelectionState`
- `ToolState`
- `DragState`
- clic sobre nodo
- clic sobre arista
- clic sobre vacío
- mover nodo
- eliminar selección
- cambio de herramienta

### Resultados esperados
- el usuario ya puede seleccionar y mover elementos
- el editor mantiene estados coherentes
- la app empieza a sentirse viva

### Entregables
- módulo `editor/` funcional en lo esencial
- integración UI ↔ editor ↔ render

### Criterio de cierre
El usuario puede interactuar de forma básica y estable con entidades visibles.

---

## 11. Fase 5 — Persistencia JSON

### Objetivo
Permitir guardar y recuperar trabajo real.

### Alcance
- serialización de documento
- deserialización de documento
- validación básica al cargar
- manejo de errores de archivo
- pruebas de round-trip

### Resultados esperados
- se puede guardar un documento
- se puede cerrarlo y abrirlo nuevamente
- se conserva estado lógico y visual relevante

### Entregables
- módulo `persistence/` funcional
- varios `samples/` válidos
- pruebas de persistencia

### Criterio de cierre
El proyecto deja de ser una sesión temporal y pasa a manejar documentos reales.

---

## 12. Fase 6 — Primera familia estructural completa

### Objetivo
Consolidar el pipeline completo sobre una familia de estructuras relativamente amigable.

### Familia recomendada para esta fase
- vector
- listas enlazadas

### Alcance
- operaciones
- validaciones
- render específico
- edición contextual
- layout básico por tipo

### Resultados esperados
- al menos una familia ya se comporta de extremo a extremo:
  - dominio
  - editor
  - render
  - persistencia

### Entregables
- soporte completo para vector y listas

### Criterio de cierre
La arquitectura demuestra que funciona de verdad sobre casos concretos.

---

## 13. Fase 7 — Stack, queue y priority queue

### Objetivo
Completar estructuras secuenciales operativas.

### Alcance
- stack
- queue
- circular queue
- priority queue

### Resultados esperados
- operaciones como `push`, `pop`, `enqueue`, `dequeue` y edición de prioridad ya funcionan
- el editor maneja bien acciones más semánticas y menos “dibujadas a mano”

### Entregables
- soporte completo para estas estructuras
- validaciones específicas
- render coherente por tipo

### Criterio de cierre
Las estructuras secuenciales principales del curso quedan cubiertas.

---

## 14. Fase 8 — Árboles y heap

### Objetivo
Cubrir las estructuras jerárquicas.

### Alcance
- árbol binario
- BST
- AVL
- heap

### Resultados esperados
- inserción y edición razonables
- validación de hijos
- validación BST
- rebalanceo AVL
- heapify y extracción principal

### Entregables
- soporte completo de estructuras arbóreas principales
- layout vertical claro

### Criterio de cierre
La parte jerárquica del proyecto ya está integrada y usable.

---

## 15. Fase 9 — Set y map

### Objetivo
Agregar estructuras de colección con reglas de unicidad y asociación.

### Alcance
- set
- map
- validación de duplicados
- validación de clave única
- edición de pares clave-valor

### Resultados esperados
- el sistema soporta estructuras no lineales sin depender de aristas tradicionales
- el modelo especializado queda confirmado

### Entregables
- soporte funcional completo de set y map

### Criterio de cierre
La arquitectura demuestra que también puede manejar nodos especializados que no encajan exactamente como lista o árbol.

---

## 16. Fase 10 — Grafos

### Objetivo
Completar una de las familias más visuales y abiertas del sistema.

### Alcance
- vértices
- aristas
- grafos dirigidos
- grafos no dirigidos
- grafos ponderados
- grafos no ponderados
- edición de peso
- edición de dirección
- movimiento libre de vértices

### Resultados esperados
- el usuario puede construir grafos complejos de forma clara
- el editor puede manejar conexiones más libres sin romper su lógica estructurada

### Entregables
- familia `graph` completa en la V1
- samples de grafos representativos

### Criterio de cierre
La familia más flexible del proyecto queda completamente integrada.

---

## 17. Fase 11 — Auto-layout y pulido por estructura

### Objetivo
Mejorar legibilidad y comodidad de edición.

### Alcance
- auto-layout por tipo
- reajustes visuales por familia
- espaciado mejorado
- legibilidad de textos
- marcadores como `top`, `front`, `rear`, `root`
- mejora de selección y estados visuales

### Resultados esperados
- el programa se siente más sólido y más útil para estudio o exportación

### Entregables
- layouts base razonables para todas las familias
- ajustes visuales finales de V1

### Criterio de cierre
Las estructuras no solo funcionan: se ven bien y se entienden bien.

---

## 18. Fase 12 — Exportación PNG

### Objetivo
Cerrar la salida visual útil del proyecto.

### Alcance
- exportación de la vista actual
- validación del resultado
- pruebas con estructuras variadas

### Resultados esperados
- el usuario puede generar material visual desde la app

### Entregables
- exportación PNG funcional
- ejemplos exportados de prueba

### Criterio de cierre
La app ya sirve también como generador de material visual educativo.

---

## 19. Fase 13 — Estabilidad, limpieza y cierre de V1

### Objetivo
Consolidar todo lo construido y cerrar la V1 completa.

### Alcance
- limpieza de código
- revisión de módulos
- reducción de acoplamiento innecesario
- revisión de errores frecuentes
- mejora de mensajes
- pruebas manuales integrales
- pruebas automáticas de partes críticas
- revisión de samples
- revisión de README e instrucciones de build

### Resultados esperados
- proyecto estable,
- coherente,
- relativamente presentable,
- y usable como pieza de portafolio y práctica técnica seria.

### Entregables
- V1 completa cerrada
- documentación base coherente
- ejecutable funcional
- ejemplos de uso

### Criterio de cierre
La aplicación ya cumple la visión general definida en `00_overview.md`.

---

## 20. Orden de prioridad real

Aunque todas las fases importan, el orden de prioridad práctica sería:

1. base técnica
2. modelo de dominio
3. UI base
4. render base
5. editor base
6. persistencia
7. familias estructurales
8. grafos
9. pulido visual
10. exportación
11. estabilización final

---

## 21. Dependencias entre fases

### Dependencias fuertes
- no conviene construir operaciones complejas sin núcleo de dominio
- no conviene insistir en pulido visual sin render base
- no conviene expandir familias completas sin persistencia relativamente estable
- no conviene cerrar V1 sin limpieza y pruebas

### Regla
Cada fase debe apoyarse en la anterior, no desordenarse por entusiasmo momentáneo.

---

## 22. Qué significa “V1 completa” en términos prácticos

La V1 estará completa cuando el proyecto ya no dependa de “luego lo arreglo” para sus pilares centrales.

Eso implica que ya debe estar resuelto, aunque sea de forma moderada y sobria:

- dominio
- UI
- editor
- render
- persistencia
- exportación
- estructuras definidas
- build system
- documentación base

No significa perfección absoluta.
Significa **coherencia funcional suficiente para considerar el producto terminado en su primera versión real**.

---

## 23. Qué no debe hacerse en este roadmap

Para respetar la idea de una sola V1 completa, conviene evitar:

- partir el producto en demasiadas micro-versiones
- intentar publicar algo como “casi V1” si aún no tiene persistencia o editor decente
- inflar la V1 con animaciones paso a paso o extras decorativos
- meter funciones experimentales que distraigan del objetivo central

---

## 24. Riesgos principales del desarrollo

### 24.1 Sobreextensión por cantidad de estructuras
Hay muchas familias, así que conviene avanzar por bloques.

### 24.2 Querer embellecer demasiado pronto
Primero debe funcionar la arquitectura.

### 24.3 Resolver cada estructura como un mundo aparte
Eso rompería la coherencia del sistema.

### 24.4 Postergar demasiado persistencia
Sin persistencia, el proyecto tarda mucho en sentirse real.

### 24.5 Querer perfección antes de cerrar la V1
La meta es una V1 sólida, no infinita.

---

## 25. Criterios de éxito del roadmap

El roadmap habrá cumplido su función si permite que el proyecto llegue a una sola V1 completa con:

- implementación ordenada,
- arquitectura coherente,
- documentación alineada,
- build reproducible,
- familias estructurales operativas,
- persistencia real,
- y una experiencia de escritorio clara y seria.

---

## 26. Relación con otros documentos

Este archivo se complementa con:

- `00_overview.md` → visión global del proyecto
- `01_product_spec.md` → alcance funcional de la V1
- `03_architecture.md` → arquitectura interna
- `06_operations.md` → operaciones que deben quedar cubiertas
- `11_build_system.md` → infraestructura técnica necesaria para construir la V1
