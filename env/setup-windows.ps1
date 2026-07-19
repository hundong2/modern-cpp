$ErrorActionPreference = 'Stop'
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
  throw 'winget is required. Install App Installer from Microsoft Store.'
}
winget install --id Kitware.CMake --exact --accept-package-agreements --accept-source-agreements
winget install --id Ninja-build.Ninja --exact --accept-package-agreements --accept-source-agreements
winget install --id Microsoft.VisualStudio.2022.BuildTools --exact --accept-package-agreements --accept-source-agreements --override '--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'
Write-Host 'Open a new Developer PowerShell before building.'

