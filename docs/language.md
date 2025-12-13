# Language Basics

Yuki Script is dynamically typed and game-focused. Values can be `nil`, number, bool, string, map, array, or function.

## Syntax highlights
- Variables: `var hp = 100;`
- Named functions: `fn update(dt) { ... }`
- Lambdas (anonymous functions): `var f = fn(x) { return x + 1; };`
- Imports:
  - `import("util_module.ys");` loads a module and (if it exports a map) injects its keys into globals.
  - `require("util_module.ys");` loads a module and returns its `exports` without injecting globals.
  - `import("util_module.ys", "util");` binds the module exports to `util`.
- Control flow: `if/else`, `while`, `for`, `do { ... } while (...)`, `break`, `continue`, `return`.
- Truthiness: `false`, `nil`, and number `0` are falsey; everything else is truthy.
- Comments: `// line` and `/* block */`.
- Strings: `"quoted"`, concatenation via `+`, escapes: `\n`, `\t`, `\"`, `\\`.
- Arrays:
  - Literal: `[1, 2, 3]`
  - Builtins: `array(1, 2, 3)`, `push(arr, v)`, `pop(arr)`, `len(arr)`
  - Indexing/assign: `arr[i]`, `arr[i] = v`
- Maps:
  - Literal (identifier keys): `{ x: 10, y: 20 }`
  - Builtins: `map("x", 10, "y", 20)`, `len(m)`
  - Property/index: `m.x`, `m["x"]`, `m.x = 5`, `m["x"] = 5`

## Runtime behavior
- Arguments missing in a call become `nil`.
- Division and mod are floating-point; division/mod by zero is a runtime error.
- Equality: strict by type; numbers use epsilon compare; maps/arrays/functions compare by reference.
- Numeric operators (`- * / % < <= > >=`) require numbers; calling a non-function is a runtime error.
- Errors: runtime errors log with a stack trace; `assert(cond, msg)` and `error(msg)` raise runtime errors (useful with `yuki2d --run`).

## Entry points
- `init()` is called once after loading.
- `update(dt)` is called each frame with delta time in seconds.
