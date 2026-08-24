# GPU Overlay Viability Notes

## Decision

La ruta correcta para reducir lag no es escribir un driver primero. La ruta viable es:

```text
Win32 UI actual + Direct3D 11 fullscreen overlay + DXGI Desktop Duplication + HLSL pixel shader
```

Driver virtual `IddCx` queda como fase futura, solo si el prototipo D3D11 demuestra que la correccion visual realmente ayuda y que el cuello de botella ya no es el algoritmo.

## Por Que No Driver Primero

Un driver de display virtual puede sonar ideal, pero tiene costos altos:

- Requiere WDK y arquitectura de driver.
- Requiere firma de driver o modo test signing.
- Debugging es mas lento y riesgoso que una app normal.
- Puede romper compatibilidad con apps, captura protegida, HDR, multi-monitor y escalado DPI.
- No resuelve por si solo el algoritmo optico; solo cambia donde vive el pipeline.

Conclusion: driver antes de probar shader GPU seria demasiado riesgo.

## Por Que No DisplayLink

DisplayLink agrega una capa de virtualizacion/compresion de video. Para este caso no conviene como base principal:

- Puede anadir latencia.
- Puede degradar nitidez por compresion/transporte.
- No da control fino del pipeline de pixeles como D3D11/HLSL.
- Depende de hardware/driver externo.

Conclusion: no usar DisplayLink como solucion principal.

## Por Que Direct3D 11 Si

D3D11 es el punto medio correcto:

- Corre en GPU.
- Permite pixel shaders HLSL rapidos.
- Usa APIs estables de Windows.
- Se puede integrar dentro del `.exe` actual.
- Permite fullscreen borderless y multi-monitor.
- Permite medir FPS/latencia antes de pensar en driver.

## Pipeline Propuesto

```text
1. UI Win32 conserva calibracion JCC y controles.
2. Al aplicar, abrir overlay fullscreen borderless/topmost.
3. Capturar escritorio con DXGI Desktop Duplication.
4. Copiar frame a textura GPU.
5. Renderizar quad fullscreen.
6. Pixel shader aplica:
   - margen 90% centrado
   - transformacion afin por sphere/cylinder/axis
   - stretch meridional y perpendicular
   - sharpening direccional suave
   - contraste lineal limitado
7. Presentar con swapchain.
```

## Transformacion Correcta Para 90% + Astigmatismo

El `10% menos` debe tratarse como margen geometrico, no como correccion optica. Sirve para que el shader pueda estirar/rotar sin cortar contenido.

Con receta aproximada:

```text
S = sphere
C = cylinder
A = axis en grados
D = distancia cm
margin = 0.90
```

Calculo inicial recomendado:

```text
distFactor = 60 / max(30, D)
sphereScale = clamp(1 + (-S) * 0.010 * distFactor, 0.86, 1.14)
cylRatio = clamp(1 + abs(C) * 0.022 * distFactor, 0.90, 1.12)
scaleAlongAxis = margin * sphereScale * cylRatio
scaleAcrossAxis = margin * sphereScale / cylRatio
```

Para `C = -1.25`, `D = 60`:

```text
cylRatio = 1 + 1.25 * 0.022 = 1.0275
```

Eso significa alrededor de `+2.75%` en un meridiano y `-2.68%` en el perpendicular, sobre una imagen base ya reducida a 90%.

## HLSL Conceptual

```hlsl
float2 p = uv - 0.5;
p /= margin;

float s = sin(axisRad);
float c = cos(axisRad);
float2x2 R = float2x2(c, -s, s, c);
float2x2 Ri = float2x2(c, s, -s, c);

float2 q = mul(R, p);
q.x /= scaleAlongAxis / margin;
q.y /= scaleAcrossAxis / margin;
float2 srcUv = mul(Ri, q) + 0.5;

if (srcUv.x < 0 || srcUv.x > 1 || srcUv.y < 0 || srcUv.y > 1)
    return float4(0, 0, 0, 1);

float4 color = sourceTexture.Sample(linearSampler, srcUv);
```

Nota: en shader normalmente se calcula la transformacion inversa, porque para cada pixel destino preguntamos de donde leer en la textura fuente.

## Latencia Esperada

GDI+/CPU actual:

```text
Captura CPU + transformacion GDI+ + loop por pixel CPU = lento en fullscreen
```

D3D11 esperado:

```text
DXGI frame GPU -> shader GPU -> Present = mucho mas rapido
```

Riesgos reales:

- Desktop Duplication puede fallar con contenido protegido.
- Si el overlay captura su propia ventana puede haber feedback loop. Hay que excluir la ventana de captura o capturar monitor antes de presentar.
- En laptops con iGPU/dGPU puede haber copia entre adaptadores.
- Multi-monitor con diferente DPI/Hz requiere manejar cada output por separado.

## Arquitectura Recomendada

Separar el codigo en archivos para no volver inmanejable `main.cpp`:

```text
main.cpp              UI Win32, calibracion, hotkeys
optical_state.h       estructura compartida de parametros
d3d_overlay.h/.cpp    ventana fullscreen, swapchain, render loop
dxgi_capture.h/.cpp   Desktop Duplication
optical_shader.hlsl   transformacion optica y sharpening
shader_embed.h        shader compilado o string embebido inicialmente
```

Para una primera version se puede mantener todo en C++ sin dependencias externas:

```text
link: d3d11 dxgi d3dcompiler user32 gdi32 gdiplus comctl32 comdlg32
```

## Definicion De Exito

La fase GPU esta lista cuando:

- Overlay fullscreen aplica correccion a escritorio completo.
- Mouse/teclado siguen funcionando o hay hotkeys confiables para salir.
- FPS percibido es estable.
- No hay loop espejo infinito.
- El margen 90% deja bordes negros controlados.
- Sliders de sphere/cylinder/axis actualizan shader en vivo.
- Si DXGI falla, vuelve al modo GDI+ sin crashear.
