# Find My Files - Clean Architecture Documentation

## Overview

Find My Files (fmf)는 C++17 기반의 파일 검색 도구로, **Clean Architecture** 원칙을 따라 설계되었습니다.

## Clean Architecture Principles

### Dependency Rule
**핵심 규칙: 의존성은 항상 안쪽(내부)를 향해야 합니다.**

```
Infrastructure → Adapters → Application → Domain
(바깥)                                    (안쪽)
```

- 외부 레이어는 내부 레이어에 의존 가능
- 내부 레이어는 외부 레이어를 알 수 없음
- Domain은 어떤 레이어에도 의존하지 않음

### Layer Structure

```
┌──────────────────────────────────────────┐
│     Infrastructure Layer                 │  ← Frameworks, Drivers, Tools
│  (Logger, ThreadPool, ConfigFile)        │
├──────────────────────────────────────────┤
│     Interface Adapters Layer             │  ← Controllers, Presenters
│  (CommandLineParser, OutputFormatter)    │
├──────────────────────────────────────────┤
│     Application Layer                    │  ← Use Cases, Services
│  (FileScanner, SemanticSearcher)         │
├──────────────────────────────────────────┤
│     Domain Layer                         │  ← Entities, Value Objects
│  (FileInfo, SearchCriteria)              │
└──────────────────────────────────────────┘
```

---

## 1. Domain Layer (가장 안쪽)

**책임**: 핵심 비즈니스 로직과 엔티티 정의
**의존성**: 없음 (순수 비즈니스 로직)

### 1.1 Entities

비즈니스 도메인의 핵심 객체

#### FileInfo (`domain/entities/file_info.h/.cpp`)
```
위치: include/domain/entities/file_info.h
      src/domain/entities/file_info.cpp

책임:
- 파일 시스템 객체의 메타데이터 표현
- 파일 타입 관리 (Regular, Directory, SymbolicLink, Unknown)
- 불변 속성 제공

주요 메서드:
- getName(), getPath(): 파일 식별자
- getSize(), getModifiedTime(): 파일 속성
- isRegularFile(), isDirectory(), isSymbolicLink(): 타입 체크

설계 원칙:
- Value Object 성격 (불변성)
- 비즈니스 로직 없음 (순수 데이터)
```

#### SearchResult (`domain/entities/search_result.h/.cpp`)
```
위치: include/domain/entities/search_result.h
      src/domain/entities/search_result.cpp

책임:
- FileInfo 컬렉션 관리
- 검색 결과의 집합 표현
- 결과 정렬 및 접근 인터페이스

주요 메서드:
- add(FileInfo): 결과 추가
- sortByName(), sortBySize(), sortByDate(): 정렬
- size(): 결과 개수
- begin(), end(): 이터레이터 지원

설계 원칙:
- Entity (식별자: 검색 세션)
- STL 컨테이너 인터페이스 제공
```

#### SemanticResult (`domain/entities/semantic_result.h`)
```
위치: include/domain/entities/semantic_result.h

책임:
- 의미론적 검색 결과 표현
- FileInfo + relevance score + matched chunks

주요 필드:
- fileInfo: 파일 정보
- relevanceScore: 유사도 점수
- matchedChunks: 매칭된 텍스트 조각

설계 원칙:
- Value Object (불변)
- 검색 결과의 확장된 형태
```

### 1.2 Value Objects

비즈니스 개념을 표현하는 불변 객체

#### SearchCriteria (`domain/value_objects/search_criteria.h/.cpp`)
```
위치: include/domain/value_objects/search_criteria.h
      src/domain/value_objects/search_criteria.cpp

책임:
- 검색 조건을 캡슐화하는 값 객체
- 검색 필터 정의 (이름, 경로, 확장자, 크기, 날짜)
- 검색 옵션 (대소문자, regex, 내용 검색)

주요 메서드:
- setNamePattern(), setPathPattern(): 패턴 설정
- addExtension(): 확장자 추가
- setMinSize(), setMaxSize(): 크기 범위
- setModifiedAfter(), setModifiedBefore(): 날짜 범위
- setUseRegex(), setIgnoreCase(): 모드 설정
- setContentPattern(): 내용 검색

설계 원칙:
- Value Object (동등성 비교는 모든 필드)
- Fluent Interface (Builder Pattern)
- 불변성은 선택적 (사용자 편의성)
```

---

## 2. Application Layer

**책임**: 애플리케이션의 비즈니스 룰, Use Cases
**의존성**: Domain Layer만 의존

### 2.1 Ports (Hexagonal Architecture)

외부 의존성을 추상화하는 인터페이스

#### IEmbeddingProvider (`application/ports/i_embedding_provider.h`)
```
위치: include/application/ports/i_embedding_provider.h

책임:
- 텍스트를 벡터로 변환하는 인터페이스 정의
- 구현체 독립적 추상화 (OpenAI, Local Model 등)

주요 메서드:
- generateEmbedding(text) → vector<float>
- batchGenerate(texts) → vector<vector<float>>
- getDimension() → size_t

설계 원칙:
- Dependency Inversion Principle (DIP)
- Port (출력 포트)
```

#### IVectorStore (`application/ports/i_vector_store.h`)
```
위치: include/application/ports/i_vector_store.h

책임:
- 벡터 저장소 인터페이스 정의
- 벡터 추가, 검색, 저장/로드 추상화

주요 메서드:
- add(id, vector, metadata): 벡터 추가
- search(queryVector, topK) → vector<SearchResult>: 유사도 검색
- remove(id): 벡터 제거
- save(path), load(path): 영속성
- size(), clear(): 관리 메서드

설계 원칙:
- Dependency Inversion Principle (DIP)
- Port (출력 포트)
- 구현체: FAISS, ChromaDB 등
```

### 2.2 Services

도메인 로직을 지원하는 애플리케이션 서비스

#### PatternMatcher (`application/services/pattern_matcher.h/.cpp`)
```
위치: include/application/services/pattern_matcher.h
      src/application/services/pattern_matcher.cpp

책임:
- 패턴 매칭 알고리즘 제공
- 와일드카드, 정규식, 정확 매칭 지원
- 대소문자 구분/비구분 처리

주요 메서드:
- matchWildcard(text, pattern, ignoreCase): 와일드카드 매칭
- matchRegex(text, pattern): 정규식 매칭
- matchExact(text, pattern, ignoreCase): 정확한 매칭

설계 원칙:
- Static Utility Methods (상태 없음)
- 단일 책임 (패턴 매칭만)
```

#### ContentSearcher (`application/services/content_searcher.h/.cpp`)
```
위치: include/application/services/content_searcher.h
      src/application/services/content_searcher.cpp

책임:
- 파일 내용 검색
- 텍스트 파일 판별 (바이너리 제외)
- 파일 내용에서 패턴 검색

주요 메서드:
- searchInFile(filepath, pattern, useRegex, ignoreCase): 검색 수행
- isTextFile(filepath): 텍스트 파일 여부 판별

알고리즘:
- Null byte 감지로 바이너리 파일 즉시 거부
- 첫 4KB 샘플링으로 텍스트 파일 판별
- 라인 단위 검색 (메모리 효율)
- 10MB 파일 크기 제한

설계 원칙:
- Domain Service (도메인 로직 지원)
- Static Utility Methods
```

#### IgnorePatterns (`application/services/ignore_patterns.h/.cpp`)
```
위치: include/application/services/ignore_patterns.h
      src/application/services/ignore_patterns.cpp

책임:
- .gitignore 스타일 패턴 관리
- 파일/디렉토리 제외 판단

주요 메서드:
- addPattern(pattern): 패턴 추가
- loadFromFile(filepath): .gitignore 파일 로드
- shouldIgnore(path, isDirectory): 제외 여부 판단
- size(): 패턴 개수

패턴 형식:
- `node_modules/`: 디렉토리 제외
- `*.tmp`: 특정 확장자 제외
- `build*`: 접두사 패턴

설계 원칙:
- Domain Service
- SRP: 패턴 관리만 담당
```

