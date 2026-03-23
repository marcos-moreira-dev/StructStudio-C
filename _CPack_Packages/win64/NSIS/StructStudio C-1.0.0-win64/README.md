# StructStudio C

StructStudio C es una aplicacion de escritorio hecha en `C17` para construir, editar, analizar y explicar estructuras de datos desde una interfaz visual nativa.

No es solo un editor de nodos. Tampoco es solo un visor teorico. El proyecto combina ambas cosas:

- un canvas interactivo para manipular TDAs,
- un sistema didactico para simular recorridos y algoritmos,
- y un panel teorico permanente para estudiar mientras trabajas.

## Vista general

![Captura principal](images/Struct%20Studio%20C.png)

## Que hace especial al sistema

StructStudio C esta pensado para clase, laboratorio, demostracion y estudio individual.

Permite:

- abrir varias estructuras dentro del mismo documento,
- compararlas por pestanas,
- cargar ejemplos didacticos listos para usar,
- ejecutar operaciones semanticas reales,
- simular recorridos paso a paso,
- reproducirlos automaticamente,
- generar estructuras derivadas desde grafos,
- editar propiedades visibles,
- guardar en JSON,
- y exportar el resultado como PNG.

En otras palabras: el sistema sirve tanto para construir como para explicar.

## Capacidades principales

### 1. Edicion visual del lienzo

El canvas central permite:

- seleccionar nodos y aristas,
- arrastrar nodos cuando la familia lo admite,
- conectar con previsualizacion,
- eliminar elementos,
- usar paneo y recentrado de vista,
- mostrar u ocultar la grilla,
- y ver cambios de layout con transiciones animadas.

### 2. Operaciones semanticas por TDA

El panel izquierdo adapta sus entradas y botones a la estructura activa.

Eso significa que el sistema:

- cambia nombres de campos segun contexto,
- oculta parametros que no aplican,
- habilita solo acciones validas,
- y muestra una guia corta de que espera del usuario.

### 3. Analisis y simulacion

El bloque `Analisis` soporta:

- resumen textual de algoritmos,
- simulacion paso a paso,
- autoplay,
- pausa,
- avance y retroceso manual,
- reinicio del playback,
- y uso de teclado para recorrer pasos.

### 4. Teoria contextual

El panel derecho no es decorativo. Explica:

- el concepto del TDA activo,
- su familia,
- variantes relacionadas,
- recorridos o algoritmos aplicables,
- y el significado didactico del analisis actual.

### 5. Workspace multipestana

El documento puede contener varias estructuras al mismo tiempo.

Eso permite:

- comparar un BST con un AVL,
- trabajar con un grafo y su arbol BFS derivado,
- o mantener varios ejemplos abiertos dentro del mismo archivo.

## Estructuras soportadas

Actualmente el sistema soporta:

- Vector
- Lista simple
- Lista doble
- Lista circular
- Stack
- Queue
- Priority Queue
- Arbol binario
- BST
- AVL
- Heap
- Set
- Map
- Grafo dirigido
- Grafo no dirigido
- Grafo dirigido ponderado
- Grafo no dirigido ponderado

## Algoritmos y recorridos incluidos

StructStudio C ya integra analisis educativos reales para:

- preorden
- inorden
- postorden
- por niveles
- BFS
- DFS
- Dijkstra
- Floyd-Warshall
- Prim
- Kruskal

Ademas:

- BFS, DFS, Dijkstra y recorridos de arbol pueden reproducirse con playback guiado,
- el sistema resalta nodos y aristas visitados,
- y cuando aplica puede abrir un arbol derivado como nueva pestana.

## Rotaciones, giros y acciones visuales

El sistema distingue entre:

- rotaciones estructurales de arboles,
- y giros visuales de grafos.

Tambien incluye:

- deshacer y rehacer rotaciones,
- mensajes de estado didacticos cuando un nodo sube o baja,
- y resaltado del elemento activo durante animaciones.

## Flujo de uso recomendado

