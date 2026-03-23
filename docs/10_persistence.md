# 10_persistence.md

## 1. Propósito del documento

Este documento define la **persistencia** de **StructStudio C** para la primera versión (V1).

Su objetivo es describir:

- qué responsabilidad tiene el subsistema de persistencia,
- cómo se guarda y carga un documento,
- qué validaciones deben realizarse,
- cómo se manejan errores de lectura y escritura,
- cómo se relaciona este módulo con el editor, el modelo y la UI,
- y qué criterios de versionado y compatibilidad debe respetar.

Este archivo no reemplaza al documento de formato JSON; aquí se define la lógica de persistencia como parte del sistema.

---

## 2. Objetivo del subsistema de persistencia

El subsistema de persistencia debe permitir que el usuario pueda:

- guardar el documento actual en archivo,
- volver a abrirlo en otro momento,
- conservar el estado lógico y visual necesario para seguir editando,
- y detectar errores de formato o integridad de forma clara.

La persistencia debe ser lo bastante robusta para una herramienta educativa seria, sin convertirse en un sistema excesivamente complejo en la V1.

---

## 3. Alcance de la persistencia en V1

La V1 cubrirá principalmente:

- guardado de documentos en JSON,
- carga de documentos desde JSON,
- validación estructural básica del contenido persistido,
- manejo de errores de archivo,
- compatibilidad con `format_version = 1`,
- exportación visual a PNG como proceso aparte pero relacionado.

La V1 no necesita incluir:

- base de datos,
- sincronización remota,
- nube,
- recuperación automática compleja,
- guardado incremental sofisticado,
- historial persistido de deshacer/rehacer.

---

## 4. Principios de diseño del módulo de persistencia

### 4.1 Separación de responsabilidades
Persistencia debe encargarse del guardado y carga, no de la lógica de dominio profunda ni de la interfaz gráfica.

### 4.2 Basado en documento
La unidad persistida es el documento completo.

### 4.3 Determinismo
Guardar y volver a cargar debe reconstruir un estado equivalente del documento.

### 4.4 Validación clara
Si un archivo es inválido, el sistema debe detectarlo y reportarlo con mensajes útiles.

### 4.5 Tolerancia razonable
Se pueden aceptar campos opcionales ausentes si no son críticos en V1.

### 4.6 Versionado explícito
La persistencia debe usar el campo `format_version` como puerta de compatibilidad.

---

## 5. Responsabilidad del módulo `persistence`

El módulo de persistencia debe encargarse de:

- serializar `Document` a JSON,
- deserializar JSON a `Document`,
- validar estructura mínima del archivo,
- reportar errores de formato,
- verificar referencias e integridad básica,
- coordinar guardado y carga con el editor,
- y entregar resultados claros de éxito o fallo.

No debe encargarse de:

- mostrar diálogos directamente,
- decidir el comportamiento del canvas,
- dibujar nada,
- ni sustituir las reglas semánticas profundas del `core`.

---

## 6. Objetos principales involucrados

El subsistema de persistencia interactúa principalmente con:

- `Document`
- `DocumentMetadata`
- `Structure`
- `Node`
- `Edge`
- `ViewState`
- `StructureConfig`
- resultados de validación de carga/guardado

Además, conceptualmente, puede usar:

- `PersistenceResult`
- `LoadResult`
- `SaveResult`
- `PersistenceError`

---

## 7. Flujo general de guardado

### 7.1 Disparador
El guardado ocurre cuando el usuario ejecuta acciones como:
- guardar,
- guardar como,
- o eventualmente cerrar documento con confirmación previa.

### 7.2 Flujo de alto nivel
1. la UI detecta la intención de guardar,
2. el editor entrega el documento activo,
3. el módulo de persistencia serializa el documento,
4. se escribe el JSON a disco,
5. se devuelve un resultado de éxito o error,
6. la UI informa el resultado.

