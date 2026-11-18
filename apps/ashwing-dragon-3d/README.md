# Ashwing — Dragon 3D

This directory is the integration target for the user's original Ashwing Drake Lite GLB asset.

## Current status

- [x] Dedicated feature branch and integration target created.
- [x] Original GLB received and parsed successfully (glTF 2.0 binary).
- [x] Mesh, skin, material, texture images and animation clips inspected directly from the GLB.
- [x] Browser-based Three.js viewer implemented; it loads a user-selected local GLB, keeps the embedded materials and animations, and provides orbit/zoom, clip selection, playback speed and exposure controls.
- [x] Viewer entry point linked from the Kode Ecosystem home page.
- [x] Public Kode SDK documentation checked for native 3D support.
- [ ] Runtime-test the viewer in a real browser with the supplied GLB.
- [ ] Verify the model's license before distributing or committing the binary asset.
- [ ] Identify a supported native rendering path, or document a clearly labelled 2D/preview fallback if the Kode Dot cannot render this GLB.
- [ ] Build the Kode Dot companion app and test it on the user's Windows/WSL2 `kode-sim` setup.

## Verified GLB facts

The uploaded `Ashwing_Drake_Lite.glb` was read and parsed directly. It is a glTF 2.0 binary containing:

- 1 mesh primitive with 9,152 vertices and 35,976 indices (11,992 triangles)
- 1 skin with 61 joints
- 1 material named `Ashwing_Drake`, using base-colour, normal and metallic/roughness textures
- 3 embedded 2048 × 2048 texture images: `Ashwing_BaseColor`, `Ashwing_Normal` and `Ashwing_MetalRough`
- 6 embedded animation clips, each with 184 animation channels:
  - `bite` — 1.2 s
  - `fly_loop` — 0.8 s
  - `idle_loop` — 3.0 s
  - `roar` — 2.0 s
  - `takeoff` — 2.4 s
  - `walk_loop` — 1.07 s

Durations above are taken from the maximum input keyframe time in each clip. They are clip lengths, not a claim about a fixed playback frame rate.

## Rendering strategy

The desktop preview is the fidelity target: load the original GLB, preserve its mesh, material, textures and armature, and play the six embedded clips.

The browser viewer uses Three.js 0.170.0 and reads the selected GLB locally in the browser. It does not upload the model to a server. The binary is deliberately not committed to this repository until its license and redistribution terms are verified.

A deeper check found that recent LVGL 9.6 does contain an optional glTF viewer, but it is not a drop-in ESP32-P4 solution. LVGL's own documentation requires OpenGL ES 2.0 support plus 3D texture support; its glTF renderer uses GLSL 300 ES shaders and external C++/WebP dependencies. The Kode SDK's current simulator LVGL configuration explicitly sets `LV_USE_GLTF 0`, and its simulator CMake excludes the LVGL `gltf` source directory. The ESP32-P4 datasheet documents PPA for 2D image operations (scale, rotate, mirror and blend), not an OpenGL/3D GPU. This does not prove that no software-only renderer could ever be written, but there is no supported hardware-accelerated GLB path in the current documented Kode Dot stack.\n\nTherefore, keep the desktop viewer as the true interactive 3D deliverable. For the physical Dot, the realistic next option is a pre-rendered sprite animation exported from the exact Ashwing GLB, with the six original clips represented as 2D frames and labelled honestly as pre-rendered 2D. Do not claim this is live 3D or that the Dot is running the GLB skeleton. A native app can still provide animation selection and controls. Any such sprite pipeline must first be rendered and measured for flash, PSRAM, frame rate and display memory on the target.\n\nThe desktop viewer and Kode Dot app remain separate deliverables. If a true on-device 3D renderer becomes available, it must be built and measured before being advertised as supported.

## Asset and licensing

The GLB has been inspected, but its redistribution license has not yet been verified in this workspace. Do not commit or redistribute the binary until its included license is checked. The model may be used locally for development in the meantime.

## Acceptance criteria

1. The desktop preview visibly uses the exact Ashwing mesh and textures from the supplied GLB.
2. The original 61-joint skin is retained.
3. All six embedded animation clips can be selected and played.
4. Missing or unsupported clips are reported rather than replaced with fabricated animation.
5. The viewer is tested in a browser with this exact GLB.
6. The Kode Dot app builds against the repository's pinned Kode SDK and is tested on the user's Windows/WSL2 `kode-sim` setup.
7. Any reduced on-device representation is documented as a fallback, not described as full 3D.
