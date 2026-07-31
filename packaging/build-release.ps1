[CmdletBinding()]
param(
    [Parameter()]
    [ValidatePattern('^\d+\.\d+\.\d+$')]
    [string]$Version = '1.1.3',

    [Parameter()]
    [switch]$SkipNativeBuild,

    [Parameter()]
    [switch]$SkipInstaller,

    [Parameter()]
    [string]$IsccPath
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$buildRoot = Join-Path $repoRoot 'build\Release'
$distRoot = Join-Path $repoRoot 'dist'
$stageName = "PresentMon-CN-$Version-Portable-x64"
$stageRoot = Join-Path $distRoot $stageName
$appRoot = Join-Path $stageRoot 'app'
$zipPath = Join-Path $distRoot "$stageName.zip"
$setupPath = Join-Path $distRoot "PresentMon-CN-$Version-Setup-x64.exe"
$versionParts = $Version.Split('.')

function Assert-ChildPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string]$Parent
    )

    $fullPath = [IO.Path]::GetFullPath($Path)
    $fullParent = [IO.Path]::GetFullPath($Parent).TrimEnd('\') + '\'
    if (-not $fullPath.StartsWith($fullParent, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to modify a path outside the expected parent: $fullPath"
    }
}

function Invoke-Checked {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,

        [Parameter()]
        [string[]]$Arguments = @()
    )

    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $FilePath"
    }
}

function Copy-RequiredFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,

        [Parameter(Mandatory = $true)]
        [string]$Destination
    )

    if (-not (Test-Path -LiteralPath $Source -PathType Leaf)) {
        throw "Required release file is missing: $Source"
    }
    Copy-Item -LiteralPath $Source -Destination $Destination -Force
}

function Copy-RequiredDirectory {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,

        [Parameter(Mandatory = $true)]
        [string]$Destination
    )

    if (-not (Test-Path -LiteralPath $Source -PathType Container)) {
        throw "Required release directory is missing: $Source"
    }
    Copy-Item -LiteralPath $Source -Destination $Destination -Recurse -Force
}

if (-not $SkipNativeBuild) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
        throw 'Visual Studio Build Tools were not found.'
    }

    $vsPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if (-not $vsPath) {
        throw 'MSBuild was not found.'
    }
    $msbuild = Join-Path $vsPath 'MSBuild\Current\Bin\MSBuild.exe'

    Invoke-Checked -FilePath $msbuild -Arguments @(
        (Join-Path $repoRoot 'IntelPresentMon\KernelProcess\KernelProcess.vcxproj'),
        '/m',
        '/t:Rebuild',
        '/p:Configuration=Release',
        '/p:Platform=x64',
        '/p:PortableBuild=true',
        "/p:OutDir=$buildRoot\",
        '/v:minimal'
    )

    Invoke-Checked -FilePath $msbuild -Arguments @(
        (Join-Path $repoRoot 'IntelPresentMon\PresentMonAPI2\PresentMonAPI2.vcxproj'),
        '/m',
        '/t:Build',
        '/p:Configuration=Release',
        '/p:Platform=x64',
        "/p:OutDir=$buildRoot\",
        '/v:minimal'
    )

    Invoke-Checked -FilePath $msbuild -Arguments @(
        (Join-Path $repoRoot 'packaging\launcher\PresentMonLauncher.vcxproj'),
        '/m',
        '/t:Rebuild',
        '/p:Configuration=Release',
        '/p:Platform=x64',
        "/p:AppVersionMajor=$($versionParts[0])",
        "/p:AppVersionMinor=$($versionParts[1])",
        "/p:AppVersionPatch=$($versionParts[2])",
        "/p:OutDir=$buildRoot\",
        '/v:minimal'
    )
}

New-Item -ItemType Directory -Path $distRoot -Force | Out-Null
Assert-ChildPath -Path $stageRoot -Parent $distRoot
Assert-ChildPath -Path $zipPath -Parent $distRoot
Assert-ChildPath -Path $setupPath -Parent $distRoot

if (Test-Path -LiteralPath $stageRoot) {
    Remove-Item -LiteralPath $stageRoot -Recurse -Force
}
if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}
if (Test-Path -LiteralPath $setupPath) {
    Remove-Item -LiteralPath $setupPath -Force
}

New-Item -ItemType Directory -Path $appRoot -Force | Out-Null

$runtimeFiles = @(
    'PresentMon.exe',
    'PresentMonUI.exe',
    'PresentMonService.exe',
    'PresentMonAPI2.dll',
    'PresentMonAPI2Loader.dll',
    'ddETWExternal.xml',
    'bootstrap.exe',
    'bootstrapc.exe',
    'chrome_elf.dll',
    'd3dcompiler_47.dll',
    'dxcompiler.dll',
    'dxil.dll',
    'libcef.dll',
    'libEGL.dll',
    'libGLESv2.dll',
    'v8_context_snapshot.bin',
    'vk_swiftshader.dll',
    'vk_swiftshader_icd.json',
    'vulkan-1.dll',
    'chrome_100_percent.pak',
    'chrome_200_percent.pak',
    'icudtl.dat',
    'resources.pak'
)

