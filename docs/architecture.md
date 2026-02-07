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