### 2.3 Use Cases

애플리케이션의 핵심 비즈니스 로직

#### FileScanner (`application/use_cases/file_scanner.h/.cpp`)
```
위치: include/application/use_cases/file_scanner.h
      src/application/use_cases/file_scanner.cpp

책임:
- 파일 시스템 스캔 및 검색 (핵심 Use Case)
- SearchCriteria에 따른 파일 필터링
- 재귀/비재귀 디렉토리 순회
- IgnorePatterns, ContentSearcher 통합

주요 메서드:
- scan(directory): 단일 디렉토리 스캔
- scanRecursive(directory): 재귀 스캔
- search(directory, criteria): 조건부 검색
- setFollowSymlinks(bool): 심볼릭 링크 추적
- setIgnorePatterns(patterns): 제외 패턴 설정
- setThreadPool(pool): 병렬 스캔 활성화

내부 메서드:
- matchesCriteria(fileInfo): 필터링 로직
- shouldProcess(path): 제외 판단

의존성:
- Domain: FileInfo, SearchResult, SearchCriteria
- Application Services: PatternMatcher, ContentSearcher, IgnorePatterns
- Infrastructure: ThreadPool (선택적)

설계 원칙:
- Use Case (애플리케이션 비즈니스 룰)
- Dependency Injection (ThreadPool, IgnorePatterns)
- Template Method Pattern (scan 공통 로직)
```

#### SemanticSearcher (`application/use_cases/semantic_searcher.h/.cpp`)
```
위치: include/application/use_cases/semantic_searcher.h
      src/application/use_cases/semantic_searcher.cpp

책임:
- 의미론적 검색 (LLM 기반)
- 파일 인덱싱 (텍스트 → 벡터)
- 자연어 쿼리 검색

주요 메서드:
- indexFile(filepath): 파일 인덱싱
- indexDirectory(dirpath): 디렉토리 인덱싱
- search(query, topK): 검색 수행
- searchSimilar(filepath, topK): 유사 파일 찾기
- clear(), size(): 인덱스 관리

의존성:
- Domain: SemanticResult, FileInfo
- Application Ports: IEmbeddingProvider, IVectorStore

설계 원칙:
- Use Case
- Dependency Inversion (Ports 사용)
- Chunking Strategy (1000 chars, 200 overlap)
```

---

## 3. Interface Adapters Layer

**책임**: 외부 인터페이스와 Use Cases 연결
**의존성**: Application Layer, Domain Layer

### 3.1 CLI Adapter

#### CommandLineParser (`adapters/cli/command_line_parser.h/.cpp`)
```
위치: include/adapters/cli/command_line_parser.h
      src/adapters/cli/command_line_parser.cpp

책임:
- 명령줄 인자 파싱
- ApplicationConfig DTO 생성
- ConfigFile과 CLI 인자 병합
- 옵션 검증 및 에러 처리

주요 구성:
- ApplicationConfig: 모든 설정을 담는 DTO
  * 검색: recursive, followLinks, maxDepth
  * 성능: threadCount
  * 출력: outputFormat, useColor, verbosity
  * 로깅: logFile
  * SearchCriteria: 검색 필터

주요 메서드:
- parse(argc, argv): 기본 파싱
- parse(argc, argv, defaultConfig): 설정 병합 파싱
- parseOption(option, value): 개별 옵션 처리
- validateConfig(config): 설정 검증
- printUsage(): 도움말 출력

지원 옵션:
- 검색: -n/--name, -e/--ext, -p/--path, -c/--content
- 필터: -i/--ignore-case, -x/--regex, -f/--files-only, -D/--dirs-only
- 크기: --min-size, --max-size
- 순회: -r/--recursive, -d/--max-depth, -l/--follow-links, --ignore
- 성능: -j/--threads
- 출력: --format, --color/--no-color
- 로깅: -v/--verbose, --log-file

설계 원칙:
- Adapter (CLI → Application)
- DTO Pattern (ApplicationConfig)
- SRP: CLI 파싱만 담당
```

### 3.2 Presenters

#### OutputFormatter (`adapters/presenters/output_formatter.h/.cpp`)
```
위치: include/adapters/presenters/output_formatter.h
      src/adapters/presenters/output_formatter.cpp

책임:
- 검색 결과 출력 형식 관리
- 3가지 출력 형식 (Default, Detailed, JSON)
- ANSI 컬러 관리

주요 메서드:
- print(searchResult): 결과 출력 (형식에 따라 분기)
- setFormat(format): 출력 형식 설정
- setUseColor(bool): 컬러 활성화

출력 형식:
- Default: 간결한 리스트 (`[TYPE] SIZE PATH`)
- Detailed: 전체 메타데이터 (경로, 타입, 크기, 날짜)
- JSON: 구조화된 출력 (`{"count": N, "files": [...]}`)

컬러 지원:
- 8가지 ANSI 색상 (Reset, Red, Green, Blue, Yellow, Cyan, Magenta, Gray)
- --color/--no-color 옵션

설계 원칙:
- Presenter (Application → View)
- Strategy Pattern (출력 형식)
```

---

## 4. Infrastructure Layer (가장 바깥)

**책임**: 프레임워크, 드라이버, 외부 도구
**의존성**: 모든 레이어에 접근 가능

### 4.1 Logging

#### Logger (`infrastructure/logging/logger.h/.cpp`)
```
위치: include/infrastructure/logging/logger.h
      src/infrastructure/logging/logger.cpp

책임:
- 애플리케이션 전역 로깅
- 로그 레벨 필터링 (DEBUG/INFO/WARN/ERROR/NONE)
- 콘솔 및 파일 출력
- 타임스탬프 포맷팅
- 스레드 안전 로깅

주요 메서드:
- instance(): 싱글톤 인스턴스
- setLevel(level), getLevel(): 로그 레벨 관리
- setLogFile(path): 로그 파일 설정
- debug(), info(), warn(), error(): 로그 출력
- flush(), close(): 버퍼 관리

로그 형식:
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message

설계 원칙:
- Singleton (전역 접근)
- Thread-safe (std::mutex)
- Cross-cutting concern (Infrastructure)
```

### 4.2 Configuration

#### ConfigFile (`infrastructure/config/config_file.h/.cpp`)
```
위치: include/infrastructure/config/config_file.h
      src/infrastructure/config/config_file.cpp

책임:
- .findmyfilesrc 파일 로드 및 파싱
- INI 스타일 설정 파일 처리
- 다중 위치 탐색 (현재 디렉토리, 홈, XDG config)
- ApplicationConfig 생성

주요 메서드:
- load(filepath): 특정 파일 로드
- loadDefault(): 기본 위치 탐색
- getDefaultConfig(): [default] 섹션 반환
- getSavedSearch(name): 저장된 검색 반환

파싱 기능:
- INI 섹션 파싱 ([section])
- Boolean: true/yes/on/1, false/no/off/0
- 인라인 주석 (#, ;)
- 쉼표 구분 확장자 목록

설계 원칙:
- Infrastructure (설정 파일 처리)
- Factory Method (ApplicationConfig 생성)
```

### 4.3 Threading

#### ThreadPool (`infrastructure/threading/thread_pool.h/.cpp`)
```
위치: include/infrastructure/threading/thread_pool.h
      src/infrastructure/threading/thread_pool.cpp

책임:
- 워커 스레드 풀 관리
- 작업 큐 및 스케줄링
- std::future 기반 결과 반환

주요 메서드:
- submit(task): 태스크 제출 (템플릿, future 반환)
- wait(): 모든 작업 완료 대기
- size(), pendingTasks(): 상태 조회

구현:
- std::packaged_task로 결과 처리
- mutex/condition_variable로 동기화
- std::atomic으로 활성 작업 추적
- RAII 기반 스레드 관리

설계 원칙:
- Infrastructure (병렬 실행 도구)
- Thread-safe
```

