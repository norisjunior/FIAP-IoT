$ErrorActionPreference = "Stop"
Push-Location $PSScriptRoot
try {
    if (-not (Test-Path -LiteralPath '.env' -PathType Leaf)) {
        throw "Arquivo .env ausente. Copie .env.exemplo para .env e ajuste os valores."
    }
    docker compose --env-file .env -f docker-compose.yml down
    if ($LASTEXITCODE -ne 0) { throw "Docker Compose falhou (codigo $LASTEXITCODE)." }
} finally {
    Pop-Location
}
