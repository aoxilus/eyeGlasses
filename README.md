# Vision Compensator - Algoritmo Clínico Jackson (JCC) & Aplicar a Monitores

Aplicación nativa de Windows en **C++ (Win32 + GDI+)** que modela el método clínico de **Cilindro Cruzado de Jackson (JCC)** con pasos finos de $\pm 5^\circ$ y botón directo de **Aplicar a Monitores**.

---

## 🚀 Cómo Ejecutar

Haz doble clic en:
📁 [**`VisionCompensator.exe`**](file:///c:/Users/aoxil/OneDrive%20-%20El%20Paso%20Community%20College/Documents/eyeGlasses/VisionCompensator.exe)

---

## 🔬 ¿Cómo adivinan la graduación los aparatos electrónicos oftalmológicos?

1. **Forópteros Digitales (Método de Cilindro Cruzado de Jackson - JCC):**
   - En lugar de saltar a ángulos lejanos ($45^\circ$ o $90^\circ$), los forópteros electrónicos mantienen el cilindro anclado y solo oscilan en **micro-pasos de $\pm 5^\circ$**.
   - Evalúan la simetría entre dos meridianos ortogonales hasta que el paciente no percibe diferencia de estiramiento.
2. **Autorefractómetros de Frente de Onda (Aberrometría Hartmann-Shack):**
   - Proyectan un punto de luz infrarroja en la retina y calculan los coeficientes de Zernike:
     - **$C_2^0$ (Defocus):** Esfera / Miopía.
     - **$C_2^{-2}, C_2^2$ (Astigmatismo vertical y oblicuo):** Cilindro y Eje.

---

## 🕹️ Nuevas Mejoras Implementadas:

1. **🚫 Fin a los giros exagerados de pantalla:**
   - La oscilación de ángulo ahora está delimitada estrictamente a **$\pm 5^\circ$** (siguiendo el estándar JCC).
   - Se acotó la deformación anamórfica de corte (*shear limit*) para que el marco del monitor se mantenga siempre perfectamente recto.
2. **🚀 Botón `[ 🚀 APLICAR A MONITORES ]`:**
   - Un solo clic activa inmediatamente la pre-compensación óptica en pantalla completa sobre tus monitores.
3. **🖥️ Multi-Monitor Integrado:**
   - Botón `[ 🖥️ Cambiar Monitor (1 / 2) ]` para dirigir la imagen al Monitor 1 o Monitor 2.
4. **⌨️ Tecla `F11`:**
   - Entra y sale de pantalla completa en cualquier instante.
