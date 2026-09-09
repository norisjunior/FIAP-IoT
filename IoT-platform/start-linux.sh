#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
if [[ ! -f .env ]]; then
    echo "Arquivo .env ausente. Copie .env.exemplo para .env e ajuste os valores." >&2
    exit 1
fi

docker compose --env-file .env -f docker-compose.yml up -d
