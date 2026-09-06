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

# 헤더 등록은 전용 headers.md에서만 찾고, 심볼·호출 계약은 얕은 색인/헤더표가
# 자기 자신을 설명으로 충족하지 않도록 실제 세부 문서에서만 찾는다.
$documentationFiles = @(Get-ChildItem -LiteralPath $docsRoot -Filter '*.md' -File | Sort-Object Name)
$headersPath = Join-Path $docsRoot 'headers.md'
if (-not (Test-Path -LiteralPath $headersPath)) {
    throw "Standard-library header registry not found: $headersPath"
}
$headerDocumentation = Get-Content -LiteralPath $headersPath -Encoding UTF8 -Raw
$documentation = (
    $documentationFiles |
        Where-Object { $_.Name -notin @('README.md', 'by-date.md', 'headers.md', 'symbol-index.md') } |
        ForEach-Object { Get-Content -LiteralPath $_.FullName -Encoding UTF8 -Raw }
) -join "`n"

$symbols = [Collections.Generic.HashSet[string]]::new()
$headers = [Collections.Generic.HashSet[string]]::new()
$observedMembers = [Collections.Generic.HashSet[string]]::new()
$observedOperations = [Collections.Generic.HashSet[string]]::new()

# std::가 이름에 드러나지 않는 표준 멤버 중 현재 학습 코드에서 자주 쓰는 항목만 검사한다.
$knownStandardMembers = [Collections.Generic.HashSet[string]]::new(
    [string[]]@(
        'acquire', 'append', 'arrive_and_wait', 'assign', 'at', 'back', 'begin', 'c_str', 'clear', 'contains', 'count_down', 'data', 'empty', 'emplace', 'emplace_back', 'end',
        'error', 'expired', 'extent', 'extract', 'fetch_add', 'file_size', 'find', 'front', 'get',
        'has_value', 'is_absolute', 'is_regular_file', 'join', 'joinable', 'lexically_normal', 'load',
        'insert', 'key', 'lock', 'mapped', 'message', 'pop', 'pop_back', 'pop_front', 'push', 'push_back', 'push_front', 'release',
        'notify_all', 'notify_one', 'reserve', 'reset', 'resize', 'size', 'store', 'str', 'substr', 'swap', 'tie', 'top', 'value', 'wait',
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

    # 점 호출로 드러나지 않는 생성자·인덱싱·스트림 연산자도 공용 문서에 대표 계약이 있는지 검사한다.
    if ($source -match 'std::vector\s*<') {
        $null = $observedOperations.Add('vector(')
        if ($source -match '\b[A-Za-z_][A-Za-z0-9_]*\s*\[[^\]\r\n]+\]') {
            $null = $observedOperations.Add('operator[]')
        }
    }
    if ($source -match 'std::cin\s*>>') {
        $null = $observedOperations.Add('operator>>')
    }
    if ($source -match 'std::(?:cout|cerr|clog)\s*<<') {
        $null = $observedOperations.Add('operator<<')
    }
}

$missingSymbols = @(
    $symbols |
        Where-Object {
            -not [regex]::IsMatch(
                $documentation,
                '(?<![A-Za-z0-9_:])' + [regex]::Escape($_) + '(?![:A-Za-z0-9_])'
            )
        } |
        Sort-Object
)

$missingHeaders = @(
    $headers |
        Where-Object { -not $headerDocumentation.Contains("<$_>") } |
        Sort-Object
)

$missingMembers = @(
    $observedMembers |
        Where-Object {
            # `name`, name(...)처럼 문서에서 코드 용어 또는 호출 형태로 나타나야 통과한다.
            -not [regex]::IsMatch(
                $documentation,
                '(?<![A-Za-z0-9_])' + [regex]::Escape($_) + '(?:`|\s*\()'
            )
        } |
        Sort-Object
)

$missingOperations = @(
    $observedOperations |
        Where-Object {
            $escaped = [regex]::Escape($_)
            -not [regex]::IsMatch($documentation, '(?<![A-Za-z0-9_])' + $escaped)
        } |
        Sort-Object
)

# 대표 색인은 현재 소스에서 추출한 std:: 심볼을 정확히 한 번씩만 등록해야 한다.
$symbolIndexPath = Join-Path $docsRoot 'symbol-index.md'
$indexedSymbolCounts = @{}
foreach ($line in Get-Content -LiteralPath $symbolIndexPath -Encoding UTF8) {
    if (-not $line.StartsWith('|')) {
        continue
    }
    foreach ($match in [regex]::Matches(
        $line,
        'std::[A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_][A-Za-z0-9_]*)*')) {
        $name = $match.Value
        if ($indexedSymbolCounts.ContainsKey($name)) {
            $indexedSymbolCounts[$name] += 1
        } else {
            $indexedSymbolCounts[$name] = 1
        }
    }
}

$missingIndexSymbols = @()
$extraIndexSymbols = @()
$duplicateIndexSymbols = @()
$missingIndexSymbols = @(
    $symbols |
        Where-Object { -not $indexedSymbolCounts.ContainsKey($_) } |
        Sort-Object
)
$duplicateIndexSymbols = @(
    $indexedSymbolCounts.Keys |
        Where-Object { $indexedSymbolCounts[$_] -ne 1 } |
        Sort-Object
)
if ($Scope -eq 'all') {
    # latest의 일부 심볼을 전체 색인과 비교하면 나머지가 거짓 extra가 되므로 extra 집합 검사만 all로 제한한다.
    $extraIndexSymbols = @(
        $indexedSymbolCounts.Keys |
            Where-Object { -not $symbols.Contains($_) } |
            Sort-Object
    )
}

# by-date.md의 각 선택 날짜 행이 해당 날짜 C++에서 추출한 헤더·심볼 집합과 정확히 같아야 한다.
$byDateLines = @(Get-Content -LiteralPath (Join-Path $docsRoot 'by-date.md') -Encoding UTF8)
$byDateIssues = @()
if ($Scope -eq 'all') {
    # 실제 날짜 폴더가 없는 오래된 행과 날짜 키 중복도 전체 감사에서 거부한다.
    $actualDateNames = @($dateDirectories | ForEach-Object { $_.Name })
    $registeredDateNames = @(
        foreach ($line in $byDateLines) {
            $dateMatch = [regex]::Match($line, '^\| \*\*(\d{4}-\d{2}-\d{2})\*\*<br>')
            if ($dateMatch.Success) {
                $dateMatch.Groups[1].Value
            }
        }
    )
    foreach ($group in $registeredDateNames | Group-Object) {
        if ($group.Count -ne 1) {
            $byDateIssues += "$($group.Name): duplicate by-date rows ($($group.Count))"
        }
    }
    foreach ($registeredDate in $registeredDateNames | Sort-Object -Unique) {
        if ($registeredDate -notin $actualDateNames) {
            $byDateIssues += "${registeredDate}: stale by-date row without a dated directory"
        }
    }
}
foreach ($directory in $dateDirectories) {
    $datePattern = '^\| \*\*' + [regex]::Escape($directory.Name) + '\*\*<br>'
    $rows = @($byDateLines | Where-Object { $_ -match $datePattern })
    if ($rows.Count -ne 1) {
        $byDateIssues += "$($directory.Name): expected one by-date row, found $($rows.Count)"
        continue
    }

    $dateSource = (
        Get-ChildItem -LiteralPath $directory.FullName -Filter '*.cpp' -File |
            ForEach-Object { Get-Content -LiteralPath $_.FullName -Encoding UTF8 -Raw }
    ) -join "`n"
    $sourceDateSymbols = @(
        [regex]::Matches(
            $dateSource,
            'std::[A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_][A-Za-z0-9_]*)*') |
            ForEach-Object { $_.Value } |
            Sort-Object -Unique
    )
    $rowDateSymbolOccurrences = @(
        [regex]::Matches(
            $rows[0],
            'std::[A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_][A-Za-z0-9_]*)*') |
            ForEach-Object { $_.Value }
    )
    $rowDateSymbols = @($rowDateSymbolOccurrences | Sort-Object -Unique)
    $sourceDateHeaders = @(
        [regex]::Matches($dateSource, '#include\s*<([^>]+)>') |
            ForEach-Object { $_.Groups[1].Value } |
            Sort-Object -Unique
    )
    $rowDateHeaderOccurrences = @(
        [regex]::Matches($rows[0], '``<([^>]+)>``') |
            ForEach-Object { $_.Groups[1].Value }
    )
    $rowDateHeaders = @($rowDateHeaderOccurrences | Sort-Object -Unique)

    foreach ($group in $rowDateSymbolOccurrences | Group-Object) {
        if ($group.Count -ne 1) {
            $byDateIssues += "$($directory.Name): duplicate symbol $($group.Name) ($($group.Count))"
        }
    }
    foreach ($group in $rowDateHeaderOccurrences | Group-Object) {
        if ($group.Count -ne 1) {
            $byDateIssues += "$($directory.Name): duplicate header <$($group.Name)> ($($group.Count))"
        }
    }

    foreach ($difference in Compare-Object $sourceDateSymbols $rowDateSymbols) {
        $side = if ($difference.SideIndicator -eq '<=') { 'missing' } else { 'extra' }
        $byDateIssues += "$($directory.Name): $side symbol $($difference.InputObject)"
    }
    foreach ($difference in Compare-Object $sourceDateHeaders $rowDateHeaders) {
        $side = if ($difference.SideIndicator -eq '<=') { 'missing' } else { 'extra' }
        $byDateIssues += "$($directory.Name): $side header <$($difference.InputObject)>"
    }
}

# 최신 날짜는 각 표준 호출의 첫 실제 코드 줄 주변과 README 호출 표를 함께 검사한다.
# 정규식은 의미를 완전히 증명하지 못하므로, 통과 뒤에도 사람이 인자 수·값 범주·상태 변화를 다시 읽어야 한다.
$contractIssues = @()
$latestDirectory = $dateDirectories[-1]
$latestReadmePath = Join-Path $latestDirectory.FullName 'README.md'
$latestReadme = Get-Content -LiteralPath $latestReadmePath -Encoding UTF8 -Raw
$tableMatch = [regex]::Match(
    $latestReadme,
    '(?s)## 오늘 사용한 표준 라이브러리\s*(.*?)(?=\r?\n## |\z)'
)
$latestTable = if ($tableMatch.Success) { $tableMatch.Groups[1].Value } else { '' }
if (-not $tableMatch.Success) {
    $contractIssues += "$($latestDirectory.Name)/README.md: missing standard-library table"
}

$contractPatterns = @(
    [pscustomobject]@{ Name = 'std::move'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*std::move\s*\('; Readme = 'std::move(' },
    [pscustomobject]@{ Name = 'std::apply'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*std::apply\s*\('; Readme = 'std::apply(' },
    [pscustomobject]@{ Name = 'sync_with_stdio'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*std::ios::sync_with_stdio\s*\('; Readme = 'sync_with_stdio(' },
    [pscustomobject]@{ Name = 'tie'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*std::cin\.tie\s*\('; Readme = 'std::cin.tie(' },
    [pscustomobject]@{ Name = 'operator>>'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*std::cin\s*>>'; Readme = 'std::cin >>' },
    [pscustomobject]@{ Name = 'operator<<'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*std::cout\s*<<'; Readme = 'std::cout <<' },
    [pscustomobject]@{ Name = 'operator<<(ostream&, char)'; Pattern = "(?m)^(?!\s*//)\s*[^\r\n]*std::cout[^\r\n]*<<\s*'(?:\\.|[^'])'"; Readme = 'operator<<(std::ostream&, char)' },
    [pscustomobject]@{ Name = 'std::min'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*std::min\s*\('; Readme = 'std::min(' },
    [pscustomobject]@{ Name = 'vector default constructor'; Pattern = '(?m)^\s*std::vector<.*>\s+\w+\s*;'; Readme = '기본 생성자' },
    [pscustomobject]@{ Name = 'vector count constructor'; Pattern = '(?m)^\s*std::vector<.*>\s+\w+\s*\([^,;\r\n]+\);'; Readme = 'count 생성자' },
    [pscustomobject]@{ Name = 'vector fill constructor'; Pattern = '(?m)^\s*std::vector<.*>\s+\w+\s*\([^;\r\n]*,[^;\r\n]*\);'; Readme = 'fill 생성자' },
    [pscustomobject]@{ Name = 'vector::operator[]'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*\b\w+\s*\[[^\]\r\n]+\]'; Readme = 'vector::operator[]' },
    [pscustomobject]@{ Name = 'vector::reserve'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*\.reserve\s*\('; Readme = 'vector::reserve' },
    [pscustomobject]@{ Name = 'vector::push_back'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*\.push_back\s*\('; Readme = 'vector::push_back' },
    [pscustomobject]@{ Name = 'vector::size'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*\.size\s*\('; Readme = 'vector::size' },
    [pscustomobject]@{ Name = 'vector::empty'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*\.empty\s*\('; Readme = 'vector::empty' },
    [pscustomobject]@{ Name = 'vector::back'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*\.back\s*\('; Readme = 'vector::back' },
    [pscustomobject]@{ Name = 'vector::pop_back'; Pattern = '(?m)^(?!\s*//)\s*[^\r\n]*\.pop_back\s*\('; Readme = 'vector::pop_back' }
)

foreach ($file in Get-ChildItem -LiteralPath $latestDirectory.FullName -Filter '*.cpp' -File) {
    $source = Get-Content -LiteralPath $file.FullName -Encoding UTF8 -Raw
    $lines = @(Get-Content -LiteralPath $file.FullName -Encoding UTF8)
    foreach ($candidate in $contractPatterns) {
        # vector 멤버·생성자 후보는 vector를 직접 사용하는 번역 단위에서만 표준 호출로 간주한다.
        if ($candidate.Name.StartsWith('vector') -and $source -notmatch 'std::vector\s*<') {
            continue
        }
        $match = [regex]::Match($source, $candidate.Pattern)
        if (-not $match.Success) {
            continue
        }

        $lineNumber = [regex]::Matches($source.Substring(0, $match.Index), "`n").Count + 1
        $contextStart = [Math]::Max(0, $lineNumber - 9)
        $context = ($lines[$contextStart..($lineNumber - 1)] -join "`n")
        $missingParts = @()
        if ($context -notmatch '인자|피연산자|수신|입력|받아|생성자에는') { $missingParts += 'inputs/receiver' }
        if ($context -notmatch '반환|\bvoid\b') { $missingParts += 'return' }
        if ($context -notmatch 'O\(|복잡도|선형|상수 시간|문자 수|소비 문자') { $missingParts += 'complexity' }
        if ($context -notmatch '예외|오류|실패|UB|미정의|범위|비어|상태 비트|noexcept|할당|bad_alloc|length_error|무효화|수명') { $missingParts += 'error/lifetime' }
        if ($missingParts.Count -gt 0) {
            $contractIssues += "$($file.Name):$lineNumber $($candidate.Name) nearby comment lacks $($missingParts -join ', ')"
        }
        if (-not $latestTable.Contains($candidate.Readme)) {
            $contractIssues += "$($latestDirectory.Name)/README.md table lacks $($candidate.Readme) for $($candidate.Name)"
        }
    }
}

Write-Output "scope=$Scope dates=$($dateDirectories.Count) cpp_files=$($cppFiles.Count)"
Write-Output "documented_symbols=$($symbols.Count) headers=$($headers.Count) members=$($observedMembers.Count) operations=$($observedOperations.Count) index_symbols=$($indexedSymbolCounts.Count)"

$auditFailures = @()
if ($missingSymbols.Count -gt 0) {
    $auditFailures += "Undocumented std:: symbols: $($missingSymbols -join ', ')"
}

if ($missingHeaders.Count -gt 0) {
    $auditFailures += "Undocumented standard headers: $($missingHeaders -join ', ')"
}

if ($missingMembers.Count -gt 0) {
    $auditFailures += "Undocumented standard member-function candidates: $($missingMembers -join ', ')"
}

if ($missingOperations.Count -gt 0) {
    $auditFailures += "Undocumented standard constructor/operator candidates: $($missingOperations -join ', ')"
}

if ($missingIndexSymbols.Count -gt 0) {
    $auditFailures += "Symbols missing from symbol-index.md: $($missingIndexSymbols -join ', ')"
}

if ($extraIndexSymbols.Count -gt 0) {
    $auditFailures += "Symbols in symbol-index.md but absent from dated C++ sources: $($extraIndexSymbols -join ', ')"
}

if ($duplicateIndexSymbols.Count -gt 0) {
    $auditFailures += "Symbols registered more than once in symbol-index.md: $($duplicateIndexSymbols -join ', ')"
}

if ($byDateIssues.Count -gt 0) {
    $auditFailures += "by-date.md mismatches: $($byDateIssues -join '; ')"
}

if ($contractIssues.Count -gt 0) {
    $auditFailures += "Latest call-contract mismatches: $($contractIssues -join '; ')"
}

if ($missingSymbols.Count + $missingHeaders.Count + $missingMembers.Count + $missingOperations.Count +
    $missingIndexSymbols.Count + $extraIndexSymbols.Count + $duplicateIndexSymbols.Count +
    $byDateIssues.Count + $contractIssues.Count -gt 0) {
    $auditFailures | ForEach-Object { Write-Output "ERROR: $_" }
    exit 1
}

Write-Output 'Standard-library symbol, header, member-function, constructor/operator, and index audit passed.'
