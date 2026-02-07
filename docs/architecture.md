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

#### 9. OutputFormatter (`output_formatter.h/cpp`) - Phase 5
검색 결과의 출력 형식을 관리하는 클래스

**책임:**
- 3가지 출력 형식 제공 (Default, Detailed, JSON)
- ANSI 컬러 코드 관리
- 파일 크기 포맷팅 (B/KB/MB/GB/TB)
- 날짜/시간 ISO 8601 포맷팅

**주요 메서드:**
- `print()`: SearchResult 출력 (형식에 따라 분기)
- `setFormat()`: 출력 형식 설정
- `setUseColor()`: 컬러 사용 여부 설정
- `colorCode()`: ANSI 컬러 코드 반환

**출력 형식:**
- Default: 간결한 리스트 (`[TYPE] SIZE PATH`)
- Detailed: 전체 메타데이터 (경로, 타입, 크기, 수정 시간)
- JSON: 구조화된 출력 (`{"count": N, "files": [...]}`)

**컬러 지원:**
- 8가지 ANSI 색상 (Reset, Red, Green, Blue, Yellow, Cyan, Magenta, Gray)
- 터미널 환경에 따라 활성화/비활성화 가능

#### 9. Logger (`logger.h/cpp`) - Phase 6
애플리케이션 전역 로깅을 제공하는 싱글톤 클래스

**책임:**
- 애플리케이션 전체에서 일관된 로깅 인터페이스 제공
- 로그 레벨 필터링 (DEBUG/INFO/WARN/ERROR/NONE)
- 콘솔 및 파일 출력 지원
- 타임스탬프와 로그 레벨 포맷팅
- 스레드 안전 로깅

**디자인 패턴:**
- Singleton: 전역 접근 가능한 단일 인스턴스
- Thread-safe: std::mutex로 동기화

**주요 메서드:**
- `instance()`: 싱글톤 인스턴스 반환
- `setLevel()`, `getLevel()`: 로그 레벨 설정/조회
- `setLogFile()`: 로그 파일 경로 설정
- `setConsoleOutput()`: 콘솔 출력 활성화/비활성화
- `debug()`, `info()`, `warn()`, `error()`: 로그 메시지 출력
- `flush()`, `close()`: 버퍼 플러시 및 파일 닫기

**로그 레벨:**
- DEBUG (0): 상세한 디버깅 정보 (-v -v)
- INFO (1): 일반 정보 메시지 (-v)
- WARN (2): 경고 메시지 (기본)
- ERROR (3): 에러 메시지
- NONE (4): 모든 로깅 비활성화

**로그 형식:**
```
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message
```

**사용 예시:**
```cpp
auto& logger = Logger::instance();
logger.setLevel(LogLevel::DEBUG);
logger.setLogFile("app.log");
logger.debug("Starting file scan");
logger.info("Found " + std::to_string(count) + " files");
logger.warn("Permission denied: " + path);
logger.error("Failed to open file: " + path);
```

**통합:**
- FileScanner: 디렉토리 접근 에러, 파일 처리 에러 로깅
- main.cpp: 애플리케이션 라이프사이클, 설정 정보 로깅
- CommandLineParser: 파라미터 파싱 시 로깅
- ContentSearcher: 파일 읽기 에러 로깅

**CLI 옵션:**
- `-v`: INFO 레벨 활성화
- `-v -v`: DEBUG 레벨 활성화
- `--log-file FILE`: 로그를 파일에 기록

#### 10. CommandLineParser (`command_line_parser.h/cpp`) - Phase 2+
CLI 인자 파싱 및 검증을 담당하는 클래스

**책임:**
- 명령줄 인자 파싱 및 검증
- ApplicationConfig 객체 생성 (DTO 패턴)
- 설정 파일과 CLI 인자 병합 (Phase 9)
- 도움말 메시지 출력
- 옵션 검증 및 에러 처리

**주요 구성 요소:**
- `ApplicationConfig` 구조체: 모든 설정을 포함하는 DTO
  * 검색 옵션: recursive, followLinks, maxDepth
  * 성능 옵션: threadCount
  * 출력 옵션: outputFormat, useColor, verbosity
  * 로깅: logFile
  * SearchCriteria: 검색 필터
- `parse(argc, argv)`: 기본 파싱
- `parse(argc, argv, defaultConfig)`: 설정 파일 병합 파싱
- `parseOption()`: 개별 옵션 처리
- `validateConfig()`: 설정 검증
- `printUsage()`: 도움말 출력