### 7.3 Requisitos
- el documento debe existir,
- los IDs deben ser serializables,
- las referencias deben poder representarse correctamente,
- la ruta de salida debe ser válida.

---

## 8. Flujo general de carga

### 8.1 Disparador
La carga ocurre cuando el usuario decide abrir un archivo existente.

### 8.2 Flujo de alto nivel
1. la UI obtiene una ruta,
2. el editor solicita carga,
3. persistencia lee el archivo,
4. valida JSON y versión,
5. reconstruye el `Document`,
6. verifica integridad básica,
7. devuelve un resultado,
8. el editor adopta el documento cargado,
9. el render lo muestra.

### 8.3 Requisitos
- el archivo debe existir,
- el JSON debe ser válido,
- el documento reconstruido debe ser coherente.

---

## 9. Flujo de exportación PNG

Aunque exportar PNG no es lo mismo que persistir JSON, funcionalmente está cerca de este subsistema.

### 9.1 Naturaleza
- JSON guarda estado editable,
- PNG guarda salida visual.

### 9.2 Flujo de alto nivel
1. la UI recibe la intención de exportar,
2. el editor entrega la estructura visible y contexto visual,
3. el subsistema de render genera la salida,
4. el archivo PNG se escribe a disco,
5. se informa resultado.

### 9.3 Observación
La exportación PNG puede vivir coordinada por persistencia o por otro módulo de exportación, pero debe mantenerse separada conceptualmente del guardado JSON.

---

## 10. Serialización

### 10.1 Definición
Serializar es transformar el modelo interno del documento en una representación JSON válida.

### 10.2 Elementos que deben serializarse
- metadatos del documento,
- estructura activa,
- estado visual general,
- estructuras contenidas,
- nodos,
- aristas,
- configuraciones,
- datos especializados,
- metadatos visuales necesarios.

### 10.3 Elementos que no deben serializarse en V1
- hover,
- selección actual,
- arrastre en curso,
- mensajes temporales,
- overlays transitorios,
- estados efímeros del editor.

### 10.4 Regla clave
El archivo debe reconstruir el trabajo, no cada detalle efímero de la sesión.

---

## 11. Deserialización

### 11.1 Definición
Deserializar es transformar un archivo JSON válido en una instancia utilizable de `Document`.

### 11.2 Pasos conceptuales
- leer contenido del archivo,
- parsear JSON,
- validar estructura mínima,
- construir `Document`,
- construir `Structure`,
- construir `Node` y `Edge`,
- resolver referencias por ID,
- reconstruir configuraciones y estado visual persistido,
- devolver resultado.

### 11.3 Regla clave
La deserialización debe fallar de forma clara si el archivo no puede producir un estado razonable.

---

## 12. Validación durante la carga

La carga debe incluir un conjunto mínimo de validaciones.

### 12.1 Validaciones sintácticas
- JSON bien formado,
- tipos de datos compatibles,
- campos obligatorios presentes.

### 12.2 Validaciones estructurales
- `format_version` presente,
- `document` presente,
- `active_structure_id` válido,
- estructuras con IDs únicos,
- nodos con IDs únicos,
- aristas con IDs únicos,
- referencias de aristas a nodos existentes.

### 12.3 Validaciones semánticas mínimas
- familia y variante compatibles,
- datos especializados mínimamente coherentes,
- estructura activa realmente existente.

### 12.4 Validaciones más profundas
Las reglas semánticas más estrictas pueden delegarse al `core` después de cargar el documento.

---

## 13. Validación previa al guardado

Antes de guardar, el sistema puede hacer validaciones ligeras.

### 13.1 Validaciones recomendadas
- existe documento activo,
- los IDs son válidos,
- referencias básicas resueltas,
- no hay nulos donde no deberían,
- hay una estructura activa válida.

### 13.2 Política recomendada
No hace falta bloquear el guardado por toda advertencia menor, pero sí por errores que romperían el archivo.

---

