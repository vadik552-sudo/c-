param(
    [string]$BuildDir = "build/bin",
    [string]$Output = "KaspiKassaAPIv3_bpo.zip"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$bundleDir = Join-Path $repoRoot "bpo_driver"
$dllPath = Join-Path $repoRoot "$BuildDir/KaspiKassaAPIv3_x64.dll"

if (!(Test-Path $dllPath)) {
    throw "DLL not found: $dllPath"
}

$stage = Join-Path $repoRoot ".pack_stage"
if (Test-Path $stage) {
    Remove-Item -Path $stage -Recurse -Force
}
New-Item -ItemType Directory -Path $stage | Out-Null

Copy-Item -Path (Join-Path $bundleDir "INFO.XML") -Destination $stage
Copy-Item -Path (Join-Path $bundleDir "MANIFEST.XML") -Destination $stage
Copy-Item -Path (Join-Path $bundleDir "TableParameters.xml") -Destination $stage
Copy-Item -Path (Join-Path $bundleDir "TableActions.xml") -Destination $stage
Copy-Item -Path $dllPath -Destination (Join-Path $stage "KaspiKassaAPIv3_x64.dll")

$outPath = Join-Path $repoRoot $Output
if (Test-Path $outPath) {
    Remove-Item -Path $outPath -Force
}
Compress-Archive -Path (Join-Path $stage "*") -DestinationPath $outPath -Force

Remove-Item -Path $stage -Recurse -Force
Write-Host "Created $outPath"
