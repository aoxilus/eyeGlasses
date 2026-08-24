# Vision Compensator 🥑

**Put your glasses prescription into your monitor and see if text gets clearer.**

Vision Compensator is an experimental Windows app that tries to make your screen behave a little bit like digital glasses. You enter or tune values similar to a prescription, then the app pre-warps the image on your monitor so your eyes may perceive sharper text.

It is not just zoom. It experiments with astigmatism axis, directional stretch, screen margin, contrast, and edge sharpening to test whether a normal display can become easier to read for a specific viewer.

Made by [aoxilus](https://github.com/aoxilus).

---

## What It Does

If your vision blurs text in a predictable way, the screen can try to send the opposite distortion first.

```text
normal monitor -> your eye blur -> blurry text
corrected monitor -> your eye blur -> clearer text
```

You can try values like:

- `SPH` / sphere: myopia or hyperopia-like correction.
- `CYL` / cylinder: astigmatism strength.
- `AXIS`: astigmatism direction.
- Viewing distance.
- Sharpness and contrast.

Then apply the correction fullscreen and test real text on your desktop.

---

## Why It Matters

Millions of people spend the entire day reading screens. Glasses and contacts are still the correct medical solution, but displays themselves are also programmable optical surfaces. This project explores a practical question:

**Can software make a normal monitor more readable for one specific person's eyes?**

Maybe partly. Maybe only for text. Maybe only for certain prescriptions. That is exactly what this prototype is trying to test.

---

## Current Prototype

- Native Windows app in C++.
- High-contrast calibration UI.
- Fullscreen correction mode.
- Click-through overlay so mouse and keyboard can still work.
- Subjective A/B visual test for tuning values.
- Early Direct3D 11 / HLSL GPU path started.
- Research notes included in `docs/`.

The current version is experimental and not a medical device.

---

## Try It

Build:

```bat
build.bat
```

Run:

```text
VisionCompensator.exe
```

Suggested test:

1. Start with your real prescription if you know it.
2. Sit at your normal monitor distance.
3. Compare Option 1 and Option 2.
4. Choose the clearer one.
5. Press `IGUAL` when you cannot tell the difference.
6. Apply fullscreen correction and read normal text.

Hotkeys:

- `Ctrl + Alt + V`: toggle correction.
- `F11`: fullscreen.
- `Esc`: return to adjustment mode.

---

## I Need Expert Feedback

Optometrists, ophthalmologists, vision scientists, computational imaging researchers, GPU developers, and accessibility engineers: please challenge this.

Questions this project needs help answering:

- Which parts are optically meaningful, and which parts are placebo?
- How should `SPH / CYL / AXIS` map to screen-space transforms?
- Can a normal 2D monitor produce useful reading improvement without light-field hardware?
- What is the safest subjective test flow?
- What visual acuity or text-readability metrics should be used?
- Could adaptive fonts help when full-screen image correction is too slow?

If you know eyes, optics, shaders, or accessibility, your opinion is valuable here.

---

## Research Notes

See `docs/`:

- `docs/prior_art_and_algorithms.md`: who has tried this before and what methods they used.
- `docs/optical_formulas.md`: screen-space correction formulas and PSF notes.
- `docs/adaptive_algorithm.md`: JCC, QUEST/ZEST, and subjective calibration.
- `docs/gpu_overlay_viability_notes.md`: why Direct3D 11 is the next practical step.
- `docs/implementation_tasks.md`: build plan.
- `docs/adaptive_font_for_vision.md`: custom font idea for clearer text.

---

## Roadmap

- Move the fullscreen correction path from CPU/GDI+ to Direct3D 11 shaders.
- Add DXGI Desktop Duplication for faster whole-screen correction.
- Add luma-only sharpening/deconvolution to reduce color artifacts.
- Add edge-aware filtering focused on text clarity.
- Add per-user vision profiles.
- Explore adaptive fonts generated from the user's visual profile.
- Evaluate virtual display / IddCx only after the GPU overlay proves useful.

---

## License

CC BY-NC-SA 4.0. See [`LICENSE`](./LICENSE).

Made with 🥑 by [aoxilus](https://github.com/aoxilus)
