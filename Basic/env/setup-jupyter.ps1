$ErrorActionPreference = "Stop"
$envDir = $PSScriptRoot
$notebookPath = if ($args.Count -gt 0) { $args[0] } else { Join-Path $envDir "..\01-start\lesson.ipynb" }
$env:UV_CACHE_DIR = Join-Path $envDir ".uv-cache"
$env:JUPYTER_RUNTIME_DIR = Join-Path $envDir ".jupyter\runtime"
$env:IPYTHONDIR = Join-Path $envDir ".jupyter\ipython"
New-Item -ItemType Directory -Force $env:JUPYTER_RUNTIME_DIR, $env:IPYTHONDIR | Out-Null

$uvCommand = Get-Command uv -ErrorAction SilentlyContinue
if ($uvCommand) {
    $uv = $uvCommand.Source
} else {
    $uv = Join-Path $envDir ".tools\uv.exe"
    if (-not (Test-Path $uv)) {
        Write-Host "uv가 없어 프로젝트 내부에 설치합니다."
        New-Item -ItemType Directory -Force (Join-Path $envDir ".tools") | Out-Null
        $env:UV_INSTALL_DIR = Join-Path $envDir ".tools"
        $env:UV_NO_MODIFY_PATH = "1"
        Invoke-RestMethod https://astral.sh/uv/install.ps1 | Invoke-Expression
    }
}

& $uv sync --project $envDir
$python = Join-Path $envDir ".venv\Scripts\python.exe"
$jupyter = Join-Path $envDir ".venv\Scripts\jupyter.exe"
& $python (Join-Path $envDir "install_kernel.py")
$env:JUPYTER_PATH = Join-Path $envDir ".jupyter"
& $jupyter kernelspec install (Join-Path $envDir ".jupyter\kernels\cpp23") --user --replace
& $jupyter nbconvert --to notebook --execute $notebookPath `
    --output (Join-Path $envDir ".jupyter\verification.ipynb") `
    --ExecutePreprocessor.timeout=60 | Out-Null

Write-Host "설정 및 C++23 검증 완료"
Write-Host "실행: .\start-jupyter.ps1"
