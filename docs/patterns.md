# Patterns and Recipes

## Directional animation switch
```ys
var move_up; var move_down; var move_right;
var idle_up; var idle_down; var idle_right;
var active;
var facing = "down";

fn update(dt) {
    var mx = (is_key_down("d") or is_key_down("right")) - (is_key_down("a") or is_key_down("left"));
    var my = (is_key_down("s") or is_key_down("down")) - (is_key_down("w") or is_key_down("up"));
    var moving = mx != 0 or my != 0;
    if (moving) {
        if (abs(mx) > abs(my)) facing = (mx > 0) ? "right" : "left";
        else facing = (my > 0) ? "down" : "up";
        active = facing == "down" ? move_down : (facing == "up" ? move_up : move_right);
    } else {
        active = facing == "down" ? idle_down : (facing == "up" ? idle_up : idle_right);
    }
    var flip_side = facing == "left";
    anim_set_flip(move_right, flip_side, false);
    anim_set_flip(idle_right, flip_side, false);
    anim_play(active, false);
    anim_draw(active);
}
```

## Simple collision move
```ys
var col = collider_create(x, y, 32, 32, "player", true);
fn move_player(dx, dy) {
    var hits = collider_move(col, dx, dy);
    var pos = collider_get_position(col);
    x = map_get(pos, "x") + 16;
    y = map_get(pos, "y") + 16;
    return hits;
}
```

## UI text block
```ys
draw_text(font, "Hello", 20, 20, "scale", 2, "color", 1, 0.9, 0.6, 1);
draw_text(font, "Wrapped body copy across a set width.", 20, 60, "max_width", 200, "line_height", 14);
```

## Aseprite tag-driven anim
```ys
var ase = ase_load("test.aseprite");
var idle = ase_anim(ase, "idle");
var walk = ase_anim(ase, "walk");
fn update(dt) {
    var moving = is_key_down("a") or is_key_down("d") or is_key_down("w") or is_key_down("s");
    var active = moving ? walk : idle;
    anim_play(active, false);
    anim_set_position(active, 160, 120);
    anim_draw(active);
}
```

## Room-based camera follow
```ys
fn init() {
    camera_follow_enable(true);
    set_virtual_resolution(640, 360);
    camera_follow_lerp(8);
}
fn update(dt) {
    var pos = player_get_position();
    camera_follow_target(map_get(pos, "x"), map_get(pos, "y"));
}
```
