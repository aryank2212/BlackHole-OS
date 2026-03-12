# ============================================================================
# BlackHole OS — PowerShell Build Script (Windows)
# ============================================================================
# Usage:
#   .\build.ps1              — build os-image.bin
#   .\build.ps1 run          — build and run in QEMU
#   .\build.ps1 clean        — remove build artifacts
#   .\build.ps1 install-deps — install NASM, MinGW (GCC), QEMU via Chocolatey
# ============================================================================

param(
    [Parameter(Position=0)]
    [ValidateSet("build", "run", "clean", "install-deps")]
    [string]$Action = "build"
)

$ErrorActionPreference = "Stop"

# --- Paths ---
$ProjectRoot = Split-Path -Parent $PSScriptRoot
if ($PSScriptRoot -eq $null -or $PSScriptRoot -eq "") {
    $ProjectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
}
$BootDir     = Join-Path $ProjectRoot "boot"
$KernelDir   = Join-Path $ProjectRoot "kernel"
$BuildDir    = Join-Path $ProjectRoot "build"

# --- Directories ---
$DriverDir   = Join-Path $ProjectRoot "drivers"
$ShellDir    = Join-Path $ProjectRoot "shell"
$LibcDir     = Join-Path $ProjectRoot "libc"

# --- Output files ---
$BootBin      = Join-Path $BuildDir "boot.bin"
$KEntryObj    = Join-Path $BuildDir "kernel_entry.o"
$IsrObj       = Join-Path $BuildDir "isr.o"
$GdtFlushObj  = Join-Path $BuildDir "gdt_flush.o"
$UserModeObj  = Join-Path $BuildDir "user_mode.o"
$SwitchObj    = Join-Path $BuildDir "switch.o"
$KernelObj    = Join-Path $BuildDir "kernel.o"
$TaskObj      = Join-Path $BuildDir "task.o"
$IdtObj       = Join-Path $BuildDir "idt.o"
$IsrCObj      = Join-Path $BuildDir "isr_c.o"
$GdtCObj      = Join-Path $BuildDir "gdt.o"
$PagingObj    = Join-Path $BuildDir "paging.o"
$VgaObj       = Join-Path $BuildDir "vga.o"
$KeyboardObj  = Join-Path $BuildDir "keyboard.o"
$TimerObj     = Join-Path $BuildDir "timer.o"
$AtaObj       = Join-Path $BuildDir "ata.o"
$MemoryObj    = Join-Path $BuildDir "memory.o"
$ShellObj     = Join-Path $BuildDir "shell.o"
$StringObj    = Join-Path $BuildDir "string.o"
$StdioObj     = Join-Path $BuildDir "stdio.o"
$KernelPe     = Join-Path $BuildDir "kernel.pe"
$KernelBin    = Join-Path $BuildDir "kernel.bin"
$OsImage      = Join-Path $BuildDir "os-image.bin"
$LinkerScript = Join-Path $BuildDir "linker.ld"

# --- Auto-detect tool paths (add known install locations to PATH) ---
$extraPaths = @(
    "C:\Program Files\NASM",
    "C:\ProgramData\mingw64\mingw64\bin",
    "C:\tools\mingw64\bin",
    "C:\mingw64\bin",
    "C:\MinGW\bin",
    "C:\Program Files\qemu",
    "C:\Program Files (x86)\qemu"
)
foreach ($p in $extraPaths) {
    if ((Test-Path $p) -and ($env:PATH -notlike "*$p*")) {
        $env:PATH = "$p;$env:PATH"
    }
}

# --- Tool detection ---
function Find-Tool($name) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    return $null
}

function Assert-Tool($name) {
    $path = Find-Tool $name
    if (-not $path) {
        Write-Host "ERROR: '$name' not found on PATH. Run '.\build.ps1 install-deps' first." -ForegroundColor Red
        exit 1
    }
    return $path
}

# ============================================================================
# Actions
# ============================================================================

