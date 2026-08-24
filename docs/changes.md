# Cambios Recientes

## 2026-08-24 - UI moderna, calibracion mas fina y docs tecnicos

### Interfaz Win32

- Se reemplazaron botones clasicos por botones `owner-draw` con alto contraste, colores fuertes y bordes redondeados.
- Se agrandaron fuentes, botones principales, controles `+/-`, valores de receta e instrucciones.
- Se amplio el panel derecho para mejorar legibilidad a distancia.
- Se ajusto el fondo principal a tema oscuro de alto contraste.
- Se limpiaron textos con iconos que podian renderizarse mal en Windows/MinGW.

### Test visual y calibracion

- El flujo JCC ahora permite hasta 5 respuestas por fase antes de avanzar automaticamente.
- Esfera y cilindro afinan hasta `0.125 D`.
- El eje afina hasta `2 grados`.
- La opcion `IGUAL / LISTO` reduce el paso antes de saltar de fase, evitando terminar demasiado pronto.
- La opcion `MUCHO PEOR` ya no reinicia a valores fijos; conserva la receta actual y reduce el paso.
- Las opciones del test se limitan a rangos seguros para evitar salirse de `-10D/+10D` en esfera y `-8D/+8D` en cilindro.

### Documentacion agregada

- `docs/optical_formulas.md`: formulas de pre-compensacion, matriz afin, Zernike simplificado y PSF/filtro inverso.
- `docs/adaptive_algorithm.md`: algoritmo subjetivo adaptativo, reglas JCC, base para QUEST/ZEST y patron visual recomendado.
- `docs/driver_architecture.md`: ruta para evolucionar de Win32/GDI+ a DXGI + Direct3D 11 y eventualmente IddCx.

### Build

- Se recompilo `VisionCompensator.exe` correctamente con MinGW/G++.
- Se elimino el warning de `UNICODE` redefinido.

## 2026-08-24 - Investigacion de estado del arte

- Se agrego `docs/prior_art_and_algorithms.md` con investigaciones previas sobre pantallas que corrigen vision.
- Se documentaron metodos usados por MIT Media Lab, Purdue/ACM, UC Berkeley/Barsky y trabajos recientes de inverse blurring.
- Se identificaron algoritmos utiles para integrar despues: PSF + Wiener deconvolution, luma-only YCbCr, edge-masked deconvolution, light-field prefiltering, Zernike y QUEST/ZEST.
- Se marco la diferencia entre lo viable en software sobre monitor 2D y lo que requiere hardware light-field/parallax/lenslet.

## 2026-08-24 - Decision de arquitectura GPU

- Se agrego `docs/gpu_overlay_viability_notes.md` con la decision tecnica: primero D3D11/DXGI/HLSL, no driver ni DisplayLink.
- Se agrego `docs/implementation_tasks.md` con fases concretas para implementar el overlay GPU.
- Se definio el rol del margen 90%: no es la graduacion, es espacio seguro para estirar/rotar sin cortar imagen.
- Se documento la formula inicial para `scaleAlongAxis` y `scaleAcrossAxis` usando `sphere`, `cylinder`, `axis`, distancia y margen.

## 2026-08-24 - Idea de fuente adaptativa

- Se agrego `docs/adaptive_font_for_vision.md` para explorar una fuente o perfil tipografico ajustado a la deficiencia visual del usuario.
- Se documento que esta tecnica puede mejorar letras sin latencia, pero no corrige fotos, video ni contenido rasterizado.
- Se agrego una fase de tareas para calibrar `weight`, `tracking`, `width`, `line-height` y exportar CSS/perfil.

## 2026-08-24 - Ruta Linux

- Se agrego `docs/linux_portability.md` con la diferencia entre el core optico portable y el codigo Windows-only.
- Se documento una ruta Linux por etapas: core C++ portable, GLSL/OpenGL, X11 primero, Wayland/PipeWire despues.
- Se agrego la opcion experimental `xrandr --transform` para probar transformaciones de pantalla completa en X11 con muy baja latencia.
- Se agrego Phase 9 en `docs/implementation_tasks.md` para invitar contribuciones Linux.

## 2026-08-24 - Inicio implementacion GPU

- Se extrajo `OpticalState` a `optical_state.h` para compartir parametros entre UI Win32 y renderer GPU.
- Se agrego `d3d_overlay.h/.cpp` con probe inicial de D3D11 hardware/WARP.
- Se agrego `optical_shader.hlsl` con transformacion conceptual de margen, eje y escalado anisotropico.
- Se actualizo `build.bat` para compilar el modulo D3D11 y enlazar `d3d11`/`dxgi`.
- El renderer activo sigue siendo GDI+; D3D11 aun esta en modo esqueleto/probe para reducir riesgo.

## 2026-08-24 - Notas de Ingenieria y Sintesis Tecnica

- Se agrego `docs/engineering_notes.md` con analisis critico de integracion:
  - Manejo de ciclo de vida DXGI (`DXGI_ERROR_ACCESS_LOST`, timeouts y adaptacion a 60/120/144Hz con 0% uso de GPU en reposo).
  - Arquitectura de *Input Passthrough* con `WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW` y atajos globales `Ctrl+Alt+V`, `F11` y `Esc`.
  - Extension del shader HLSL con deconvolucion PSF direccional y omnidireccional (Laplaciano dependiente del eje de astigmatismo).
- Alineacion total en la estrategia de ejecucion por fases para habilitar el pipeline Direct3D 11.
