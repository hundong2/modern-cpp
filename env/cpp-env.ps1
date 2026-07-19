param(
  [ValidateSet('help','init','configure','build','test','clean')] [string]$Action = 'help',
  [string]$Project = '.',
  [ValidateSet('17','20','23')] [string]$Standard = '23',
  [ValidateSet('Debug','Release','RelWithDebInfo','MinSizeRel')] [string]$Config = 'Debug'
)
$ErrorActionPreference = 'Stop'

if ($Action -eq 'help') {
  @'
cpp-env.ps1 - CMake 기반 C++17/20/23 공통 빌드 도구

사용법:
  .\env\cpp-env.ps1 <command> [project] [standard] [config]
  .\env\cpp-env.ps1 help

명령:
  help        이 도움말을 표시합니다.
  init        프로젝트에 CMake 프리셋과 VS Code 설정을 추가합니다.
  configure   CMake configure를 실행합니다.
  build       configure 후 프로젝트를 병렬 빌드합니다.
  test        configure/build 후 CTest를 실행합니다.
  clean       선택한 빌드 디렉터리만 삭제합니다.

위치 인자:
  project     CMakeLists.txt가 있는 프로젝트 경로 (기본값: .)
  standard    17 | 20 | 23                     (기본값: 23)
  config      Debug | Release | RelWithDebInfo | MinSizeRel
                                                (기본값: Debug)

예제:
  .\env\cpp-env.ps1 init .
  .\env\cpp-env.ps1 build . 23 Debug
  .\env\cpp-env.ps1 test C:\work\my-project 20 Release
  .\env\cpp-env.ps1 clean . 23 Debug

추가 옵션:
  $env:CC='clang'; $env:CXX='clang++'
  $env:CPP_ENV_CMAKE_ARGS='-DBUILD_TESTING=ON -DCPP_ENV_SANITIZERS=ON'
  .\env\cpp-env.ps1 build . 23 Debug

더 자세한 설명: env\README.md
'@ | Write-Host
  exit 0
}

$Project = (Resolve-Path $Project).Path
$BuildDir = Join-Path $Project "build/cpp$Standard-$Config"
$EnvDir = $PSScriptRoot

if ($Action -eq 'init') {
  if (-not (Test-Path (Join-Path $Project 'CMakeLists.txt'))) { throw "CMakeLists.txt not found: $Project" }
  New-Item -ItemType Directory -Force (Join-Path $Project '.vscode') | Out-Null
  @('CMakeUserPresets.json','.vscode/tasks.json','.vscode/launch.json','.vscode/settings.json') | ForEach-Object {
    $destination = Join-Path $Project $_
    if (Test-Path $destination) { Write-Host "kept existing $_" }
    else {
      $template = Join-Path $EnvDir "templates/$_"
      (Get-Content $template -Raw).Replace('@ENV_DIR@', $EnvDir.Replace('\','/')) | Set-Content $destination -Encoding utf8
      Write-Host "created $_"
    }
  }
  exit 0
}

if ($Action -eq 'clean') { if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }; exit 0 }
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) { throw 'cmake is required; run setup-windows.ps1' }
$cmakeArgs = @('-S',$Project,'-B',$BuildDir,'-G','Ninja',"-DCMAKE_BUILD_TYPE=$Config","-DCMAKE_CXX_STANDARD=$Standard",'-DCMAKE_CXX_STANDARD_REQUIRED=ON','-DCMAKE_CXX_EXTENSIONS=OFF','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
if ($env:CPP_ENV_CMAKE_ARGS) { $cmakeArgs += $env:CPP_ENV_CMAKE_ARGS -split '\s+' }
& cmake @cmakeArgs
if ($LASTEXITCODE) { exit $LASTEXITCODE }
if ($Action -in @('build','test')) { & cmake --build $BuildDir --config $Config --parallel; if ($LASTEXITCODE) { exit $LASTEXITCODE } }
if ($Action -eq 'test') { & ctest --test-dir $BuildDir -C $Config --output-on-failure; exit $LASTEXITCODE }