foreach ($file in $runtimeFiles) {
    Copy-RequiredFile -Source (Join-Path $buildRoot $file) -Destination (Join-Path $appRoot $file)
}

$runtimeDirectories = @(
    'BlockLists',
    'Presets',
    'Shaders',
    'ipm-ui-vue',
    'locales'
)

foreach ($directory in $runtimeDirectories) {
    Copy-RequiredDirectory -Source (Join-Path $buildRoot $directory) -Destination (Join-Path $appRoot $directory)
}

$blockListSource = Join-Path $buildRoot 'BlockLists\TargetBlockList.txt'
$installedBlockList = Join-Path $appRoot 'TargetBlockList.txt'
$portableBlockList = Join-Path $appRoot 'BlockLists\TargetBlockList.txt'
Copy-RequiredFile -Source $blockListSource -Destination $installedBlockList

if ((Get-FileHash -LiteralPath $installedBlockList -Algorithm SHA256).Hash -ne
    (Get-FileHash -LiteralPath $portableBlockList -Algorithm SHA256).Hash) {
    throw 'Installed and portable target block lists do not match.'
}

Copy-RequiredFile -Source (Join-Path $buildRoot 'PresentMon-CN.exe') -Destination (Join-Path $stageRoot 'PresentMon-CN.exe')
Copy-RequiredFile -Source (Join-Path $repoRoot 'LICENSE.txt') -Destination (Join-Path $stageRoot 'LICENSE.txt')
Copy-RequiredFile -Source (Join-Path $repoRoot 'THIRD_PARTY.txt') -Destination (Join-Path $stageRoot 'THIRD_PARTY.txt')
Copy-RequiredFile -Source (Join-Path $PSScriptRoot 'README.zh-CN.txt') -Destination (Join-Path $stageRoot 'README.zh-CN.txt')
New-Item -ItemType File -Path (Join-Path $stageRoot 'portable.mode') -Force | Out-Null

$forbiddenFiles = Get-ChildItem -LiteralPath $stageRoot -Recurse -File | Where-Object {
    $_.Extension -in @('.pdb', '.lib', '.exp', '.idb', '.csv', '.etl', '.dmp') -or
    $_.Name -like '*Tests*' -or
    $_.Name -like 'SampleClient*' -or
    $_.Name -like 'pmui-init-log-*' -or
    $_.Name -eq 'preferences.json'
}
if ($forbiddenFiles) {
    throw "Forbidden files were found in the release payload: $($forbiddenFiles.FullName -join ', ')"
}

$forbiddenDirectories = Get-ChildItem -LiteralPath $stageRoot -Recurse -Directory | Where-Object {
    $_.Name -in @('cef-cache', 'logs', 'Captures', 'Etl', 'Loadouts')
}
if ($forbiddenDirectories) {
    throw "Forbidden directories were found in the release payload: $($forbiddenDirectories.FullName -join ', ')"
}

Compress-Archive -LiteralPath $stageRoot -DestinationPath $zipPath -CompressionLevel Optimal

if (-not $SkipInstaller) {
    if (-not $IsccPath) {
        $isccCandidates = @(
            (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
            (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe'),
            (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe')
        )
        $IsccPath = $isccCandidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
    }
    if (-not $IsccPath) {
        throw 'Inno Setup 6 was not found. Install it or use -SkipInstaller.'
    }

    Invoke-Checked -FilePath $IsccPath -Arguments @(
        "/DSourceDir=$stageRoot",
        "/DOutputDir=$distRoot",
        "/DAppVersion=$Version",
        (Join-Path $PSScriptRoot 'installer\PresentMonCN.iss')
    )
    if (-not (Test-Path -LiteralPath $setupPath -PathType Leaf)) {
        throw "The installer was not created at the expected path: $setupPath"
    }
}

$artifacts = @($zipPath)
if (Test-Path -LiteralPath $setupPath -PathType Leaf) {
    $artifacts += $setupPath
}

$checksumLines = foreach ($artifact in $artifacts) {
    $hash = Get-FileHash -LiteralPath $artifact -Algorithm SHA256
    "$($hash.Hash.ToLowerInvariant())  $([IO.Path]::GetFileName($artifact))"
}
$checksumPath = Join-Path $distRoot 'SHA256SUMS.txt'
$checksumLines | Set-Content -LiteralPath $checksumPath -Encoding ascii

$artifacts += $checksumPath
$artifacts | ForEach-Object {
    $item = Get-Item -LiteralPath $_
    [pscustomobject]@{
        File = $item.FullName
        SizeMiB = [Math]::Round($item.Length / 1MB, 2)
    }
} | Format-Table -AutoSize