1. Elige una variante desde `Nueva`.
2. Crea o reinicia una pestana.
3. Inserta o carga un ejemplo.
4. Usa `Seleccionar`, `Insertar`, `Conectar` o `Eliminar`.
5. Consulta `Operaciones` para la accion contextual.
6. Ejecuta `Analisis` si quieres ver recorridos o algoritmos.
7. Usa el panel derecho para estudiar teoria y editar propiedades.
8. Guarda el documento o exporta la vista a PNG.

## Interfaz actual

La UI principal se organiza asi:

- barra de menus
- panel izquierdo fijo de trabajo
- barra de pestanas del documento
- canvas central
- panel derecho opcional de propiedades y teoria
- barra de estado inferior

El workspace actual tiene dos modos:

- `Lienzo + herramientas`
- `Lienzo + herramientas + teoria`

## Stack tecnico

- `C17`
- `CMake`
- `libui-ng`
- `cJSON`
- `stb_image_write`
- `stb_easy_font`

## Arquitectura del proyecto

La base del codigo esta separada por responsabilidades:

- `common/`: utilidades compartidas
- `core/`: modelo y operaciones del dominio
- `editor/`: estado interactivo, playback, historial y coordinacion
- `render/`: canvas y exportacion PNG
- `persistence/`: guardado y carga JSON
- `ui/`: ventana y widgets nativos
- `app/`: arranque, manifiesto y bootstrap de Windows

## Build en Windows

Requisitos:

- `gcc` MinGW-w64
- `cmake`
- `ninja`
- `python`
- `meson`

Comandos:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
.\build\StructStudioC.exe
```

Runner simplificado:

```powershell
.\scripts\run_structstudio.ps1
```

Lanzador por doble clic:

```text
scripts\run_structstudio.cmd
```

Instalador o paquete distribuible:

```powershell
.\scripts\build_installer.ps1
```

## Build en Linux

Requisitos:

- `gcc` o `clang`
- `cmake`
- `ninja`
- `python3`
- `meson`
- `pkg-config`
- `gtk+-3.0` de desarrollo

Comandos:

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
./build/StructStudioC
```

## Estado actual del proyecto

Lo que ya esta fuerte:

- base desktop funcional
- multiples TDAs
- playback guiado
- teoria contextual
- ejemplos didacticos
- exportacion PNG
- guardado/carga JSON
- soporte Windows bien priorizado

Lo que sigue siendo deuda tecnica real:

- `src/ui/main_window.c` sigue siendo grande,
- `src/render/render.c` todavia concentra mucha logica visual,
- y la carga JSON todavia merece endurecimiento adicional.

## Documentacion recomendada

Si quieres entender bien el sistema, estas son las lecturas mas utiles:

- [Guia completa de capacidades](docs/25_system_capabilities_guide.md)
- [Auditoria de deuda tecnica](docs/26_technical_debt_audit.md)
- [Guia de estudio del codigo](docs/28_code_study_guide.md)
- [Guia de implementacion](docs/13_implementation_guide.md)
- [UI y experiencia de uso](docs/02_ui_ux.md)
- [Arquitectura](docs/03_architecture.md)
- [Operaciones por estructura](docs/06_operations.md)
- [Analisis integrados](docs/17_analysis_features.md)
- [Playback guiado](docs/19_guided_playback.md)
- [Sistema de animacion](docs/22_animation_system.md)
- [DPI y nitidez en Windows](docs/23_windows_dpi_and_visual_sharpness.md)
- [Runner de build, tests y logs](docs/24_run_pipeline.md)
- [Empaquetado e instalador](docs/27_installer_packaging.md)
- [Guia de estudio del instalador](docs/29_installer_study_guide.md)

## Resumen corto

StructStudio C ya es una herramienta de escritorio real para visualizar y estudiar estructuras de datos.

Si lo que buscas es:

- construir TDAs,
- compararlos visualmente,
- ejecutar algoritmos,
- ver teoria contextual,
- y tener un proyecto serio en C con arquitectura modular,

este sistema ya lo hace.
