# Yuki Quickstart

## What is Yuki?
Yuki2D is an experimental 2D engine with its own game-focused scripting language. APIs and semantics may change; use at your own risk.

## Build and run
- Prereqs: CMake, a C++17 compiler, OpenGL/GLFW dev libs.
- Build: `cmake -S . -B build && cmake --build build`
- Run demo: `./run.sh` or `./run.sh demo/main.ys`
- Headless scripting:
  - Parse-only: `./build/yuki2d --check demo/main.ys`
  - Run `init()` only (no window): `./build/yuki2d --run demo/main.ys`

## Your first script
```ys
var sheet;
var idle;
var x = 200;
var y = 200;

fn init() {
    sheet = load_sprite_sheet("asset_pack/characters/player.png", 48, 48);
    idle = anim_create(sheet, array(0, 1, 2, 3, 4, 5), 6, true);
    anim_set_origin(idle, 24, 24);
    anim_set_scale(idle, 2, 2);
}

fn update(dt) {
    var speed = 120;
    if (is_key_down("a")) x = x - speed * dt;
    if (is_key_down("d")) x = x + speed * dt;
    if (is_key_down("w")) y = y - speed * dt;
    if (is_key_down("s")) y = y + speed * dt;
    anim_set_position(idle, x, y);
    anim_play(idle, false);
    anim_draw(idle);
}
```

Save as `scripts/example_main.ys` (or point the executable at it). This shows:
- Loading a sprite sheet and creating a looping animation.
- Reading input.
- Updating position with `dt`.
- Drawing via the animation API.
