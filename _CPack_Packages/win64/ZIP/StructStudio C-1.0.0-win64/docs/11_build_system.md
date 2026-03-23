# 11_build_system.md

## 1. Propósito del documento

Este documento define el **sistema de build** y la **infraestructura técnica base** de **StructStudio C** para la primera versión (V1).

Su objetivo es describir:

- cómo se organizará el proyecto en el repositorio,
- cómo se compilará con CMake,
- cómo se manejarán perfiles de compilación,
- cómo se integrarán dependencias externas,
- qué políticas de warnings y validación se usarán,
- y qué lineamientos de portabilidad se seguirán entre Windows y Linux.

Este documento no pretende ser la guía completa de CMake, sino la especificación técnica del build system de este proyecto.

---

## 2. Objetivo del sistema de build

El sistema de build de StructStudio C debe permitir:

- compilar el proyecto de forma ordenada,
- separar claramente código fuente, headers, assets, tests y documentación,
- manejar dependencias externas sin caos,
- generar builds de desarrollo y de distribución,
- mantener compatibilidad razonable entre Windows y Linux,
- y servir como práctica seria de infraestructura de proyecto en C.

---

## 3. Decisiones base ya tomadas

Las decisiones técnicas base para la V1 son:

- lenguaje principal: **C**
- estándar recomendado: **C17**
- sistema de build: **CMake**
- GUI: **libui-ng**
- aplicación de escritorio local
- objetivo multiplataforma: **Windows + Linux**
- formato persistido: **JSON**
- exportación visual: **PNG**

---

## 4. Principios del build system

### 4.1 Build reproducible
El proyecto debe poder compilarse de forma predecible siguiendo pasos claros.

### 4.2 Organización modular
La estructura del repositorio debe reflejar la arquitectura del sistema.

### 4.3 Separación entre código propio y dependencias
Las librerías externas no deben mezclarse desordenadamente con el código del proyecto.

### 4.4 Portabilidad razonable
Se debe evitar amarrar innecesariamente el proyecto a una sola plataforma.

### 4.5 Debug primero
La configuración de desarrollo debe facilitar depuración, validación y estabilidad.

### 4.6 Release limpia
La configuración de distribución debe generar binarios listos para probar o compartir.

---

## 5. Estándar del lenguaje

### 5.1 Decisión
La recomendación principal es usar:

- **C17**

### 5.2 Motivo
C17 ofrece una base moderna, estable y suficientemente portable para este proyecto.

### 5.3 Regla
El código debe mantenerse dentro de un subconjunto razonable y portable de C, evitando extensiones innecesarias de compilador salvo que se documenten claramente.

---

## 6. Estructura recomendada del repositorio

La estructura base recomendada es:

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
  scripts/
  build/
