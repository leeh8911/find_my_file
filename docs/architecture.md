# Find My Files - Architecture Documentation

## Overview
Find My Files (fmf)는 C++17 기반의 파일 검색 도구로, 명령줄 인터페이스를 통해 강력한 파일 검색 기능을 제공합니다.

## Architecture

### Core Components

#### 1. FileInfo (`file_info.h/cpp`)
파일 메타데이터를 캡슐화하는 클래스

**책임:**
- 파일 시스템에서 파일 정보 읽기
- 파일 타입 (Regular, Directory, SymbolicLink, Unknown) 관리
- 파일 속성 제공 (이름, 경로, 크기, 수정 시간)

**주요 메서드:**
- `getName()`, `getPath()`: 파일 이름/경로
- `getSize()`, `getModifiedTime()`: 파일 크기/수정 시간
- `isRegularFile()`, `isDirectory()`, `isSymbolicLink()`: 타입 체크

#### 2. SearchCriteria (`search_criteria.h/cpp`)
검색 조건을 정의하고 관리하는 클래스

**책임:**
- 검색 필터 설정 (이름, 경로 패턴, 확장자)
- 파일 속성 필터 (크기, 날짜 범위, 타입)
- 검색 옵션 (대소문자 구분, regex 사용, content 검색)

**주요 메서드:**
- `setNamePattern()`, `setPathPattern()`: 패턴 설정
- `addExtension()`: 확장자 필터 추가
- `setMinSize()`, `setMaxSize()`: 크기 범위
- `setModifiedAfter()`, `setModifiedBefore()`: 날짜 범위
- `setUseRegex()`: Regex 모드 활성화
- `setContentPattern()`: 내용 검색 패턴

#### 3. PatternMatcher (`pattern_matcher.h/cpp`)
패턴 매칭 알고리즘을 제공하는 유틸리티 클래스

**책임:**
- 와일드카드 패턴 매칭 (`*`, `?`)
- 정규식 패턴 매칭
- 대소문자 구분/비구분 처리

**주요 메서드:**
- `matchWildcard()`: 와일드카드 패턴 매칭 (재귀 알고리즘)
- `matchRegex()`: std::regex 기반 정규식 매칭
- `matchExact()`: 정확한 문자열 매칭

#### 4. SearchResult (`search_result.h/cpp`)
검색 결과를 관리하는 컨테이너 클래스

**책임:**
- FileInfo 객체 컬렉션 관리
- 결과 정렬 (이름, 크기, 날짜 기준)
- 결과 접근 인터페이스

**주요 메서드:**
- `add()`: 결과 추가
- `sortByName()`, `sortBySize()`, `sortByDate()`: 정렬
- `size()`: 결과 개수
- `begin()`, `end()`: 이터레이터 지원

#### 5. FileScanner (`file_scanner.h/cpp`)
파일 시스템 스캔 및 검색을 수행하는 중심 클래스

**책임:**
- 디렉토리 순회 (재귀/비재귀, 심볼릭 링크 처리)
- SearchCriteria에 따른 파일 필터링
- ContentSearcher, IgnorePatterns 통합
- 검색 결과 생성

**주요 메서드:**
- `scan()`: 기본 디렉토리 스캔
- `scanRecursive()`: 재귀적 디렉토리 스캔
- `search()`: 조건에 맞는 파일 검색
- `setFollowSymlinks()`: 심볼릭 링크 추적 설정
- `setIgnorePatterns()`: 제외 패턴 설정

**내부 메서드:**
- `matchesCriteria()`: 파일이 검색 조건을 만족하는지 확인
- `shouldProcess()`: IgnorePatterns에 따라 파일 처리 여부 결정

#### 6. ContentSearcher (`content_searcher.h/cpp`) - Phase 3
파일 내용 검색을 담당하는 유틸리티 클래스