### 4.4 Providers (Infrastructure Adapters)

#### MockEmbeddingProvider (`infrastructure/providers/mock_embedding_provider.h`)
```
위치: include/infrastructure/providers/mock_embedding_provider.h

책임:
- IEmbeddingProvider 테스트 구현
- 결정론적 벡터 생성 (해시 기반)

설계 원칙:
- Adapter (Port의 Concrete 구현)
- Mock Object (테스트용)
```

#### MockVectorStore (`infrastructure/providers/mock_vector_store.h`)
```
위치: include/infrastructure/providers/mock_vector_store.h

책임:
- IVectorStore 테스트 구현
- In-memory map 기반 저장
- Cosine similarity 검색

설계 원칙:
- Adapter (Port의 Concrete 구현)
- Mock Object (테스트용)
```

---

## Proposed Clean Architecture Directory Structure

### 새로운 폴더 구조

```
find_my_files/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── CONTRIBUTING.md
├── install.sh
│
├── .github/
│   ├── workflows/ci.yml
│   └── instructions/
│       ├── README.instructions.md
│       ├── workflow.instructions.md
│       ├── tdd-cycle.instructions.md
│       ├── commit-style.instructions.md
│       ├── architecture.instructions.md
│       ├── testing.instructions.md
│       └── cpp-standards.instructions.md
│
├── docs/
│   ├── architecture.md              # 본 문서
│   ├── todos.md                     # Phase 기반 TODO
│   └── requirements.md              # 기능 요구사항
│
├── examples/
│   └── .findmyfilesrc
│
├── include/                          # Public headers
│   ├── domain/                       # Domain Layer
│   │   ├── entities/
│   │   │   ├── file_info.h
│   │   │   ├── search_result.h
│   │   │   └── semantic_result.h
│   │   └── value_objects/
│   │       └── search_criteria.h
│   │
│   ├── application/                  # Application Layer
│   │   ├── ports/                    # Port interfaces
│   │   │   ├── i_embedding_provider.h
│   │   │   └── i_vector_store.h
│   │   ├── services/                 # Domain services
│   │   │   ├── pattern_matcher.h
│   │   │   ├── content_searcher.h
│   │   │   └── ignore_patterns.h
│   │   └── use_cases/                # Use cases
│   │       ├── file_scanner.h
│   │       └── semantic_searcher.h
│   │
│   ├── adapters/                     # Interface Adapters Layer
│   │   ├── cli/
│   │   │   └── command_line_parser.h
│   │   └── presenters/
│   │       └── output_formatter.h
│   │
│   └── infrastructure/               # Infrastructure Layer
│       ├── logging/
│       │   └── logger.h
│       ├── config/
│       │   └── config_file.h
│       ├── threading/
│       │   └── thread_pool.h
│       └── providers/
│           ├── mock_embedding_provider.h
│           └── mock_vector_store.h
│
├── src/                              # Implementation files
│   ├── domain/
│   │   ├── entities/
│   │   │   ├── file_info.cpp
│   │   │   └── search_result.cpp
│   │   └── value_objects/
│   │       └── search_criteria.cpp
│   │
│   ├── application/
│   │   ├── services/
│   │   │   ├── pattern_matcher.cpp
│   │   │   ├── content_searcher.cpp
│   │   │   └── ignore_patterns.cpp
│   │   └── use_cases/
│   │       ├── file_scanner.cpp
│   │       └── semantic_searcher.cpp
│   │
│   ├── adapters/
│   │   ├── cli/
│   │   │   └── command_line_parser.cpp
│   │   └── presenters/
│   │       └── output_formatter.cpp
│   │
│   ├── infrastructure/
│   │   ├── logging/
│   │   │   └── logger.cpp
│   │   ├── config/
│   │   │   └── config_file.cpp
│   │   └── threading/
│   │       └── thread_pool.cpp
│   │
│   └── main.cpp                      # Entry point (의존성 조립)
│
├── tests/                            # Unit tests
│   ├── domain/
│   │   ├── entities/
│   │   │   ├── test_file_info.cpp
│   │   │   └── test_search_result.cpp
│   │   └── value_objects/
│   │       └── test_search_criteria.cpp
│   │
│   ├── application/
│   │   ├── services/
│   │   │   ├── test_pattern_matcher.cpp
│   │   │   ├── test_content_searcher.cpp
│   │   │   └── test_ignore_patterns.cpp
│   │   └── use_cases/
│   │       ├── test_file_scanner.cpp
│   │       ├── test_semantic_interfaces.cpp
│   │       └── test_semantic_searcher.cpp
│   │
│   ├── adapters/
│   │   ├── test_command_line_parser.cpp
│   │   └── test_output_formatter.cpp
│   │
│   └── infrastructure/
│       ├── test_logger.cpp
│       ├── test_config_file.cpp
│       └── test_thread_pool.cpp
│
└── test/                             # Integration tests
    └── integrationtest/
        ├── run_all.sh
        ├── uc_regex_search.sh
        ├── uc_content_search.sh
        ├── uc_ignore_patterns.sh
        ├── uc_parallel_scan.sh
        ├── uc_output_formats.sh
        ├── uc_logging.sh
        └── uc_config_file.sh
```

---

## Dependency Flow

### Clean Architecture Dependency Graph

```
main.cpp (최외곽 - 의존성 조립)
    ↓
Infrastructure Layer
    ├─ Logger (싱글톤)
    ├─ ConfigFile → ApplicationConfig
    ├─ ThreadPool
    └─ Providers (MockEmbeddingProvider, MockVectorStore)
    ↓
Adapters Layer
    ├─ CommandLineParser → ApplicationConfig (DTO)
    │   └─ SearchCriteria (Domain VO)
    └─ OutputFormatter
        └─ SearchResult (Domain Entity)
    ↓
Application Layer
    ├─ Use Cases
    │   ├─ FileScanner
    │   │   ├─ Services: PatternMatcher, ContentSearcher, IgnorePatterns
    │   │   ├─ Domain: FileInfo, SearchResult, SearchCriteria
    │   │   └─ Infrastructure: ThreadPool (optional, DI)
    │   └─ SemanticSearcher
    │       ├─ Ports: IEmbeddingProvider, IVectorStore
    │       └─ Domain: SemanticResult, FileInfo
    ├─ Services
    │   ├─ PatternMatcher (stateless utility)
    │   ├─ ContentSearcher (stateless utility)
    │   └─ IgnorePatterns (state: patterns)
    └─ Ports (interfaces only)
        ├─ IEmbeddingProvider
        └─ IVectorStore
    ↓
Domain Layer (의존성 없음)
    ├─ Entities
    │   ├─ FileInfo
    │   ├─ SearchResult
    │   └─ SemanticResult
    └─ Value Objects
        └─ SearchCriteria
```

### Key Dependency Inversions

1. **FileScanner → ThreadPool**
   - FileScanner는 ThreadPool을 직접 의존하지 않음
   - 생성자/setter로 주입 받음 (DI)
   - ThreadPool 없이도 동작 가능 (순차 스캔)

2. **SemanticSearcher → IEmbeddingProvider/IVectorStore**
   - 인터페이스(Port)에만 의존
   - 구현체는 Infrastructure Layer에 위치
   - mock 구현으로 테스트 가능

3. **Use Cases → Domain**
   - Use Cases는 Domain Entities/VOs만 조작
   - Domain은 Use Cases를 알 수 없음

---

## Data Flow

### Traditional Search Flow

