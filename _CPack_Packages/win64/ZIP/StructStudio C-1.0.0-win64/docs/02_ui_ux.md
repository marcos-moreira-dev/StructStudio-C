# 02_ui_ux.md

## 1. Proposito

Este documento describe la interfaz y la experiencia de uso actual de StructStudio C.

La idea central es combinar dos necesidades:

- editar TDAs de manera visual,
- y estudiar teoria sin abandonar el area de trabajo.

La interfaz busca sentirse como una aplicacion de escritorio clasica, sobria y tecnica, en una linea cercana a herramientas de la era Windows 7.

---

## 2. Objetivo de UX

La aplicacion debe sentirse:

- clara,
- ordenada,
- estable,
- educativa,
- y suficientemente formal para laboratorio o clase.

El usuario no deberia preguntarse:

- donde edito,
- donde veo el estado,
- o donde consulto la teoria del TDA.

---

## 3. Principios

### 3.1 Claridad antes que espectacularidad

La pantalla puede ser rica en informacion, pero no debe repartir la misma idea en muchos sitios.

### 3.2 Flujo continuo

Editar, analizar y estudiar teoria deben convivir en una sola vista principal.

### 3.3 Feedback constante

La interfaz debe indicar en todo momento:

- estructura activa,
- herramienta activa,
- seleccion actual,
- analisis activo,
- y mensaje de estado.

### 3.4 Teoria visible, no escondida

La teoria contextual debe vivir en el panel derecho y actualizarse sola.

Eso evita abrir modales repetitivos y mantiene el aprendizaje pegado al canvas.

---

## 4. Distribucion general

La ventana principal se divide en seis zonas:

1. barra de menus superior
2. panel lateral izquierdo
3. barra de pestanas del lienzo
4. canvas central
5. panel lateral derecho
6. barra de estado inferior

---

## 5. Menus

### 5.1 Archivo

- Nuevo
- Abrir
- Guardar
- Guardar como
- Exportar PNG
- Salir

### 5.2 Editar

- Deseleccionar
- Eliminar seleccion
- Limpiar estructura

### 5.3 Ver

- Lienzo + herramientas
- Lienzo + herramientas + teoria
- Mostrar grilla

### 5.4 Ayuda

- Informacion del documento
- Acerca de

El menu se mantiene corto a proposito: esta aplicacion privilegia acciones visibles en los paneles laterales.

---

## 6. Panel izquierdo

El panel izquierdo es la zona de trabajo activa.

Contiene:

- seleccion de nueva variante,
- carga de ejemplo didactico para la variante activa,
- herramientas del canvas,
- operaciones propias del TDA,
- analisis y playback guiado,
- guia contextual.

### 6.1 Estructura

Permite:

- elegir la variante de la siguiente pestana,
- abrir otra estructura dentro del documento,
- reinicializar la pestana actual con otra variante,
- cargar un ejemplo guiado para no empezar desde cero.

La seleccion de la estructura activa ya no vive en un combo: se hace desde la barra de pestanas sobre el lienzo.

## 7. Barra de pestanas del lienzo

La barra superior del area de trabajo cumple una doble funcion:

- mostrar todas las estructuras abiertas,
- y permitir cambiar de contexto sin abandonar el canvas.

Sobre esa misma franja viven acciones de enfoque del workspace:

- `Lienzo + herramientas`
- `Lienzo + herramientas + teoria`

El panel izquierdo queda siempre visible para simplificar el flujo de trabajo. La unica decision de workspace es si el panel derecho de teoria y propiedades debe mostrarse o no.

Cuando un analisis genera un arbol derivado desde un grafo, ese resultado aparece como otra pestana. Esto facilita comparar el grafo original con su estructura derivada sin perder el estado del documento.

### 6.2 Herramientas

Incluye:

- Seleccionar
- Insertar
- Conectar
- Eliminar
- Mostrar grilla

Los botones dejan visible cual herramienta esta activa.

### 6.3 Operaciones

La V1 dejo de depender de tres cajas de texto siempre visibles.

Ahora la interfaz:

- renombra los campos segun el TDA activo (`Indice`, `Clave`, `Valor`, `Destino`, `Peso`, etc.),
- oculta entradas que no aplican a la operacion del momento,
- mantiene el entero opcional plegado cuando solo sirve como apoyo didactico,
- y muestra dos mensajes cortos: uno para explicar que hace el bloque y otro para decir que seleccion hace falta en el lienzo.

Esto reduce ruido visual y hace mas evidente una regla importante del producto:

- si un dato no es necesario en ese TDA, el usuario no deberia verlo como requisito.

### 6.4 Analisis

Desde aqui el usuario puede:

