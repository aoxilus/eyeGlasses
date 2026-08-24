# Driver Architecture

Ruta tecnica para mover el procesamiento desde Win32/GDI+ CPU hacia GPU/driver en Windows.

## Fase 1: Ejecutable Actual Optimizado

Stack actual:

```text
Win32 + GDI+
Captura desktop con BitBlt
Transformacion afin GDI+
Sharpening CPU por pixeles
```

Ventajas:

```text
Simple, portable con MinGW
No requiere driver firmado
Facil de depurar
```

Limitaciones:

```text
Latencia CPU
Captura puede generar feedback loop
Transformacion limitada por GDI+
```

## Fase 2: DXGI + Direct3D 11

Pipeline recomendado:

```text
DXGI Desktop Duplication API
ID3D11Texture2D frameTexture
Fullscreen borderless swapchain
Pixel shader HLSL para pre-compensacion
```

Componentes C++:

```text
IDXGIOutputDuplication::AcquireNextFrame
ID3D11Device
ID3D11DeviceContext
IDXGISwapChain
ID3D11ShaderResourceView
ID3D11RenderTargetView
```

Uniforms para shader:

```hlsl
cbuffer OpticalParams : register(b0)
{
    float sphere;
    float cylinder;
    float axisRad;
    float distanceCm;
    float deconv;
    float contrast;
    float marginRatio;
    float padding;
};
```

Shader conceptual:

```hlsl
float2 centered = uv - 0.5;
float2 r = rotate(centered, axisRad);
r.x *= scaleX;
r.y *= scaleY;
float2 srcUv = rotate(r, -axisRad) + 0.5;
float4 color = source.Sample(linearSampler, srcUv);
```

Luego aplicar high-pass direccional con offsets texel-size.

## Fase 3: IddCx Virtual Display

Objetivo: crear una pantalla virtual corregida por hardware.

Tecnologia:

```text
Windows Indirect Display Driver Class Extension (IddCx)
WDF / KMDF o UMDF segun plantilla
Swapchain entregada por el sistema
Render GPU con Direct3D
```

Requisitos y riesgos:

```text
Driver signing
WDK instalado
Mayor complejidad de debugging
Posibles restricciones de seguridad/captura DRM
```

Arquitectura propuesta:

```text
VisionCompensator.exe = UI, calibracion, perfil del usuario
VisionCompensatorService.exe = IPC y persistencia de perfiles
VisionCompensatorIdd.dll/sys = display virtual + shader GPU
```

IPC posible:

```text
Named pipes locales
Shared memory con estructura OpticalParams
Evento Win32 para notificar cambios
```

## Recomendacion Pragmatica

Implementar primero DXGI + D3D11 dentro del exe. Solo pasar a IddCx cuando el shader y la latencia esten probados.