```

---

## 7. Propósito de cada carpeta

### 7.1 `CMakeLists.txt`
Archivo raíz de configuración del build.

### 7.2 `cmake/`
Scripts auxiliares de CMake, funciones reutilizables, módulos personalizados y configuraciones adicionales.

### 7.3 `external/`
Dependencias externas integradas o referenciadas por el proyecto.

### 7.4 `assets/`
Recursos visuales o complementarios necesarios para la aplicación.

### 7.5 `docs/`
Markdown del diseño del proyecto.

### 7.6 `include/`
Headers públicos por módulo.

### 7.7 `src/`
Implementación `.c` organizada por módulo.

### 7.8 `tests/`
Pruebas del proyecto.

### 7.9 `samples/`
Archivos JSON de ejemplo y documentos de prueba.

### 7.10 `scripts/`
Scripts auxiliares de desarrollo, empaquetado o automatización simple.

### 7.11 `build/`
Directorio de compilación fuera del árbol lógico del código fuente.

---

## 8. Organización por módulos dentro de `src/` e `include/`

La estructura de carpetas debe seguir la arquitectura definida en `03_architecture.md`.

### Módulos principales
- `app`
- `ui`
- `editor`
- `render`
- `core`
- `persistence`
- `common`

### Regla de organización
Cada módulo debe tener:
- headers claros,
- implementación agrupada por responsabilidad,
- nombres consistentes,
- y límites razonables.

---

## 9. Estrategia de targets en CMake

El proyecto debe evitar un único target gigantesco y desordenado.

### Estrategia recomendada
Definir varios targets internos o al menos varios grupos lógicos, por ejemplo:

- biblioteca o target de `common`
- biblioteca o target de `core`
- biblioteca o target de `persistence`
- biblioteca o target de `render`
- biblioteca o target de `editor`
- biblioteca o target de `ui`
- ejecutable principal de la app

### Beneficio
Esto ayuda a:
- aislar responsabilidades,
- compilar más ordenadamente,
- mejorar mantenibilidad,
- facilitar pruebas.

---

## 10. Target principal de la aplicación

### Nombre conceptual
El ejecutable principal puede llamarse, por ejemplo:

- `StructStudioC`

### Responsabilidad
Este ejecutable compone los módulos y lanza la aplicación de escritorio.

### Requisito
El target final debe depender de los módulos necesarios y no concentrar toda la lógica directamente.

---

## 11. Build out-of-source

### Regla recomendada
La compilación debe hacerse fuera del árbol fuente.

### Ejemplo conceptual
- fuente en `project/`
- build en `project/build/`

### Motivo
Evita ensuciar el repositorio con artefactos de compilación.

---

## 12. Configuraciones de build

La V1 debe manejar como mínimo dos configuraciones principales:

### 12.1 Debug
Orientada a desarrollo y depuración.

### 12.2 Release
Orientada a builds más limpias para pruebas o distribución.

---

## 13. Configuración Debug

### Objetivos
- facilitar depuración,
- detectar errores temprano,
- mantener símbolos de debug,
- permitir validaciones adicionales.

### Recomendaciones
- símbolos de depuración habilitados,
- optimización baja o nula,
- warnings altos,
- sanitizers en Linux si es viable,
- asserts activos si decides usarlos.

---

## 14. Configuración Release

### Objetivos
- generar binario limpio,
- reducir artefactos de desarrollo innecesarios,
- mantener comportamiento estable.

### Recomendaciones
- optimización habilitada,
- símbolos reducidos o controlados según necesidad,
- desactivar verificaciones solo si no afectan confiabilidad,
- empaquetado razonable por plataforma.

---

## 15. Warnings y calidad de compilación

La política de warnings debe ser seria desde el inicio.

### Recomendación general
Activar warnings altos en compiladores compatibles.

### Objetivo
Detectar cuanto antes:
- conversiones peligrosas,
- variables no usadas,
- firmas inconsistentes,
- problemas de tipos,
- errores potenciales de control de flujo.

### Regla
El proyecto debe tender a compilar sin warnings propios del código del proyecto.

---

## 16. Sanitizers y análisis en desarrollo

### Recomendación
En entorno Linux de desarrollo, conviene usar sanitizers cuando sea posible, especialmente en Debug.

### Casos útiles
- AddressSanitizer
- UndefinedBehaviorSanitizer

### Beneficio
Ayudan a detectar:
- accesos inválidos,
- uso indebido de memoria,
- comportamientos indefinidos comunes en C.

### Nota
No es obligatorio que todo esto exista en Windows desde el día 1, pero sí conviene contemplarlo como práctica seria.

---

## 17. Dependencias externas

### 17.1 Principio general
Las dependencias externas deben manejarse de forma explícita y ordenada.

### 17.2 Dependencia principal conocida
- **libui-ng**

### 17.3 Dependencias auxiliares posibles
- librería JSON en C, si decides no escribir una propia
- utilidades mínimas para PNG si el flujo lo requiere externamente

### 17.4 Regla
No conviene esconder dependencias críticas en el sistema del usuario sin documentarlo claramente.

---

## 18. Estrategia para manejar dependencias

Hay varias opciones razonables. Para este proyecto, la más sana en V1 es una política simple y explícita.

### Opción recomendada
Mantener una estrategia documentada y reproducible, por ejemplo:
- dependencia obtenida o integrada en `external/`
- o localizada por CMake con instrucciones claras

### Regla
La forma de construir el proyecto no debe depender de “magia” no documentada.

---

## 19. Inclusión de headers

### Regla general
Los headers del proyecto deben organizarse por módulo y exponerse con rutas claras.

### Recomendación
Evitar includes caóticos o relativos excesivos.

### Ejemplo conceptual
- includes del tipo `core/document.h`
- `editor/editor_state.h`
- `persistence/document_io.h`

---

## 20. Convención de nombres de archivos

### Regla sugerida
Usar nombres por responsabilidad y módulo.

### Ejemplos
- `document.h` / `document.c`
- `graph_ops.h` / `graph_ops.c`
- `editor_state.h` / `editor_state.c`
- `main_window.h` / `main_window.c`
- `document_io.h` / `document_io.c`

### Objetivo
Que el proyecto sea fácil de navegar y mantener.

---

## 21. Assets y recursos

### 21.1 Uso previsto en V1
La V1 probablemente necesitará pocos assets, pero el sistema debe contemplarlos.

### Posibles recursos
- iconos,
- imágenes auxiliares,
- archivos de ejemplo,
- fuentes si más adelante decides controlar alguna.

### Regla
Los assets deben vivir fuera de `src/`.

---

## 22. Samples y archivos de ejemplo

### Propósito
Tener documentos JSON de prueba para desarrollo, depuración y demostración.

### Ejemplos de samples útiles
- lista simple mínima
- BST pequeño
- AVL con balance visible
- map con varias entradas
- grafo dirigido ponderado

### Beneficio
Acelera validación manual y pruebas de persistencia.

---

## 23. Pruebas

La infraestructura debe contemplar pruebas desde el inicio, aunque no sean demasiadas en la V1.

### Áreas ideales para tests
- `core`
- `persistence`
- validación estructural
- serialización / deserialización
- operaciones semánticas básicas

### Regla
Las pruebas no deben depender completamente de la GUI.

---

## 24. Integración de tests con CMake

### Recomendación
Configurar el proyecto para poder compilar y ejecutar pruebas desde CMake.

### Beneficio
Permite tener una infraestructura más profesional y escalable.

### Alcance V1
No necesitas una gran suite, pero sí que el sistema esté preparado para ello.

---

## 25. Portabilidad entre Windows y Linux

### Objetivo
El proyecto debe poder construirse en ambos sistemas con el menor número posible de ajustes manuales extraños.

### Principios
- evitar APIs exclusivas de una plataforma en el núcleo,
- aislar detalles específicos si hacen falta,
- documentar cualquier diferencia relevante,
- usar CMake como punto de unificación.

### Regla
La portabilidad no significa “cero diferencias”, sino control razonable de esas diferencias.

---

## 26. Consideraciones específicas por plataforma

### 26.1 Windows
- el ejecutable final será `.exe`
- la distribución puede requerir acompañar DLLs si una dependencia lo exige
- conviene mantener el proyecto fácil de abrir en entornos típicos de Windows

### 26.2 Linux
- conviene usar Debug con sanitizers cuando sea posible
- algunas dependencias pueden requerir paquetes del sistema o rutas bien documentadas

### 26.3 Regla común
Las diferencias deben quedar documentadas y no esparcidas desordenadamente por todo el código.

---

## 27. Política de macros y código condicional por plataforma

### Recomendación
Usar condicionales por plataforma solo cuando sea realmente necesario.

### Regla
Si aparece código específico de sistema operativo:
- debe aislarse,
- debe comentarse razonablemente,
- y no debe invadir módulos de dominio.

---

## 28. Scripts auxiliares

### Propósito
Automatizar tareas repetitivas del proyecto.

### Posibles usos
- configurar build de desarrollo
- compilar Debug o Release
- ejecutar tests
- copiar assets si luego hace falta

### Ubicación
`scripts/`

### Regla
Los scripts ayudan, pero no deben reemplazar una configuración clara de CMake.

---

## 29. Política de documentación técnica del build

El repositorio debe explicar cómo construir el proyecto.

### Documentos mínimos deseables
- README principal
- este documento `11_build_system.md`
- instrucciones puntuales por plataforma si luego son necesarias

### Reglas
La persona que abra el proyecto debe entender:
- qué necesita instalar,
- cómo configurar CMake,
- cómo compilar,
- cómo ejecutar.

---

## 30. Flujo básico de build esperado

El flujo conceptual de compilación debe ser algo así:

1. preparar dependencias
2. configurar CMake
3. generar build
4. compilar targets
5. ejecutar aplicación o tests

### Regla
El flujo debe ser corto, repetible y entendible.

---

## 31. Estrategia de empaquetado inicial

La V1 no necesita un empaquetado sofisticado, pero sí una idea mínima.

### Recomendación práctica
- generar ejecutable funcional por plataforma
- acompañar dependencias necesarias si aplica
- documentar qué hay que distribuir junto al binario

### Futuro posible
Más adelante se puede pensar en empaquetado más formal.

---

## 32. `.gitignore` y limpieza del repositorio

### Recomendación
El repositorio debe ignorar:
- `build/`
- binarios generados
- artefactos temporales
- archivos locales de IDE si no son parte de la política del proyecto

### Objetivo
Mantener el proyecto limpio y profesional.

---

## 33. Estrategia de compiladores

### Recomendación
El proyecto debe aspirar a compilar con compiladores razonables de cada entorno.

### Ejemplos
- GCC
- Clang
- toolchain compatible en Windows según entorno elegido

### Regla
No depender de una extensión extremadamente específica sin documentarlo.

---

## 34. Estrategia de configuración futura

El build system debe dejar espacio para crecer hacia:

- más tests,
- más módulos,
- empaquetado,
- análisis estático,
- CI futuro si luego lo deseas,
- perfiles adicionales.

Pero en V1 no hay que sobrediseñarlo demasiado.

---

## 35. Riesgos a evitar

### 35.1 Un único CMake gigantesco y caótico
Debe mantenerse organizado.

### 35.2 Mezclar dependencias externas dentro del código propio
Debe existir separación.

### 35.3 No distinguir Debug y Release
Perjudica desarrollo y distribución.

### 35.4 Compilar “a mano” sin política clara
Quita reproducibilidad.

### 35.5 Usar hacks de plataforma en módulos centrales
Rompe mantenibilidad.

### 35.6 No documentar cómo construir
Perjudica muchísimo el proyecto.

---

## 36. Resultado esperado del build system

Al finalizar la V1, la infraestructura del proyecto debe permitir:

- configurar el proyecto con CMake,
- compilarlo en Windows y Linux,
- mantener módulos separados,
- manejar dependencias claramente,
- ejecutar pruebas básicas,
- generar un ejecutable funcional,
- y sostener crecimiento futuro sin caos.

---

## 37. Relación con otros documentos

Este archivo se complementa con:

- `03_architecture.md` → módulos y responsabilidades internas
- `04_data_model.md` → entidades del sistema
- `10_persistence.md` → persistencia y archivos
- `12_roadmap.md` → fases de desarrollo del proyecto
