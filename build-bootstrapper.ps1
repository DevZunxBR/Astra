param(
    [string]$Version = "0.2.0.0"
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$msiPath = Join-Path $root "dist-msi\AstraInstall.msi"
$bundleWxs = Join-Path $root "installer\AstraBundle.wxs"
$outDir = Join-Path $root "dist-msi"
$bundleExe = Join-Path $outDir "AstraSetup.exe"

if (-not (Test-Path $msiPath)) {
    Write-Host "MSI nao encontrado em $msiPath" -ForegroundColor Red
    Write-Host "Execute primeiro: .\build-msi.ps1" -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $bundleWxs)) {
    Write-Host "Arquivo nao encontrado: $bundleWxs" -ForegroundColor Red
    exit 1
}

$wixCmd = Get-Command wix -ErrorAction SilentlyContinue
if (-not $wixCmd) {
    Write-Host "WiX v4 (comando 'wix') nao encontrado no PATH." -ForegroundColor Red
    Write-Host "Instale o WiX v4 e tente novamente." -ForegroundColor Yellow
    Write-Host "Download: https://wixtoolset.org/" -ForegroundColor Yellow
    exit 1
}

New-Item -ItemType Directory -Force -Path $outDir | Out-Null

Write-Host "Gerando bootstrapper com WiX v4..." -ForegroundColor Cyan
& wix build $bundleWxs `
  -ext WixToolset.Bal.wixext `
  -d BundleVersion=$Version `
  -o $bundleExe

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Bootstrapper gerado com sucesso:" -ForegroundColor Green
Write-Host "  $bundleExe"
