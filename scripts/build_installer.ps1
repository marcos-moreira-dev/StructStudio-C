param(
    [string] $BuildDir = "build"
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$buildPath = Join-Path $projectRoot $BuildDir
$artifactRoot = Join-Path $projectRoot "artifacts"
$sessionRoot = Join-Path $artifactRoot "installer_logs"
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$runDir = Join-Path $sessionRoot $timestamp

New-Item -ItemType Directory -Force -Path $runDir | Out-Null

function Invoke-LoggedStep {
    param(
        [Parameter(Mandatory = $true)][string] $Name,
        [Parameter(Mandatory = $true)][string] $Command
    )

    $logPath = Join-Path $runDir "$Name.log"
    Write-Host ""
    Write-Host "=============================================================================="
    Write-Host $Name
    Write-Host "=============================================================================="
    Write-Host $Command
    & powershell -NoProfile -ExecutionPolicy Bypass -Command $Command 2>&1 | Tee-Object -FilePath $logPath
    if ($LASTEXITCODE -ne 0) {
        throw "La etapa '$Name' fallo con codigo de salida $LASTEXITCODE."
    }
}

Invoke-LoggedStep -Name "configure" -Command "cmake -S `"$projectRoot`" -B `"$buildPath`" -G Ninja"
Invoke-LoggedStep -Name "build" -Command "cmake --build `"$buildPath`""
Invoke-LoggedStep -Name "test" -Command "ctest --test-dir `"$buildPath`" --output-on-failure"
Invoke-LoggedStep -Name "package" -Command "cpack --config `"$buildPath\CPackConfig.cmake`""

$packages = Get-ChildItem -Path $buildPath -File | Where-Object { $_.Extension -in '.exe', '.zip', '.tar.gz', '.tgz' }
$summaryPath = Join-Path $runDir "summary.md"

@(
    "# Resumen de empaquetado"
    ""
    "- Fecha: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    "- Build dir: `$BuildDir"
    "- Paquetes detectados:"
) + ($packages | ForEach-Object { "- $($_.Name)" }) | Set-Content -Path $summaryPath -Encoding UTF8

Write-Host ""
Write-Host "Paquetes generados:"
$packages | ForEach-Object { Write-Host " - $($_.FullName)" }
Write-Host ""
Write-Host "Logs:"
Write-Host " - $runDir"
