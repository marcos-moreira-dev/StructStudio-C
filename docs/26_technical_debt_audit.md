# 26_technical_debt_audit.md

## 1. Propósito

Este documento resume la deuda técnica relevante detectada en la versión actual de StructStudio C.

La idea no es criticar el proyecto, sino dejar trazabilidad de:

- qué zonas ya están razonablemente sanas,
- qué zonas siguen cargadas,
- qué refactors ya se aplicaron,
- y qué conviene atacar después.

---

## 2. Diagnóstico corto

El proyecto ya tiene una base modular real y utilizable.

Las capas `core`, `editor`, `render`, `persistence`, `ui` y `app` existen y no están completamente mezcladas.

Sin embargo, la deuda técnica sigue concentrada en tres frentes:

- una UI principal demasiado grande,
- documentación abundante pero dispersa,
- y algunas rutas de interacción/animación todavía sensibles al costo del hilo de UI.

---

## 3. Estado por módulo

### 3.1 `core/`

Estado: aceptable a bueno.

Fortalezas:

- la API del dominio ya se dividió en archivos más pequeños,
- las operaciones por familia están razonablemente aisladas,
- y el núcleo no depende de la GUI.

Deuda restante:

- algunos módulos siguen largos, especialmente análisis y teoría,
- faltan pruebas más agresivas para entradas inválidas y combinaciones raras,
- y la carga de JSON merece mayor endurecimiento.

### 3.2 `editor/`

Estado: bueno, pero sensible.

Fortalezas:

- concentra interacción y coordinación,
- separa bien el dominio de la UI,
- y ya tiene un subsistema explícito de animación y playback.

Deuda restante:

- el editor coordina muchas responsabilidades temporales,
- varias transiciones dependen del rendimiento del hilo principal,
- y todavía hay margen para separar mejor playback, historial y tool-state.

### 3.3 `render/`

Estado: aceptable.

Fortalezas:

- el render no es dueño del dominio,
- el PNG está desacoplado,
- y el canvas ya tiene una dirección visual clara.

Deuda restante:

- el archivo sigue siendo grande,
- la complejidad crece con badges, teoría visual y resaltados,
- y la fluidez sigue dependiendo de no pedir resincronizaciones completas desde UI.

### 3.4 `ui/`

Estado: deuda alta.

Fortalezas:

- el comportamiento visible ya funciona,
- la ventana principal cubre bastante funcionalidad,
- y se han corregido varios problemas reales de workspace, paneles y autoplay.

Deuda restante:

- `main_window.c` sigue demasiado grande,
- mezcla construcción de widgets, wiring de eventos, sincronización y reglas de presentación,
- y es el mayor foco de mantenimiento del proyecto.

### 3.5 `persistence/`

Estado: medio.

Fortalezas:

- el almacenamiento JSON está encapsulado.

Deuda restante:

- sigue faltando robustecer rutas de carga ante JSON incompleto o malformado.

---

## 4. Refactors aplicados recientemente

En la etapa reciente se introdujeron refactors acotados pero útiles:

- separación entre sincronización completa de UI y sincronización ligera de animaciones en `main_window.c`,
- reducción de reconstrucciones innecesarias del workspace,
- reubicación de rotaciones e historial en el bloque `Herramientas`,
- y endurecimiento del flujo de conexión ponderada en el lienzo.

Estos cambios no reescriben la arquitectura, pero sí bajan costo cognitivo y riesgo de regresión.

---

## 5. Deuda técnica prioritaria

### 5.1 Prioridad alta

- dividir `src/ui/main_window.c` en módulos internos de UI:
  - construcción de panel izquierdo,
  - construcción de panel derecho,
  - menús,
  - sincronización de estado,
  - eventos del canvas.
- robustecer `src/persistence/document_io.c` ante JSON inválido.
- seguir reduciendo trabajo en el hilo de UI durante animaciones automáticas.

### 5.2 Prioridad media

- compactar y separar `src/render/render.c`,
- separar teoría contextual de algoritmos en `src/core/api_theory.c`,
- y ampliar pruebas de interacción y regresión.

### 5.3 Prioridad baja

- limpiar aún más textos y consistencia terminológica,
- mejorar scroll real de ciertos paneles si se decide usar una solución específica de Windows,
- y revisar afinación visual menor.

---

## 6. Riesgos arquitectónicos si no se actúa

Si no se sigue refactorizando, los riesgos más probables son:

- más lentitud para introducir funcionalidades nuevas,
- regresiones en la UI por tocar `main_window.c`,
- más dificultad para entender qué cambio pertenece a qué capa,
- y documentación que envejece más rápido de lo que el código cambia.

---

## 7. Estrategia recomendada

La estrategia correcta no es reescribir el proyecto.

La estrategia correcta es:

1. seguir con refactors pequeños y seguros,
2. documentar contratos de módulo,
3. reforzar pruebas en zonas críticas,
4. y mantener separados UI, editor, render y dominio.

---

## 8. Conclusión

StructStudio C ya no está en estado de prototipo improvisado.

La base actual sí permite evolucionar el sistema, pero todavía hay una deuda técnica clara en la capa de UI y en la documentación dispersa.

El objetivo de este documento es dejar esa realidad explícita para que futuras mejoras no se hagan a ciegas.
