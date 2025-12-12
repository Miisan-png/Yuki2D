# Engine API Reference

All functions are available globally after engine init. Booleans accept `true`/`false` or nonzero/zero numbers.

## Render
- `set_clear_color(r, g, b)`
- `load_sprite(path)` -> spriteId
- `load_sprite_sheet(path, frame_w, frame_h)` -> sheetId
- `draw_sprite(id, x, y)`
- `draw_sprite_ex(id, x, y, rot_deg, scale_x, scale_y, flip_x=false, flip_y=false, origin_x=-1, origin_y=-1, alpha=1)`
- `draw_sprite_frame(sheet_id, frame, x, y, rot_deg, scale_x, scale_y, flip_x, flip_y=false, origin_x=-1, origin_y=-1, alpha=1)`
- `draw_rect(x, y, w, h, r, g, b)`
- `load_font(image_path, metrics_json)` -> fontId
- `draw_text(font_id, text, x, y, [k/v: scale, color r g b a, align left|center|right, max_width, line_height])`
- `measure_text_width(font_id, text, scale=1, max_width=0, line_height=0)`
- `measure_text_height(font_id, text, scale=1, max_width=0, line_height=0)`
- Camera: `set_camera(x, y)`, `set_camera_zoom(z)`, `set_camera_rotation(deg)` (see renderer bindings if added).

## Animation
- `anim_create(sheet_id, frames_array, fps, loop_bool)` -> animId
- `anim_play(anim_id, reset_bool=true)`
- `anim_stop(anim_id)`
- `anim_reset(anim_id)`
- `anim_set_position(anim_id, x, y)`
- `anim_set_origin(anim_id, ox, oy)`
- `anim_set_scale(anim_id, sx, sy)`
- `anim_set_rotation(anim_id, deg)`
- `anim_set_flip(anim_id, flip_x, flip_y=false)`
- `anim_set_alpha(anim_id, alpha)`
- `anim_draw(anim_id)`

## Collision
- `collider_create(x, y, w, h, tag, solid=true)` -> colId
- `collider_set_position(id, x, y)`
- `collider_set_size(id, w, h)`
- `collider_get_position(id)` -> map("x", x, "y", y)
- `collider_get_size(id)` -> map("w", w, "h", h)
- `collider_move(id, dx, dy)` -> array of maps with hit ids/tags
- `rect_overlaps(x1, y1, w1, h1, x2, y2, w2, h2)` -> bool
- `point_in_rect(px, py, rx, ry, rw, rh)` -> bool
- Areas: `create_area_rect(x, y, w, h, tag)` -> areaId; `set_area_rect(id, x, y, w, h)`; `area_overlaps_tag(id, tag)`; `area_entered(id, tag)`/`area_exited(id, tag)` track changes frame-to-frame.

## Input
- `is_key_down(key)` -> bool; `is_key_pressed(key)` -> bool
- Keys: GLFW names (e.g., `"a"`, `"left"`, `"space"`).

## Tween
- `tween_value(from, to, duration, easing="linear", on_complete=nil)` -> tweenId
- `tween_property(target_type, target_id, property, to, duration, easing="linear", on_complete=nil)` -> tweenId
- Control: `tween_cancel(id)`, `tween_pause(id)`, `tween_resume(id)`
- Sequences/parallels: `sequence_create(array_of_tween_ids)` -> seqId; `sequence_play(seqId)`, similar for parallels.

## Core
- `get_time()` -> seconds since start
- `get_screen_size()` -> map("w", w, "h", h)
- `print(...)` logs to console
- `import("path")` to load other scripts relative to main script dir