**책임:**
- 텍스트 파일 판별 (바이너리 파일 제외)
- 파일 내용에서 패턴 검색 (literal/regex)
- 효율적인 파일 읽기 (10MB 제한)

**주요 메서드:**
- `searchInFile()`: 파일 내용 검색 (static)
- `isTextFile()`: 텍스트 파일 여부 판별 (static)

**알고리즘:**
- Null byte 감지로 바이너리 파일 즉시 거부
- 첫 4KB 샘플링으로 텍스트 파일 판별
- 라인 단위 검색으로 메모리 효율성 확보

#### 7. IgnorePatterns (`ignore_patterns.h/cpp`) - Phase 3
.gitignore 스타일의 제외 패턴을 관리하는 클래스

**책임:**
- .gitignore 형식 파일 파싱
- 와일드카드 패턴 매칭 (디렉토리/파일 구분)
- 경로 제외 판단

**주요 메서드:**
- `addPattern()`: 패턴 추가
- `loadFromFile()`: .gitignore 파일 로드
- `shouldIgnore()`: 경로가 무시되어야 하는지 판단
- `size()`: 로드된 패턴 수

**패턴 형식:**
- `node_modules/`: 디렉토리 제외
- `*.tmp`: 특정 확장자 제외
- `build*`: 접두사 패턴

#### 8. ThreadPool (`thread_pool.h/cpp`) - Phase 4
병렬 작업 실행을 위한 스레드 풀 클래스

**책임:**
- 워커 스레드 풀 관리
- 작업 큐를 통한 태스크 제출
- std::future를 통한 결과 반환
- 스레드 안전한 작업 스케줄링

**주요 메서드:**
- `submit()`: 태스크 제출 (템플릿 기반, future 반환)
- `wait()`: 모든 작업 완료 대기
- `size()`, `pendingTasks()`: 풀 상태 조회

**구현 세부사항:**
- std::packaged_task로 결과 처리
- mutex/condition_variable로 동기화
- std::atomic으로 활성 작업 추적
- RAII 기반 스레드 관리 (소멸자에서 대기)

## Data Flow

### Search Operation Flow
```
1. main() → 명령줄 파싱
2. SearchCriteria 생성 및 설정
3. FileScanner 생성
4. IgnorePatterns 로드 (optional)
5. FileScanner::search() 호출
   ├─ 디렉토리 순회 (scan/scanRecursive)
   └─ 각 파일에 대해:
      ├─ shouldProcess() → IgnorePatterns 체크
      ├─ matchesCriteria() → 필터링
      │  ├─ PatternMatcher → 이름/경로 매칭
      │  ├─ 크기/날짜 범위 체크
      │  └─ ContentSearcher → 내용 검색 (optional)
      └─ SearchResult에 추가
6. 결과 정렬 (optional)
7. 결과 출력
```

### Component Dependencies
```
main.cpp
  ├─ FileScanner
  │  ├─ FileInfo
  │  ├─ SearchCriteria
  │  ├─ SearchResult
  │  ├─ PatternMatcher
  │  ├─ ContentSearcher
  │  └─ IgnorePatterns
  └─ 출력 포맷팅
```

## Design Patterns

### 1. Strategy Pattern
- **PatternMatcher**: 다양한 매칭 전략 (wildcard, regex, exact)
- **SearchCriteria**: 검색 전략 캡슐화

### 2. Template Method Pattern
- **FileScanner**: scan()과 scanRecursive()가 공통 필터링 로직 공유

### 3. Builder Pattern
- **SearchCriteria**: 유창한 인터페이스로 검색 조건 구성

### 4. Static Helper Pattern
- **ContentSearcher**: 정적 메서드로 파일 내용 검색 유틸리티 제공
- **PatternMatcher**: 정적 메서드로 패턴 매칭 유틸리티 제공

## Technology Stack

### Core Technologies
- **Language**: C++17
- **Build System**: CMake 3.14+
- **Standard Library**: STL (std::filesystem, std::regex, std::optional)

