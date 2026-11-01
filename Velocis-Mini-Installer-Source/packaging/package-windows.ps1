$ErrorActionPreference = 'Stop'

# Run this script on Windows with a full JDK (including jpackage) and WiX Toolset installed.
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$StudioDir = Join-Path $ProjectRoot 'velocis-studio'
$BuildDir = Join-Path $ProjectRoot 'build\windows-package'
$InputDir = Join-Path $BuildDir 'input'
$CompilerDir = Join-Path $InputDir 'compiler'
$ClassesDir = Join-Path $BuildDir 'classes'
$DistDir = Join-Path $ProjectRoot 'dist'

if (-not (Get-Command javac -ErrorAction SilentlyContinue)) { throw 'JDK not found. Install a JDK that includes javac and jpackage.' }
if (-not (Get-Command jpackage -ErrorAction SilentlyContinue)) { throw 'jpackage not found. Use a full JDK installation.' }
if (-not (Test-Path (Join-Path $ProjectRoot 'velocis.exe'))) { throw 'velocis.exe was not found in the project root.' }

Remove-Item $BuildDir -Recurse -Force -ErrorAction SilentlyContinue
New-Item $InputDir -ItemType Directory -Force | Out-Null
New-Item $CompilerDir -ItemType Directory -Force | Out-Null
New-Item $ClassesDir -ItemType Directory -Force | Out-Null
New-Item $DistDir -ItemType Directory -Force | Out-Null

Copy-Item (Join-Path $ProjectRoot 'velocis.exe') $CompilerDir
Copy-Item (Join-Path $StudioDir 'Velocis-icon.png') $InputDir

$JavaFiles = Get-ChildItem $StudioDir -Filter '*.java' | ForEach-Object { $_.FullName }
if (-not $JavaFiles) { throw 'No Java source files found in velocis-studio.' }

& javac -encoding UTF-8 -d $ClassesDir $JavaFiles
if ($LASTEXITCODE -ne 0) { throw 'Java compilation failed.' }
Copy-Item (Join-Path $StudioDir 'Velocis-icon.png') $ClassesDir

$JarPath = Join-Path $InputDir 'VelocisStudio.jar'
& jar --create --file $JarPath --main-class VelocisStudio -C $ClassesDir .
if ($LASTEXITCODE -ne 0) { throw 'JAR creation failed.' }

# --app-content copies the compiler folder into the packaged application's app directory.
& jpackage `
    --type exe `
    --name 'Velocis Mini' `
    --icon (Join-Path $StudioDir 'Velocis-icon.ico') `
    --app-version '1.0.0' `
    --vendor 'Velocis Mini' `
    --description 'Velocis Mini Compiler Studio' `
    --input $InputDir `
    --main-jar 'VelocisStudio.jar' `
    --main-class 'VelocisStudio' `
    --dest $DistDir `
    --win-menu `
    --win-shortcut `
    --win-menu-group 'Velocis Mini'

if ($LASTEXITCODE -ne 0) { throw 'jpackage failed. Check that WiX Toolset is installed and available on PATH.' }
Write-Host "Installer created in: $DistDir" -ForegroundColor Green
