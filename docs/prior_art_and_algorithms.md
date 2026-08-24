# Estado Del Arte: Pantallas Que Corrigen Vision

Investigacion inicial sobre si alguien ya intento corregir miopia, hipermetropia, astigmatismo u otras aberraciones directamente desde una pantalla, y que metodos usaron.

> Nota: esto no sustituye un examen optometrico. El objetivo aqui es identificar algoritmos y arquitectura util para mejorar `VisionCompensator`.

## Resumen Ejecutivo

Si, ya existen investigaciones academicas y patentes sobre esto. El enfoque mas fuerte no es solamente agrandar letras ni aplicar un sharpening normal: los trabajos mas serios modelan el ojo como un sistema optico con aberraciones y calculan una imagen pre-distorsionada para que, despues de pasar por el ojo desenfocado, llegue mas clara a la retina.

Hay cuatro familias principales:

1. **Pre-deformacion 2D / inverse filtering**: modificar la imagen con deconvolucion o filtros inversos de PSF. Es lo mas cercano a nuestro prototipo actual.
2. **Pantallas light-field / parallax barrier / lenslet array**: hardware adicional delante del display para emitir diferentes rayos por angulo y corregir mejor el foco.
3. **Displays multilayer**: varias capas LCD o patrones temporales para hacer el problema menos mal condicionado.
4. **Holographic / wavefront displays**: generar frente de onda corregido, mas poderoso pero mucho mas complejo.

Para nuestro `.exe` actual, la ruta pragmatica es mejorar primero la familia 1 con GPU/HLSL, PSF mas realista, luma-only deconvolution y eye/distance tracking opcional. Las familias 2-4 requieren hardware extra o driver/display virtual.

## Trabajos Encontrados

### 1. Tailored Displays to Compensate for Visual Aberrations

Autores: Vitor F. Pamplona, Manuel M. Oliveira, Daniel G. Aliaga, Ramesh Raskar.

Venue: ACM Transactions on Graphics, 2012.

DOI: `10.1145/2185520.2185577`.

Links:

- https://www.media.mit.edu/publications/tailored-displays-to-compensate-for-visual-aberrations/
- https://www.cs.purdue.edu/cgvlab/www/publications/pamplona2012tailored/

Idea principal:

- Crear `tailored displays` que compensan aberraciones visuales usando light fields.
- El display descompone objetos virtuales en piezas anisotropicas que quedan dentro del rango focal del usuario.
- Usa mapas de aberracion y scattering para refractive errors y cataratas.
- Divide el light field de cada objeto en multiples instancias, cada una enfocada para una sub-apertura del ojo.

Metodos relevantes:

- Light-field rendering.
- Lenslet array o stack de LCDs.
- Aberration/scattering maps.
- Multi-focus / multi-depth rendering.
- Sub-aperture dependent image placement.

Que nos sirve:

- Confirma que corregir vision desde pantalla es viable, pero con hardware light-field se logra mejor que con un panel 2D normal.
- Para nuestro programa actual, podemos imitar parte del concepto con transformacion meridional y PSF, pero no podemos recrear todos los rayos del light field sin hardware extra.

Limitacion para nuestro caso:

- Requiere lenslet array, parallax/light-field o capas multiples para resultados fuertes.
- En monitor normal 2D, la correccion completa esta limitada por perdida de contraste y ringing.

## 2. Computational Light Field Display for Correcting Visual Aberrations

Autores: Fu-Chung Huang, Gordon Wetzstein, Brian A. Barsky, Ramesh Raskar.

Venue: SIGGRAPH/ACM work around 2013-2014; relacionado con `Eyeglasses-free Display`.

Links:

- https://www.media.mit.edu/publications/computational-light-field-display-for-correcting-visual-aberrations/
- DOI reportado en fuentes academicas: `10.1145/2601097.2601122`.

Idea principal:

- Crear una pantalla light-field computacional que corrige aberraciones visuales.
- Mejor resolucion y contraste que metodos anteriores porque disena hardware optico y algoritmo de prefiltrado juntos.
- Usa componentes disponibles comercialmente y forma delgada para dispositivos moviles.

Metodos relevantes:

- 4D light-field prefiltering.
- Parallax barrier o lenslet array sobre un display de alta densidad.
- Modelo de proyeccion retina-pupila-display.
- Problema inverso resuelto con optimizacion no negativa.
- Evaluacion de condicionamiento de matriz para saber cuantas vistas angulares entran en la pupila.

Detalles utiles encontrados en patente/descripcion relacionada:

```text
Objetivo: encontrar estados de pixeles del display que, al pasar por el elemento light-field y el ojo aberrado, produzcan la imagen deseada en retina.
Modelo: y = P * x
Solucion: x = argmin ||P*x - target|| con restricciones de no negatividad y rango dinamico.
Solver citado: LBFGSB / non-negative optimization.
```

Que nos sirve:

- Si algun dia se usa pantalla virtual o overlay con hardware, este es el camino mas serio.
- Para GPU/HLSL, la idea transferible es tratar la correccion como un problema inverso con parametros del ojo, no solo como filtro estetico.
- El proyecto actual puede evolucionar hacia prefiltering 2D o multi-view si se usa VR/headset o una pantalla secundaria con mascara.

Limitacion para nuestro caso:

- La version fuerte necesita elemento light-field. En un monitor normal solo podemos aproximar con inverse blur, transformaciones afines y contraste.

## 3. Patente US10529059B2

Titulo: Vision correcting display with aberration compensation using inverse blurring and a light field display.

Inventores: Fu-Chung Huang, Gordon Wetzstein, Brian Barsky, Ramesh Raskar.

Assignees: MIT y University of California.

Link: https://patents.google.com/patent/US10529059B2/en

Idea principal:

- Sistema y metodo para compensar aberraciones opticas del usuario usando parametros del ojo y caracteristicas del elemento light-field.
- Recibe parametros como `SPH`, `CYL`, `AXIS`, focal length, higher-order aberrations o datos de aberrometro/autorefractor.
- Calcula una imagen compensada para mostrarla en el display.

Metodos mencionados:

- Inverse blurring.
- Light-field prefiltering.
- Parallax barrier / pinhole array.
- Microlens array, lenslet array, lenticular array.
- Eye tracking o posicion fija del ojo.
- Compensacion de low-order aberrations: defocus, astigmatism, prism.
- Compensacion de high-order aberrations: trefoil, coma, spherical aberration.

Que nos sirve:

- Lista clara de inputs que deberia soportar el software:
  - `sphere`
  - `cylinder`
  - `axis`
  - distancia al ojo
  - tamano estimado de pupila
  - posicion de ojo o cabeza
  - PSF/Zernike opcional
- Tambien confirma que GPU o procesador especializado es apropiado para tiempo real.

Advertencia:

- Es patente activa. Para investigacion personal esta bien documentar; para producto comercial hay que revisar propiedad intelectual con cuidado.

## 4. Real-Time Computational Visual Aberration Correcting Display Through High-Contrast Inverse Blurring

Autores: Akhilesh Balaji, Dhruv Ramu.

Fuente: arXiv `2501.01450v1`.

Link: https://arxiv.org/html/2501.01450v1

Idea principal:

- Framework de display corrector en tiempo real sin lentes usando deconvolucion con PSF del ojo.
- Usa inverse blurring, mascara para reducir ringing, luma-only deconvolution en YUV/YCbCr y PSF adaptada a posicion del usuario.

Metodos relevantes:

- PSF de disco para desenfoque simple.
- PSF con Zernike para aberraciones de orden alto.
- Wiener deconvolution para estabilidad.
- Procesamiento solo en canal de luminancia `Y` para reducir color bleeding.
- Tiling para acelerar FFT/deconvolucion.
- Edge mask para aplicar deconvolucion donde importa y reducir halos/ringing.
- Face tracking con camara para estimar distancia y angulo.
- Perspectiva de PSF segun posicion del observador.

