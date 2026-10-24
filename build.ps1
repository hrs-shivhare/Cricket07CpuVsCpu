param([string]$Zig, [switch]$Install, [switch]$Upgrade)
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
if (-not $Zig) {
    $Zig = Join-Path $projectRoot '..\..\work\toolchains\zig-x86_64-windows-0.15.2\zig.exe'
}
if (-not (Test-Path -LiteralPath $Zig)) { throw 'Pass -Zig with the path to Zig 0.15.2 zig.exe.' }
$Zig = (Resolve-Path -LiteralPath $Zig).Path
$dist = Join-Path $projectRoot 'dist'
$testDir = Join-Path $projectRoot 'tests\bin'
New-Item -ItemType Directory -Force -Path $dist, $testDir | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $projectRoot '..\..\work\toolchains\zig-cache'
$plugin = Join-Path $dist 'Cricket07CpuVsCpu.asi'
& $Zig cc -target x86-windows-gnu -std=c11 -O2 -Wall -Wextra -Werror -shared `
    (Join-Path $projectRoot 'src\plugin.c') (Join-Path $projectRoot 'src\patch.c') `
    (Join-Path $projectRoot 'src\exports.def') -ladvapi32 -lkernel32 -o $plugin
if ($LASTEXITCODE) { throw 'ASI compilation failed' }
$test = Join-Path $testDir 'native_test.exe'
& $Zig cc -target x86-windows-gnu -std=c11 -O2 -Wall -Wextra -Werror `
    (Join-Path $projectRoot 'tests\native_test.c') (Join-Path $projectRoot 'src\patch.c') `
    -lkernel32 -o $test
if ($LASTEXITCODE) { throw 'Native test compilation failed' }
& $test $plugin
if ($LASTEXITCODE) { throw 'Native tests failed' }
Get-FileHash -LiteralPath $plugin -Algorithm SHA256
if ($Install -or $Upgrade) {
    $game = 'E:\Cricket07'
    if (Get-Process Cricket07 -ErrorAction SilentlyContinue) { throw 'Close Cricket07 before installing.' }
    $expected = 'D776DBA0CC506B968F7D228E036CF42AEB4ABE45A8D1AD0D7226CDEBF91553A2'
    if ((Get-FileHash -LiteralPath (Join-Path $game 'Cricket07.exe') -Algorithm SHA256).Hash -ne $expected) {
        throw 'Unsupported executable; installation stopped.'
    }
    $destination = Join-Path $game 'Cricket07CpuVsCpu.asi'
    if (Test-Path -LiteralPath $destination) {
        if (-not $Upgrade) { throw 'Destination already exists; use -Upgrade for the known version 1.0.' }
        $installedHash = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash
        $newHash = (Get-FileHash -LiteralPath $plugin -Algorithm SHA256).Hash
        if ($installedHash -eq $newHash) {
            Write-Output 'This build is already installed.'
            return
        }
        if ($installedHash -ne '084AE352E9C8227CB08EF7FE4FED0B73D043B39ABEC99902E54B2722E28F99B5') {
            throw 'Installed plugin is not the known version 1.0; replacement stopped.'
        }
        $backupDir = Join-Path $projectRoot '..\..\output\plugins\backups'
        New-Item -ItemType Directory -Force -Path $backupDir | Out-Null
        $backup = Join-Path $backupDir 'Cricket07CpuVsCpu-1.0.asi.bak'
        if (Test-Path -LiteralPath $backup) {
            if ((Get-FileHash -LiteralPath $backup -Algorithm SHA256).Hash -ne $installedHash) {
                throw 'Existing backup differs; replacement stopped.'
            }
        } else {
            Copy-Item -LiteralPath $destination -Destination $backup
        }
        Copy-Item -LiteralPath $plugin -Destination $destination -Force
    } else {
        Copy-Item -LiteralPath $plugin -Destination $destination
    }
    Get-FileHash -LiteralPath $destination -Algorithm SHA256
}
