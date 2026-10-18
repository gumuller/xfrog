[CmdletBinding()]
param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',
    [switch]$WithDDrawCompat,
    [switch]$SmokeTest,
    [switch]$Run
)
$ErrorActionPreference = 'Stop'
$xfrogRoot = $PSScriptRoot
$xfrogCmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
if ($xfrogCmakeCommand) {
    $xfrogCmake = $xfrogCmakeCommand.Source
} else {
    $xfrogVswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $xfrogVswhere)) {
        throw 'Install Visual Studio 2022 with Desktop development with C++ and CMake tools.'
    }
    $xfrogVsPath = & $xfrogVswhere -latest -version '[17.0,18.0)' -products '*' -property installationPath
    $xfrogCmake = Join-Path $xfrogVsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    if (-not (Test-Path -LiteralPath $xfrogCmake)) { throw 'Visual Studio CMake tools are missing.' }
}
Push-Location -LiteralPath $xfrogRoot
try {
    & $xfrogCmake --preset windows-x86
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
    & $xfrogCmake --build --preset $Configuration.ToLowerInvariant()
    if ($LASTEXITCODE -ne 0) { throw 'CMake build failed.' }
    $xfrogOutput = Join-Path $xfrogRoot "build\windows-x86\bin\$Configuration"
    if ($WithDDrawCompat) {
        # Official v0.7.2 binary release (marked prerelease upstream).
        $xfrogArchiveHash = 'A932A4DCB2100552B6918FD4A3CC7DE138BB97448BBCA2A7051954993A397B39'
        $xfrogCache = Join-Path $xfrogRoot 'build\downloads'
        New-Item -ItemType Directory -Path $xfrogCache -Force | Out-Null
        $xfrogArchive = Join-Path $xfrogCache 'DDrawCompat-v0.7.2.zip'
        if (-not (Test-Path -LiteralPath $xfrogArchive)) {
            Invoke-WebRequest 'https://github.com/narzoul/DDrawCompat/releases/download/v0.7.2/DDrawCompat-v0.7.2.zip' -OutFile $xfrogArchive
        }
        if ((Get-FileHash -LiteralPath $xfrogArchive -Algorithm SHA256).Hash -ne $xfrogArchiveHash) {
            throw "Checksum mismatch: $xfrogArchive"
        }
        $xfrogExtracted = Join-Path $xfrogCache 'DDrawCompat-v0.7.2'
        Expand-Archive -LiteralPath $xfrogArchive -DestinationPath $xfrogExtracted -Force
        $xfrogDll = Join-Path $xfrogOutput 'ddraw.dll'
        $xfrogSourceDll = Join-Path $xfrogExtracted 'ddraw.dll'
        if ((Test-Path -LiteralPath $xfrogDll) -and
            (Get-FileHash -LiteralPath $xfrogDll).Hash -ne (Get-FileHash -LiteralPath $xfrogSourceDll).Hash) {
            throw 'A different ddraw.dll is already present. Move it aside before installing DDrawCompat.'
        }
        if (-not (Test-Path -LiteralPath $xfrogDll)) {
            Copy-Item -LiteralPath $xfrogSourceDll -Destination $xfrogDll
        }
        Copy-Item -LiteralPath (Join-Path $xfrogRoot 'cmake\DDrawCompat-LICENSE.txt') -Destination $xfrogOutput
    }
    Write-Host "Built: $xfrogOutput\xfrog.exe"
    if ($SmokeTest) {
        $xfrogTest = Start-Process -FilePath (Join-Path $xfrogOutput 'xfrog.exe') -ArgumentList '--smoke-test' -WorkingDirectory $xfrogOutput -PassThru
        if (-not $xfrogTest.WaitForExit(20000)) {
            Stop-Process -Id $xfrogTest.Id
            throw 'Smoke test timed out after 20 seconds.'
        }
        Get-Content -LiteralPath (Join-Path $xfrogOutput 'xfrog-smoke.log')
        if ($xfrogTest.ExitCode -ne 0) { throw "Smoke test failed: exit code $($xfrogTest.ExitCode)" }
    }
    if ($Run) { Start-Process -FilePath (Join-Path $xfrogOutput 'xfrog.exe') -WorkingDirectory $xfrogOutput }
} finally {
    Pop-Location
}