**지원 옵션:**
- 검색: `-n/--name`, `-e/--ext`, `-p/--path`, `-c/--content`
- 필터: `-i/--ignore-case`, `-x/--regex`, `-f/--files-only`, `-D/--dirs-only`
- 크기: `--min-size`, `--max-size`
- 순회: `-r/--recursive`, `-d/--max-depth`, `-l/--follow-links`, `--ignore`
- 성능: `-j/--threads`
- 출력: `--format`, `--color/--no-color`
- 로깅: `-v/--verbose`, `--log-file`

**통합:**
- ConfigFile: 기본 설정 제공 (Phase 9)
- main.cpp: 파싱된 설정으로 FileScanner 구성
- SearchCriteria: 검색 조건 전달

**디자인 패턴:**
- DTO (Data Transfer Object): ApplicationConfig
- SRP: CLI 파싱 로직만 담당

#### 11. ConfigFile (`config_file.h/cpp`) - Phase 9
INI 스타일 설정 파일 파싱 클래스

**책임:**
- .findmyfilesrc 파일 로드 및 파싱
- 다중 위치 탐색 (현재 디렉토리, 홈, XDG config)
- `[default]` 섹션에서 기본 설정 추출
- `[search.*]` 섹션에서 저장된 검색 프로필 추출
- ApplicationConfig 객체 생성

**주요 메서드:**
- `load(filepath)`: 특정 파일 로드
- `loadDefault()`: 기본 위치 탐색 및 로드
- `getDefaultConfig()`: [default] 섹션 반환
- `getSavedSearch(name)`: 저장된 검색 반환
- `getSavedSearchNames()`: 모든 저장된 검색 이름

