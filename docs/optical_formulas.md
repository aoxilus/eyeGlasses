# Optical Formulas

Notas de investigacion para integrar pre-compensacion optica mas exacta en C++/HLSL.

## Modelo Esfero-Cilindrico

Prescripcion subjetiva convencional:

```text
S = esfera en dioptrias
C = cilindro en dioptrias
A = eje en grados, rango [0, 180]
D = distancia ojo-monitor en metros
```

Potencia meridional aproximada:

```text
P(theta) = S + C * sin(theta - A)^2
```

Para pantalla, usar una pre-deformacion afin centrada en el viewport:

```text
M = T(cx, cy) * R(A) * Scale(sx, sy) * R(-A) * T(-cx, -cy)
```

Donde:

```text
sx = sphereScale * cylRatio
sy = sphereScale / cylRatio
sphereScale = clamp(1 - kS * S * distanceFactor, 0.70, 1.45)
cylRatio = clamp(1 - kC * C * distanceFactor, 0.85, 1.25)
distanceFactor = 0.60 / max(0.30, D)
```

Valores iniciales seguros:

```text
kS = 0.025
kC = 0.025
```

## Aberracion De Frente De Onda

Relaciones aproximadas con Zernike de segundo orden:

```text
Defocus: C20 ~= S + C / 2
Astig 0/90: C22 ~= (C / 2) * cos(2A)
Astig 45/135: C2m2 ~= (C / 2) * sin(2A)
```

Esto permite mover el algoritmo interno a un espacio vectorial continuo:

```text
z = [C20, C22, C2m2]
```

Ventaja: las respuestas del usuario pueden actualizar un vector de creencia y luego reconvertirse a `S/C/A`.

## PSF Y Filtro Inverso

Para compensacion en tiempo real, evitar deconvolucion completa inestable. Usar realce direccional limitado:

```text
highPassDir = center * 2 - sample(+axis) - sample(-axis)
highPassOmni = center * 4 - top - bottom - left - right
pixelOut = center + a * highPassDir + b * highPassOmni
```

Coeficientes iniciales:

```text
a = effectiveSharpness * 0.16
b = effectiveSharpness * 0.10
effectiveSharpness = clamp(deconv + abs(S) * 0.15 + abs(C) * 0.25, 0, 4)
```

Limites importantes:

```text
Nunca permitir contraste o sharpening sin clamp [0, 255]
Mantener alpha en 255
Aplicar margen visual 0.90-0.94 para evitar clipping por transformacion
```
