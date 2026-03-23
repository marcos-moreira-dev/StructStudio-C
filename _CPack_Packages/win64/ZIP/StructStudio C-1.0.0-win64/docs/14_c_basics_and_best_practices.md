# 14_c_basics_and_best_practices.md

## 1. Para quien es esta guia

Esta guia esta pensada para alguien que:

- sabe poco de C,
- quiere entender por que el proyecto esta organizado asi,
- y quiere aprender buenas practicas aplicadas a un caso real.

---

## 2. C no tiene clases, pero si puede tener arquitectura

Uno de los errores mas comunes al empezar con C es pensar:

> "como no hay clases, entonces todo va junto"

Eso es falso.

En C se puede construir buena arquitectura usando:

- modulos,
- prefijos de nombres,
- `struct`,
- `enum`,
- funciones privadas `static`,
- contratos en headers,
- y ownership claro de memoria.

---

## 3. Que significa desacoplar en C

Desacoplar no significa inventar ceremonias raras.

Significa algo mucho mas concreto:

- que la UI no conozca detalles internos del JSON,
- que el dominio no dependa de la ventana,
- que el render no decida reglas de negocio,
- que los helpers comunes no se conviertan en basurero.

Eso mismo se aplico aqui.

---

## 4. Principios importantes

## 4.1 Una razon principal por modulo

Un archivo debe tener una razon clara para existir.

Ejemplos sanos:

- `error.c`: errores
- `document_io.c`: JSON
- `main_window.c`: ventana
- `editor.c`: coordinacion de interaccion

Cuando un archivo hace demasiadas cosas, leerlo se vuelve caro.

---

## 4.2 Datos coherentes

Antes de escribir botones o canvas, hay que tener claro:

- que es un documento,
- que es una estructura,
- que es un nodo,
- que es una arista.

Eso se resolvio primero en `core/model.h`.

---

## 4.3 Errores explicitos

En C no hay excepciones como en otros lenguajes.

Por eso se usa una estructura de error.

Eso obliga a preguntar:

- la operacion salio bien?
- si no, que mensaje devuelvo?

Esa explicitud es una fortaleza si se usa bien.

---

## 4.4 Ownership de memoria

Pregunta clave:

> quien es responsable de liberar esto?

Ejemplo en este proyecto:

- `SsDocument` es dueno de sus estructuras
- `SsStructure` es duena de sus nodos y aristas

Si eso no esta claro, en C aparecen fugas o corrupciones.

---

## 5. Buenas practicas concretas usadas aqui

## 5.1 Inicializar estructuras completas

Se usa `memset(..., 0, sizeof(...))` al crear estados o entidades nuevas.

Eso evita basura de memoria no inicializada.

---

## 5.2 Copiar strings con limites

Se usa helper `ss_str_copy()` en vez de copiar texto sin control.

Eso evita overflow y hace el codigo mas uniforme.

---

## 5.3 Prefijos estables

Todo usa `ss_`.

Eso es una forma de namespace manual en C.

Sin eso, proyectos medianos se vuelven confusos muy rapido.

---

## 5.4 `static` para helpers internos

Si una funcion solo sirve dentro de un archivo, debe ser `static`.

Eso reduce superficie publica innecesaria.

---

## 5.5 Headers pequenos

Los headers deben mostrar solo lo que otros modulos necesitan.

No deben convertirse en una filtracion masiva de detalles internos.

---

## 5.6 Comentarios con valor de mantenimiento

Los comentarios mas utiles en C no son:

- "incrementa i"
- "asigna el valor"

Los comentarios que si valen la pena explican:

- responsabilidad del archivo
- por que existe un helper interno
- por que una decision tecnica se tomo de esa manera

Por eso en los archivos nuevos se uso una convencion simple:

- encabezado de modulo
- comentario corto en bloques delicados
- nada de ruido trivial

---

## 6. Anti-patrones que conviene evitar

## 6.1 Un archivo Dios

Cuando un archivo:

- conoce la UI,
- conoce JSON,
- conoce layout,
- conoce validacion,
- y conoce reglas de negocio,

ese archivo ya esta pidiendo refactor.

En esta base eso ya se atendio especialmente en el `core`, donde la API del dominio se repartio en varios archivos mas pequenos y especializados.

---

## 6.2 `common` como vertedero

No todo helper merece vivir en `common`.

Si un helper existe solo porque una estructura de arbol lo necesita, no deberia terminar en utilidades globales por comodidad.

---

## 6.3 Variables globales desordenadas

En desktop C esto pasa mucho.

Aqui se redujo ese problema centralizando el estado principal en:

- `SsEditorState`
- `SsMainWindow`

No es perfecto, pero es mucho mejor que docenas de variables sueltas.

---

## 6.4 Reglas duplicadas entre UI y dominio

Si el dominio dice:

- "set no admite duplicados"

la UI no deberia inventar otra regla distinta.

La UI solo debe reflejar y delegar.

---

## 7. Estrategia de crecimiento recomendada

Para seguir mejorando el proyecto sin desordenarlo:

1. aumentar pruebas del `core`
2. agregar mas validaciones por familia
3. documentar ownership y convenciones internas
4. vigilar archivos que vuelvan a crecer demasiado
5. solo despues ampliar features adicionales

Ese orden es mas profesional que agregar cosas visuales al azar.

---

## 8. Que aprender de este proyecto si estas empezando en C

Este proyecto sirve para aprender:

- separacion modular sin POO clasica
- manejo manual de memoria con disciplina
- build serio con CMake
- integracion de dependencia nativa real
- estado de editor desacoplado de la GUI
- persistencia estructurada
- render 2D y exportacion de imagen

---

## 9. Recomendacion de lectura

Si recien empiezas con C, no intentes comprender todo al mismo tiempo.

Lee en este orden:

1. `common/error.h`
2. `common/util.h`
3. `core/model.h`
4. `core/api.h`
5. `src/core/api_base.c`
6. `editor/editor.h`
7. `persistence/document_io.h`
8. `render/render.h`
9. `ui/main_window.c`

Primero entiende **que datos existen**.

Despues entiende **quien los modifica**.

Recien al final mira el detalle del dibujo.
