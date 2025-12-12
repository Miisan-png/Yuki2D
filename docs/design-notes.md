# Design Notes

- Update order: input is polled, then `update(dt)` runs, then engine ticks animations/tweens and renders.
- Booleans: only `false` and `nil` are falsey; numbers/strings are truthy regardless of value.
- Animations: frame time = `1/fps`; non-looping anims clamp on last frame and stop playing.
- Collision: AABB, axis-resolved, non-swept; fast movers may need sub-stepping.
- Coordinate system: origin top-left, +x right, +y down; rotations in degrees, clockwise positive.
- Asset paths: resolved relative to the main script directory.
