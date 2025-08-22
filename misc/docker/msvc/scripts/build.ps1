param(
  [string]$SrcDir = $env:SRC_DIR,
  [string]$OutDir = $env:OUT_DIR
)

if ([string]::IsNullOrEmpty($SrcDir)) { $SrcDir = "C:\src" }
if ([string]::IsNullOrEmpty($OutDir)) { $OutDir = "C:\out" }

Write-Host "==> Source: $SrcDir"
Write-Host "==> Output: $OutDir"

if (!(Test-Path $SrcDir)) { throw "SRC_DIR inexistente: $SrcDir" }
if (!(Test-Path $OutDir)) { New-Item -ItemType Directory -Path $OutDir | Out-Null }

# Caminho temporário de build
$BuildDir = "C:\_build"
if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
New-Item -ItemType Directory -Path $BuildDir | Out-Null

# Se existir CMakeLists.txt, usar CMake+Ninja
if (Test-Path (Join-Path $SrcDir "CMakeLists.txt")) {
    Write-Host "==> Detectado CMakeLists.txt — usando CMake+Ninja (Release)"
    cd $BuildDir
    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release $SrcDir
    if ($LASTEXITCODE -ne 0) { throw "cmake configure falhou" }

    cmake --build . --config Release -- -v
    if ($LASTEXITCODE -ne 0) { throw "cmake build falhou" }

    # Copiar executáveis e DLLs/Libs resultantes
    Get-ChildItem -Recurse -Path $BuildDir -Include *.exe,*.dll,*.lib |
      Copy-Item -Destination $OutDir -Force
    exit 0
}

# Fallback: compilar todos .c em um único app.exe simples
Write-Host "==> Sem CMake; compilando todos os .c em app.exe (Release, /O2 /MD)"
$allC = Get-ChildItem -Path $SrcDir -Recurse -Include *.c
if ($allC.Count -eq 0) { throw "Nenhum .c encontrado" }

# Montar lista de includes: diretórios contendo headers
$incDirs = (Get-ChildItem -Path $SrcDir -Recurse -Directory | Select-Object -Expand FullName) + $SrcDir
$incArgs = ($incDirs | ForEach-Object { '/I"' + $_ + '"' }) -join ' '

# Montar lista de fontes
$srcArgs = ($allC | ForEach-Object { '"' + $_.FullName + '"' }) -join ' '

# Compilar e linkar
$exe = Join-Path $OutDir "app.exe"
$cmd = "cl /nologo /O2 /MD $incArgs $srcArgs /Fe:`"$exe`""
Write-Host "==> $cmd"
cmd /c $cmd
if ($LASTEXITCODE -ne 0) { throw "compilação falhou" }

Write-Host "==> Artefato: $exe"