### Testing
- **Framework**: Google Test (gtest)
- **Unit Tests**: 45 tests across 6 test suites
- **Integration Tests**: 11 use case tests (bash scripts)

### Development Tools
- **Code Style**: clang-format
- **Static Analysis**: clang-tidy, cpplint
- **Compiler Flags**: `-Wall -Wextra -Wpedantic -Werror`

## Testing Strategy

### Unit Tests
각 클래스별로 독립적인 테스트 스위트 구성:
- FileInfoTest: FileInfo 클래스 (6 tests)
- SearchCriteriaTest: SearchCriteria 클래스 (9 tests)
- PatternMatcherTest: PatternMatcher 유틸리티 (8 tests)
- FileScannerTest: FileScanner 코어 로직 (6 tests)
- ContentSearcherTest: ContentSearcher 유틸리티 (6 tests)
- IgnorePatternsTest: IgnorePatterns 클래스 (10 tests)
- ThreadPoolTest: ThreadPool 병렬 실행 (12 tests)

**Total: 57 unit tests**

### Integration Tests
실제 파일 시스템 사용 시나리오 테스트:
- uc_regex_search.sh: Regex 패턴 매칭 (3 tests)
- uc_content_search.sh: 파일 내용 검색 (4 tests)
- uc_ignore_patterns.sh: 제외 패턴 (.gitignore) (4 tests)
- uc_parallel_scan.sh: 병렬 스캔 성능 (4 tests)

**Total: 15 integration tests**

## Phase Implementation Status

### Phase 1: 기본 구조 ✅
- CMake 빌드 시스템
- FileInfo, SearchResult, FileScanner 기본 구현
- 디렉토리 순회 기능

### Phase 2: 기본 검색 ✅
- SearchCriteria 클래스
- PatternMatcher (wildcard 지원)
- CLI 인터페이스

### Phase 3: 고급 검색 ✅
- Regex 지원 (PatternMatcher 확장)
- ContentSearcher (파일 내용 검색)
- IgnorePatterns (.gitignore 스타일)
- 날짜 필터링

### Phase 4: 성능 최적화 ✅
- 멀티스레딩 (ThreadPool 기반 병렬 스캔)
- 스레드 안전 결과 수집 (mutex 기반 SearchResult)
- CLI 스레드 개수 옵션 (-j, --threads)
- 성능 벤치마크 테스트

### Upcoming Phases
- **Phase 5**: CLI 개선 (컬러 출력, 진행률 표시, JSON 출력)
- **Phase 6**: 추가 기능 (심볼릭 링크 처리, 중복 파일 검색)
- **Phase 7**: 성능 테스트 및 벤치마크
- **Phase 8**: 문서화 및 사용자 가이드
- **Phase 9**: 배포 준비 (패키징, 설치 스크립트)

## Build and Run

### Build
```bash
mkdir build && cd build
cmake ..
make
```

### Run Tests
```bash
# Unit tests
cd build
./tests

# Integration tests
cd test/integrationtest
bash run_all.sh
```

### CLI Usage
```bash
# Basic search
./find_my_files /path/to/search

# Recursive search with name pattern
./find_my_files -r -n "*.cpp" /path

# Content search with regex
./find_my_files -r -c "TODO|FIXME" -x src/

# Ignore patterns
./find_my_files -r --ignore .gitignore .
```

## Code Organization

