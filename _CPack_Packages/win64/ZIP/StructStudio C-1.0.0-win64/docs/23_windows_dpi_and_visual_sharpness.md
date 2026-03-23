# 23_windows_dpi_and_visual_sharpness.md

## 1. Proposito

Este documento explica, de forma academica, por que una aplicacion de escritorio en Windows puede verse:

- nitida,
- borrosa,
- correctamente escalada,
- o visualmente anticuada aunque no este borrosa.

Tambien documenta como **StructStudio C** trata este problema en Windows 10/11 y que ocurre en Windows 7/8.

---

## 2. Conceptos base

### 2.1 DPI

`DPI` significa `dots per inch` o `puntos por pulgada`. En la practica describe cuanta densidad de pixeles tiene una pantalla.

En Windows, durante muchos anos se tomo `96 DPI` como base logica de referencia.

### 2.2 Pixeles fisicos y unidades logicas

No siempre un control dibujado con ancho `100` ocupa `100` pixeles fisicos reales en pantalla.

Hay dos niveles distintos:

- **pixel fisico**: pixel real del monitor
- **unidad logica**: medida abstracta con la que muchas APIs de UI trabajan antes de mapear a pixeles reales

Cuando el sistema opera con escalado, Windows traduce medidas logicas a medidas fisicas.

### 2.3 Factor de escala

Una forma simple de verlo es:

```text
factor de escala = DPI real / 96
```

Ejemplos:

- `96 DPI` -> escala `100%`
- `120 DPI` -> escala `125%`
- `144 DPI` -> escala `150%`
- `192 DPI` -> escala `200%`

---

## 3. El problema real: no todo lo que se ve "viejo" esta borroso

En aplicaciones Win32 nativas hay que separar dos problemas:

### 3.1 Borrosidad por escalado bitmap

Ocurre cuando el proceso **no es DPI-aware**. En ese caso Windows deja que la app se dibuje como si todo estuviera en `96 DPI` y despues escala la ventana completa como una imagen.

Ese proceso se conoce, en la practica, como **bitmap scaling** o **DPI virtualization**.

Consecuencias:

- texto suavizado de mas
- bordes menos definidos
- canvas entero con apariencia blanda
- controles nativos con perdida de nitidez

### 3.2 Apariencia clasica o antigua

Incluso si la app esta nitida, puede seguir viendose clasica por otras razones:

- usa controles nativos Win32/Common Controls
- hereda la fuente y metricas base del sistema
- respeta paddings nativos conservadores
- no usa una libreria visual moderna como WinUI o Qt con estilo propio

Conclusion importante:

- **borroso** y **clasico** no son el mismo problema
- primero hay que resolver DPI
- despues se afinan espaciado, pesos visuales, tipografia y canvas

---

## 4. Modelos de DPI awareness en Windows

Windows distingue varios niveles de conciencia DPI del proceso.

### 4.1 DPI-unaware

El proceso se comporta como si siempre viviera en `96 DPI`.

Windows:

- virtualiza las coordenadas
- renderiza como si la escala fuera `100%`
- y luego agranda el resultado como bitmap

Es el peor caso para nitidez.

### 4.2 System DPI aware

La app lee el DPI del sistema al arrancar y dibuja con ese valor inicial.

Ventaja:

- evita gran parte de la borrosidad inicial

Limite:

- si la ventana se mueve a otro monitor con otro DPI, la adaptacion es limitada o inexistente

### 4.3 Per-Monitor DPI aware

La app puede reaccionar mejor cuando cambia de monitor y el DPI del monitor destino es diferente.

Ventaja:

- mejor comportamiento en entornos con multiples monitores

Limite:

- sigue siendo un modelo menos refinado que `Per-Monitor V2`

### 4.4 Per-Monitor V2

Es la evolucion mas completa del modelo en Windows moderno.

Ventajas:

- mejor recalculo de metricas por monitor
- mejor integracion con controles nativos modernos
- mejor ajuste de non-client area, dialogos y layout
- menor probabilidad de resultados inconsistentes al mover la ventana entre monitores con escalas distintas

Para una app didactica moderna sobre Win32, este es el objetivo razonable en Windows 10/11.

---

## 5. Como declara Windows esa capacidad

Hay dos mecanismos principales:

### 5.1 Manifiesto del ejecutable

El manifiesto puede declarar:

- compatibilidad del sistema operativo
- activacion de `Common Controls v6`
- nivel de DPI awareness

En StructStudio C, esto vive en:

- `src/app/app.manifest`
- `src/app/windows_resources.rc`

### 5.2 Bootstrap en tiempo de arranque

Tambien puede fijarse desde codigo, idealmente **antes de inicializar la UI**.

En StructStudio C eso se centraliza en:

- `src/app/windows_bootstrap.c`

La estrategia actual es:

1. intentar `PerMonitorV2`
2. si no existe, intentar `PerMonitor`
3. si tampoco existe, caer a `System DPI aware`

Esta cadena evita hacks dispersos en `ui/` y mantiene la politica de DPI como una responsabilidad del arranque del proceso.

---

## 6. Que implementa StructStudio C

