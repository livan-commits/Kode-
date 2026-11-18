# Ashwing — Dragon 3D

This directory is the integration target for the user's Ashwing Drake Lite GLB asset.

## Current status

- [x] Dedicated feature branch created.
- [x] Native Kode app integration target documented.
- [ ] Add the original `Ashwing_Drake_Lite.glb` asset after confirming its redistribution terms.
- [ ] Validate the GLB (mesh, materials, skeleton, animation clips) with a local GLB inspection tool.
- [ ] Build a desktop 3D preview that uses the original GLB without replacing its rig or animations.
- [ ] Build a Kode Dot companion app for selecting animation states and controls.
- [ ] Test the native app on `kode-sim` and measure memory, frame time, and asset limits.

## Expected animation controls

The supplied asset is described in the project conversation as containing these clips:

- Idle
- Walk
- Roar
- Bite
- Takeoff
- Fly

Clip names and availability must be confirmed from the actual GLB before implementation.

## Rendering strategy

The desktop preview is the fidelity target: load the original GLB, preserve its materials and armature, and play the embedded clips.

The ESP32-P4 Kode Dot build is a separate constrained target. Do not assume that a full skeletal GLB renderer is available in the current Kode SDK. First establish renderer support and measure the asset's runtime footprint. If full 3D is not feasible on-device, keep the native app as the controller and use a tested lightweight representation on the device; do not silently substitute a different dragon model.

## Asset and licensing

Do not commit or redistribute the GLB until its included license has been checked. The model may be referenced locally during development, but the repository must not contain the model unless its terms permit that use.

## Acceptance criteria

1. The desktop preview visibly uses the exact Ashwing mesh and textures from the supplied GLB.
2. The original skeleton is retained.
3. Every embedded animation clip can be selected and played.
4. Missing or unsupported clips are reported rather than replaced with fabricated animation.
5. The Kode Dot app builds against the repository's pinned Kode SDK and is tested on the user's Windows/WSL2 `kode-sim` setup.
6. Any reduced on-device representation is documented as a fallback, not described as full 3D.
