# 16_usability_improvements.md

## 1. Proposito

Este documento resume las mejoras de usabilidad implementadas despues de la base funcional inicial de StructStudio C.

La idea es dejar claro:

- que cambios se hicieron para volver el programa mas intuitivo,
- por que esas decisiones ayudan al usuario,
- y como encajan con una GUI clasica tipo herramienta de escritorio.

---

## 2. Problema detectado

La aplicacion ya permitia crear, editar, conectar y exportar estructuras, pero todavia habia friccion de uso:

- algunas operaciones exigian conocer reglas no visibles,
- no siempre era obvio que campo debia llenarse,
- el usuario podia intentar acciones que no aplicaban al contexto actual,
- y faltaban accesos rapidos de UX previstos por la especificacion.

En terminos de arquitectura, la capacidad existia.

Lo que faltaba era hacerla mas explicita para el usuario.

---

## 3. Mejoras implementadas

## 3.1 Guia contextual persistente

La ventana principal ahora incluye una seccion de guia contextual que explica:

- estructura activa,
- herramienta activa,
- pasos recomendados,
- significado practico de las operaciones,
- y recordatorios de `Esc` / `Delete`.

Esto reduce la necesidad de memorizar reglas internas.

---

## 3.2 Herramienta activa visible

Los botones de herramienta ahora muestran de forma explicita cual esta activa.

Ademas:

- `Conectar` se deshabilita si la estructura actual no soporta conexiones manuales.

Esto evita que el usuario entre en un modo sin sentido para la variante actual.

---

## 3.3 Operaciones guiadas por contexto

El panel de operaciones ahora comunica mejor que espera cada campo.

Ejemplos:

- en `Map`, `Principal` representa la clave y `Secundario` el valor,
- en `Priority Queue`, `Numero` representa la prioridad,
- en grafos ponderados, `Numero` se usa como peso.

Tambien se habilitan o deshabilitan botones segun prerequisitos de seleccion.

Por ejemplo:

- operaciones que requieren nodo seleccionado quedan bloqueadas hasta que exista esa seleccion,
- y eliminar arista en grafo exige tener una arista seleccionada.

---

## 3.4 Propiedades con permisos coherentes

El panel derecho ahora refleja mejor el contexto:

- sin seleccion editable, queda en modo solo lectura,
- con nodo seleccionado, habilita los campos que realmente aplican,
- con arista seleccionada, habilita solo lo relevante para aristas.

Esto evita la falsa expectativa de que cualquier texto visible puede aplicarse siempre.

---

## 3.5 Menus de uso diario completados

Se implementaron funciones de UX que ya estaban previstas en la especificacion:

- `Editar > Deseleccionar`
- `Editar > Limpiar estructura`
- `Ver > Mostrar grilla`
- `Ayuda > Informacion del documento`

Estas acciones reducen pasos innecesarios y acercan la experiencia al estilo clasico de software de escritorio.

---

## 4. Principios de software aplicados

## 4.1 UX guiada por estado

La UI no solo muestra datos: tambien comunica cuando una accion esta disponible y cuando no.

Eso es importante porque:

- baja carga cognitiva,
- evita errores evitables,
- y mejora el aprendizaje del producto.

---

## 4.2 Desacoplamiento

Las reglas de uso no se implementaron metiendo logica del dominio en cualquier callback.

Se mantuvo el criterio:

- `editor/` coordina acciones frecuentes como limpiar estructura o alternar grilla,
- `ui/` refleja estado y traduce eventos,
- `core/` sigue gobernando la semantica real de las estructuras.

Eso mantiene legibilidad y facilita evolucionar la UX sin romper el nucleo.

---

## 4.3 Prevencion de error antes que correccion tardia

En vez de dejar que el usuario falle siempre y luego mostrar un error:

- varios controles ahora se deshabilitan cuando no aplican.

Esta es una buena practica clasica de UX en herramientas tecnicas:

- prevenir primero,
- validar despues,
- y explicar siempre.

---

## 5. Estado de verificacion

Despues de estas mejoras se verifico:

- compilacion con `cmake --build build`
- pruebas con `ctest --test-dir build --output-on-failure`
- arranque del ejecutable en Windows

Ademas, la suite minima ahora cubre tambien:

- limpieza de estructura activa,
- y cambio del estado de grilla a nivel de editor.

---

## 6. Siguiente paso natural

Las siguientes mejoras de intuicion que tendrian mejor retorno son:

- hover visual para nodos y aristas,
- ayuda contextual tambien sobre el canvas,
- atajos de teclado globales para nuevo / abrir / guardar,
- y ejemplos precargados por tipo de estructura.
