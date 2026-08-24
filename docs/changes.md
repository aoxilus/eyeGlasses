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
