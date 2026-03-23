# 24_run_pipeline.md

## 1. Proposito

Este documento describe el runner local de despliegue y diagnostico para StructStudio C.

El objetivo no es empaquetar una release final, sino simplificar el flujo cotidiano de:

- configurar,
- compilar,
- probar,
- registrar logs,
- y abrir la aplicacion.

---

## 2. Archivos

Los archivos agregados para este flujo son:

- `scripts/run_structstudio.ps1`
- `scripts/run_structstudio.cmd`

El `.cmd` sirve como lanzador simple para Windows.
El `.ps1` contiene la logica real.

---

## 3. Que hace el runner

Por defecto el script:

1. crea una carpeta de logs por sesion en `artifacts/run_logs/<timestamp>/`
2. registra informacion del entorno
3. ejecuta `cmake --version`, `ctest --version` y `ninja --version`
4. configura el proyecto con `cmake -S . -B build -G Ninja`
5. compila la app y todos los tests principales
6. ejecuta `ctest --test-dir build --output-on-failure`
7. intenta lanzar `StructStudioC.exe`
8. deja un `summary.md` con el resultado de cada etapa

---

## 4. Logs generados

Cada sesion deja archivos como:

- `summary.md`
- `session_console.log`
- `environment.txt`
- `configure.log`
- `build.log`
- `test.log`
- `launch.log`

Ademas se actualiza:

- `artifacts/run_logs/latest.txt`

Ese archivo apunta a la ultima carpeta de logs generada.

---

## 5. Uso rapido

### 5.1 Doble clic

En Windows puedes ejecutar:

- `scripts/run_structstudio.cmd`

### 5.2 PowerShell

Tambien puedes usar:

```powershell
.\scripts\run_structstudio.ps1
```

---

## 6. Parametros utiles

El runner soporta parametros simples:

- `-SkipConfigure`
- `-SkipBuild`
- `-SkipTests`
- `-SkipLaunch`
- `-BuildDir <ruta>`
- `-LogRoot <ruta>`
- `-LaunchProbeSeconds <n>`

Ejemplos:

```powershell
.\scripts\run_structstudio.ps1 -SkipLaunch
```

```powershell
.\scripts\run_structstudio.ps1 -BuildDir build -LogRoot artifacts\run_logs
```

---

## 7. Comportamiento diagnostico

Si una etapa falla:

- el script corta la ejecucion,
- deja el error en `summary.md`,
- conserva el log especifico de la etapa,
- y mantiene la carpeta de sesion lista para compartir.

Esto es util para soporte tecnico o para continuar el trabajo en otra conversacion sin perder contexto.

---

## 8. Manejo de `.ninja_lock`

Durante desarrollo puede quedar un archivo `.ninja_lock` huerfano si un build anterior fue interrumpido.

El runner intenta removerlo **solo** cuando:

- el archivo existe,
- y no hay ningun proceso `ninja` activo.

Eso evita bloquear recompilaciones por un residuo temporal del sistema de build.

---

## 9. Alcance y limites

El runner ayuda con el ciclo local de desarrollo, pero no reemplaza:

- empaquetado formal de release
- instaladores MSI
- firma de binarios
- telemetria de produccion

Su funcion es dar un flujo repetible y una evidencia local clara de que:

- la configuracion paso,
- el build paso,
- los tests pasaron,
- y el ejecutable al menos intento arrancar.