```
1. main()
   ├─ ConfigFile::loadDefault() → defaultConfig (옵션)
   ├─ CommandLineParser::parse(argc, argv, defaultConfig)
   │   └─ ApplicationConfig 생성
   │       └─ SearchCriteria 포함
   └─ Logger::instance().setLevel(config.verbosity)

2. FileScanner 초기화 (Use Case)
   ├─ SearchCriteria 설정
   ├─ IgnorePatterns 로드 (옵션)
   └─ ThreadPool 주입 (DI, 옵션)

3. FileScanner::search(directory, criteria)
   ├─ scan() / scanRecursive()
   │   ├─ shouldProcess() → IgnorePatterns 체크
   │   └─ matchesCriteria() → 필터링
   │       ├─ PatternMatcher::matchWildcard/matchRegex()
   │       ├─ 크기/날짜 범위 체크
   │       └─ ContentSearcher::searchInFile() (옵션)
   └─ SearchResult 반환

4. 정렬 (옵션)
   └─ SearchResult::sortByName/Size/Date()

5. 출력
   └─ OutputFormatter::print(searchResult)
```

### Semantic Search Flow (Phase 10)

```
1. main() → semantic search 옵션 감지

2. SemanticSearcher 초기화 (Use Case)
   ├─ EmbeddingProvider 주입 (DI, Port)
   │   └─ 구현: OpenAIProvider, LocalModelProvider, MockEmbeddingProvider
   └─ VectorStore 주입 (DI, Port)
       └─ 구현: FAISSStore, ChromaDBStore, MockVectorStore

3. Index 확인
   ├─ VectorStore::load(.fmf_index/) → 실패 시 인덱싱
   └─ 오래된 파일 확인 → 재인덱싱

4. SemanticSearcher::search(query, topK)
   ├─ EmbeddingProvider::generateEmbedding(query) → queryVector
   ├─ VectorStore::search(queryVector, topK) → vector<SemanticResult>
   └─ (옵션) Hybrid: ContentSearcher 결과와 병합

5. 출력
   └─ OutputFormatter::print(results with relevance score)
```

---

## Multimodal Document Embedding Pipeline (추가)

### 목적
Find My File의 의미 검색은 **다양한 파일 포맷(PDF/PPT/DOCX/HWP/이미지/영상)**을
**하나의 의미 공간(Shared Semantic Space)**에서 검색하는 것을 목표로 한다.
따라서 “파일을 그대로 임베딩”하는 것이 아니라,
**파일을 텍스트(또는 텍스트 설명)로 변환 → 단일 텍스트 임베딩 모델**로 통일한다.

### 핵심 원칙 요약
- ✅ **임베딩 모델은 1개**: 모든 모달리티를 텍스트로 정규화 후 동일 모델 사용
- ✅ **검색 일관성 보장**: 같은 벡터 공간에서 유사도 계산
- ✅ **추가 학습 없음**: Hugging Face의 pretrained 모델만 사용
- ❌ **파일 직접 임베딩은 불가**: 항상 파싱/요약/설명 단계 필요

### 처리 전략 (단계별)

#### 1) 파일 파싱 & 텍스트 추출
- **PDF / PPT / DOCX / HWP**
  - 본문 텍스트 추출
  - 구조 정보(페이지/슬라이드/섹션/헤딩) 보존
  - 표/이미지는 별도 처리 대상으로 분리
- **이미지**
  - VLM/Caption 모델 또는 OCR로 **텍스트 설명** 생성
- **영상/오디오**
  - STT(예: Whisper)로 텍스트 변환
  - 필요 시 요약 생성

#### 2) Chunking & 요약
- **PDF**: 페이지/섹션 단위
- **PPT**: 슬라이드 단위(제목 + 본문 + 노트)
- **Word/HWP**: Heading 기반 섹션/문단 단위
- **이미지/도표**: 설명 텍스트 + 태그

> 핵심: “문서”를 임베딩하는 것이 아니라 **문서의 의미 단위**를 임베딩한다.

#### 3) 단일 텍스트 임베딩
모든 텍스트/설명은 **하나의 Text Embedding 모델**로 벡터화한다.
이 방식으로 파일 포맷이 달라도 **검색 결과의 일관성을 유지**한다.

### 전체 파이프라인 흐름

```
[파일 (PDF/PPT/DOCX/HWP/IMG/VIDEO)]
        ↓
[포맷별 파서/추출기]
        ↓
[텍스트 + 이미지 설명 + 구조 메타데이터]
        ↓
[Chunking & 요약]
        ↓
[단일 Text Embedding 모델]
        ↓
[Vector Index / Vector DB]
```

### 왜 이 구조가 적합한가?
- **모델 교체/버저닝이 단순**
- **메타데이터와 함께 저장**해 재인덱싱 전략 수립 가능
- **설명 기반 검색 결과**로 이해도/설명 가능성 향상
- 파일 타입 추가 시 **파서만 추가**하면 확장 가능

### 운영 설계 포인트
- **Descriptor versioning 필수**
  - embedding_version, encoder_type, dimension, normalization 메타데이터 저장
  - 모델 변경 시 재인덱싱 전제
- **LLM/VLM의 역할 분리**
  - LLM/VLM: 설명/요약/쿼리 확장
  - Embedding 모델: 검색 전용 벡터 생성

---

## Design Patterns

### 1. Clean Architecture Patterns

#### Dependency Inversion Principle (DIP)
- **Ports**: IEmbeddingProvider, IVectorStore
- **Adapters**: MockEmbeddingProvider, OpenAIProvider, FAISSStore

#### Hexagonal Architecture (Ports & Adapters)
- **Ports**: Application Layer의 인터페이스
- **Adapters**: Infrastructure Layer의 구현체

### 2. Domain Patterns

#### Entity
- FileInfo, SearchResult: 식별자를 가진 비즈니스 객체

#### Value Object
- SearchCriteria, SemanticResult: 동등성으로 비교되는 불변 객체

### 3. Application Patterns

#### Use Case
- FileScanner, SemanticSearcher: 애플리케이션 비즈니스 룰

#### Service
- PatternMatcher, ContentSearcher, IgnorePatterns: 도메인 로직 지원

### 4. Infrastructure Patterns

#### Singleton
- Logger: 전역 접근 가능한 단일 인스턴스

#### DTO (Data Transfer Object)
- ApplicationConfig: 레이어 간 데이터 전달

#### Strategy
- PatternMatcher: 다양한 매칭 전략
- OutputFormatter: 다양한 출력 형식

#### Builder
- SearchCriteria: Fluent Interface

---

## Technology Stack

### Core Technologies
- **Language**: C++17
- **Build System**: CMake 3.14+
- **Standard Library**: STL (std::filesystem, std::regex, std::optional)

### Testing
- **Framework**: Google Test (gtest)
- **Unit Tests**: 114 tests (Domain, Application, Adapters, Infrastructure)
- **Integration Tests**: 40 tests (bash scripts)

### Development Tools
- **Code Style**: clang-format
- **Static Analysis**: clang-tidy, cpplint
- **Compiler Flags**: `-Wall -Wextra -Wpedantic -Werror`

---

## Testing Strategy

### Unit Tests by Layer

#### Domain Layer (3 test files)
- test_file_info.cpp (6 tests): FileInfo 엔티티
- test_search_result.cpp: SearchResult 엔티티
- test_search_criteria.cpp (9 tests): SearchCriteria 값 객체

#### Application Layer (7 test files)
- test_pattern_matcher.cpp (8 tests): PatternMatcher 서비스
- test_content_searcher.cpp (6 tests): ContentSearcher 서비스
- test_ignore_patterns.cpp (10 tests): IgnorePatterns 서비스
- test_file_scanner.cpp (6 tests): FileScanner Use Case
- test_semantic_interfaces.cpp (11 tests): Ports & Mock implementations
- test_semantic_searcher.cpp (5 tests): SemanticSearcher Use Case