Formula base de PSF circular:

```text
k(x, y) = chi, si sqrt(x^2 + y^2) <= r
k(x, y) = 0, si no
sum(k) = 1
```

Modelo de blur/deblur:

```text
B = I * k
P = I * h
Objetivo percibido: P * k ~= I
```

Donde:

```text
k = PSF del ojo
h = filtro inverso / Wiener inverse filter
I = imagen deseada
P = imagen pre-distorsionada que se muestra
```

Tecnicas anti-ringing:

```text
1. Detectar bordes/texto.
2. Crear mascara M.
3. Aplicar deconvolucion fuerte solo en zonas de borde/texto.
4. Mantener zonas planas menos alteradas.
5. Combinar: corrected = original * inverse(M) + deconvolved * M
```

Que nos sirve:

- Es el paper mas cercano a nuestro `VisionCompensator.exe` porque trabaja con pantalla normal/captura y procesamiento de imagen.
- Proxima mejora realista: cambiar el sharpening actual por una aproximacion Wiener/PSF en luma, con tiles y mascara de bordes.

Limitaciones:

- La deconvolucion puede producir halos fuertes si no se regulariza.
- PSF incorrecta empeora la imagen.
- Tiling necesita overlap/padding para evitar costuras.

## 5. Image Pre-compensation / Digital Inverse Filtering Para Accesibilidad

Referencias citadas por el arXiv:

- M. Alonso Jr, A. Barreto, J. G. Cremades, `Image pre-compensation to facilitate computer access for users with refractive errors`, ACM SIGACCESS Accessibility and Computing, 2003.
- M. Alonso Jr. y A. Barreto, `Digital image inverse filtering for improving visual acuity for computer users with visual aberrations`, Inverse Problems in Science and Engineering, 2008.

Idea principal:

- Pre-filtrar imagen de computadora para usuarios con errores refractivos.
- Usa inverse filtering/deconvolution sin necesariamente requerir hardware light-field.

Que nos sirve:

- Justifica una version puramente software de nuestro proyecto.
- Es la linea mas compatible con Win32/DXGI: capturar pantalla, filtrar, mostrar overlay corregido.

Limitaciones:

- Un display 2D no puede recrear completamente la optica de anteojos, especialmente para grandes errores refractivos.
- El filtro inverso puede necesitar bajar contraste o limitar frecuencias para evitar ringing.

## 6. NETRA: Self-Evaluation Del Ojo

Autores: Vitor Pamplona, Ankit Mohan, Manuel Menezes de Oliveira Neto, Ramesh Raskar.

Link: https://www.media.mit.edu/publications/netra-interactive-display-for-self-evaluation-of-an-eye-for-visual-accommodation-and-focal-range/

Idea principal:

- Herramienta interactiva para autoevaluar acomodacion/rango focal usando display.
- Relacionado con optometria computacional, no necesariamente con corregir toda la pantalla.

Que nos sirve:

- Reafirma que un test subjetivo guiado por pantalla puede estimar parametros del usuario.
- Podemos tomar la idea de interaccion simple: el usuario alinea o compara patrones, y el algoritmo infiere refraccion.

## Algoritmos Que Vale La Pena Integrar

### A. PSF + Wiener Deconvolution En Luma

Prioridad alta para nuestro proyecto.

Pipeline propuesto:

```text
captura RGB
convertir RGB -> YCbCr
calcular PSF segun S/C/A/distancia
aplicar Wiener inverse filter solo al canal Y
mantener Cb/Cr o suavizarlos muy poco
convertir YCbCr -> RGB
clamp
```

Beneficio:

- Menos color bleeding.
- Mas estable que sharpen RGB directo.
- Mejor base para GPU.

Riesgo:

- FFT real-time en CPU puede ser pesada; conviene GPU/tiles.

### B. Edge-Masked Deconvolution

Prioridad alta.

Pipeline:

