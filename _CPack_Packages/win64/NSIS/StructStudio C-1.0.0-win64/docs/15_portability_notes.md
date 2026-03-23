# 15_portability_notes.md

## 1. Proposito

Este documento resume el estado real de portabilidad de StructStudio C entre:

- Windows
- Linux

No es una promesa abstracta. Es una nota tecnica de lo que ya esta preparado y de lo que aun requiere validacion practica.

---

## 2. Estado actual

La base del proyecto ya esta organizada para ser razonablemente portable en:

- dominio
- persistencia
- render
- editor

El principal punto delicado no era el codigo de negocio, sino el sistema de build y la integracion de `libui-ng`.

---

## 3. Que ya es portable

### 3.1 Nucleo del dominio

Los modulos de `core/` trabajan con:

- `struct`
- `enum`
- memoria dinamica
- strings
- reglas del dominio

Sin depender de Win32 directamente.

### 3.2 Persistencia

La capa `persistence/` usa:

- `cJSON`
- lectura y escritura de archivos

Eso es portable entre Windows y Linux con cambios minimos o nulos.

### 3.3 Exportacion PNG

La exportacion usa `stb_image_write`, por lo que no depende de capturas del sistema operativo.

---

## 4. Que se ajusto para mejorar la portabilidad

## 4.1 CMake condicionado por plataforma

El `CMakeLists.txt` ahora separa:

- configuracion Windows
- configuracion Linux

Ejemplos:

- `RC` solo se habilita en Windows
- el manifiesto del ejecutable solo se agrega en Windows
- las librerias del sistema ya no estan cableadas siempre a Win32
- en Linux se resuelve `gtk+-3.0` via `pkg-config`

## 4.2 Buffer de rutas menos dependiente de Windows clasico

`SS_PATH_CAPACITY` se aumento para no quedar amarrado a una longitud tipo `MAX_PATH` heredada de Windows.

---

## 5. Dependencias por plataforma

## 5.1 Windows

Requiere:

- MinGW-w64 o toolchain compatible
- `cmake`
- `ninja`
- `python`
- `meson`

El ejecutable incluye manifiesto de Common Controls v6 porque `libui-ng` estatico lo necesita en Windows.
El soporte visual y de DPI se orienta principalmente a Windows 10/11; la explicacion academica del tema vive en `23_windows_dpi_and_visual_sharpness.md`.

## 5.2 Linux

Requiere:

- `gcc` o `clang`
- `cmake`
- `ninja`
- `python3`
- `meson`
- `pkg-config`
- paquete de desarrollo `gtk+-3.0`

En Linux el backend de `libui-ng` usa GTK3.

---

## 6. Limites actuales

## 6.1 Validacion real en Linux

En esta iteracion la validacion completa se hizo en Windows:

- build
- tests
- arranque del ejecutable

El build system ya quedo preparado para Linux, pero todavia falta comprobarlo en una maquina Linux real.

## 6.2 UX visual

La aplicacion mantiene una estetica clasica inspirada en Windows 7, pero en Linux el look final dependera de:

- tema GTK
- fuentes del sistema
- configuracion de la distro

Eso es normal en una GUI nativa multiplataforma.

## 6.3 Windows 7 y Windows 8

Windows 7 y Windows 8 no son la plataforma objetivo principal de esta etapa.

Se documentan por completitud tecnica, pero deben entenderse como un escenario de compatibilidad degradada o de mejor esfuerzo:

- sin garantia de la misma experiencia visual de Windows 10/11
- sin garantia de equivalencia completa en DPI awareness moderno
- y sin la misma calidad esperable en entornos multi-monitor con escalas mezcladas

---

## 7. Recomendacion tecnica

Si el objetivo es tener una portabilidad seria, el siguiente paso correcto es:

1. compilar en una distro Linux real
2. ejecutar la app
3. probar guardado, carga y exportacion PNG
4. registrar diferencias visuales o de dependencias

La portabilidad real no se cierra solo con CMake; se cierra con verificacion practica en ambos sistemas.
