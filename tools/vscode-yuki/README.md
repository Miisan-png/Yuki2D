# Yuki VS Code Support

This folder contains a minimal VS Code extension that provides syntax highlighting for `*.ys`.

This is intentionally kept inside the main repo for fast iteration. Once the language stabilizes, it’s best to move this into a dedicated repository and publish it to the VS Code Marketplace.


## Package (.vsix)

From the repo root:

- `bash tools/vscode-yuki/package_vsix.sh`

Then install the generated `.vsix`:

## What it includes

- TextMate grammar highlighting (keywords, comments, strings, numbers, common builtins, common engine API prefixes)
- Language configuration (comments, brackets, auto-close pairs)
- Starter snippets (init/update, scene exports, common statements)

## Structure

- `syntaxes/`: TextMate grammar
- `snippets/`: VS Code snippets
- `data/symbols.json`: source-of-truth list for keywords/builtins (for future autocomplete work)
