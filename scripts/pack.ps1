param(
    [string]$BuildDir = "build/bin/Release",
    [string]$OutputZip = "KaspiKassaAPIv3_BPO.zip",
    [switch]$CheckExports
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$bpoDir = Join-Path $repoRoot "bpo_driver"
$dllName = "KaspiKassaAPIv3_x64.dll"
$buildDll = Join-Path $repoRoot (Join-Path $BuildDir $dllName)
$bpoDll = Join-Path $bpoDir $dllName

if (-not (Test-Path $bpoDir)) {
    New-Item -ItemType Directory -Path $bpoDir | Out-Null
}

if (Test-Path $buildDll) {
    Copy-Item -Path $buildDll -Destination $bpoDll -Force
} elseif (-not (Test-Path $bpoDll)) {
    throw "DLL not found in build output or bpo_driver: $dllName"
}

if ($CheckExports) {
    $dumpbin = Get-Command dumpbin.exe -ErrorAction SilentlyContinue
    if ($null -eq $dumpbin) {
        Write-Warning "dumpbin.exe not found; export check skipped"
    } else {
        $exportsText = & $dumpbin.Source /EXPORTS $bpoDll | Out-String
        $required = @("GetClassObject", "DestroyObject", "GetClassNames", "SetGlobalMemoryManager", "SetMemManager")
        foreach ($name in $required) {
            if ($exportsText -notmatch [regex]::Escape($name)) {
                throw "Required export missing: $name"
            }
        }
    }
}

$requiredFiles = @("INFO.XML", "MANIFEST.XML", "TableParameters.xml", "TableActions.xml")
foreach ($f in $requiredFiles) {
    $p = Join-Path $bpoDir $f
    if (-not (Test-Path $p)) {
        throw "Missing required file in bpo_driver: $f"
    }
}

$stageDir = Join-Path $repoRoot ".pack_stage"
if (Test-Path $stageDir) {
    Remove-Item -Path $stageDir -Recurse -Force
}
New-Item -ItemType Directory -Path $stageDir | Out-Null

Copy-Item -Path (Join-Path $bpoDir "INFO.XML") -Destination $stageDir -Force
Copy-Item -Path (Join-Path $bpoDir "MANIFEST.XML") -Destination $stageDir -Force
Copy-Item -Path (Join-Path $bpoDir "TableParameters.xml") -Destination $stageDir -Force
Copy-Item -Path (Join-Path $bpoDir "TableActions.xml") -Destination $stageDir -Force
Copy-Item -Path $bpoDll -Destination (Join-Path $stageDir $dllName) -Force

$outPath = Join-Path $repoRoot $OutputZip
if (Test-Path $outPath) {
    Remove-Item -Path $outPath -Force
}

Compress-Archive -Path (Join-Path $stageDir "*") -DestinationPath $outPath -Force

Remove-Item -Path $stageDir -Recurse -Force
Write-Host "BPO package created: $outPath"
