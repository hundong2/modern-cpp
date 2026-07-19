$ErrorActionPreference = "Stop"
$envDir = $PSScriptRoot
$basicDir = (Resolve-Path (Join-Path $envDir "..")).Path
$notebookPath = if ($args.Count -gt 0) { $args[0] } else { Join-Path $basicDir "01-start\lesson.ipynb" }
$extraArgs = if ($args.Count -gt 1) { $args[1..($args.Count - 1)] } else { @() }
$env:JUPYTER_PATH = Join-Path $envDir ".jupyter"
$env:JUPYTER_RUNTIME_DIR = Join-Path $envDir ".jupyter\runtime"
$env:IPYTHONDIR = Join-Path $envDir ".jupyter\ipython"
New-Item -ItemType Directory -Force $env:JUPYTER_RUNTIME_DIR, $env:IPYTHONDIR | Out-Null

& (Join-Path $envDir ".venv\Scripts\jupyter.exe") lab $notebookPath `
    --ServerApp.root_dir=$basicDir @extraArgs