#### Adapters Layer (2 test files)
- test_command_line_parser.cpp: CommandLineParser CLI 어댑터
- test_output_formatter.cpp (12 tests): OutputFormatter Presenter

#### Infrastructure Layer (3 test files)
- test_logger.cpp (12 tests): Logger 싱글톤
- test_config_file.cpp (17 tests): ConfigFile 파서
- test_thread_pool.cpp (12 tests): ThreadPool 병렬 실행

**Total: 114 unit tests**

### Integration Tests (7 test suites, 40 tests)
- uc_regex_search.sh (3 tests)
- uc_content_search.sh (4 tests)
- uc_ignore_patterns.sh (4 tests)
- uc_parallel_scan.sh (4 tests)
- uc_output_formats.sh (7 tests)
- uc_logging.sh (10 tests)
- uc_config_file.sh (8 tests)

---

## Migration Plan (Phase 11 - Architecture Refactoring)

### 목표
현재 flat 구조를 Clean Architecture 기반 계층 구조로 리팩터링

### 작업 범위
1. 폴더 구조 재편성
2. CMakeLists.txt 업데이트 (include path 변경)
3. #include 경로 수정
4. 테스트 파일 이동 및 재편성
5. 문서 업데이트

### 이동 계획

#### Phase 11.1: Domain Layer 이동
```bash
# Entities
include/file_info.h → include/domain/entities/file_info.h
include/search_result.h → include/domain/entities/search_result.h
include/semantic_result.h → include/domain/entities/semantic_result.h
src/file_info.cpp → src/domain/entities/file_info.cpp
src/search_result.cpp → src/domain/entities/search_result.cpp

# Value Objects
include/search_criteria.h → include/domain/value_objects/search_criteria.h
src/search_criteria.cpp → src/domain/value_objects/search_criteria.cpp
```

#### Phase 11.2: Application Layer 이동
```bash
# Ports
include/i_embedding_provider.h → include/application/ports/i_embedding_provider.h
include/i_vector_store.h → include/application/ports/i_vector_store.h

# Services
include/pattern_matcher.h → include/application/services/pattern_matcher.h
include/content_searcher.h → include/application/services/content_searcher.h
include/ignore_patterns.h → include/application/services/ignore_patterns.h
src/pattern_matcher.cpp → src/application/services/pattern_matcher.cpp
src/content_searcher.cpp → src/application/services/content_searcher.cpp
src/ignore_patterns.cpp → src/application/services/ignore_patterns.cpp

# Use Cases
include/file_scanner.h → include/application/use_cases/file_scanner.h
include/semantic_searcher.h → include/application/use_cases/semantic_searcher.h
src/file_scanner.cpp → src/application/use_cases/file_scanner.cpp
src/semantic_searcher.cpp → src/application/use_cases/semantic_searcher.cpp
```

#### Phase 11.3: Adapters Layer 이동
```bash
# CLI
include/command_line_parser.h → include/adapters/cli/command_line_parser.h
src/command_line_parser.cpp → src/adapters/cli/command_line_parser.cpp

# Presenters
include/output_formatter.h → include/adapters/presenters/output_formatter.h
src/output_formatter.cpp → src/adapters/presenters/output_formatter.cpp
```

#### Phase 11.4: Infrastructure Layer 이동
```bash
# Logging
include/logger.h → include/infrastructure/logging/logger.h
src/logger.cpp → src/infrastructure/logging/logger.cpp

# Config
include/config_file.h → include/infrastructure/config/config_file.h
src/config_file.cpp → src/infrastructure/config/config_file.cpp

# Threading
include/thread_pool.h → include/infrastructure/threading/thread_pool.h
src/thread_pool.cpp → src/infrastructure/threading/thread_pool.cpp

# Providers
include/mock_embedding_provider.h → include/infrastructure/providers/mock_embedding_provider.h
include/mock_vector_store.h → include/infrastructure/providers/mock_vector_store.h
```

#### Phase 11.5: Tests 이동
```bash
# Domain tests
tests/test_file_info.cpp → tests/domain/entities/test_file_info.cpp
tests/test_search_result.cpp → tests/domain/entities/test_search_result.cpp
tests/test_search_criteria.cpp → tests/domain/value_objects/test_search_criteria.cpp

# Application tests
tests/test_pattern_matcher.cpp → tests/application/services/test_pattern_matcher.cpp
tests/test_content_searcher.cpp → tests/application/services/test_content_searcher.cpp
tests/test_ignore_patterns.cpp → tests/application/services/test_ignore_patterns.cpp
tests/test_file_scanner.cpp → tests/application/use_cases/test_file_scanner.cpp
tests/test_semantic_interfaces.cpp → tests/application/use_cases/test_semantic_interfaces.cpp
tests/test_semantic_searcher.cpp → tests/application/use_cases/test_semantic_searcher.cpp

# Adapters tests
tests/test_command_line_parser.cpp → tests/adapters/test_command_line_parser.cpp
tests/test_output_formatter.cpp → tests/adapters/test_output_formatter.cpp

# Infrastructure tests
tests/test_logger.cpp → tests/infrastructure/test_logger.cpp
tests/test_config_file.cpp → tests/infrastructure/test_config_file.cpp
tests/test_thread_pool.cpp → tests/infrastructure/test_thread_pool.cpp
```

#### Phase 11.6: Build System 업데이트
```cmake
# CMakeLists.txt 수정
include_directories(include)  # 기존 유지 (include/ 루트만 지정)

# 소스 파일 경로 업데이트
src/domain/entities/file_info.cpp
src/domain/entities/search_result.cpp
src/domain/value_objects/search_criteria.cpp
src/application/services/pattern_matcher.cpp
src/application/services/content_searcher.cpp
src/application/services/ignore_patterns.cpp
src/application/use_cases/file_scanner.cpp
src/application/use_cases/semantic_searcher.cpp
src/adapters/cli/command_line_parser.cpp
src/adapters/presenters/output_formatter.cpp
src/infrastructure/logging/logger.cpp
src/infrastructure/config/config_file.cpp
src/infrastructure/threading/thread_pool.cpp
src/main.cpp
```

#### Phase 11.7: Include 경로 업데이트
```cpp
// 모든 소스 파일에서 #include 수정

// Before
#include "file_info.h"
#include "search_criteria.h"
#include "file_scanner.h"

// After
#include "domain/entities/file_info.h"
#include "domain/value_objects/search_criteria.h"
#include "application/use_cases/file_scanner.h"
```

### 검증 단계
1. 각 Phase 완료 후 빌드 성공 확인
2. 모든 단위 테스트 통과 확인 (114개)
3. 모든 통합 테스트 통과 확인 (40개)
4. clang-format / clang-tidy / cpplint 통과 확인

---

## Performance Considerations

### Current Implementation
- **Multi-threaded**: ThreadPool-based parallel directory scanning
- **Thread-safe**: Mutex-protected SearchResult
- **Memory Efficient**: 라인 단위 파일 읽기, 10MB 제한
- **Optimized Pattern Matching**: 재귀적 와일드카드 알고리즘
- **Early Rejection**: Null byte 검사로 바이너리 파일 제외

### Performance Characteristics
- **Parallel Speedup**: ~3x faster on large directory trees
- **Thread Safety Overhead**: Minimal (mutex only during result insertion)
- **Scalability**: Automatic subdirectory parallelization

---

## Security Considerations

- **Path Traversal**: std::filesystem::canonical() 사용
- **Symbolic Link Loops**: 기본적으로 심볼릭 링크 추적 비활성화
- **Resource Limits**: 파일 크기 제한 (10MB), 재귀 깊이 제한 가능
- **Input Validation**: Regex 패턴 검증, 파일 존재 여부 확인

---

## GUI Architecture (Future Work)

### Overview

