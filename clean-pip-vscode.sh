#!/usr/bin/env bash

set -u

EXECUTE=0

if [[ "${1:-}" == "--execute" ]]; then
  EXECUTE=1
fi

RAIZ="$(cd "$(dirname "$0")" && pwd)"
ALVOS=(".pio" ".vscode")

echo "Raiz: $RAIZ"

if [[ "$EXECUTE" -eq 0 ]]; then
  echo "Modo simulacao: nenhum diretorio sera removido."
  echo "Use --execute para excluir de verdade."
fi

find "$RAIZ" -mindepth 1 -maxdepth 1 -type d | while IFS= read -r dirProjeto; do
  for nome in "${ALVOS[@]}"; do
    alvo="$dirProjeto/$nome"

    if [[ -d "$alvo" ]]; then
      if [[ "$EXECUTE" -eq 1 ]]; then
        rm -rf "$alvo"
        echo "Removido: $alvo"
      else
        echo "[SIMULACAO] Removeria: $alvo"
      fi
    fi
  done
done