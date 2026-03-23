# Teoria Contextual

Este documento describe el apartado `Teoria y recorridos` de StructStudio C.

## 1. Objetivo

El software ya permitia editar y analizar estructuras, pero faltaba una via directa para responder una pregunta muy comun de estudiantes sin obligarlos a abrir cuadros extra:

> "Que concepto estoy aplicando aqui exactamente?"

Por eso la teoria paso a formar parte fija del panel derecho.

## 2. Que explica

El contenido depende del estado actual de la aplicacion y se actualiza automaticamente:

- siempre explica la estructura activa,
- describe su familia y variantes relacionadas,
- enumera los recorridos o algoritmos relevantes para esa variante,
- y si existe un analisis seleccionado, agrega tambien una seccion de foco sobre ese algoritmo.

Ejemplos:

- `BST` + `Inorden`
- `Queue` sin analisis activo
- `Grafo dirigido ponderado` + `Dijkstra`

## 3. Arquitectura aplicada

La teoria no se escribio directamente dentro de la ventana principal.

Se centralizo en:

- `src/core/api_theory.c`

La razon tecnica es simple:

- la UI no deberia almacenar conocimiento academico,
- el texto educativo debe poder probarse como cualquier otra API,
- y si luego se agrega exportacion o ayuda enriquecida, el catalogo ya existe fuera de la GUI.

## 4. Flujo de uso

1. el usuario activa una estructura
2. opcionalmente escoge un analisis
3. el panel derecho se refresca en vivo
4. el usuario puede estudiar teoria y editar propiedades sin abandonar el canvas

El cambio busca reducir fragmentacion visual: ya no hace falta abrir un modal para consultar conceptos basicos mientras se edita.

## 5. Buenas practicas que se aplicaron

### 5.1 Responsabilidad unica

- `core` sabe que explicar
- `ui` solo sabe como mostrarlo

### 5.2 Reutilizacion

El mismo resumen teorico puede usarse en:

- el panel derecho persistente,
- pruebas automatizadas,
- futuras exportaciones o paneles laterales mas ricos.

### 5.3 Legibilidad para principiantes

Cada resumen intenta incluir:

- definicion corta,
- idea clave,
- relacion con lo que el usuario ve en StructStudio.

Eso evita una teoria demasiado abstracta o separada del canvas.

## 6. Cobertura automatizada

Se agrego `tests/theory_smoke.c`.

La prueba verifica que:

- un `BST` con `Inorden` describa estructura, variantes y recorridos,
- un grafo dirigido ponderado con `Dijkstra` describa estructura, variantes y algoritmos disponibles,
- y una estructura sin analisis seleccionado siga mostrando teoria util y recorridos aplicables.
