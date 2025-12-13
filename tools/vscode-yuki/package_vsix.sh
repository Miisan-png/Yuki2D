#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="${ROOT_DIR}/dist"
mkdir -p "${OUT_DIR}"

cd "${ROOT_DIR}"

VERSION="$(node -p "require('./package.json').version")"
OUT_FILE="${OUT_DIR}/yuki-language-${VERSION}.vsix"

npx --yes @vscode/vsce package --out "${OUT_FILE}" --allow-missing-repository
echo "${OUT_FILE}"