## 14. Versionado del formato

### 14.1 Campo obligatorio
Todo archivo debe incluir:

- `format_version`

### 14.2 V1
La primera versión usará:

- `format_version = 1`

### 14.3 Política de compatibilidad
En V1, el cargador puede ser estricto y aceptar solo la versión 1.

### 14.4 Evolución futura
Más adelante se podrá:
- aceptar varias versiones,
- migrar formatos,
- o rechazar versiones incompatibles con mensajes claros.

---

## 15. Estrategia de IDs persistidos

### 15.1 Regla general
Toda referencia persistida debe hacerse por IDs.

### 15.2 IDs mínimos
- `document_id`
- `structure_id`
- `node_id`
- `edge_id`

### 15.3 Beneficio
Esto desacopla el archivo de la representación en memoria y facilita reconstrucción y validación.

---

## 16. Estrategia de referencias

### 16.1 Aristas
Toda arista debe referir:
- nodo origen,
- nodo destino.

### 16.2 Datos especializados
Si una lista, árbol o map guarda relaciones internas en `data`, esas referencias también deben expresarse por IDs.

### 16.3 Regla
No deben existir referencias a IDs inexistentes dentro de una estructura.

---

## 17. Estado visual persistido

La persistencia debe guardar el estado visual necesario para que el documento reabierto conserve utilidad práctica.

### 17.1 Debe guardarse
- posiciones de nodos,
- tamaños básicos,
- configuración visual general de la estructura,
- visibilidad básica de grilla o paneles si decides conservarla.

### 17.2 No es obligatorio guardar
- hover,
- selección,
- arrastre en curso,
- resaltados temporales.

### 17.3 Regla
Se persiste lo necesario para continuar el trabajo, no toda la sesión exacta.

---

## 18. Resultado de persistencia

Conviene modelar conceptualmente resultados explícitos.

### 18.1 `SaveResult`
Puede incluir:
- `success`
- `path`
- `warning_count`
- `error_code`
- `message`

### 18.2 `LoadResult`
Puede incluir:
- `success`
- `document`
- `warning_count`
- `error_code`
- `message`

### 18.3 Beneficio
Esto evita depender solo de excepciones implícitas o mensajes ambiguos.

---

## 19. Errores de persistencia

Conviene agrupar errores por tipo.

### 19.1 Errores de lectura
- archivo inexistente,
- permiso denegado,
- error de lectura física.

### 19.2 Errores de parseo
- JSON inválido,
- tipo inesperado,
- estructura truncada.

### 19.3 Errores de integridad
- ID duplicado,
- referencia rota,
- estructura activa inexistente,
- campos obligatorios ausentes.

### 19.4 Errores de escritura
- ruta inválida,
- permiso denegado,
- disco no disponible,
- fallo de escritura.

### 19.5 Errores de compatibilidad
- versión no soportada,
- variante desconocida,
- combinación familia/variante inválida.

---

## 20. Política de mensajes de error

Los errores de persistencia deben traducirse en mensajes claros para el usuario.

### Ejemplos
- No se pudo abrir el archivo
- El archivo JSON no tiene un formato válido
- La versión del documento no es compatible
- Falta el campo `document`
- La estructura activa no existe en el archivo
- La arista `e1` apunta a un nodo inexistente

### Regla
El mensaje debe ayudar a entender la causa sin exponer detalles técnicos inútiles.

---

## 21. Relación con el editor

### 21.1 Guardado
El editor entrega el documento activo y recibe un resultado.

### 21.2 Carga
El editor solicita carga y recibe un documento reconstruido.

### 21.3 Importante
Persistencia no debe gestionar selección, arrastre o estados transitorios del editor.

---

## 22. Relación con el `core`

### 22.1 Durante guardado
Persistencia necesita leer las entidades del modelo.

### 22.2 Durante carga
Persistencia reconstruye una forma válida del documento y luego el `core` o el editor pueden ejecutar validaciones más profundas.