- elegir un recorrido o algoritmo,
- indicar origen si aplica,
- ver resumen textual,
- simular un recorrido paso a paso,
- activar `Auto-reproducir`,
- avanzar o retroceder en el playback.

La separacion es deliberada:

- `Ver resumen` sirve para obtener el resultado textual del algoritmo sin entrar en modo simulacion.
- `Simular` prepara la animacion e inicia la reproduccion automatica si el algoritmo genera pasos.

El playback puede pausarse, reiniciarse o recorrerse paso a paso desde los mismos controles.

Estas secciones no usan ya un `tab` interno pesado; el panel izquierdo cambia entre `Operaciones`, `Análisis` y `Guía` mediante botones de seccion para evitar ensanchamientos raros y reducir complejidad de layout.

### 6.5 Guia contextual

La guia explica:

- que espera la estructura actual,
- que significan los campos,
- que seleccion hay,
- y que paso del playback esta activo si existe.

---

## 8. Canvas central

El canvas es la zona principal de manipulacion.

Debe permitir:

- seleccionar nodos o aristas,
- arrastrar nodos cuando la variante lo admite,
- conectar entidades,
- eliminar,
- y observar resaltados de analisis.

### 7.1 Direccion visual

El canvas usa una estetica sobria con:

- grilla discreta,
- fondo uniforme, sin una banda inferior semitransparente que parta visualmente el espacio,
- resaltado clasico de seleccion,
- badges para pesos o marcas semanticas.

### 7.2 Comportamientos clave

- clic para seleccionar,
- drag para mover,
- dos clics para conectar,
- `Esc` para cancelar,
- `Delete` para borrar,
- flechas izquierda/derecha para playback guiado,
- `Space` para alternar la reproduccion automatica del recorrido preparado.

En grafos, el canvas tambien debe exponer el identificador tecnico de cada vertice para que las operaciones escritas en el panel izquierdo no dependan de memoria del usuario.
Cuando el usuario arrastra o panea, el render reduce decoraciones no esenciales para priorizar respuesta visual.

### 7.3 Animacion funcional

La animacion no se usa como adorno aislado.

Se aplica cuando ayuda a comprender:

- que nodo o arista es el paso actual de un algoritmo,
- como un auto-layout recoloca la estructura,
- o como una operacion reordena un arbol, heap o familia lineal.

Por eso el canvas ahora combina:

- pulso temporal del paso actual,
- reacomodo interpolado de nodos,
- y un badge `AUTO` cuando el playback esta reproduciendose solo.

En grafos tambien existe rotacion visual controlada:

- `Girar izq`
- `Girar der`

Esta transformacion no altera la teoria del algoritmo subyacente; reorienta la geometria dibujada para estudiar mejor relaciones, arboles derivados o comparaciones visuales.

En BST y AVL la rotacion debe explicarse mejor que un simple cambio de forma:

- al pulsar una rotacion se selecciona el nodo promovido,
- la barra de estado resume quien sube y quien baja,
- y la guia contextual indica si el pivote actual tiene o no los hijos necesarios para cada rotacion.

El titulo del canvas se apoya sobre una tarjeta discreta para que el nombre de la estructura siga siendo legible aun con grilla y elementos semanticos detras.

---

## 9. Panel derecho

El panel derecho ya no es solo de propiedades.

Ahora combina dos bloques:

- `Propiedades`
- `Teoria y recorridos`

### 8.1 Propiedades

Permite editar el elemento seleccionado cuando eso tiene sentido:

- nodos,
- aristas,
- o informacion resumida de la estructura completa.

### 8.2 Teoria y recorridos

Es un panel multilinea con scroll vertical.

Debe mostrar automaticamente:

- descripcion del TDA activo,
- familia semantica,
- variantes relacionadas,
- recorridos o algoritmos aplicables,
- un caso de referencia listo para relacionar teoria con representacion visual,
- y, si hay analisis seleccionado, una seccion que profundiza en ese algoritmo.

La teoria debe poder leerse sin tapar el canvas ni abrir ventanas nuevas.

Si el usuario necesita mas espacio visual, puede ocultar este panel y recuperarlo desde el menu `Ver`.

---

## 10. Barra de estado

La barra inferior resume:

- mensaje operativo mas reciente,
- y resultado de validacion mas importante.

No debe duplicar toda la guia contextual; solo debe servir como lectura rapida.

---

## 11. Decisiones de UX aplicadas

### 10.1 Menos redundancia

La teoria ya no vive en un boton separado del flujo principal.

### 10.2 Aprendizaje situado

El estudiante puede leer la explicacion del TDA justo al lado del canvas que lo representa.

### 10.3 Coherencia visual

El panel izquierdo concentra acciones.
El panel derecho concentra consulta y edicion.

Esa separacion ayuda a que la interfaz se entienda rapido.
