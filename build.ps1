[CmdletBinding()]
param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',
    [switch]$SmokeTest,
    [switch]$Run
)
$ErrorActionPreference = 'Stop'
$xfrogCmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
if ($xfrogCmakeCommand) {
    $xfrogCmake = $xfrogCmakeCommand.Source
} else {
    $xfrogVswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $xfrogVswhere)) { throw 'Install Visual Studio 2022 with Desktop development with C++ and CMake tools.' }
    $xfrogVsPath = & $xfrogVswhere -latest -version '[17.0,18.0)' -products '*' -property installationPath
    $xfrogCmake = Join-Path $xfrogVsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    if (-not (Test-Path -LiteralPath $xfrogCmake)) { throw 'Visual Studio CMake tools are missing.' }
}
Push-Location -LiteralPath $PSScriptRoot
try {
    & $xfrogCmake --preset windows-x86
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
    & $xfrogCmake --build (Join-Path $PSScriptRoot "build\windows-x86") --config $Configuration --parallel 2
    if ($LASTEXITCODE -ne 0) { throw 'CMake build failed.' }
    $xfrogOutput = Join-Path $PSScriptRoot "build\windows-x86\bin\$Configuration"
    Write-Host "Built Xfrog 2.0: $xfrogOutput\xfrog.exe"
    if ($SmokeTest) {
        $xfrogTest = Start-Process -FilePath (Join-Path $xfrogOutput 'xfrog.exe') -ArgumentList '--smoke-test' -WorkingDirectory $xfrogOutput -PassThru
        if (-not $xfrogTest.WaitForExit(30000)) { Stop-Process -Id $xfrogTest.Id; throw 'Smoke test timed out.' }
        if ($xfrogTest.ExitCode -ne 0) { throw "Smoke test failed: $($xfrogTest.ExitCode)" }
    }
    if ($Run) { Start-Process -FilePath (Join-Path $xfrogOutput 'xfrog.exe') -WorkingDirectory $xfrogOutput }
} finally { Pop-Location }
