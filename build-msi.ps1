param(
    [string]$WixBin = ""
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$wxs = Join-Path $root "installer\AstraInstall.wxs"
$objDir = Join-Path $root "dist-msi\obj"
$outDir = Join-Path $root "dist-msi"
$msiFile = Join-Path $outDir "AstraInstall.msi"

if (-not (Test-Path $wxs)) {
    Write-Host "Arquivo nao encontrado: $wxs" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path (Join-Path $root "astra.exe"))) {
    Write-Host "Nao encontrei astra.exe na raiz do projeto." -ForegroundColor Red
    Write-Host "Compile antes de gerar o MSI." -ForegroundColor Yellow
    exit 1
}

if ([string]::IsNullOrWhiteSpace($WixBin)) {
    $candidates = @(
        "${env:ProgramFiles(x86)}\WiX Toolset v3.11\bin",
        "${env:ProgramFiles}\WiX Toolset v3.11\bin"
    )
    foreach ($candidate in $candidates) {
        if (Test-Path (Join-Path $candidate "candle.exe")) {
            $WixBin = $candidate
            break
        }
    }
}

if ([string]::IsNullOrWhiteSpace($WixBin)) {
    Write-Host "WiX Toolset v3 nao encontrado." -ForegroundColor Red
    Write-Host "Instale o WiX v3 e tente novamente." -ForegroundColor Yellow
    Write-Host "Download: https://wixtoolset.org/releases/" -ForegroundColor Yellow
    exit 1
}

$candle = Join-Path $WixBin "candle.exe"
$light = Join-Path $WixBin "light.exe"

if (-not (Test-Path $candle) -or -not (Test-Path $light)) {
    Write-Host "WiX Toolset v3 nao encontrado." -ForegroundColor Red
    Write-Host "Instale o WiX v3 e tente novamente." -ForegroundColor Yellow
    Write-Host "Download: https://wixtoolset.org/releases/" -ForegroundColor Yellow
    exit 1
}

New-Item -ItemType Directory -Force -Path $objDir | Out-Null
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$wixobj = Join-Path $objDir "AstraInstall.wixobj"

Write-Host "Compilando WXS..." -ForegroundColor Cyan
& $candle -nologo -ext WixUIExtension -out $wixobj $wxs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Gerando MSI..." -ForegroundColor Cyan
& $light -nologo -ext WixUIExtension -out $msiFile $wixobj
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "MSI gerado com sucesso:" -ForegroundColor Green
Write-Host "  $msiFile"