Find My Files GUI는 **Everything** 스타일의 빠르고 직관적인 데스크톱 검색 인터페이스를 제공합니다.
CLI와 동일한 Domain/Application 레이어를 공유하며, 오직 Adapters/Infrastructure 레이어만 GUI 전용으로 구현됩니다.

### Design Principles

**Everything UI 참고 특징:**
- **실시간 검색**: 타이핑 즉시 결과 업데이트 (debounce 200ms)
- **미니멀 인터페이스**: 깔끔한 단일 창 UI
- **빠른 반응성**: 비동기 검색, UI 스레드 블로킹 없음
- **강력한 필터**: 파일 타입, 크기, 날짜, 경로 등 다중 필터 지원
- **컨텍스트 통합**: 더블 클릭 열기, 우클릭 메뉴, 드래그앤드롭
- **키보드 네비게이션**: 전체 UI를 키보드만으로 조작 가능

### Framework Selection

#### Recommended: Qt 6 (LTS)
```
선택 이유:
✅ 크로스 플랫폼 (Linux, macOS, Windows)
✅ 성능 최적화된 위젯 (QTreeView, QTableView)
✅ 비동기 UI 업데이트 (Signals & Slots)
✅ CMake 통합 용이
✅ 풍부한 문서 및 커뮤니티
✅ C++ 네이티브, Clean Architecture 적용 가능

대안:
- GTK4: Linux 네이티브, 경량
- wxWidgets: 네이티브 룩앤필
- Dear ImGui: 게임 UI 스타일 (즉시 모드)
```

### Clean Architecture Mapping

```
┌────────────────────────────────────────────────┐
│  Infrastructure Layer                          │
│  • QtApplication (main event loop)             │
│  • QtStyleManager (테마 관리)                   │
│  • QtSettingsManager (QSettings)               │
└────────────────────────────────────────────────┘
          ↓ depends on
┌────────────────────────────────────────────────┐
│  Interface Adapters Layer (GUI)                │
│  • MainWindow (View)                           │
│  • SearchPresenter (Presenter)                 │
│  • FileListViewModel (ViewModel)               │
│  • FilterPanelController (Controller)          │
└────────────────────────────────────────────────┘
          ↓ depends on
┌────────────────────────────────────────────────┐
│  Application Layer (공유)                       │
│  • FileScanner, SemanticSearcher               │
│  • PatternMatcher, ContentSearcher             │
│  • IgnorePatterns                              │
└────────────────────────────────────────────────┘
          ↓ depends on
┌────────────────────────────────────────────────┐
│  Domain Layer (공유)                            │
│  • FileInfo, SearchResult, SearchCriteria      │
└────────────────────────────────────────────────┘
```

**핵심**: CLI와 GUI는 **동일한 Application/Domain 레이어를 공유**합니다.

### Component Design (MVP Pattern)

#### 1. MainWindow (View)

```cpp
위치: include/adapters/gui/main_window.h
      src/adapters/gui/main_window.cpp

책임:
- 메인 UI 렌더링 (QMainWindow)
- 사용자 입력 수신 (검색어, 버튼 클릭)
- Presenter에게 이벤트 전달
- 프리젠테이션 로직 없음 (순수 View)

주요 구성:
- QLineEdit: 검색어 입력 (Everything 스타일)
- QTableView: 검색 결과 리스트 (이름, 경로, 크기, 날짜)
- QToolBar: 필터 버튼 (파일만, 폴더만, 정규식, 대소문자)
- QStatusBar: 검색 통계 + 인덱싱 상태 (2개 섹션)
  * 왼쪽: 검색 결과 통계 (N개 파일 / 검색 시간)
  * 오른쪽: 인덱싱 상태 (진행률, 남은 파일 수, QProgressBar)
- QDockWidget: 고급 필터 패널 (크기, 날짜 범위, 확장자)

Qt Signals:
- searchTextChanged(QString): 검색어 변경
- fileDoubleClicked(QString): 파일 더블 클릭
- filterChanged(FilterOptions): 필터 옵션 변경
- contextMenuRequested(QPoint, QString): 우클릭 메뉴
- indexingRequested(): 수동 인덱싱 트리거

설계 원칙:
- Passive View (비즈니스 로직 없음)
- Qt Signals/Slots로 Presenter와 통신
- QAbstractTableModel 사용 (데이터 분리)
```

#### 2. SearchPresenter (Presenter)

```cpp
위치: include/adapters/gui/search_presenter.h
      src/adapters/gui/search_presenter.cpp

책임:
- View와 Application Layer 중재
- 사용자 입력을 SearchCriteria로 변환
- FileScanner/SemanticSearcher 호출
- SearchResult를 ViewModel로 변환
- 에러 처리 및 사용자 피드백

주요 메서드:
- onSearchTextChanged(QString): 검색 트리거
- onFilterChanged(FilterOptions): 필터 업데이트
- onFileOpen(QString): 파일 열기 로직
- onExportResults(): CSV/JSON 내보내기
- onStartIndexing(): 인덱싱 시작
- onIndexingProgress(int current, int total): 인덱싱 진행 상황 업데이트
- onIndexingComplete(): 인덱싱 완료 처리

비동기 처리:
- std::async 또는 QtConcurrent 사용
- UI 스레드 블로킹 방지
- 취소 가능한 검색 (std::future)
- 인덱싱 진행 상황 실시간 업데이트 (QTimer, 100ms 간격)

의존성:
- FileScanner (Application Layer)
- SemanticSearcher (Application Layer)
- OutputFormatter (공유 프리젠테이션 로직)
- MainWindow (View)

설계 원칙:
- Supervising Controller 패턴
- View 상태 관리
- Application Layer와 View 분리
```

#### 3. FileListViewModel (ViewModel)

```cpp
위치: include/adapters/gui/file_list_view_model.h
      src/adapters/gui/file_list_view_model.cpp

책임:
- SearchResult를 Qt Model로 변환 (QAbstractTableModel)
- 정렬, 필터링 로직 (QSortFilterProxyModel 통합)
- 컬럼 포맷팅 (크기, 날짜 표시 형식)

주요 메서드:
- rowCount(), columnCount(): 테이블 크기
- data(index, role): 셀 데이터 반환
- headerData(): 컬럼 헤더
- setSearchResult(SearchResult): 데이터 업데이트
- sort(column, order): 정렬
- getFileInfoAt(row): FileInfo 반환

Qt Roles:
- DisplayRole: 화면 표시 텍스트
- UserRole: 원본 FileInfo 객체
- DecorationRole: 파일 아이콘 (QFileIconProvider)
- ForegroundRole: 텍스트 색상 (확장자별 색상)

설계 원칙:
- Model-View 분리
- Qt Model/View 프레임워크 활용
- 대량 데이터 효율 처리 (가상화)
```

#### 4. FilterPanelController (Controller)

```cpp
위치: include/adapters/gui/filter_panel_controller.h
      src/adapters/gui/filter_panel_controller.cpp

책임:
- 고급 필터 UI 관리 (QDockWidget)
- 필터 상태 유지
- SearchCriteria 생성

주요 구성:
- 파일 타입 체크박스 (파일, 폴더, 심볼릭 링크)
- 크기 범위 슬라이더 (0B ~ 100GB)
- 날짜 범위 캘린더 (QDateEdit)
- 확장자 다중 선택 (QListWidget)
- 정규식/와일드카드 토글
- 대소문자 구분 체크박스

주요 메서드:
- getCurrentCriteria(): SearchCriteria 반환
- loadPreset(name): 저장된 필터 로드
- savePreset(name): 현재 필터 저장
- reset(): 필터 초기화

설계 원칙:
- 단일 책임 (필터 UI 관리)
- Observable Pattern (필터 변경 시 알림)
```

#### 5. IndexingStatusWidget (Custom Widget)

