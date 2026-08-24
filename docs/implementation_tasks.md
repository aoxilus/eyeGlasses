# Implementation Tasks

Lista de tareas para construir la version rapida con GPU sin saltar directo a driver.

## Phase 0 - Stabilize Current Win32 Build

- [x] Confirmar que los cambios locales actuales en `main.cpp` compilan.
- [ ] Revisar hotkeys globales: `Ctrl+Alt+V`, `F11`, `Esc`.
- [ ] Verificar que el overlay no bloquee mouse/teclado cuando esta en pantalla completa.
- [ ] Mantener `.gitignore` sin subir `VisionCompensator.exe`.

Acceptance:

```text
build.bat genera VisionCompensator.exe sin errores
```

## Phase 1 - Extract Shared Optical State

- [x] Crear `optical_state.h`.
- [x] Mover `OpticalState`, enums y constantes opticas fuera de `main.cpp`.
- [x] Mantener compatibilidad con UI actual.
- [x] Compilar.

Acceptance:

```text
main.cpp sigue funcionando igual, pero el estado optico queda reusable por D3D11
```

## Phase 2 - Add D3D11 Overlay Skeleton

- [x] Crear `d3d_overlay.h` y `d3d_overlay.cpp`.
- [ ] Crear ventana fullscreen borderless/topmost por monitor.
- [x] Inicializar `ID3D11Device`, `ID3D11DeviceContext`.
- [ ] Inicializar `IDXGISwapChain`.
- [ ] Renderizar color solido o textura dummy.
- [ ] Salir con `Esc` / hotkey.

Acceptance:

```text
Se abre overlay GPU fullscreen y se cierra sin congelar Windows
```

Status actual:

```text
Probe D3D11 agregado y compilando. Todavia no renderiza overlay GPU; la app sigue usando fallback GDI+.
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

## Phase 8 - Adaptive Font For Text Legibility

- [ ] Crear modo `Font Test` con comparacion A/B de estilos de letra.
- [ ] Probar fuentes base: Atkinson Hyperlegible, Lexend, Inter, Segoe UI Variable.
- [ ] Optimizar `fontWeight`, `letterSpacing`, `fontWidth`, `lineHeight` y `minimumSizePx` usando respuestas del usuario.
- [ ] Guardar resultado en `vision_profile.json`.
- [ ] Exportar recomendaciones CSS para navegador/apps web.
- [ ] Investigar generacion `.ttf`/`.otf` con `fonttools` o FontForge si una fuente open-source lo permite.
- [ ] Documentar limitaciones: solo ayuda texto, no fotos/video/iconos, y no sustituye overlay optico.

Acceptance:

```text
El usuario puede elegir un preset tipografico que haga letras mas legibles sin activar overlay ni agregar latencia
```

## Phase 9 - Linux Community Path

- [ ] Documentar que `main.cpp` actual es Windows-only por Win32/GDI+/D3D11.
- [ ] Extraer matematicas de correccion a C++ portable sin dependencias Win32.
- [ ] Crear shader GLSL equivalente a `optical_shader.hlsl`.
- [ ] Crear demo Linux con GLFW/SDL2 + OpenGL usando una imagen de prueba.
- [ ] Investigar backend X11 con `XComposite`, `XDamage`, `XShm` y ventana click-through.
- [ ] Investigar backend Wayland con PipeWire/xdg-desktop-portal.
- [ ] Documentar alternativa experimental con `xrandr --transform` para X11.
- [ ] Exportar perfil de fuente para `fontconfig` como primera utilidad Linux sin overlay.

Acceptance:

```text
Linux puede ejecutar una demo de correccion visual con el mismo core optico, aunque el overlay completo llegue despues
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
