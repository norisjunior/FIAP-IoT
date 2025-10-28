
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Desligando plataforma IoT..." -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan


Write-Host ""
Set-Location "IoTStack"
docker compose down
Write-Host ""


Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Todos os serviços foram parados" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
