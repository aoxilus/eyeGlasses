# Adaptive Font For Vision Correction

Idea: ademas de deformar toda la pantalla, `VisionCompensator` puede generar o recomendar una fuente optimizada para la deficiencia visual del usuario. Esto no reemplaza lentes ni corrige imagenes, pero puede mejorar mucho la lectura de texto en Windows, navegador, IDE, documentos y UI.

## Hipotesis

Si conocemos aproximadamente:

```text
sphere
cylinder
axis
deconv/sharpness preference
contrast preference
```

podemos crear una fuente o estilo tipografico que compense parcialmente los errores mas molestos al leer:

```text
blur horizontal/vertical
smear diagonal por astigmatismo
confusion entre letras parecidas
perdida de contraste en bordes finos
fatiga por letras delgadas
```

La fuente no puede corregir toda la optica del ojo, pero puede aumentar robustez perceptual.

## Que Puede Mejorar

- Letras mas gruesas en trazos finos.
- Mayor x-height para lectura rapida.
- Aperturas mas grandes en `c`, `e`, `a`, `s`.
- Diferencias mas claras entre `O/0`, `I/l/1`, `B/8`, `S/5`, `P/R/F`.
- Contraste alto sin depender de subpixel rendering.
- Espaciado extra para reducir fusion de letras.
- Ligera compensacion direccional segun axis del astigmatismo.

## Que No Puede Hacer

- No corrige fotos, video ni iconos.
- No corrige ambos ojos distinto a la vez si Windows usa una sola fuente.
- No reemplaza PSF/deconvolucion para desenfoque fuerte.
- No cambia apps que no permitan cambiar fuente.
- No puede alterar texto rasterizado dentro de imagenes.

## Dos Enfoques Posibles

### 1. Fuente Estatica Generada

Generar una familia `.ttf` o `.otf` basada en una fuente open-source, modificando parametros globales:

```text
weight: mas bold si hay blur
width: mas expanded si hay fusion horizontal
spacing: mas tracking si hay ghosting
aperture: aperturas mas abiertas
stroke contrast: bajo, para que no desaparezcan trazos delgados
```

Ventaja:

```text
Funciona en todo Windows donde se pueda escoger fuente.
No requiere overlay.
Cero latencia.
```

Desventaja:

```text
Solo ayuda texto.
Instalar/cambiar fuente del sistema puede ser limitado por Windows/apps.
```

### 2. Fuente Variable / Presets

Usar o generar una variable font con ejes personalizados:

```text
wght = grosor
wdth = ancho
opsz = optical size
TRAK = espaciado/tracking
ASTX = compensacion horizontal
ASTY = compensacion vertical
SLNT = compensacion angular leve
```

Ventaja:

```text
Permite crear presets por usuario sin muchos archivos.
Puede adaptarse a distancia y pantalla.
```

Desventaja:

```text
No todas las apps soportan ejes variables personalizados.
Windows no aplica ejes custom globalmente en todos lados.
```

## Relacion Con Astigmatismo

El astigmatismo afecta mas un meridiano. En letras, eso se puede atacar con:

```text
axis cerca de 0/180: reforzar trazos verticales y espaciado horizontal
axis cerca de 90: reforzar trazos horizontales y altura/x-height
axis diagonal: evitar formas muy cerradas y aumentar grosor general
```

No conviene deformar brutalmente la fuente. Mejor usar cambios pequenos:

```text
stroke boost: 5% - 18%
tracking boost: 2% - 8%
width boost: 0% - 6%
aperture boost: 5% - 15%
```

## Parametros Iniciales Desde Receta

Reglas heuristicas:

```text
blurStrength = clamp(abs(sphere) * 0.08 + abs(cylinder) * 0.12, 0, 1)
astigStrength = clamp(abs(cylinder) / 4.0, 0, 1)
baseWeight = 500 + blurStrength * 250
tracking = 0.01em + blurStrength * 0.04em
width = 100% + astigStrength * 5%
```

Para el perfil actual aproximado:

```text
sphere = -4.50
cylinder = -1.25
axis = 95
```

Preset sugerido:

```text
weight: 650-750
width: 102%-105%
tracking: +0.025em a +0.045em
line-height: 1.25-1.40
font smoothing: evitar trazos ultraligeros
tema: alto contraste, fondo oscuro, texto claro
```

## Implementacion Practica

Primero no generar TTF. Primero crear un modo de prueba dentro de la app:

```text
1. Renderizar cartilla con estilos tipograficos A/B.
2. El usuario escoge cual lee mejor.
3. Guardar perfil tipografico.
4. Exportar recomendaciones CSS/Windows.
```

Ejemplo CSS exportable:

```css
:root {
  font-family: "Atkinson Hyperlegible", "Segoe UI", sans-serif;
  font-weight: 700;
  letter-spacing: 0.035em;
  line-height: 1.35;
  color: #ffffff;
  background: #050811;
}
```

Fuentes base recomendadas para experimentar:

```text
Atkinson Hyperlegible
Lexend
Inter
Segoe UI Variable
OpenDyslexic, solo como referencia; no asumir que ayuda astigmatismo
```

## Integracion Con Windows

Opciones realistas:

```text
Exportar CSS para navegador/apps web
Crear tema alto contraste recomendado
Permitir seleccionar fuente en apps compatibles
Generar una TTF personalizada mas adelante
Aplicar overlay GPU solo sobre texto detectado, futuro OCR/edge-mask
```

Windows moderno no permite cambiar absolutamente toda la tipografia del sistema de forma limpia como antes. Por eso conviene combinar:

```text
fuente optimizada para apps configurables
overlay GPU para todo lo demas
tema alto contraste
ClearType calibrado
```

## Algoritmo De Calibracion De Fuente

Mini test A/B:

```text
1. Mostrar palabra critica: O0Il1B8S5PREFCGQ
2. Generar dos estilos con diferente weight/tracking/width
3. Usuario elige 1, 2, igual o peor
4. Actualizar parametros con staircase o bayesiano
5. Repetir 8-12 veces
6. Guardar preset final
```

Parametros a optimizar:

```text
fontWeight: 400-850
letterSpacing: 0-0.08em
fontWidth: 95%-110%
strokeContrast: bajo/medio
fontSizeMinimum: 14-22 px
lineHeight: 1.15-1.50
```

## Formato De Perfil

```json
{
  "visualProfile": {
    "sphere": -4.5,
    "cylinder": -1.25,
    "axis": 95,
    "distanceCm": 60
  },
  "fontProfile": {
    "family": "Atkinson Hyperlegible",
    "weight": 700,
    "widthPercent": 104,
    "letterSpacingEm": 0.035,
    "lineHeight": 1.35,
    "minimumSizePx": 18,
    "contrastTheme": "dark-high-contrast"
  }
}
```

## Roadmap

1. Agregar una pantalla `Font Test` al programa.
2. Renderizar A/B con diferentes parametros tipograficos.
3. Guardar recomendaciones en `vision_profile.json`.
4. Exportar CSS y una pagina HTML de prueba.
5. Investigar generacion TTF con `fonttools` o Glyphs/FontForge, usando una fuente open-source compatible con licencia.
6. Opcional: usar overlay GPU para realzar texto detectado, no solo deformar toda la pantalla.

## Conclusion

La idea es buena y viable como complemento. No corrige toda la vision, pero puede hacer que las letras sean mas tolerantes al blur y al astigmatismo. Es especialmente util porque no agrega latencia y puede mejorar lectura incluso antes de tener el overlay GPU perfecto.
