# Implementation Tasks

Lista de tareas para construir la version rapida con GPU sin saltar directo a driver.

## Phase 0 - Stabilize Current Win32 Build

- [ ] Confirmar que los cambios locales actuales en `main.cpp` compilan.
- [ ] Revisar hotkeys globales: `Ctrl+Alt+V`, `F11`, `Esc`.
- [ ] Verificar que el overlay no bloquee mouse/teclado cuando esta en pantalla completa.
- [ ] Mantener `.gitignore` sin subir `VisionCompensator.exe`.

Acceptance:

```text
build.bat genera VisionCompensator.exe sin errores
```

## Phase 1 - Extract Shared Optical State

- [ ] Crear `optical_state.h`.
- [ ] Mover `OpticalState`, enums y constantes opticas fuera de `main.cpp`.
- [ ] Mantener compatibilidad con UI actual.
- [ ] Compilar.

Acceptance:

```text
main.cpp sigue funcionando igual, pero el estado optico queda reusable por D3D11
```

## Phase 2 - Add D3D11 Overlay Skeleton

- [ ] Crear `d3d_overlay.h` y `d3d_overlay.cpp`.
- [ ] Crear ventana fullscreen borderless/topmost por monitor.
- [ ] Inicializar `ID3D11Device`, `ID3D11DeviceContext`, `IDXGISwapChain`.
- [ ] Renderizar color solido o textura dummy.
- [ ] Salir con `Esc` / hotkey.

Acceptance:

```text
Se abre overlay GPU fullscreen y se cierra sin congelar Windows
```

## Phase 3 - DXGI Desktop Duplication Capture

- [ ] Crear `dxgi_capture.h` y `dxgi_capture.cpp`.
- [ ] Enumerar adaptador/output activo.
- [ ] Usar `IDXGIOutputDuplication::AcquireNextFrame`.
- [ ] Copiar frame a `ID3D11Texture2D` usable por shader.
- [ ] Manejar `DXGI_ERROR_ACCESS_LOST` recreando duplicator.

Acceptance:

```text
Overlay muestra el escritorio capturado en fullscreen con latencia baja
```

## Phase 4 - Optical Shader MVP

- [ ] Crear `optical_shader.hlsl`.
- [ ] Pasar constantes `sphere`, `cylinder`, `axisRad`, `distanceCm`, `marginRatio`, `deconv`, `contrast`.
- [ ] Implementar crop/margin 90% centrado.
- [ ] Implementar transformacion meridional: rotar -> scale -> desrotar.
- [ ] Implementar borde negro si `srcUv` queda fuera de `[0,1]`.
- [ ] Implementar sharpening direccional simple con samples vecinos.

Acceptance:

```text
Al cambiar axis/stretch se ve deformacion en vivo y sin CPU pixel loop
```

## Phase 5 - UI Integration

- [ ] Boton `APLICAR A MONITORES` usa D3D11 overlay si inicializa correctamente.
- [ ] Si D3D11 falla, fallback al modo GDI+ actual.
- [ ] Sliders y botones actualizan constant buffer del shader.
- [ ] Mostrar estado: `GPU ON`, `GDI fallback`, FPS aproximado.

Acceptance:

```text
El usuario calibra en UI y aplica la receta al overlay GPU
```

## Phase 6 - Quality Improvements

- [ ] Convertir sharpening RGB a luma-only aproximado.
- [ ] Agregar edge mask simple en shader usando luminance gradient.
- [ ] Limitar ringing con clamp suave.
- [ ] Agregar presets: `Texto`, `Desktop`, `Suave`, `Fuerte`.
- [ ] Guardar perfil local `vision_profile.json`.

Acceptance:

```text
Texto se ve mas claro sin invertir colores ni crear halos excesivos
```

## Phase 7 - Research Branch: Driver/Virtual Display

- [ ] Evaluar IddCx solo despues de D3D11 estable.
- [ ] Crear prototipo separado, no mezclar con app principal.
- [ ] Documentar requisitos WDK, firma y rollback.
- [ ] No instalar driver automaticamente.

Acceptance:

```text
Decision documentada basada en mediciones reales de latencia/calidad
```

## Build Command Target

Cuando se agreguen archivos D3D11, `build.bat` deberia evolucionar hacia:

```bat
g++ -O3 -std=c++17 -municode -mwindows main.cpp d3d_overlay.cpp dxgi_capture.cpp -o VisionCompensator.exe -ld3d11 -ldxgi -ld3dcompiler -lgdiplus -lgdi32 -luser32 -lcomctl32 -lcomdlg32
```

Si MinGW complica D3D headers/linking, alternativa pragmatica:

```text
Visual Studio Build Tools + cl.exe + Windows SDK
```
