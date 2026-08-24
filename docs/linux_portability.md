# Linux Portability Notes

## Short Answer

Yes, Vision Compensator can support Linux, but the current app is not Linux-compatible as-is.

The optical math is portable C/C++ logic. The current windowing, capture, overlay, hotkeys, and rendering are Windows-specific:

```text
Windows-only today:
Win32 windowing
GDI+ rendering
BitBlt desktop capture
WS_EX_TRANSPARENT click-through overlay
RegisterHotKey
Direct3D 11 / DXGI plan
```

Portable today or easy to extract:

```text
OpticalState
SPH/CYL/AXIS math
90% margin transform
anisotropic stretch logic
adaptive JCC calibration logic
future adaptive font profile
```

## Best Linux Strategy

Do not try to port Win32/GDI+ directly. Split the project into:

```text
core optical engine: portable C++
platform backend: Windows / Linux / web / future driver
renderer backend: GDI+ / D3D11 / OpenGL / Vulkan
```

Recommended Linux architecture:

```text
Linux UI + OpenGL/Vulkan shader overlay
X11 capture path: XComposite / XDamage / XShm
Wayland capture path: PipeWire screen capture portal
Wayland compositor path: KWin/GNOME/wlroots plugin only for deeper integration
```

## X11 Option

X11 is easier than Wayland for experimentation.

Possible methods:

- Capture desktop using `XComposite`, `XDamage`, or `XShm`.
- Show borderless fullscreen OpenGL window.
- Use XShape/XFixes input region to make overlay click-through.
- Apply correction in GLSL fragment shader.
- Use global shortcuts through X11 grabs.

Pros:

- More permissive screen capture.
- Easier click-through overlay.
- Good for open-source prototype.

Cons:

- X11 is legacy on many distros.
- Multi-monitor and HiDPI can be messy.
- Compositor behavior varies.

## Wayland Option

Wayland is more secure but harder.

Possible methods:

- Capture screen with PipeWire + `xdg-desktop-portal`.
- Render correction in OpenGL/Vulkan.
- Use a normal fullscreen window or compositor-specific layer shell.

Major limitation:

```text
Wayland usually does not allow arbitrary global overlays that see/capture everything and pass clicks through without compositor permission.
```

For serious Wayland support, a compositor plugin may be required:

- KWin effect for KDE Plasma.
- GNOME Shell/Mutter extension or patch, more limited for raw pixel filtering.
- wlroots-based compositor plugin/layer-shell approach.

Pros:

- Modern Linux display stack.
- Better security model.

Cons:

- No universal overlay API.
- Portal capture can require user approval.
- Click-through behavior is compositor-specific.

## Shortcut: XRandR Transform

On X11, Linux already has output transforms through `xrandr --transform`. This can scale, translate, rotate, and shear the entire output at the display server level.

This is interesting for our 90% + stretch idea.

Example concept only:

```bash
xrandr --output HDMI-1 --transform a,b,c,d,e,f,g,h,i
```

Potential use:

```text
Apply 90% centered screen transform
Apply small anisotropic stretch
Apply shear/rotation approximation
```

Pros:

- Very low latency.
- No screen capture loop.
- System-wide.

Cons:

- X11 only.
- No deconvolution/sharpening.
- Can make desktop awkward if transform is too strong.
- Exact matrix tuning is tricky and distro/driver dependent.

This could become a Linux experimental backend before building full OpenGL capture.

## Adaptive Font On Linux

Linux is actually good for the adaptive font idea.

Useful systems:

- `fontconfig` for font substitution.
- GNOME/KDE font settings.
- Browser CSS/user styles.
- Terminal font configuration.

Possible deliverables:

```text
vision_profile.json
generated CSS
fontconfig snippet
recommended font preset
optional patched TTF/OTF later
```

This path has zero overlay latency and helps text-first users.

## Recommended Cross-Platform Refactor

Target layout:

```text
src/core/optical_state.h
src/core/optical_math.h/.cpp
src/core/adaptive_calibration.h/.cpp
src/platform/windows/win32_app.cpp
src/platform/windows/d3d_overlay.cpp
src/platform/linux/linux_overlay_x11.cpp
src/platform/linux/linux_overlay_wayland.cpp
shaders/optical_shader.hlsl
shaders/optical_shader.glsl
```

Keep shader math equivalent across HLSL and GLSL:

```text
HLSL for Windows D3D11
GLSL for Linux OpenGL/Vulkan
```

## Build Options

Windows:

```text
MinGW or MSVC
D3D11/DXGI backend
```

Linux:

```text
CMake
OpenGL or Vulkan
X11 backend first
Wayland/PipeWire backend later
```

Libraries to consider:

- GLFW or SDL2 for cross-platform windows/input.
- OpenGL for first Linux shader prototype.
- Vulkan later if latency and control matter.
- PipeWire for Wayland screen capture.
- X11/XComposite for initial Linux desktop capture.

## Practical Roadmap

1. Move all optical calculations out of Win32 code into portable C++.
2. Keep Windows GDI+ as fallback.
3. Finish Windows D3D11 shader backend first because current machine can test it.
4. Add GLSL shader matching the HLSL math.
5. Ask Linux contributors to test X11 backend.
6. Add Wayland/PipeWire backend only after X11/OpenGL proof works.
7. Add Linux adaptive font export because that is low-risk and useful quickly.

## Conclusion

The project can become Linux-friendly, but not by compiling `main.cpp` directly. The right move is to make the optical engine cross-platform and keep display capture/rendering behind platform backends.

For Linux community traction, the first realistic feature should be:

```text
adaptive font profiles + GLSL demo window + X11 transform notes
```

Then build toward full desktop correction.
