# Ejemplos y Espacio de Trabajo

Este documento resume dos mejoras recientes orientadas a uso real:

- ejemplos didacticos precargados,
- y control de visibilidad de paneles.

Tambien conviene leerlo junto con una mejora posterior pequena pero importante:

- en grafos, el destino escrito en el panel ya puede resolverse por `ID`, etiqueta o valor,
- y el canvas deja visible el identificador tecnico de cada vertice.

## 1. Ejemplos didacticos

Cada variante soportada puede cargar un caso base desde el boton `Cargar ejemplo`.

La intencion no es "rellenar por rellenar", sino dejar una estructura que ya permita:

- leer teoria con algo visible en canvas,
- probar operaciones sin empezar desde cero,
- ejecutar recorridos o analisis sobre un caso no vacio.

### 1.1 Donde vive esta logica

La receta de cada ejemplo no se puso en la ventana principal.

Se centralizo en:

- `src/core/api_examples.c`

Y el editor la expone con:

- `ss_editor_load_example()`

Eso mantiene una buena separacion:

- `core` sabe construir el ejemplo,
- `editor` sabe incorporarlo al documento activo,
- `ui` solo dispara la accion y refresca la ventana.

### 1.2 Por que es una mejor arquitectura

Si la UI construyera manualmente cada estructura:

- duplicaria reglas del dominio,
- seria dificil probarla sin la ventana,
- y `main_window.c` seguiria creciendo.

En cambio, al usar una API dedicada:

- el ejemplo se puede probar con smoke tests,
- el comportamiento queda reutilizable,
- y la interfaz solo orquesta.

## 2. Control de paneles

El menu `Ver` ahora permite:

- mostrar u ocultar panel izquierdo,
- mostrar u ocultar panel derecho,
- mostrar u ocultar grilla.

Esto mejora la experiencia por dos motivos:

- cuando el usuario quiere editar mucho, puede dar mas espacio al canvas,
- cuando quiere estudiar, puede mantener visible el panel de teoria.

## 3. Limpieza de codigo aplicada

En el editor habia varias secuencias repetidas:

- limpiar analisis,
- limpiar seleccion,
- validar,
- tocar el documento.

Se encapsulo ese patron con helpers internos pequeños en `src/editor/editor.c`.

La meta no fue "hacerlo elegante" en abstracto, sino reducir repeticion y bajar el riesgo de olvidar un paso de sincronizacion cuando se agrega una accion nueva.

## 4. Cobertura

Se agrego:

- `tests/examples_smoke.c`

La prueba revisa ejemplos representativos de:

- `BST`,
- `Grafo no dirigido ponderado`,
- `Map`.