### 22.3 Regla
Persistencia no debe convertirse en el lugar donde vive toda la semántica del dominio.

---

## 23. Relación con la UI

### 23.1 Responsabilidad de la UI
- pedir rutas,
- mostrar diálogos,
- mostrar éxito o fallo,
- pedir confirmaciones.

### 23.2 Responsabilidad de persistencia
- leer,
- escribir,
- serializar,
- deserializar,
- validar estructura mínima,
- devolver resultados claros.

---

## 24. Reglas para `guardar como`

### 24.1 Propósito
Guardar el documento activo en una nueva ruta.

### 24.2 Flujo
- la UI obtiene la nueva ruta,
- persistencia serializa,
- se escribe el archivo,
- el documento puede actualizar su ruta actual si decides manejarla en memoria.

### 24.3 Nota
Esto sigue siendo guardado del mismo documento, no duplicación lógica del contenido.

---

## 25. Reglas para documento modificado

Aunque este tema toca también al editor, la persistencia está relacionada.

### 25.1 Estado esperado
El sistema debería poder saber si el documento tiene cambios sin guardar.

### 25.2 Uso
Sirve para:
- confirmar cierre,
- confirmar reemplazo al abrir otro documento,
- recordar al usuario que aún no ha guardado.

### 25.3 Nota
Ese estado puede vivir fuera del módulo de persistencia, pero la persistencia participa al marcar un guardado exitoso.

---

## 26. Escritura segura

Para V1 no necesitas estrategias complejas, pero sí una política razonable.

### 26.1 Recomendación
- serializar completamente,
- intentar escribir de forma limpia,
- no dejar archivos parcialmente escritos si puede evitarse.

### 26.2 Futuro posible
Más adelante podrías usar escritura a archivo temporal + reemplazo, pero no es indispensable en la primera etapa si eso complica demasiado.

---

## 27. Carga parcial

### 27.1 Política recomendada para V1
La V1 debería preferir carga estricta:
- si el archivo es inválido de forma relevante, se rechaza.

### 27.2 Motivo
Mantiene el comportamiento predecible y simplifica la implementación.

### 27.3 Futuro posible
En versiones futuras se puede considerar carga tolerante con advertencias.

---

## 28. Integridad mínima tras cargar

Después de cargar, el documento debe poder responder afirmativamente a preguntas como:

- ¿existe el documento?
- ¿existe la estructura activa?
- ¿los nodos tienen IDs válidos?
- ¿las aristas apuntan a nodos reales?
- ¿la familia y variante son reconocibles?
- ¿la UI puede renderizar el documento sin datos esenciales faltantes?

Si no, la carga debe fallar o reportar advertencias claras.

---

## 29. Persistencia y pruebas

El módulo de persistencia debe ser una de las partes más fáciles de probar de forma aislada.

### 29.1 Casos de prueba recomendados
- guardar documento válido,
- cargar documento válido,
- rechazar JSON inválido,
- rechazar referencias rotas,
- rechazar versión no soportada,
- guardar y volver a cargar conservando equivalencia básica,
- validar campos obligatorios.

### 29.2 Beneficio
Esto ayuda bastante a estabilizar el proyecto.

---

## 30. Resultado esperado del subsistema de persistencia

Al finalizar la V1, la persistencia debe permitir:

- guardar documentos completos en JSON,
- cargar documentos válidos,
- rechazar documentos inválidos con mensajes claros,
- preservar el estado necesario para continuar edición,
- y coordinar correctamente con editor, UI y modelo.

---

## 31. Relación con otros documentos

Este archivo se complementa con:

- `03_architecture.md` → rol del módulo `persistence`
- `04_data_model.md` → entidades que se guardan y cargan
- `06_operations.md` → acciones que modifican el documento
- `07_file_format.md` → contrato exacto del JSON
- `09_editor_logic.md` → coordinación entre editor y persistencia
