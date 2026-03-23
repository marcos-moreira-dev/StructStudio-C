# 29_installer_study_guide.md

## Guia de estudio del instalador

Este documento explica no solo como ejecutar el instalador, sino tambien que conceptos hay que entender para mantenerlo o extenderlo.

## 1. Objetivo del instalador

El instalador sirve para distribuir StructStudio C como aplicacion de escritorio en Windows sin obligar al usuario final a conocer `CMake`, `Ninja` o la estructura interna del repositorio.

El proyecto maneja dos salidas principales:

- un instalador `.exe`
- un paquete `.zip` portable

## 2. Piezas tecnicas que intervienen

### 2.1. CMake

`CMake` configura el proyecto, genera el build y tambien define la fase de instalacion.

Lo importante aqui es entender:

- `add_executable(...)`
- `install(TARGETS ...)`
- `install(FILES ...)`
- `install(DIRECTORY ...)`

Sin `install()`, `CPack` no sabe que archivos debe empacar.

Archivo clave:

- [CMakeLists.txt](/C:/Users/MARCOS%20MOREIRA/Downloads/StructStudio%20C/CMakeLists.txt)

### 2.2. CPack

`CPack` toma la descripcion de instalacion generada por `CMake` y construye el paquete final.

En este proyecto:

- en Windows puede generar `NSIS;ZIP`
- en Linux puede generar `TGZ`

Variables importantes:

- `CPACK_PACKAGE_NAME`
- `CPACK_PACKAGE_VERSION`
- `CPACK_PACKAGE_INSTALL_DIRECTORY`
- `CPACK_PACKAGE_EXECUTABLES`
- `CPACK_GENERATOR`
- `CPACK_NSIS_*`

### 2.3. NSIS

`NSIS` es el generador que produce el instalador `.exe` en Windows.

Conceptos utiles:

- usa scripts de instalacion
- `CPack` genera ese script automaticamente
- el icono del instalador se puede personalizar
- si `NSIS` no esta instalado, no sale el `.exe`

Comando tipico que debe existir:

```powershell
makensis
```

## 3. Flujo mental correcto

Cuando se quiere "hacer un instalador", en realidad ocurren estas fases:

1. configurar el proyecto
2. compilarlo
3. verificar que pase pruebas
4. instalarlo en un staging temporal
5. empaquetar ese staging en `.exe`, `.zip` o `.tgz`

Eso significa que el instalador no es una pieza aislada. Depende del build y de la fase `install()`.

## 4. Donde se define cada cosa

### Build

- [CMakeLists.txt](/C:/Users/MARCOS%20MOREIRA/Downloads/StructStudio%20C/CMakeLists.txt)

### Script de automatizacion

- [build_installer.ps1](/C:/Users/MARCOS%20MOREIRA/Downloads/StructStudio%20C/scripts/build_installer.ps1)
- [build_installer.cmd](/C:/Users/MARCOS%20MOREIRA/Downloads/StructStudio%20C/scripts/build_installer.cmd)

### Logs del proceso

- `artifacts/installer_logs/<timestamp>/`

### Configuracion generada por CPack

- [CPackConfig.cmake](/C:/Users/MARCOS%20MOREIRA/Downloads/StructStudio%20C/build/CPackConfig.cmake)

## 5. Temas que hay que saber para mantener el instalador

### 5.1. install()

Hay que entender que `install()` no instala en la maquina durante el configure. Lo que hace es describir que contenido debe formar parte del paquete final.

Ejemplo conceptual:

- binario principal
- documentacion
- samples
- iconos

### 5.2. Generadores de CPack

No todos los generadores producen lo mismo:

- `NSIS` produce un instalador de Windows
- `ZIP` produce un paquete portable
- `TGZ` produce un tarball comprimido

### 5.3. Recursos visuales

El instalador puede usar:

- icono principal
- nombre del producto
- directorio por defecto

Pero hay que tener cuidado con recursos fragiles. Por ejemplo, una captura `.png` como cabecera puede romper el script generado de `NSIS` si no encaja con lo que ese generador espera.

### 5.4. Checksums

El proyecto genera archivos `.sha256` para que se pueda verificar la integridad del instalador y del `.zip`.

Eso es util para:

- distribucion
- auditoria
- comprobacion de artefactos en otro equipo

## 6. Errores comunes

### No sale el instalador `.exe`

Posibles causas:

- `NSIS` no esta instalado
- `makensis` no esta en el `PATH`
- `CPACK_GENERATOR` no incluye `NSIS`

### CPack falla al empaquetar

Posibles causas:

- recurso grafico invalido
- ruta a icono inexistente
- archivo bloqueado por estar abierto
- falta algun archivo declarado por `install()`

### El instalador sale, pero faltan archivos

Posible causa:

- el archivo o carpeta no estaba declarado con `install(FILES ...)` o `install(DIRECTORY ...)`

## 7. Flujo recomendado para un estudiante

Si quieres aprender esta parte del proyecto, el orden recomendado es:

1. leer [CMakeLists.txt](/C:/Users/MARCOS%20MOREIRA/Downloads/StructStudio%20C/CMakeLists.txt)
2. leer [27_installer_packaging.md](/C:/Users/MARCOS%20MOREIRA/Downloads/StructStudio%20C/docs/27_installer_packaging.md)
3. leer este documento
4. ejecutar `build_installer.ps1`
5. revisar los logs generados
6. abrir `build/CPackConfig.cmake`

## 8. Idea final

El instalador no debe verse como un paso magico al final. Es la consecuencia de una cadena bien definida:

- modelo de build correcto
- pruebas pasando
- reglas de `install()` correctas
- configuracion de `CPack` coherente
- generador disponible

Entender eso hace mucho mas facil mantener el despliegue de una aplicacion desktop en C.