La solucion actual en Windows se apoya en tres piezas coordinadas.

### 6.1 Common Controls v6

El manifiesto embebido declara la dependencia:

```text
Microsoft.Windows.Common-Controls 6.0.0.0
```

Esto no vuelve automaticamente moderna la app, pero si habilita la familia correcta de controles comunes en Windows.

### 6.2 Declaracion de DPI en el manifiesto

El manifiesto declara:

- `dpiAware = true/pm`
- `dpiAwareness = PerMonitorV2, PerMonitor`

Interpretacion:

- en Windows 10/11, el sistema puede usar el modelo moderno
- en sistemas anteriores, Windows usa la mejor opcion que entienda

### 6.3 Bootstrap con fallback por API

El arranque usa:

- `SetProcessDpiAwarenessContext()` cuando existe
- `SetProcessDpiAwareness()` si la anterior no esta disponible
- `SetProcessDPIAware()` como ultimo fallback

Esto reduce la probabilidad de que el proceso quede `DPI-unaware` por una sola via de configuracion que falle.

---

## 7. Por que esto mejora la nitidez

Cuando el proceso deja de ser `DPI-unaware`:

- Windows deja de agrandar la ventana completa como bitmap
- los controles nativos pueden medir mejor sus dimensiones reales
- el texto del canvas puede generarse con una relacion mas correcta entre unidades logicas y pixeles fisicos
- lineas, bordes y tipografia pierden la sensacion de "lavado"

En otras palabras:

- no se esta "maquillando" una imagen ya borrosa
- se corrige el modelo de render desde el origen del proceso

---

## 8. Lo que DPI no arregla por si solo

Aunque DPI awareness mejore mucho la nitidez, no resuelve automaticamente:

- un layout con espacios torpes
- iconos raster demasiado pequenos
- texto del canvas con jerarquia debil
- el look clasico de controles nativos

Por eso, despues de DPI, siguen importando:

- espaciados
- tamanos minimos
- contrastes
- grosor de lineas
- y calidad visual del canvas

---

## 9. Soporte objetivo de StructStudio C

El soporte principal del proyecto queda orientado a:

- **Windows 10**
- **Windows 11**

Esta decision es tecnica y no solo comercial. Esos sistemas entienden mucho mejor el modelo moderno de DPI y ofrecen el mejor entorno para:

- `PerMonitorV2`
- multiples monitores con escalas mixtas
- y una experiencia visual mas predecible

---

## 10. Que pasa en Windows 7 y Windows 8

### 10.1 Windows 7

Windows 7 no soporta `Per-Monitor V2`.

En la practica:

- la entrada moderna `dpiAwareness` se ignora
- `true/pm` no se comporta como `PerMonitorV2`
- el mejor resultado realista es caer a un comportamiento cercano a `System DPI aware`

Consecuencia:

- la app puede verse mas nitida que en modo `DPI-unaware`
- pero no tendra la misma adaptacion fina al cambiar entre monitores con distinto DPI

### 10.2 Windows 8.0

Windows 8.0 sigue sin ofrecer el modelo moderno completo esperado hoy.

En terminos practicos para este proyecto:

- no es la plataforma objetivo
- el fallback puede evitar la peor borrosidad
- pero no se considera una base seria para validar calidad visual final

### 10.3 Windows 8.1

Windows 8.1 ya introduce un modelo `Per-Monitor` mas util que el de Windows 7/8.0.

Gracias al fallback por API con `shcore.dll`, StructStudio C puede aproximarse a un comportamiento mejor que el de `System DPI aware`.

Sin embargo:

- sigue sin ser `PerMonitorV2`
- y no es la plataforma objetivo oficial del proyecto

### 10.4 Implicacion academica

La compatibilidad con Windows 7/8 debe entenderse como:

- **compatibilidad degradada o de mejor esfuerzo**

No como:

- **paridad visual plena con Windows 10/11**

---

## 11. Relacion con libui-ng

`libui-ng` ayuda, pero no hace magia.

Puntos honestos:

- la biblioteca usa controles nativos del sistema
- en Windows eso significa una base visual Win32/Common Controls
- no va a parecer WinUI 3 sin cambiar de toolkit o repintar todo manualmente

Entonces, aun con buena nitidez:

- la app seguira teniendo una identidad nativa y clasica

Eso es una limitacion estructural razonable del stack, no un bug aislado.

---

## 12. Conclusiones

### 12.1 Conclusiones tecnicas

- si el proceso es `DPI-unaware`, Windows puede volver borrosa toda la app
- el manifiesto y el bootstrap deben trabajar juntos
- `PerMonitorV2` es la opcion correcta para Windows 10/11
- Windows 7/8 solo pueden aspirar a una compatibilidad parcial o degradada

### 12.2 Conclusiones de producto

Para un software didactico como StructStudio C, la estrategia correcta no es reescribir toda la interfaz:

1. corregir DPI awareness
2. mantener `Common Controls v6`
3. mejorar canvas, espaciados y jerarquia visual
4. aceptar que `libui-ng` conserva un look nativo y clasico

Esa combinacion produce una mejora real, mantenible y coherente con la arquitectura actual.
