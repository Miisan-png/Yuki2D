# Yuki VS Code Support

This folder contains a minimal VS Code extension that provides syntax highlighting for `*.ys`.

This is intentionally kept inside the main repo for fast iteration. Once the language stabilizes, it’s best to move this into a dedicated repository and publish it to the VS Code Marketplace.

## Install (local dev)

1. Open `tools/vscode-yuki/` in VS Code.
2. Open the Run & Debug sidebar.
3. Choose `Run Yuki Syntax Extension`, then press `F5`.
4. In the new VS Code window, open any `*.ys` file to see highlighting.

If the file doesn’t highlight, click the language mode in the bottom-right and select `Yuki`.

## Package (.vsix)

From the repo root:

- `bash tools/vscode-yuki/package_vsix.sh`

Then install the generated `.vsix`:

- VS Code UI: Extensions → `...` → “Install from VSIX…”
- CLI: `code --install-extension tools/vscode-yuki/dist/yuki-language-0.0.1.vsix`

## What it includes

- TextMate grammar highlighting (keywords, comments, strings, numbers, common builtins, common engine API prefixes)
- Language configuration (comments, brackets, auto-close pairs)
- Starter snippets (init/update, scene exports, common statements)

## Structure

- `syntaxes/`: TextMate grammar
- `snippets/`: VS Code snippets
- `data/symbols.json`: source-of-truth list for keywords/builtins (for future autocomplete work)