**파싱 기능:**
- INI 스타일 섹션 파싱
- Boolean 파싱: true/yes/on/1, false/no/off/0
- 인라인 주석 처리 (#, ;)
- 쉼표로 구분된 확장자 목록
- 모든 CLI 옵션 지원

**통합:**
- CommandLineParser: 기본 설정 제공
- main.cpp: 설정 우선순위 관리 (CLI > Config > 기본값)

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

### Semantic Search Flow (Phase 10)
```
1. main() → semantic search 옵션 감지
2. SemanticSearcher 초기화
   ├─ VectorStore 로드 (.fmf_index/)
   └─ EmbeddingProvider 초기화
3. Index 확인
   ├─ 없음 → FileScanner로 파일 목록 수집 → 인덱싱
   └─ 오래됨 → 변경된 파일만 재인덱싱
4. SemanticSearcher::search(query) 호출
   ├─ Query embedding 생성
   ├─ VectorStore에서 유사도 검색
   ├─ Top-K 결과 추출
   └─ (Optional) Hybrid: ContentSearcher 결과와 병합
5. SemanticResult → SearchResult 변환
6. Relevance score 기준 정렬
7. 결과 출력 (score와 함께)
```

### Component Dependencies
```
main.cpp
  ├─ ConfigFile (Phase 9)
  │  └─ ApplicationConfig 생성
  ├─ CommandLineParser
  │  ├─ ApplicationConfig (DTO)
  │  └─ SearchCriteria 설정
  ├─ Logger (Phase 6)
  │  ├─ 파일 로깅
  │  └─ 콘솔 로깅
  ├─ FileScanner (코어)
  │  ├─ FileInfo
  │  ├─ SearchCriteria
  │  ├─ SearchResult
  │  ├─ PatternMatcher
  │  ├─ ContentSearcher
  │  ├─ IgnorePatterns
  │  ├─ ThreadPool (Phase 4)
  │  └─ Logger
  ├─ SemanticSearcher (Phase 10 - 계획)
  │  ├─ EmbeddingGenerator
  │  │  └─ IEmbeddingProvider (Port)
  │  │     ├─ OpenAIProvider
  │  │     └─ LocalModelProvider
  │  ├─ VectorStore
  │  │  └─ IVectorStore (Port)
  │  │     ├─ FAISSStore
  │  │     └─ ChromaDBStore
  │  ├─ QueryProcessor
  │  ├─ FileScanner (파일 목록)
  │  └─ Logger
  └─ OutputFormatter (Phase 5)
     └─ SearchResult 출력

Configuration Flow:
ConfigFile → CommandLineParser → ApplicationConfig → main → FileScanner/SemanticSearcher
(CLI args override config file settings)

Semantic Search Flow:
SemanticSearcher → EmbeddingProvider → VectorStore → SemanticResult → SearchResult
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
- OutputFormatterTest: OutputFormatter 출력 (10 tests)
- LoggerTest: Logger 로깅 시스템 (12 tests)
- ConfigFileTest: ConfigFile 설정 파일 (17 tests)

**Total: 98 unit tests (10 test suites)**

### Integration Tests
실제 파일 시스템 사용 시나리오 테스트:
- uc_regex_search.sh: Regex 패턴 매칭 (3 tests)
- uc_content_search.sh: 파일 내용 검색 (4 tests)
- uc_ignore_patterns.sh: 제외 패턴 (.gitignore) (4 tests)
- uc_parallel_scan.sh: 병렬 스캔 성능 (4 tests)
- uc_output_formats.sh: 출력 형식 테스트 (7 tests)
- uc_logging.sh: 로깅 기능 테스트 (10 tests)
- uc_config_file.sh: 설정 파일 지원 (8 tests)

**Total: 40 integration tests (7 test suites)**

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

### Phase 5: CLI 개선 ✅
- OutputFormatter 클래스 (3가지 출력 형식)
- Default format: 간결한 리스트 출력
- Detailed format: 전체 메타데이터 출력
- JSON format: 구조화된 기계 판독 가능 출력
- ANSI 컬러 지원 (--color, --no-color)
- CLI 옵션 (--format default|detailed|json)
- 7개 통합 테스트 (uc_output_formats.sh)

### Phase 6: 에러 처리 및 로깅 ✅
- Logger 싱글톤 클래스 구현
- 4가지 로그 레벨 (DEBUG/INFO/WARN/ERROR)
- 콘솔 및 파일 로깅 지원
- 타임스탬프와 로그 레벨 포맷팅
- 스레드 안전 로깅 (std::mutex)
- CLI 옵션 (-v, -v -v, --log-file)
- FileScanner, main.cpp 통합
- 12개 단위 테스트, 10개 통합 테스트

### Phase 7: 테스트 및 CI/CD ✅
- GitHub Actions CI/CD 파이프라인
- 빌드 매트릭스 (Ubuntu/macOS × Debug/Release)
- 자동화된 단위 테스트 실행
- 코드 품질 체크 (clang-format, cppcheck, clang-tidy)
- 아티팩트 업로드 및 릴리스 자동화
- 81개 단위 테스트, 32개 통합 테스트

### Phase 8: 문서화 및 배포 ✅
- 포괄적인 README.md (470+ 라인, 40+ 예제)
- MIT LICENSE 추가
- CONTRIBUTING.md (기여 가이드, 코딩 스타일)
- install.sh 자동 설치 스크립트
- 아키텍처 문서화 (architecture.md)
- 프로젝트 로드맵 (todos.md)

### Phase 9: 추가 기능 (진행 중 - 85% 완료) 🔄
- ✅ ConfigFile 클래스 (INI 스타일 파서)
- ✅ .findmyfilesrc 파일 지원
- ✅ 다중 위치 로드 (현재 디렉토리, 홈, XDG)
- ✅ CLI 우선순위 (CLI > Config > 기본값)
- ✅ 17개 단위 테스트, 8개 통합 테스트
- ✅ 예제 설정 파일 및 문서화
- ⏳ 플러그인 시스템 (향후)
- ⏳ GUI 버전 (장기 목표)
- ⏳ 데이터베이스 인덱싱 (장기 목표)

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
├── LICENSE                     # MIT License
├── README.md                   # Project documentation
├── CONTRIBUTING.md             # Contribution guidelines
├── install.sh                  # Installation script
├── .github/
│   └── workflows/
│       └── ci.yml             # GitHub Actions CI/CD
├── docs/
│   ├── architecture.md         # This document
│   └── todos.md               # Phase-based TODO list
├── examples/
│   └── .findmyfilesrc         # Example config file
├── include/                    # Public headers (12 files, ~2100 lines)
│   ├── command_line_parser.h  # CLI argument parser
│   ├── config_file.h          # Configuration file parser
│   ├── content_searcher.h     # File content search
│   ├── file_info.h            # File metadata
│   ├── file_scanner.h         # Directory scanner
│   ├── ignore_patterns.h      # .gitignore style patterns
│   ├── logger.h               # Logging system
│   ├── output_formatter.h     # Output formatting
│   ├── pattern_matcher.h      # Pattern matching utilities
│   ├── search_criteria.h      # Search filters
│   ├── search_result.h        # Result container
│   └── thread_pool.h          # Thread pool
├── src/                        # Implementation files (13 files, ~2000 lines)
│   ├── command_line_parser.cpp
│   ├── config_file.cpp
│   ├── content_searcher.cpp
│   ├── file_info.cpp
│   ├── file_scanner.cpp
│   ├── ignore_patterns.cpp
│   ├── logger.cpp
│   ├── main.cpp               # CLI entry point
│   ├── output_formatter.cpp
│   ├── pattern_matcher.cpp
│   ├── search_criteria.cpp
│   ├── search_result.cpp
│   └── thread_pool.cpp
├── test/                       # Integration tests
│   └── integrationtest/        # 7 test suites, 40 tests
│       ├── run_all.sh
│       ├── uc_config_file.sh
│       ├── uc_content_search.sh
│       ├── uc_ignore_patterns.sh
│       ├── uc_logging.sh
│       ├── uc_output_formats.sh
│       ├── uc_parallel_scan.sh
│       └── uc_regex_search.sh
└── tests/                      # Unit tests (10 files, 98 tests)
    ├── test_command_line_parser.cpp
    ├── test_config_file.cpp
    ├── test_content_searcher.cpp
    ├── test_file_info.cpp
    ├── test_file_scanner.cpp
    ├── test_ignore_patterns.cpp
    ├── test_logger.cpp
    ├── test_output_formatter.cpp
    ├── test_pattern_matcher.cpp
    ├── test_search_criteria.cpp
    └── test_thread_pool.cpp

Total: ~4100 lines of C++ code (headers + implementation)
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

### v0.5.0 (Phase 5) - CLI Improvements
- ✅ OutputFormatter class with 3 output formats
- ✅ Default format: compact file listing
- ✅ Detailed format: full metadata per file
- ✅ JSON format: machine-readable output
- ✅ ANSI color support (--color/--no-color)
- ✅ CLI options: --format (default/detailed/json)
- ✅ 69 unit tests, 16 integration tests (5 test suites)

### v0.4.0 (Phase 4) - Multi-threading
- ✅ ThreadPool class for parallel task execution
- ✅ Thread-safe SearchResult with mutex
- ✅ Parallel directory scanning
- ✅ CLI option: -j/--threads
- ✅ Performance: ~3x speedup on large directories
- ✅ 57 unit tests, 15 integration tests

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

## Configuration System (Phase 9)

### ConfigFile Class

#### Purpose
Provides configuration file support for Find My Files, allowing users to define default settings and saved search profiles in `.findmyfilesrc` files.

#### Key Features
- **INI-style parsing**: Simple section-based configuration format
- **Multiple locations**: Current directory, home directory, XDG config
- **Default settings**: `[default]` section for base configuration
- **Search profiles**: `[search.*]` sections for saved searches
- **CLI override**: Command-line arguments take precedence

#### Architecture

```
ConfigFile
├── load(filepath)           : Load specific config file
├── loadDefault()            : Search default locations
├── getDefaultConfig()       : Get [default] section
├── getSavedSearch(name)     : Get [search.name] section
└── getSavedSearchNames()    : List all saved searches

Private:
├── parse(content)           : Parse INI-style content
├── sectionToConfig(section) : Convert section to ApplicationConfig
├── parseBool(value)         : Parse boolean values
└── trim(str)                : String utility
```

#### Design Pattern
- **Strategy Pattern**: Different parsing strategies for different value types
- **Factory Method**: `sectionToConfig` creates ApplicationConfig objects
- **SRP**: Only handles config file parsing, delegates to CommandLineParser for merging

#### Configuration Format

```ini
# Comments start with # or ;
[default]
recursive=true
threads=4
format=detailed
color=true

[search.cpp_files]
extensions=.cpp,.h,.hpp
files_only=true
```

#### Integration with CLI

```cpp
// main.cpp workflow:
1. ConfigFile.loadDefault()           // Load from default locations
2. configFile.getDefaultConfig()      // Extract default settings
3. CommandLineParser.parse(argc, argv, defaultConfig)  // Parse CLI with defaults
4. Result: CLI args override config file settings
```

#### Testing
- **Unit Tests**: 17 tests covering parsing, sections, data types
  - Empty files, comments, whitespace handling
  - Boolean parsing (true/yes/on/1, false/no/off/0)
  - Multiple sections and saved searches
  - All configuration option types
- **Integration Tests**: 8 test scenarios
  - Config file loading from current directory
  - Recursive search from config
  - CLI override of config settings
  - Logging and verbosity from config
  - Ignore patterns and depth limits

#### Example Usage

```cpp
ConfigFile config;
if (config.loadDefault()) {
    auto defaultConfig = config.getDefaultConfig();
    // Use as base for CLI parsing
    parser.parse(argc, argv, *defaultConfig);
}
```

### Configuration Precedence

```
CLI Arguments (highest)
    ↓
Config File (middle)
    ↓
Built-in Defaults (lowest)
```

This design ensures flexibility: users can set project-wide defaults in `.findmyfilesrc` while still overriding specific options via CLI.