```cpp
위치: include/adapters/gui/indexing_status_widget.h
      src/adapters/gui/indexing_status_widget.cpp

책임:
- 인덱싱 진행 상황 시각화
- 상태바 오른쪽에 표시
- 실시간 통계 업데이트

주요 구성:
- QProgressBar: 인덱싱 진행률 (0-100%)
- QLabel: 상태 텍스트 ("Indexing: 1,234 / 5,678 files (22%)")
- QPushButton: 취소 버튼 (인덱싱 중단)
- 상태 아이콘: 대기/진행중/완료/에러

상태 종류:
- Idle: 인덱싱 필요 없음
- Pending: 인덱싱 대기 중
- InProgress: 진행 중 (진행률 표시)
- Completed: 완료 (초록색 체크)
- Error: 에러 발생 (빨간색 경고)
- Cancelled: 사용자 취소

주요 메서드:
- setProgress(current, total): 진행률 업데이트
- setStatus(IndexingStatus): 상태 변경
- reset(): 초기 상태로 리셋
- showDetails(): 상세 정보 다이얼로그 표시
  * 인덱싱된 파일 목록
  * 건너뛴 파일 (바이너리, 너무 큼)
  * 에러 파일 목록
  * 인덱스 크기, 소요 시간

Qt Signals:
- cancelRequested(): 취소 버튼 클릭
- detailsRequested(): 상세 정보 요청

설계 원칙:
- 단일 책임 (인덱싱 상태 표시만)
- 재사용 가능한 커스텀 위젯
- Non-blocking UI 업데이트
```

### GUI-specific Infrastructure

#### QtApplication

```cpp
위치: include/infrastructure/gui/qt_application.h
      src/infrastructure/gui/qt_application.cpp

책임:
- QApplication 초기화
- 테마 설정 (다크 모드, 라이트 모드)
- 폰트 설정
- 아이콘 테마 로드

주요 메서드:
- initialize(argc, argv): Qt 앱 초기화
- setTheme(Theme): 테마 변경
- exec(): 이벤트 루프 시작
```

#### QtSettingsManager

```cpp
위치: include/infrastructure/gui/qt_settings_manager.h
      src/infrastructure/gui/qt_settings_manager.cpp

책임:
- QSettings 래퍼
- 윈도우 크기/위치 저장/복원
- 최근 검색어 저장
- 필터 프리셋 관리

주요 메서드:
- saveWindowGeometry(QWidget*)
- restoreWindowGeometry(QWidget*)
- getRecentSearches(): QStringList
- addRecentSearch(QString)
```

### UI Layout (Everything Style)

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│ File  Edit  View  Tools  Help                                     [_][□][X]     │
├─────────────────────────────────────────────────────────────────────────────────┤
│ [🔍] Search: _____________________________  [≣Filter] [⚙]  [🔄 Index]         │
│      [📁] Path: /home/user/projects      [ .. ]                               │
│      ☐ Match Case  ☐ Regex  ☐ Content   ☐ Semantic                           │
├─────────────────────────────────────────────────────────────────────────────────┤
│ Name              │ Path           │ Size  │ Modified       │ Relevance        │
├───────────────────┼────────────────┼───────┼────────────────┼──────────────────┤
│ 📄 main.cpp       │ /src/          │ 4.2KB │ 2024-02-07     │ -                │
│ 📄 file_info.h    │ /include/      │ 1.8KB │ 2024-02-06     │ -                │
│ 📁 tests          │ /              │ -     │ 2024-02-05     │ -                │
│ 📄 README.md      │ /              │ 12KB  │ 2024-02-04     │ -                │
│ ...                                                                             │
├─────────────────────────────────────────────────────────────────────────────────┤
│ 1,234 files found (456 shown) | Search: 0.089s │ ⚡ Indexing: 3,421/5,678 (60%) │
└─────────────────────────────────────────────────────────────────────────────────┘
```

**상태바 레이아웃 (2개 섹션):**
```
┌────────────────────────────────────────────────────────────┐
│ 검색 통계 (왼쪽 - 확장)        │ 인덱싱 상태 (오른쪽 - 고정) │
├────────────────────────────────┼───────────────────────────┤
│ 1,234 files | 0.089s | 2.4GB  │ ⚡ Indexing: [▓▓▓░░] 60%  │
│                                │    3,421 / 5,678 files    │
└────────────────────────────────┴───────────────────────────┘

인덱싱 상태 표시 예시:
- 대기:     "⏸ Index ready (5,678 files)"
- 진행중:   "⚡ Indexing: 3,421/5,678 (60%) [▓▓▓▓▓░░░░░]"
- 완료:     "✓ Indexed: 5,678 files (2.3GB) in 12.4s"
- 에러:     "⚠ Indexing failed: 156 errors"
- 취소됨:   "⏹ Indexing cancelled (3,421/5,678)"

진행률 바 색상:
- 0-30%:   노란색 (시작 단계)
- 31-70%:  파란색 (진행 중)
- 71-99%:  초록색 (거의 완료)
- 100%:    진한 초록색 (완료)
- Error:   빨간색
```

**핵심 UI 요소:**
- **검색 바**: 메인 입력 필드 (포커스 자동, Ctrl+F로 이동)
- **인덱스 버튼**: 수동 인덱싱 트리거 (툴바 우측)
- **경로 선택**: 검색 대상 디렉토리 (Browse 버튼)
- **빠른 필터**: 자주 쓰는 옵션 (체크박스)
- **결과 테이블**: 가상화된 리스트 (수만 개 파일 처리 가능), Semantic 검색 시 Relevance 컬럼 추가
- **상태 바**: 2개 섹션
  * 왼쪽: 검색 통계 (파일 수, 검색 시간, 총 크기)
  * 오른쪽: 인덱싱 상태 (진행률, 남은 파일, 프로그레스 바)

### Context Menu (Right Click)

```cpp
파일 우클릭 메뉴:
- Open                  (기본 앱으로 열기)
- Open With >           (앱 선택)
- ──────────────
- Copy Path             (경로 복사)
- Copy Name             (이름 복사)
- ──────────────
- Show in File Manager  (Nautilus/Finder/Explorer에서 열기)
- Open Terminal Here    (터미널 열기)
- ──────────────
- Properties            (상세 정보 다이얼로그)
- Delete                (확인 후 삭제)
```

### Keyboard Shortcuts

```
검색:
- Ctrl+F: 검색 바 포커스
- F3 or Enter: 다음 결과
- Shift+F3: 이전 결과
- Escape: 검색 취소

네비게이션:
- ↑/↓: 결과 이동
- Page Up/Down: 페이지 이동
- Home/End: 처음/마지막
- Enter: 파일 열기

필터:
- Ctrl+M: Match Case 토글
- Ctrl+R: Regex 토글
- Ctrl+Shift+F: Content Search 토글

앱:
- Ctrl+Q: 종료
- Ctrl+,: 설정
- F1: 도움말
```

### Performance Optimization

#### 1. Virtual Scrolling (QTableView)
```cpp
- 대량 결과 처리 (100만 개 파일도 가능)
- QAbstractItemModel의 lazy loading
- 보이는 행만 렌더링 (메모리 효율)
```

#### 2. Debounced Search
```cpp
위치: src/adapters/gui/search_presenter.cpp

void SearchPresenter::onSearchTextChanged(QString text) {
    // 200ms 디바운스 (타이핑 중 검색 방지)
    m_searchTimer.stop();
    m_searchTimer.start(200);  // QTimer::singleShot
}

