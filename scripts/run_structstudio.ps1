<#
  StructStudio C
  --------------
  Runner de despliegue local para desarrollo y diagnostico.

  Objetivo:
  - configurar el proyecto con CMake
  - compilar la app y los tests
  - ejecutar ctest
  - dejar logs separados por etapa
  - lanzar la aplicacion al final

  El script centraliza el flujo para que, si algo falla, el usuario pueda
  compartir una sola carpeta de logs con contexto suficiente.
#>

[CmdletBinding()]
param(
    [string]$BuildDir = "build",
    [string]$Generator = "Ninja",
    [string]$LogRoot = "artifacts\\run_logs",
    [switch]$SkipConfigure,
    [switch]$SkipBuild,
    [switch]$SkipTests,
    [switch]$SkipLaunch,
    [int]$LaunchProbeSeconds = 3
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$script:SessionRoot = $null
$script:SummaryPath = $null

function Write-Section {
    param([string]$Title)

    Write-Host ""
    Write-Host ("=" * 78)
    Write-Host $Title
    Write-Host ("=" * 78)
}

function Add-SummaryLine {
    param([string]$Line)

    if ($script:SummaryPath) {
        Add-Content -Path $script:SummaryPath -Value $Line
    }
}

function Get-AbsolutePath {
    param(
        [string]$BasePath,
        [string]$RelativeOrAbsolutePath
    )

    if ([System.IO.Path]::IsPathRooted($RelativeOrAbsolutePath)) {
        return [System.IO.Path]::GetFullPath($RelativeOrAbsolutePath)
    }

    return [System.IO.Path]::GetFullPath((Join-Path $BasePath $RelativeOrAbsolutePath))
}

function Clear-StaleNinjaLock {
    param([string]$ResolvedBuildDir)

    $lockPath = Join-Path $ResolvedBuildDir ".ninja_lock"
    if (-not (Test-Path $lockPath)) {
        return
    }

    $ninjaProcesses = @(Get-Process -Name "ninja" -ErrorAction SilentlyContinue)
    if ($ninjaProcesses.Count -eq 0) {
        Remove-Item $lockPath -Force -ErrorAction SilentlyContinue
        Add-SummaryLine "- Se elimino un archivo .ninja_lock huerfano antes del build."
    }
}

function Convert-ToCmdArgument {
    param([string]$Value)

    if ([string]::IsNullOrEmpty($Value)) {
        return '""'
    }

    if ($Value.Contains('"')) {
        $Value = $Value.Replace('"', '\"')
    }

    if ($Value -match '\s') {
        return '"' + $Value + '"'
    }

    return $Value
}

function Invoke-LoggedStep {
    param(
        [string]$Name,
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$WorkingDirectory,
        [string]$LogFilePath
    )

    Write-Section $Name
    Add-SummaryLine ""
    Add-SummaryLine "## $Name"
    Add-SummaryLine "- Comando: $FilePath $($Arguments -join ' ')"
    Add-SummaryLine "- Log: $LogFilePath"

    if (Test-Path $LogFilePath) {
        Remove-Item $LogFilePath -Force -ErrorAction SilentlyContinue
    }

    $cmdPieces = @()
    $cmdPieces += Convert-ToCmdArgument -Value $FilePath
    foreach ($argument in $Arguments) {
        $cmdPieces += Convert-ToCmdArgument -Value $argument
    }

    $stepScriptPath = Join-Path $script:SessionRoot ($Name + ".cmd")
    $commandLine = ($cmdPieces -join " ") + ' > "' + $LogFilePath + '" 2>&1'
    $stepScriptLines = @(
        "@echo off",
        ('cd /d "{0}"' -f $WorkingDirectory),
        $commandLine,
        "exit /b %ERRORLEVEL%"
    )

    Set-Content -Path $stepScriptPath -Value ($stepScriptLines -join "`r`n") -Encoding ASCII

    try {
        $global:LASTEXITCODE = 0
        & cmd.exe /d /c $stepScriptPath
    } catch {
        $message = "No se pudo iniciar el proceso '$FilePath'. $($_.Exception.Message)"
        $message | Tee-Object -FilePath $LogFilePath -Append | Out-Host
        Add-SummaryLine "- Estado: error al iniciar proceso"
        throw
    }

    $exitCode = if ($null -eq $global:LASTEXITCODE) { 0 } else { $global:LASTEXITCODE }
    Add-SummaryLine "- ExitCode: $exitCode"

    if (Test-Path $LogFilePath) {
        Get-Content $LogFilePath | Out-Host
    }

    if ($exitCode -ne 0) {
        throw "La etapa '$Name' fallo con codigo de salida $exitCode."
    }
}

function Write-EnvironmentSnapshot {
    param(
        [string]$ProjectRoot,
        [string]$ResolvedBuildDir,
        [string]$EnvironmentPath
    )

    $lines = @()
    $lines += "StructStudio C - Session Snapshot"
    $lines += "Timestamp: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    $lines += "ProjectRoot: $ProjectRoot"
    $lines += "BuildDir: $ResolvedBuildDir"
    $lines += "PowerShell: $($PSVersionTable.PSVersion)"
    $lines += "OS: $([System.Environment]::OSVersion.VersionString)"
    $lines += "CPU_COUNT: $env:NUMBER_OF_PROCESSORS"
    $lines += ""
    $lines += "Tools:"
    $lines += "-----"

    foreach ($tool in @("cmake", "ctest", "ninja", "gcc")) {
        $command = Get-Command $tool -ErrorAction SilentlyContinue
        if ($null -ne $command) {
            $lines += "$tool -> $($command.Source)"
        } else {
            $lines += "$tool -> NO ENCONTRADO"
        }
    }

    Set-Content -Path $EnvironmentPath -Value $lines
}

function Get-BuildCommand {
    param(
        [string]$ResolvedBuildDir,
        [string]$SelectedGenerator,
        [string[]]$Targets
    )

    if ($SelectedGenerator -eq "Ninja") {
        return @{
            FilePath = "ninja"
            Arguments = @("-C", $ResolvedBuildDir)
        }
    }

    $parallelJobs = if ([string]::IsNullOrWhiteSpace($env:NUMBER_OF_PROCESSORS)) { "4" } else { $env:NUMBER_OF_PROCESSORS }
    $arguments = @("--build", $ResolvedBuildDir, "--parallel", $parallelJobs)
    if ($Targets.Count -gt 0) {
        $arguments += @("--target") + $Targets
    }

    return @{
        FilePath = "cmake"
        Arguments = $arguments
    }
}

$projectRoot = Split-Path -Parent $PSScriptRoot
$resolvedBuildDir = Get-AbsolutePath -BasePath $projectRoot -RelativeOrAbsolutePath $BuildDir
$resolvedLogRoot = Get-AbsolutePath -BasePath $projectRoot -RelativeOrAbsolutePath $LogRoot
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$script:SessionRoot = Join-Path $resolvedLogRoot $timestamp

New-Item -ItemType Directory -Path $script:SessionRoot -Force | Out-Null
New-Item -ItemType Directory -Path $resolvedBuildDir -Force | Out-Null

$script:SummaryPath = Join-Path $script:SessionRoot "summary.md"
$transcriptPath = Join-Path $script:SessionRoot "session_console.log"
$environmentPath = Join-Path $script:SessionRoot "environment.txt"
$latestPointerPath = Join-Path $resolvedLogRoot "latest.txt"

Set-Content -Path $script:SummaryPath -Value @(
    "# StructStudio C - resumen de sesion"
    ""
    "- Fecha: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    "- Proyecto: $projectRoot"
    "- Build: $resolvedBuildDir"
    "- Logs: $script:SessionRoot"
)

Write-EnvironmentSnapshot -ProjectRoot $projectRoot -ResolvedBuildDir $resolvedBuildDir -EnvironmentPath $environmentPath
Set-Content -Path $latestPointerPath -Value $script:SessionRoot

Start-Transcript -Path $transcriptPath -Force | Out-Null

try {
    Add-SummaryLine ""
    Add-SummaryLine "## Etapas"

    Invoke-LoggedStep `
        -Name "preflight_cmake_version" `
        -FilePath "cmake" `
        -Arguments @("--version") `
        -WorkingDirectory $projectRoot `
        -LogFilePath (Join-Path $script:SessionRoot "preflight_cmake_version.log")

    Invoke-LoggedStep `
        -Name "preflight_ctest_version" `
        -FilePath "ctest" `
        -Arguments @("--version") `
        -WorkingDirectory $projectRoot `
        -LogFilePath (Join-Path $script:SessionRoot "preflight_ctest_version.log")

    Invoke-LoggedStep `
        -Name "preflight_ninja_version" `
        -FilePath "ninja" `
        -Arguments @("--version") `
        -WorkingDirectory $projectRoot `
        -LogFilePath (Join-Path $script:SessionRoot "preflight_ninja_version.log")

    if (-not $SkipConfigure) {
        Invoke-LoggedStep `
            -Name "configure" `
            -FilePath "cmake" `
            -Arguments @("-S", $projectRoot, "-B", $resolvedBuildDir, "-G", $Generator) `
            -WorkingDirectory $projectRoot `
            -LogFilePath (Join-Path $script:SessionRoot "configure.log")
    } else {
        Add-SummaryLine "- Se omitio la etapa de configuracion."
    }

    if (-not $SkipBuild) {
        Clear-StaleNinjaLock -ResolvedBuildDir $resolvedBuildDir

        $buildTargets = @(
            "StructStudioC",
            "structstudio_core_smoke",
            "structstudio_core_variants_smoke",
            "structstudio_analysis_smoke",
            "structstudio_graph_analysis_advanced_smoke",
            "structstudio_analysis_playback_smoke",
            "structstudio_theory_smoke",
            "structstudio_examples_smoke",
            "structstudio_animation_smoke",
            "structstudio_transform_smoke"
        )
        $buildCommand = Get-BuildCommand `
            -ResolvedBuildDir $resolvedBuildDir `
            -SelectedGenerator $Generator `
            -Targets $buildTargets

        Invoke-LoggedStep `
            -Name "build" `
            -FilePath $buildCommand.FilePath `
            -Arguments $buildCommand.Arguments `
            -WorkingDirectory $projectRoot `
            -LogFilePath (Join-Path $script:SessionRoot "build.log")
    } else {
        Add-SummaryLine "- Se omitio la etapa de build."
    }

    if (-not $SkipTests) {
        $testArgs = @("--test-dir", $resolvedBuildDir, "--output-on-failure")
        Invoke-LoggedStep `
            -Name "test" `
            -FilePath "ctest" `
            -Arguments $testArgs `
            -WorkingDirectory $projectRoot `
            -LogFilePath (Join-Path $script:SessionRoot "test.log")
    } else {
        Add-SummaryLine "- Se omitio la etapa de pruebas."
    }

    $exePath = Join-Path $resolvedBuildDir "StructStudioC.exe"
    if (-not (Test-Path $exePath)) {
        throw "No se encontro el ejecutable esperado en '$exePath'."
    }

    if (-not $SkipLaunch) {
        $launchLogPath = Join-Path $script:SessionRoot "launch.log"

        Write-Section "launch"
        Add-SummaryLine ""
        Add-SummaryLine "## launch"
        Add-SummaryLine "- Ejecutable: $exePath"
        Add-SummaryLine "- Log: $launchLogPath"

        $beforeIds = @(
            Get-Process -Name "StructStudioC" -ErrorAction SilentlyContinue |
            ForEach-Object { $_.Id }
        )

        $launchedProcess = Start-Process `
            -FilePath $exePath `
            -WorkingDirectory $resolvedBuildDir `
            -PassThru

        Start-Sleep -Seconds $LaunchProbeSeconds

        $launchLines = @()
        $launchLines += "StructStudio C launch probe"
        $launchLines += "ProbeSeconds: $LaunchProbeSeconds"
        $launchLines += "Proceso iniciado por Start-Process: $($launchedProcess.Id)"

        $afterProcesses = @(
            Get-Process -Name "StructStudioC" -ErrorAction SilentlyContinue
        )
        $newProcesses = @(
            $afterProcesses | Where-Object { $beforeIds -notcontains $_.Id }
        )

        if ($newProcesses.Count -gt 0) {
            $launchLines += "Estado: se detecto al menos un proceso nuevo despues del lanzamiento."
            $launchLines += "PIDs: $($newProcesses.Id -join ', ')"
            Add-SummaryLine "- Estado: lanzado correctamente con PID(s) $($newProcesses.Id -join ', ')"
        } elseif ($afterProcesses.Count -gt 0) {
            $launchLines += "Estado: la aplicacion ya estaba abierta o no se pudo distinguir el proceso nuevo."
            $launchLines += "PIDs actuales: $($afterProcesses.Id -join ', ')"
            Add-SummaryLine "- Estado: la app parece abierta, pero no se distinguio un PID nuevo."
        } else {
            $launchLines += "Estado: no se detecto un proceso activo despues del probe."
            Add-SummaryLine "- Estado: no se detecto un proceso activo despues del lanzamiento."
        }

        $launchLines += ""
        $launchLines += "Nota: el lanzamiento de la GUI se hace en modo desacoplado, por lo que este log"
        $launchLines += "no captura stdout/stderr del proceso. Su valor principal es dejar trazabilidad"
        $launchLines += "del intento de arranque y del estado de los procesos observados."

        Set-Content -Path $launchLogPath -Value $launchLines
        Get-Content $launchLogPath | Out-Host
    } else {
        Add-SummaryLine "- Se omitio el lanzamiento de la aplicacion."
    }

    Add-SummaryLine ""
    Add-SummaryLine "## Resultado"
    Add-SummaryLine "- Flujo completado."
    Add-SummaryLine "- Comparte la carpeta de logs si necesitas depuracion: $script:SessionRoot"

    Write-Host ""
    Write-Host "Sesion completada. Logs disponibles en:"
    Write-Host $script:SessionRoot
}
catch {
    Add-SummaryLine ""
    Add-SummaryLine "## Error"
    Add-SummaryLine "- $($_.Exception.Message)"
    Write-Error $_
    exit 1
}
finally {
    Stop-Transcript | Out-Null
}
