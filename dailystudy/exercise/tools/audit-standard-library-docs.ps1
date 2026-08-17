param(
    # 기본값 all은 지금까지 만든 모든 날짜 폴더를 검사한다.
    [ValidateSet('all', 'latest')]
    [string]$Scope = 'all'
)

$ErrorActionPreference = 'Stop'

# 이 스크립트는 exercise/tools에 있으므로 부모가 날짜 폴더와 문서 폴더를 가진 exercise 루트다.
$exerciseRoot = Split-Path -Parent $PSScriptRoot
$docsRoot = Join-Path $exerciseRoot 'standard-library'

if (-not (Test-Path -LiteralPath $docsRoot)) {
    throw "Standard-library documentation directory not found: $docsRoot"
}

# YYYY-MM-DD 형식 디렉터리만 오래된 순서로 수집한다.
$dateDirectories = @(
    Get-ChildItem -LiteralPath $exerciseRoot -Directory |
        Where-Object { $_.Name -match '^\d{4}-\d{2}-\d{2}$' } |
        Sort-Object Name
)

if ($dateDirectories.Count -eq 0) {
    throw 'No dated exercise directories found.'
}

if ($Scope -eq 'latest') {
    # 배열 슬라이스로 마지막 날짜 폴더 하나만 남긴다.
    $dateDirectories = @($dateDirectories[-1])
}

# 빌드 산출물을 제외하고 날짜 폴더의 실제 C++ 학습 소스만 모은다.
$cppFiles = @(
    foreach ($directory in $dateDirectories) {
        Get-ChildItem -LiteralPath $directory.FullName -Filter '*.cpp' -File
    }
)

# 공용 문서를 하나의 검색 문자열로 합쳐 정확한 심볼과 헤더 이름이 한 번 이상 설명됐는지 확인한다.
$documentation = (
    Get-ChildItem -LiteralPath $docsRoot -Filter '*.md' -File |
        Sort-Object Name |
        ForEach-Object { Get-Content -LiteralPath $_.FullName -Encoding UTF8 -Raw }
) -join "`n"

$symbols = [Collections.Generic.HashSet[string]]::new()
$headers = [Collections.Generic.HashSet[string]]::new()
$observedMembers = [Collections.Generic.HashSet[string]]::new()

# std::가 이름에 드러나지 않는 표준 멤버 중 현재 학습 코드에서 자주 쓰는 항목만 검사한다.
$knownStandardMembers = [Collections.Generic.HashSet[string]]::new(
    [string[]]@(
        'append', 'at', 'back', 'begin', 'c_str', 'clear', 'data', 'empty', 'end',
        'error', 'expired', 'fetch_add', 'file_size', 'find', 'front', 'get',
        'has_value', 'is_absolute', 'is_regular_file', 'lexically_normal', 'load',
        'lock', 'message', 'pop', 'pop_back', 'push', 'push_back', 'release',
        'reserve', 'reset', 'resize', 'size', 'store', 'substr', 'top', 'value',
        'value_or'
    )
)

foreach ($file in $cppFiles) {
    $source = Get-Content -LiteralPath $file.FullName -Encoding UTF8 -Raw

    # 중첩 namespace 형태를 포함한 std:: 심볼을 추출한다.
    foreach ($match in [regex]::Matches(
        $source,
        'std::[A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_][A-Za-z0-9_]*)*')) {
        $null = $symbols.Add($match.Value)
    }

    # 직접 포함한 표준 헤더 이름을 추출한다.
    foreach ($match in [regex]::Matches($source, '#include\s*<([^>]+)>')) {
        $null = $headers.Add($match.Groups[1].Value)
    }

    # 점 또는 화살표 뒤의 호출 이름 중 표준 멤버 화이트리스트만 남긴다.
    foreach ($match in [regex]::Matches($source, '(?:\.|->)\s*([A-Za-z_][A-Za-z0-9_]*)\s*\(')) {
        $member = $match.Groups[1].Value
        if ($knownStandardMembers.Contains($member)) {
            $null = $observedMembers.Add($member)
        }
    }
}

$missingSymbols = @(
    $symbols |
        Where-Object { -not $documentation.Contains($_) } |
        Sort-Object
)

$missingHeaders = @(
    $headers |
        Where-Object { -not $documentation.Contains("<$_>") } |
        Sort-Object
)

$missingMembers = @(
    $observedMembers |
        Where-Object {
            # `name`, name(...)처럼 문서에서 코드 용어 또는 호출 형태로 나타나야 통과한다.
            -not [regex]::IsMatch(
                $documentation,
                [regex]::Escape($_) + '(?:`|\s*\()'
            )
        } |
        Sort-Object
)

Write-Output "scope=$Scope dates=$($dateDirectories.Count) cpp_files=$($cppFiles.Count)"
Write-Output "documented_symbols=$($symbols.Count) headers=$($headers.Count) members=$($observedMembers.Count)"

if ($missingSymbols.Count -gt 0) {
    Write-Error "Undocumented std:: symbols: $($missingSymbols -join ', ')"
}

if ($missingHeaders.Count -gt 0) {
    Write-Error "Undocumented standard headers: $($missingHeaders -join ', ')"
}

if ($missingMembers.Count -gt 0) {
    Write-Error "Undocumented standard member-function candidates: $($missingMembers -join ', ')"
}

if ($missingSymbols.Count + $missingHeaders.Count + $missingMembers.Count -gt 0) {
    exit 1
}

Write-Output 'Standard-library symbol, header, and member-function audit passed.'