void SearchPresenter::performSearch() {
    // 이전 검색 취소
    if (m_searchFuture.valid()) {
        m_searchCancelled = true;
        m_searchFuture.wait();
    }
    
    // 비동기 검색
    m_searchCancelled = false;
    m_searchFuture = std::async(std::launch::async, [this]() {
        auto result = m_fileScanner->scan(m_currentCriteria);
        QMetaObject::invokeMethod(this, [this, result]() {
            m_view->updateResults(result);
        });
    });
}
```

#### 3. Incremental Updates
```cpp
검색 결과:
- 검색 중간 결과 표시 (1000개 단위)
- Progress indicator (QProgressBar in status bar)
- 취소 가능한 검색 (Cancel 버튼)

인덱싱 진행:
- 실시간 진행 상황 업데이트 (100ms 간격, QTimer)
- IndexingStatusWidget 상태 업데이트
  * setProgress(current, total)
  * setStatus(IndexingStatus::InProgress)
- SemanticSearcher → SearchPresenter → MainWindow 이벤트 체인
  * SemanticSearcher::indexDirectory() 내부에서 진행 콜백 호출
  * std::function<void(int, int)> progressCallback
  * QMetaObject::invokeMethod()로 UI 스레드에서 업데이트
- 취소 가능한 인덱싱 (Cancel 버튼 → std::atomic<bool> 플래그)
- 완료 시 통계 표시:
  * "✓ Indexed: 5,678 files (2.3GB) in 12.4s"
  * 건너뛴 파일 수 (바이너리, 권한 없음)
  * 에러 파일 수
```

#### 4. Icon Caching
```cpp
- QFileIconProvider 캐싱
- 확장자별 아이콘 미리 로드
- 메모리 제한 (LRU 캐시)
```

### Integration with Existing Code

#### CLI와 코드 공유
```
공유 레이어:
✅ Domain: FileInfo, SearchResult, SearchCriteria
✅ Application: FileScanner, SemanticSearcher, ContentSearcher
✅ Infrastructure: Logger, ThreadPool, ConfigFile

GUI 전용:
🖥️ Adapters/GUI: MainWindow, SearchPresenter, FileListViewModel
🖥️ Infrastructure/GUI: QtApplication, QtSettingsManager
```

#### CMake 설정
```cmake
# CMakeLists.txt
option(BUILD_GUI "Build GUI application" OFF)

if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
        Concurrent
    )
    
    add_executable(find_my_files_gui
        src/adapters/gui/main_window.cpp
        src/adapters/gui/search_presenter.cpp
        src/adapters/gui/file_list_view_model.cpp
        src/infrastructure/gui/qt_application.cpp
        src/gui_main.cpp  # GUI entry point
    )
    
    target_link_libraries(find_my_files_gui
        PRIVATE
        find_my_files_core  # Domain + Application + Infrastructure 공유
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
    )
endif()
```

### Testing Strategy

#### 1. Unit Tests
```cpp
테스트 대상:
- SearchPresenter 로직 (Mock View 사용)
- FileListViewModel 변환 로직
- FilterPanelController 상태 관리

도구:
- Google Test + Qt Test (QTest)
- Mock View (IMainWindow 인터페이스)
```

#### 2. Integration Tests
```cpp
테스트 대상:
- MainWindow + SearchPresenter 통합
- 실제 Qt 이벤트 루프
- 키보드/마우스 시뮬레이션

도구:
- QTest::qWait(), QTest::keyClick()
- QSignalSpy (시그널 검증)
```

#### 3. Manual Testing
```
체크리스트:
- 다양한 해상도에서 UI 레이아웃
- 다크/라이트 테마 전환
- 대량 결과 스크롤 성능
- 메모리 누수 (Qt Creator Profiler)
```

### Future Enhancements

#### Phase 1: Basic GUI (MVP)
- [x] 아키텍처 설계 (현재 문서)
- [ ] Qt 프로젝트 설정
- [ ] MainWindow 기본 레이아웃
- [ ] 단순 파일 이름 검색 (CLI 기능 활용)
- [ ] 결과 테이블 표시

#### Phase 2: Advanced Features
- [ ] 고급 필터 패널
- [ ] 정규식/컨텐츠 검색 지원
- [ ] 컨텍스트 메뉴
- [ ] 키보드 단축키

#### Phase 3: Polish
- [ ] 아이콘 및 테마
- [ ] 애니메이션 (검색 중 표시)
- [ ] 프리뷰 패널 (파일 미리보기)
- [ ] 드래그앤드롭 지원

#### Phase 4: Semantic Search Integration
- [ ] Semantic Search 모드
- [ ] 유사 파일 찾기 UI
- [ ] 인덱싱 진행 상황 표시
  * IndexingStatusWidget 통합
  * 실시간 진행률 업데이트 (현재/전체 파일 수, 퍼센티지)
  * 취소 가능한 인덱싱
  * 상세 정보 다이얼로그 (인덱싱된 파일, 에러, 통계)
  * 인덱스 유효성 표시 (최신/오래됨)
- [ ] 벡터 시각화 (선택적)

### Reference Implementations

- **Everything**: https://www.voidtools.com/
- **fzf-gui**: Terminal fuzzy finder의 GUI 버전
- **Qt File Manager Examples**: Qt Creator 내장 예제
- **KDE Dolphin**: KDE 파일 관리자 (Qt 기반)

---

## Coding Standards

### Naming Conventions
- **Classes**: PascalCase (e.g., `FileScanner`)
- **Methods**: camelCase (e.g., `matchesCriteria()`)
- **Variables**: camelCase (e.g., `fileInfo`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_SEARCH_SIZE`)
- **Namespace**: lowercase (e.g., `fmf`)

### Code Style
- **Indentation**: 4 spaces
- **Line Length**: 80 characters
- **Braces**: Allman style (opening brace on new line)
- **Header Guards**: `#pragma once`

### Error Handling
- **Exceptions**: Used for unrecoverable errors (std::runtime_error)
- **Return Values**: `bool` for success/failure, `std::optional` for nullable returns
- **File Operations**: Exception handling with try-catch blocks

---

## SOLID Principles Compliance

### Single Responsibility Principle (SRP)
- **FileInfo**: 파일 메타데이터만 관리
- **SearchCriteria**: 검색 조건만 정의
- **PatternMatcher**: 패턴 매칭만 담당
- **FileScanner**: 파일 검색 Use Case만 수행
- **Logger**: 로깅만 담당

### Open/Closed Principle (OCP)
- **IEmbeddingProvider/IVectorStore**: 확장 가능, 수정 불가
- **OutputFormatter**: 새로운 형식 추가 가능 (Strategy)

### Liskov Substitution Principle (LSP)
- **IEmbeddingProvider**: 모든 구현체는 상호 교환 가능
- **IVectorStore**: 모든 구현체는 상호 교환 가능

### Interface Segregation Principle (ISP)
- **IEmbeddingProvider**: 최소한의 인터페이스 (3 메서드)
- **IVectorStore**: 응집된 인터페이스 (벡터 저장소 운영에 필요한 메서드만)

### Dependency Inversion Principle (DIP)
- **FileScanner → ThreadPool**: 생성자 주입 (DI)
- **SemanticSearcher → IEmbeddingProvider/IVectorStore**: 인터페이스 의존
- **Use Cases → Domain**: 추상화에 의존

---

## Next Steps

### Phase 11: Architecture Refactoring (최우선)
- Clean Architecture 기반 폴더 구조 재편성
- 의존성 방향 검증
- 테스트 커버리지 유지

### Phase 12: Advanced Semantic Search
- FAISS 통합 (FAISSStore)
- OpenAI Provider 구현 (OpenAIProvider)
- CLI 통합 (--semantic-search, --similar)

### Phase 13: Hybrid Search
- Keyword + Semantic 병합
- Score normalization
- Result ranking

---

## References

- **Clean Architecture**: Robert C. Martin (Uncle Bob)
- **Hexagonal Architecture**: Alistair Cockburn
- **SOLID Principles**: Robert C. Martin
- **Domain-Driven Design**: Eric Evans
