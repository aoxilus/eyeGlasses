# Vision Compensator 🥑

**Computational vision-correction experiments for Windows. by [aoxilus](https://github.com/aoxilus)**

Aplicación nativa de Windows en **C++ (Win32 + GDI+)** que pre-compensa tu visión sin lentes con soporte de **Click-Through Transparente (Ratón y Teclado funcionales)**, **Color Verdadero sin Inversión**, y **Atajos Globales**.

---

## 🚀 Cómo Ejecutar

Haz doble clic en:
📁 [**`VisionCompensator.exe`**](file:///c:/Users/aoxil/OneDrive%20-%20El%20Paso%20Community%20College/Documents/eyeGlasses/VisionCompensator.exe)

---

## 🛠️ Correcciones y Mejoras Críticas:

1. **🖱️ Ratón y Teclado 100% Funcionales (`WS_EX_TRANSPARENT` Click-Through):**
   - Al pulsar **`[ 🚀 APLICAR A MONITORES ]`** o **`F11`**, el overlay se vuelve **transparente para clics y teclado**.
   - Puedes hacer clic en enlaces de Chrome, programar en VS Code, escribir y seleccionar texto con normalidad mientras la pantalla permanece corregida ópticamente.

2. **🎨 Corrección del Color Invertido / Negativo:**
   - Se reemplazó la fórmula no-lineal de contraste por un escalado lineal directo. Los blancos, negros, colores saturados y fuentes conservan su tono y brillo original 1:1.

3. **⌨️ Atajos Globales de Teclado (Funcionan en todo Windows):**
   - **`Ctrl + Alt + V`**: Activar / Desactivar la compensación óptica global.
   - **`F11`**: Alternar Pantalla Completa.
   - **`Escape`**: Salir de Pantalla Completa y volver al panel de ajuste y calibración.

---

## Investigación

Ver `docs/` para notas sobre:

- Fórmulas ópticas y PSF.
- Algoritmos adaptativos tipo JCC/QUEST/ZEST.
- Arquitectura GPU con DXGI + Direct3D 11 + HLSL.
- Estado del arte sobre vision-correcting displays.
- Fuente adaptativa para mejorar legibilidad de texto.

---

## License

CC BY-NC-SA 4.0. Ver [`LICENSE`](./LICENSE).

Made with 🥑 by [aoxilus](https://github.com/aoxilus)
