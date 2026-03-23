# 27_installer_packaging.md

## 1. Propósito

Este documento explica cómo generar un instalador o paquete distribuible de StructStudio C.

La integración actual está resuelta desde `CMake` + `CPack`, sin depender de pasos manuales dispersos.

---

## 2. Qué genera hoy el sistema

En Windows:

- si `NSIS` está instalado, `CPack` genera un instalador `.exe` y además un `.zip` portable;
- si `NSIS` no está instalado, `CPack` genera al menos un `.zip` portable.

En Linux:

- se genera un paquete `.tgz`.

---

## 3. Qué incluye el paquete

La instalación o paquete actual incluye:

- `StructStudioC.exe`
- `README.md`
- carpeta `docs/`
- carpeta `samples/`
- icono principal del proyecto

---

## 4. Script recomendado en Windows

Para generar el instalador con logs:

```powershell
.\scripts\build_installer.ps1
```

O por doble clic:

```text
scripts\build_installer.cmd
```

El flujo ejecuta:

1. `configure`
2. `build`
3. `test`
4. `package`

Y deja logs en:

```text
artifacts/installer_logs/<timestamp>/
```

---

## 5. Flujo manual

También puede hacerse manualmente:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
cpack --config build/CPackConfig.cmake
```

---

## 6. Decisión técnica

Se eligió `CPack` porque:

- ya forma parte del ecosistema `CMake`,
- evita scripts ad hoc para instalación,
- permite fallback portable sin bloquear al usuario,
- y mantiene el empaquetado ligado al build real del proyecto.

---

## 7. Limitaciones actuales

- el instalador todavía no crea una experiencia compleja de wizard personalizada;
- el contenido instalado está enfocado en uso local y material didáctico básico;
- si se quisiera un instalador empresarial más elaborado, habría que profundizar en `NSIS` o cambiar a una solución específica.
