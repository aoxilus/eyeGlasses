# Notas de Ingenieria y Sintesis Tecnica

Documento tecnico de analisis, optimizaciones de arquitectura y roadmap para la pre-compensacion visual en tiempo real en Windows.

---

## 1. Estado del Proyecto y Puntos de Alineacion

Hemos revisado los cambios e investigaciones agregados en `docs/`:

1. `prior_art_and_algorithms.md`: fundamentacion teorica con MIT Tailored Displays, NETRA, Barsky/UC Berkeley y trabajos de inverse blurring. Queda clara la diferencia entre pre-compensacion 2D por software y light-fields multicapa con barreras de paralaje.
2. `gpu_overlay_viability_notes.md` y `implementation_tasks.md`: la decision estrategica es priorizar Direct3D 11 + DXGI Desktop Duplication antes de drivers kernel o IddCx.
3. `adaptive_font_for_vision.md`: propuesta complementaria de tipografia adaptativa para lectura en web/desktop.
4. `optical_state.h`, `d3d_overlay.cpp`, `optical_shader.hlsl`: modularizacion inicial de estado y probe D3D11 compilable.

---

## 2. Puntos Criticos Para DXGI + D3D11

### A. Ciclo De Vida DXGI Desktop Duplication

Al capturar la pantalla con `IDXGIOutputDuplication::AcquireNextFrame`:

- Timeout: usar `16ms` para 60Hz o `8ms` para 120-144Hz.
- Si devuelve `DXGI_ERROR_WAIT_TIMEOUT`, no redibujar inutilmente.
- Si devuelve `DXGI_ERROR_ACCESS_LOST`, liberar duplicacion y reinicializar.
- Multi-monitor: enumerar `IDXGIOutput` y duplicar el monitor seleccionado por el usuario.

### B. Input Passthrough

Para que el usuario pueda trabajar mientras el overlay esta activo:

```cpp
WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE
```

Hotkeys de respaldo:

- `Ctrl + Alt + V`: activar/desactivar compensacion.
- `F11`: pantalla completa / salir.
- `Escape`: restaurar interfaz de calibracion.

### C. Shader Optico

El shader debe hacer:

- Margen 90% centrado.
- Rotacion al eje de astigmatismo.
- Escalado anisotropico meridional.
- Deconvolucion/sharpening direccional.
- Contraste lineal limitado.
- Bordes negros controlados cuando `srcUv` sale de `[0,1]`.

Fragmento conceptual:

```hlsl
float2 texelSize = 1.0 / screenSize;
float2 dirOffset = float2(cos(axisRad), sin(axisRad)) * texelSize;

float4 cCenter = sourceTexture.Sample(linearSampler, src);
float4 cDir1 = sourceTexture.Sample(linearSampler, src + dirOffset);
float4 cDir2 = sourceTexture.Sample(linearSampler, src - dirOffset);

float4 highPassDir = cCenter * 2.0 - cDir1 - cDir2;

float4 cTop = sourceTexture.Sample(linearSampler, src + float2(0, texelSize.y));
float4 cBottom = sourceTexture.Sample(linearSampler, src - float2(0, texelSize.y));
float4 cLeft = sourceTexture.Sample(linearSampler, src - float2(texelSize.x, 0));
float4 cRight = sourceTexture.Sample(linearSampler, src + float2(texelSize.x, 0));
float4 highPassOmni = cCenter * 4.0 - cTop - cBottom - cLeft - cRight;

float effectiveSharpness = deconv + (abs(sphere) * 0.12) + (abs(cylinder) * 0.20);
float4 sharpened = cCenter + highPassDir * (effectiveSharpness * 0.15) + highPassOmni * (effectiveSharpness * 0.08);
sharpened.rgb = saturate((sharpened.rgb - 0.5) * contrast + 0.5);
```

---

## 3. Proximos Pasos

1. Completar `d3d_overlay.cpp` con swapchain fullscreen y render de textura dummy.
2. Agregar `dxgi_capture.cpp` para Desktop Duplication.
3. Conectar la textura capturada al shader HLSL.
4. Actualizar constantes del shader desde `OpticalState`.
5. Medir FPS y latencia antes de investigar driver virtual.