function Install-Deps {
    Write-Host "Installing OS development tools via Chocolatey..." -ForegroundColor Cyan
    Write-Host "This requires an ELEVATED (Admin) PowerShell." -ForegroundColor Yellow

    $packages = @("nasm", "mingw", "qemu")
    foreach ($pkg in $packages) {
        Write-Host "  -> Installing $pkg..." -ForegroundColor Gray
        choco install $pkg -y --no-progress
    }

    Write-Host ""
    Write-Host "Done! Please restart your terminal so PATH changes take effect." -ForegroundColor Green
    Write-Host "Then run: .\build.ps1 run" -ForegroundColor Green
}

function Clean-Build {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    $artifacts = @($BootBin, $KEntryObj, $IsrObj, $GdtFlushObj, $UserModeObj, $KernelObj, $IdtObj, $GdtCObj, $IsrCObj, $PagingObj, $VgaObj, $KeyboardObj, $TimerObj, $AtaObj, $MemoryObj, $ShellObj, $StringObj, $StdioObj, $KernelPe, $KernelBin, $OsImage)
    foreach ($f in $artifacts) {
        if (Test-Path $f) { Remove-Item $f -Force }
    }
    Write-Host "Clean." -ForegroundColor Green
}

function Build-OS {
    $nasm    = Assert-Tool "nasm"
    $gcc     = Assert-Tool "gcc"
    $ld      = Assert-Tool "ld"
    $objcopy = Assert-Tool "objcopy"

    Write-Host "=== BlackHole OS Build ===" -ForegroundColor Cyan

    # 1. Assemble bootloader
    Write-Host "[1/5] Assembling bootloader..." -ForegroundColor Gray
    & $nasm -f bin (Join-Path $BootDir "boot.asm") -o $BootBin
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: bootloader assembly" -ForegroundColor Red; exit 1 }

    # 2. Assemble kernel entry (elf32 format for compatibility)
    Write-Host "[2/5] Assembling kernel entry..." -ForegroundColor Gray
    & $nasm -f win32 (Join-Path $KernelDir "kernel_entry.asm") -o $KEntryObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: kernel entry assembly" -ForegroundColor Red; exit 1 }

    # 3. Assemble ISR stubs (using GCC/GAS for consistent symbol mangling)
    Write-Host "[3/15] Assembling ISR stubs..." -ForegroundColor Gray
    & $gcc -m32 -c (Join-Path $KernelDir "isr_stub.S") -o $IsrObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: ISR assembly" -ForegroundColor Red; exit 1 }

    & $nasm -f win32 (Join-Path $KernelDir "gdt_flush.S") -o $GdtFlushObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: gdt flush assembly" -ForegroundColor Red; exit 1 }

    & $nasm -f win32 (Join-Path $KernelDir "user_mode.S") -o $UserModeObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: user mode assembly" -ForegroundColor Red; exit 1 }

    & $nasm -f win32 (Join-Path $KernelDir "switch.S") -o $SwitchObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: switch assembly" -ForegroundColor Red; exit 1 }

    # 4. Compile kernel C code
    Write-Host "[4/9] Compiling kernel..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $KernelDir "kernel.c") -o $KernelObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: kernel compilation" -ForegroundColor Red; exit 1 }

    # 5. Compile IDT
    Write-Host "[5/15] Compiling IDT..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $KernelDir "idt.c") -o $IdtObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: IDT compilation" -ForegroundColor Red; exit 1 }

    Write-Host "[5b/15] Compiling GDT..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $KernelDir "gdt.c") -o $GdtCObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: GDT compilation" -ForegroundColor Red; exit 1 }

    # 6. Compile ISR C handler
    Write-Host "[6/10] Compiling ISR handler..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $KernelDir "isr.c") -o $IsrCObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: ISR C compilation" -ForegroundColor Red; exit 1 }

    # 6b. Compile Paging module
    Write-Host "[6b/10] Compiling paging module..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $KernelDir "paging.c") -o $PagingObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: Paging compilation" -ForegroundColor Red; exit 1 }

    # 6c. Compile Scheduler module
    Write-Host "[6c/10] Compiling scheduler module..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $KernelDir "task.c") -o $TaskObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: Scheduler compilation" -ForegroundColor Red; exit 1 }

    # 7. Compile VGA driver
    Write-Host "[7/9] Compiling VGA driver..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $DriverDir "vga.c") -o $VgaObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: VGA driver compilation" -ForegroundColor Red; exit 1 }

    # 7b. Compile Keyboard driver
    Write-Host "[7b/14] Compiling keyboard driver..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $DriverDir "keyboard.c") -o $KeyboardObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: keyboard driver compilation" -ForegroundColor Red; exit 1 }

    # 7c. Compile Timer driver
    Write-Host "[7c/14] Compiling timer driver..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $DriverDir "timer.c") -o $TimerObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: timer driver compilation" -ForegroundColor Red; exit 1 }

    # 7d. Compile ATA driver
    Write-Host "[7d/15] Compiling ATA driver..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $DriverDir "ata.c") -o $AtaObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: ATA driver compilation" -ForegroundColor Red; exit 1 }

    # 7e. Compile Memory Allocator
    Write-Host "[7d/14] Compiling memory allocator..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $KernelDir "memory.c") -o $MemoryObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: memory allocator compilation" -ForegroundColor Red; exit 1 }

    # 7d. Compile Shell
    Write-Host "[7d/11] Compiling shell..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $ShellDir "shell.c") -o $ShellObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: shell compilation" -ForegroundColor Red; exit 1 }

    # 7e. Compile libc (string, stdio)
    Write-Host "[7e/13] Compiling libc string..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $LibcDir "string.c") -o $StringObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: string compilation" -ForegroundColor Red; exit 1 }

    Write-Host "[7f/13] Compiling libc stdio..." -ForegroundColor Gray
    & $gcc -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin `
           -fno-stack-protector -nostartfiles -nodefaultlibs `
           -Wall -Wextra -c (Join-Path $LibcDir "stdio.c") -o $StdioObj
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: stdio compilation" -ForegroundColor Red; exit 1 }

    # 8. Link kernel as PE
    Write-Host "[8/15] Linking kernel..." -ForegroundColor Gray
    & $ld -m i386pe -T $LinkerScript -o $KernelPe $KEntryObj $IsrObj $KernelObj $TaskObj $IdtObj $GdtCObj $GdtFlushObj $IsrCObj $PagingObj $VgaObj $KeyboardObj $TimerObj $AtaObj $MemoryObj $UserModeObj $SwitchObj $ShellObj $StringObj $StdioObj 2> (Join-Path $BuildDir "ld_error.log")
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: kernel linking (check ld_error.log)" -ForegroundColor Red; exit 1 }

    # 9. Convert PE to flat binary
    Write-Host "[9/9] Converting to flat binary..." -ForegroundColor Gray
    & $objcopy -O binary $KernelPe $KernelBin
    if ($LASTEXITCODE -ne 0) { Write-Host "FAILED: objcopy" -ForegroundColor Red; exit 1 }

    # 6. Create OS image (concatenate boot + kernel)
    Write-Host "Creating OS image..." -ForegroundColor Gray
    $bootBytes   = [System.IO.File]::ReadAllBytes($BootBin)
    $kernelBytes = [System.IO.File]::ReadAllBytes($KernelBin)
    $imageBytes  = $bootBytes + $kernelBytes
    [System.IO.File]::WriteAllBytes($OsImage, $imageBytes)

    $sizeKB = [math]::Round($imageBytes.Length / 1024, 1)
    Write-Host "=== Build successful: os-image.bin ($sizeKB KB) ===" -ForegroundColor Green
}

function Run-OS {
    Build-OS

    $qemu = Find-Tool "qemu-system-i386"
    if (-not $qemu) {
        $qemu = Find-Tool "qemu-system-x86_64"
    }
    if (-not $qemu) {
        Write-Host "ERROR: QEMU not found. Install it or add to PATH." -ForegroundColor Red
        exit 1
    }

    Write-Host "Creating HDD Image if not exists..." -ForegroundColor Gray
    $hddImage = Join-Path $BuildDir "hdd.img"
    if (-not (Test-Path $hddImage)) {
        & fsutil file createnew $hddImage 1048576 | Out-Null
    }

    Write-Host "Launching QEMU..." -ForegroundColor Cyan
    & $qemu -fda $OsImage -hda $hddImage
}

# ============================================================================
# Dispatch
# ============================================================================

switch ($Action) {
    "build"        { Build-OS }
    "run"          { Run-OS }
    "clean"        { Clean-Build }
    "install-deps" { Install-Deps }
}
