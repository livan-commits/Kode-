# Ashwing — Dragon 3D

This directory is the integration target for the user's Ashwing Drake Lite GLB asset.

## Current status

- [x] Dedicated feature branch created.
- [x] Native Kode app integration target documented.
- [x] Confirmed that the public Kode SDK is a C/ESP-IDF SDK for ESP32-P4 and lists LVGL display support.
- [ ] Obtain the original `Ashwing_Drake_Lite.glb` as a readable file in the current development workspace.
- [ ] Validate the GLB (mesh, materials, skeleton, animation clips).
- [ ] Build a desktop 3D preview that uses the original GLB without replacing its rig or animations.
- [ ] Build a Kode Dot companion app for selecting animation states and controls.
- [ ] Test the native app on `kode-sim` and measure memory, frame time, and asset limits.

## Asset facts to verify

The supplied README describes the Lite model as:

- 11,992 triangles
- 61-bone rig
- Six animation clips: idle, walk, roar, bite, takeoff, fly
- 30 fps
- Embedded 2K base-colour, normal and metallic/roughness textures

These are source-description facts, not yet independently verified against the GLB binary. The actual animation names, durations, skeleton and material bindings must be inspected from the file before coding the viewer.

## Rendering strategy

The desktop preview is the fidelity target: load the original GLB, preserve its materials and armature, and play the embedded clips.

The public Kode SDK README documents a C application SDK for ESP32-P4 and an LVGL display example. It does not document a native glTF/GLB skeletal renderer. The existing Kode app in this repository pins SDK components at v0.1.2, so current SDK documentation must not be mistaken for proof that a renderer exists in that pinned version.

Therefore, treat the desktop viewer and Kode Dot app as separate deliverables. The Dot app can provide animation selection and controls, but do not claim full 3D on-device until a compatible renderer is identified and measured. If full 3D is not feasible, any reduced on-device representation must be documented as a fallback, never as the original real-time 3D model.

## Asset and licensing

Do not commit or redistribute the GLB until its included license has been checked. The model may be referenced locally during development, but the repository must not contain the model unless its terms permit that use.

## Acceptance criteria

1. The desktop preview visibly uses the exact Ashwing mesh and textures from the supplied GLB.
2. The original skeleton is retained.
3. Every embedded animation clip can be selected and played.
4. Missing or unsupported clips are reported rather than replaced with fabricated animation.
5. The Kode Dot app builds against the repository's pinned Kode SDK and is tested on the user's Windows/WSL2 `kode-sim` setup.
6. Any reduced on-device representation is documented as a fallback, not described as full 3D.
