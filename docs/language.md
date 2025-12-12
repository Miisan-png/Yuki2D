# Language Basics

Yuki Script is dynamically typed and game-focused. Values can be `nil`, number, bool, string, map, array, or function.

## Syntax highlights
- Variables: `var hp = 100;`
- Functions: `fn update(dt) { ... }`
- Imports: `import("util_module.ys");`
- Control: `if`, `while`; logical `and`/`or`; `return`.
- Truthiness: `false`, `nil`, and zero-number? No—only `false` and `nil` are falsey; numbers are truthy unless zero-check is explicit.
- Strings: quoted; concatenation via `+`.
- Arrays: `array(1, 2, 3)`; `array_push(arr, v)`; `array_get(arr, i)`.
- Maps: `map("x", 10, "y", 20)`; `map_get(m, "x")`; `map_has(m, "x")`.
- Functions are first-class; closures capture outer scope.

## Runtime behavior
- Arguments missing in a call become `nil`.
- Division and mod are floating-point.
- Equality: strict by type; numbers use epsilon compare; maps/arrays compare by reference.
- Errors: runtime errors log with a stack trace and halt further script calls until cleared.

## Entry points
- `init()` is called once after loading.
- `update(dt)` is called each frame with delta time in seconds.