```
find_my_files/
├── CMakeLists.txt              # Build configuration
├── CPPLINT.cfg                 # Linter configuration
├── docs/
│   ├── architecture.md         # This document
│   └── todos.md               # Phase-based TODO list
├── include/                    # Public headers
│   ├── content_searcher.h
│   ├── file_info.h
│   ├── file_scanner.h
│   ├── ignore_patterns.h
│   ├── pattern_matcher.h
│   ├── search_criteria.h
│   └── search_result.h
├── src/                        # Implementation files
│   ├── content_searcher.cpp
│   ├── file_info.cpp
│   ├── file_scanner.cpp
│   ├── ignore_patterns.cpp
│   ├── main.cpp                # CLI entry point
│   ├── pattern_matcher.cpp
│   ├── search_criteria.cpp
│   └── search_result.cpp
├── test/                       # Testing
│   └── integrationtest/        # Integration test scripts
│       ├── run_all.sh
│       ├── uc_content_search.sh
│       ├── uc_ignore_patterns.sh
│       └── uc_regex_search.sh
└── tests/                      # Unit tests
    ├── test_content_searcher.cpp
    ├── test_file_info.cpp
    ├── test_file_scanner.cpp
    ├── test_ignore_patterns.cpp
    ├── test_pattern_matcher.cpp
    └── test_search_criteria.cpp
```

## Performance Considerations

### Current Implementation
- **Multi-threaded**: ThreadPool-based parallel directory scanning
- **Thread-safe**: Mutex-protected SearchResult for concurrent access
- **Memory Efficient**: 라인 단위 파일 읽기, 10MB 파일 크기 제한
- **Optimized Pattern Matching**: 재귀적 와일드카드 알고리즘
- **Early Rejection**: Null byte 검사로 바이너리 파일 즉시 제외
- **Configurable Parallelism**: --threads 옵션으로 병렬 레벨 조정

### Performance Characteristics
- **Parallel Speedup**: ~3x faster on large directory trees (878 files)
- **Thread Safety Overhead**: Minimal (mutex only during result insertion)
- **Scalability**: Automatic subdirectory parallelization
- **Sequential Fallback**: --threads 0 for sequential mode

### Future Optimizations (Phase 5+)
- 결과 캐싱 (변경되지 않은 디렉토리)
- SIMD를 활용한 문자열 검색 최적화
- 메모리 사용량 최적화

## Security Considerations

- **Path Traversal**: std::filesystem의 canonical() 사용으로 안전한 경로 처리
- **Symbolic Link Loops**: 기본적으로 심볼릭 링크 추적 비활성화
- **Resource Limits**: 파일 크기 제한 (10MB), 재귀 깊이 제한 가능
- **Input Validation**: Regex 패턴 검증, 파일 존재 여부 확인

## Coding Standards

### Naming Conventions
- **Classes**: PascalCase (e.g., `FileScanner`)
- **Methods**: camelCase (e.g., `matchesCriteria()`)
- **Variables**: camelCase (e.g., `fileInfo`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_SEARCH_SIZE`)
- **Namespace**: lowercase (e.g., `fmf`)

### Code Style
- **Indentation**: 4 spaces
- **Line Length**: 80 characters (enforced by clang-format)
- **Braces**: Allman style (opening brace on new line)
- **Header Guards**: `#pragma once`

### Error Handling
- **Exceptions**: Used for unrecoverable errors (std::runtime_error)
- **Return Values**: `bool` for success/failure, `std::optional` for nullable returns
- **File Operations**: Exception handling with try-catch blocks

## Version History

### v0.4.0 (Phase 4) - Current
- ✅ ThreadPool for parallel task execution
- ✅ Multi-threaded directory scanning
- ✅ Thread-safe SearchResult with mutex
- ✅ CLI --threads option
- ✅ 57 unit tests, 15 integration tests
- ✅ ~3x performance improvement on large trees

### v0.3.0 (Phase 3)
- ✅ Regex pattern matching
- ✅ File content search
- ✅ .gitignore style ignore patterns
- ✅ Date range filtering
- ✅ 45 unit tests, 11 integration tests

### v0.2.0 (Phase 2)
- ✅ Wildcard pattern matching
- ✅ Extension filtering
- ✅ Path-based search
- ✅ CLI implementation

### v0.1.0 (Phase 1)
- ✅ Basic project structure
- ✅ Directory traversal
- ✅ File metadata reading
- ✅ CMake build system
