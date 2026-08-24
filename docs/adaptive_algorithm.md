# Adaptive Algorithm

Notas para mejorar la refraccion subjetiva rapida del usuario.

## Objetivo

Converger en una receta util en menos de 6-15 respuestas, sin saltos bruscos y sin depender de un valor inicial fijo.

No reemplaza un examen medico. Es un calibrador psicofisico de pantalla.

## Flujo Actual Recomendado

Secuencia simple y robusta:

```text
1. Esfera: busqueda binaria adaptativa sobre S
2. Cilindro: busqueda binaria adaptativa sobre C
3. Eje: JCC digital alrededor del eje actual
4. Afinacion manual: +/- sobre S, C, A y deconv
```

Reglas de respuesta:

```text
Usuario elige 1: centro = opcion1, step *= 0.55
Usuario elige 2: centro = opcion2, step *= 0.55
Usuario elige Igual: mantener centro, step *= 0.50
Usuario elige Mucho peor: mantener receta actual, step *= 0.50
```

Umbrales practicos:

```text
S minimo: 0.125 D
C minimo: 0.125 D
A minimo: 2 grados
max respuestas por fase: 5
```

## QUEST / ZEST Simplificado

Para una version mas predictiva, modelar cada parametro como distribucion discreta.

Ejemplo para esfera:

```text
gridS = [-10.00, -9.875, ..., +10.00]
prior uniforme o gaussiano alrededor del valor inicial
```

Cada pregunta presenta dos candidatos `a` y `b`. La probabilidad de elegir `a` se puede modelar con una funcion logistica de utilidad:

```text
P(choose a | true x) = sigmoid((error(b, x) - error(a, x)) / sigma)
error(candidate, true) = abs(candidate - true)
```

Despues de cada respuesta:

```text
posterior[x] = prior[x] * P(response | x)
normalizar posterior
centro = media o MAP(posterior)
step = ancho del intervalo creible 68% o 95%
```

## Seleccion De La Proxima Pregunta

Elegir el par que maximice informacion esperada:

```text
candidate1 = centro + step / 2
candidate2 = centro - step / 2
step = max(minStep, credibleWidth * 0.5)
```

Para eje, usar circularidad 0/180:

```text
deltaAxis(a, b) = min(abs(a - b), 180 - abs(a - b))
```

## Patron Visual

El test debe mezclar:

```text
Letras Snellen grandes y pequenas
Radial astigmatic dial cada 15 grados
Texto serif/sans/mono
Numeros y pares confundibles: C/O/G/Q, B/8/3/S, P/R/F/E
Bordes de alto contraste blanco/negro
```

La instruccion debe pedir comparar halos, doble sombra, direccion de smear y nitidez de bordes, no solo "se ve mejor".
