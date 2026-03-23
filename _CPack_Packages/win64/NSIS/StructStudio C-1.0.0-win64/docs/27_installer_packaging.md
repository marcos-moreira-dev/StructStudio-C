# 27_installer_packaging.md

## 1. Proposito

Este documento resume como se empaqueta StructStudio C para distribucion.

La integracion actual usa:

- `CMake`
- `install()`
- `CPack`
- `NSIS` en Windows cuando esta disponible

La idea es evitar pasos manuales dispersos y mantener el empaquetado alineado con el build real del proyecto.

## 2. Que genera hoy el sistema

En Windows:

- si `NSIS` esta instalado, `CPack` genera un instalador `.exe`;
- ademas genera un `.zip` portable;
- si `NSIS` no esta instalado, al menos genera el `.zip`.

En Linux:

- `CPack` queda preparado para generar un `.tgz`.

## 3. Que incluye el paquete

El paquete o instalador actual incluye:

- `StructStudioC.exe`
- `README.md`
- carpeta `docs/`
- carpeta `samples/`
- icono principal del proyecto en `assets/icons/`

## 4. Flujo recomendado

La forma recomendada en Windows es:

```powershell
.\scripts\build_installer.ps1
```

O por doble clic:

```text
scripts\build_installer.cmd
```

Ese flujo ejecuta:

1. `configure`
2. `build`
3. `test`
4. `package`

Y deja logs en:

```text
artifacts/installer_logs/<timestamp>/
```

## 5. Flujo manual

Tambien se puede hacer a mano:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
cpack --config build/CPackConfig.cmake
```

## 6. Decision tecnica

Se eligio `CPack` porque:

- ya forma parte del ecosistema `CMake`;
- reutiliza el modelo de `install()` del proyecto;
- evita scripts artesanales para copiar archivos;
- y permite generar varios formatos desde una misma configuracion.

## 7. Limitaciones actuales

- el instalador no usa un wizard muy personalizado;
- el contenido instalado esta enfocado en uso local y estudio;
- si en el futuro se quisiera una experiencia mas corporativa, habria que profundizar en `NSIS` o cambiar de tecnologia de instalacion.
