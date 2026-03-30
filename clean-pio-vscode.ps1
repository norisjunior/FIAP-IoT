param(
    [switch]$Execute
)

$raiz = $PSScriptRoot
$alvos = @(".pio", ".vscode")

Write-Host "Raiz: $raiz"

if (-not $Execute) {
    Write-Host "Modo simulacao: nenhum diretorio sera removido."
    Write-Host "Use -Execute para excluir de verdade."
}

Get-ChildItem -Path $raiz -Directory | ForEach-Object {
    $dirProjeto = $_.FullName

    foreach ($nome in $alvos) {
        $alvo = Join-Path $dirProjeto $nome

        if (Test-Path $alvo) {
            if ($Execute) {
                Remove-Item -Path $alvo -Recurse -Force
                Write-Host "Removido: $alvo"
            }
            else {
                Write-Host "[SIMULACAO] Removeria: $alvo"
            }
        }
    }
}