```text
detectar bordes con Sobel/Laplacian
dilatar mascara 1-3 px
aplicar filtro fuerte en bordes/texto
aplicar filtro suave en zonas planas
mezclar con feather para evitar halos
```

Beneficio:

- Texto mas legible.
- Menos ringing en fondos planos.

### C. Parametros Zernike Opcionales

Prioridad media.

Mapear receta a low-order Zernike:

```text
defocus ~= S + C / 2
astig_0_90 ~= (C / 2) * cos(2A)
astig_45_135 ~= (C / 2) * sin(2A)
```

Mas adelante permitir importar coeficientes de aberrometro:

```text
coma_x, coma_y, trefoil_x, trefoil_y, spherical
```

### D. Bayesian/QUEST/ZEST Para Calibracion

Prioridad media-alta.

El JCC actual es staircase adaptativo. Para hacerlo mas predictivo:

```text
mantener distribucion posterior para S, C y A
actualizar probabilidad segun respuesta 1/2/igual/peor
elegir siguiente par que maximice informacion esperada
terminar cuando incertidumbre < umbral
```

Respuesta del usuario como modelo probabilistico:

```text
P(elige candidato A | true x) = sigmoid((error(B,x) - error(A,x)) / sigma)
```

### E. Eye/Head Tracking Opcional

Prioridad media.

Usar camara o manual input para:

```text
distancia al monitor
angulo horizontal/vertical
posicion relativa del ojo
```

Aplicar ajuste:

```text
PSF_perspective = perspectiveWarp(PSF, headPose)
```

## Recomendacion Para VisionCompensator

Ruta realista, sin hardware nuevo:

1. Mantener UI Win32 actual para calibracion subjetiva.
2. Reemplazar `ApplyPixelProcessing` por modo hibrido: luma sharpening + edge mask.
3. Agregar PSF simple parametrica para defocus/astigmatismo.
4. Implementar version GPU D3D11/HLSL cuando el algoritmo CPU ya este estable.
5. Guardar perfil por usuario: OD/OS, distancia, deconv, contraste, PSF mode.
6. Solo investigar light-field/parallax barrier si se quiere hardware externo.

## Referencias

- Pamplona, V. F., Oliveira, M. M., Aliaga, D. G., Raskar, R. `Tailored displays to compensate for visual aberrations`. ACM TOG 31(4), 2012. DOI: `10.1145/2185520.2185577`.
- MIT Media Lab. `Tailored Displays to Compensate for Visual Aberrations`. https://www.media.mit.edu/publications/tailored-displays-to-compensate-for-visual-aberrations/
- Purdue CGVLab. `Tailored displays to compensate for visual aberrations`. https://www.cs.purdue.edu/cgvlab/www/publications/pamplona2012tailored/
- Huang, F.-C., Wetzstein, G., Barsky, B. A., Raskar, R. `Computational light field display for correcting visual aberrations`. MIT Media Lab, 2013. https://www.media.mit.edu/publications/computational-light-field-display-for-correcting-visual-aberrations/
- Huang, F.-C., Wetzstein, G., Barsky, B. A., Raskar, R. `Eyeglasses-free display: towards correcting visual aberrations with computational light field displays`. ACM TOG, 2014. DOI: `10.1145/2601097.2601122`.
- Patent US10529059B2. `Vision correcting display with aberration compensation using inverse blurring and a light field display`. https://patents.google.com/patent/US10529059B2/en
- Balaji, A., Ramu, D. `Real-Time Computational Visual Aberration Correcting Display Through High-Contrast Inverse Blurring`. arXiv:2501.01450v1. https://arxiv.org/html/2501.01450v1
- Pamplona, V., Mohan, A., Oliveira Neto, M. M., Raskar, R. `NETRA: Interactive Display for Self-evaluation of an Eye for Visual Accommodation and Focal Range`. https://www.media.mit.edu/publications/netra-interactive-display-for-self-evaluation-of-an-eye-for-visual-accommodation-and-focal-range/
