# 18_visual_assets_and_icon_sources.md

## 1. Proposito

Este documento registra la procedencia de los iconos descargados desde internet y explica como se integraron en StructStudio C.

La meta es dejar trazabilidad tecnica y legal:

- de donde salieron los recursos,
- bajo que licencia se usaron,
- y como se transformaron para el ejecutable de Windows.

---

## 2. Fuente de iconos usada

Se tomo como base el paquete `Silk` del repositorio:

- [legacy-icons/famfamfam-silk](https://github.com/legacy-icons/famfamfam-silk)

Archivos descargados:

- `assets/icons/famfamfam/chart_organisation.png`
- `assets/icons/famfamfam/shape_handles.png`
- `assets/icons/famfamfam/bricks.png`
- `assets/icons/dialogs/dialog_info.png`
- `assets/icons/dialogs/dialog_warning.png`
- `assets/icons/dialogs/dialog_error.png`
- `assets/icons/dialogs/dialog_question.png`
- `assets/icons/dialogs/dialog_theory.png`

Segun el `README` y `LICENSE` del repositorio:

- el paquete corresponde al set `Silk`
- el autor original de los iconos es **Mark James**
- la licencia del set de iconos es **Creative Commons Attribution 2.5**

Referencia de licencia:

- [LICENSE.md del repositorio](https://github.com/legacy-icons/famfamfam-silk/blob/master/LICENSE.md)

---

## 3. Como se integraron

Los PNG descargados no se incrustaron de forma cruda en la ventana.

En su lugar se hizo esto:

1. se descargaron los PNG de 16x16 desde internet
2. se compuso un icono propio del proyecto con una base visual mas acorde a StructStudio C
3. se exporto el resultado como:
   - `assets/icons/structstudio_classic_256.png`
   - `assets/icons/structstudio_classic.ico`
4. el `.ico` se registro en `src/app/windows_resources.rc`
5. para los dialogs tambien se generaron `.ico` dedicados:
   - `assets/icons/dialogs/dialog_info.ico`
   - `assets/icons/dialogs/dialog_warning.ico`
   - `assets/icons/dialogs/dialog_error.ico`
   - `assets/icons/dialogs/dialog_question.ico`
   - `assets/icons/dialogs/dialog_theory.ico`

Esto permite:

- tener un icono visible en la barra de titulo y la barra de tareas,
- reutilizar iconos propios en message boxes y dialogs informativos,
- respetar una estetica mas clasica tipo escritorio,
- y conservar los recursos fuente separados para futuras iteraciones.

---

## 4. Criterio visual aplicado

No se uso un icono moderno plano generico.

La composicion final busca una sensacion mas cercana a software de escritorio clasico:

- base azul vidriada,
- bordes definidos,
- iconografia pequeña y legible,
- y una lectura clara incluso en tamaños de 16x16 y 32x32.

Ese criterio encaja mejor con la direccion visual que ya sigue el proyecto.

---

## 5. Nota de mantenimiento

Si en el futuro se cambian los iconos:

- conservar los archivos fuente descargados,
- documentar la licencia del set,
- y regenerar el `.ico` para Windows en varios tamaños, no solo en uno.
