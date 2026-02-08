# Find My Files - Code Architecture (Actual Code Structure)

## Overview
이 문서는 **현재 코드베이스의 실제 구조**와 주요 구성요소를 정리합니다.

아키텍처 설계 방법론, 원칙, 패턴, 의존성 흐름, 코딩 표준 등은
`.github/instructions/architecture.instructions.md`에 통합되어 있으니 그 문서를 참고하세요.

---

## Repository Layout (Current)

```
find_my_file/
├── include/                 # Public headers (flat 구조)
├── src/                     # Implementation files (flat 구조)
├── tests/                   # Unit tests
├── test/                    # Integration tests (쉘 스크립트)
├── docs/
│   └── architecture.md      # 본 문서
└── ...
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
즉, “파일을 그대로 임베딩”하는 방식이 아니라
**파일을 텍스트(또는 텍스트 설명)로 변환 → 단일 텍스트 임베딩 모델**로 통일한다.

### 핵심 원칙 요약
- ✅ **임베딩 모델은 1개**: 모든 모달리티를 텍스트로 정규화 후 동일 모델 사용
- ✅ **검색 일관성 보장**: 같은 벡터 공간에서 유사도 계산
- ✅ **추가 학습 없음**: Hugging Face의 pretrained 모델만 사용 (zero-shot)
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

#### 4) 문서 포맷별 파서 예시 (구현 가이드)
- **PDF**: pdfplumber / pymupdf
- **PPTX**: python-pptx
- **DOCX**: python-docx
- **HWP**: pyhwp

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

---

## Image Normalization Pipeline

### 목적
이미지 콘텐츠를 문서와 **동일한 임베딩 공간**에서 검색할 수 있도록
**OCR 텍스트 + 캡션 텍스트를 하나의 index_text로 정규화**한 뒤
텍스트 임베딩 모델로 벡터화한다.

### 핵심 원칙
- **단일 임베딩 공간**: 이미지도 텍스트로 변환 후 동일 모델 사용
- **추가 학습 없음**: pretrained OCR/VLM 모델만 사용
- **재현성**: OCR/캡션 결과, 모델 버전, 규칙 버전을 메타데이터로 저장
- **모델 캐시**: 다운로드 모델은 ~/.fmf/models 아래에 저장
- **자동 다운로드**: `FMF_EMBEDDING_MODEL_URL` 설정 시 모델이 자동 수신

### 처리 흐름 (개요)

```
Image → OCR + Caption → index_text → Text Embedding → Vector Index
```

### 구현 방식 (아키텍처 레벨)
- **C++ 파이프라인**: OCR/Caption/정규화/임베딩/저장 모두 C++ 모듈로 구성
- **OCR 엔진**: Tesseract C++ API 사용, 언어팩 미존재 시 영어로 폴백
- **Caption 모델**: 기본은 ONNX 기반 BLIP 계열, 필요 시 외부 CLI/서버 호출로 대체
- **Caption 실행**: 기본 커맨드 `blip-caption` 사용, 필요 시 `FMF_CAPTION_CMD`로 변경
- **임베딩 모델**: 문서와 동일한 all-MiniLM-L6-v2 사용
- **스토리지**: SQLite에 메타데이터 + 임베딩 저장

### 정제/병합 규칙 (deterministic)
- 공백 정규화, 중복 라인 제거
- 단일 문자/반복 구두점 등 비정보성 OCR 제거
- 숫자/단위는 보존

### 저장 메타데이터 (필수)
- file_path, file_hash, created_at, updated_at
- ocr_text, caption_text, index_text
- ocr_confidence (가능한 경우), ocr_engine_version
- caption_model_name
- embedding_model_name, embedding_dim, embedding_version_id

### 버전 정책
embedding_version_id = (embedding_model, caption_model, ocr_langs, merge_rule_version)

버전이 변경되면 기존 인덱스는 **stale** 처리하고
동일한 검색 결과에 섞지 않는다.
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
## Logical Layer Map (현재 파일 위치 기준)

> 아래 "레이어"는 역할 구분을 위한 **논리적 분류**이며,
> 실제 파일은 현재 `include/`, `src/`에 **flat 구조**로 존재합니다.

### 1) Domain (핵심 엔티티/값 객체)
- **FileInfo**
  - 헤더: `include/file_info.h`
  - 구현: `src/file_info.cpp`
  - 책임: 파일 메타데이터(이름/경로/크기/수정일/타입) 표현

- **SearchResult**
  - 헤더: `include/search_result.h`
  - 구현: `src/search_result.cpp`
  - 책임: 검색 결과 집합(추가/정렬/이터레이터 제공)

- **SemanticResult**
  - 헤더: `include/semantic_result.h`
  - 책임: 의미론 검색 결과(파일 정보 + 유사도 + 매칭 청크)

- **SearchCriteria**
  - 헤더: `include/search_criteria.h`
  - 구현: `src/search_criteria.cpp`
  - 책임: 검색 조건(패턴, 확장자, 크기, 날짜, 내용 검색 등)

---

### 2) Application (Use Case / Service / Port)
- **Ports (추상화 인터페이스)**
  - `IEmbeddingProvider`: `include/i_embedding_provider.h`
  - `IVectorStore`: `include/i_vector_store.h`

- **Services**
  - **PatternMatcher**: `include/pattern_matcher.h`, `src/pattern_matcher.cpp`
  - **ContentSearcher**: `include/content_searcher.h`, `src/content_searcher.cpp`
  - **IgnorePatterns**: `include/ignore_patterns.h`, `src/ignore_patterns.cpp`

- **Use Cases**
  - **FileScanner**: `include/file_scanner.h`, `src/file_scanner.cpp`
  - **SemanticSearcher**: `include/semantic_searcher.h`, `src/semantic_searcher.cpp`

---

### 3) Interface Adapters (CLI/Presenter)
- **CommandLineParser**
  - 헤더: `include/command_line_parser.h`
  - 구현: `src/command_line_parser.cpp`
  - 책임: CLI 인자 파싱 및 `ApplicationConfig` 구성

- **OutputFormatter**
  - 헤더: `include/output_formatter.h`
  - 구현: `src/output_formatter.cpp`
  - 책임: 출력 포맷(기본/상세/JSON) 처리

---

### 4) Infrastructure (기술 구현)
- **Logger**
  - 헤더: `include/logger.h`
  - 구현: `src/logger.cpp`

- **ConfigFile**
  - 헤더: `include/config_file.h`
  - 구현: `src/config_file.cpp`

- **ThreadPool**
  - 헤더: `include/thread_pool.h`
  - 구현: `src/thread_pool.cpp`

- **MockEmbeddingProvider**
  - 헤더: `include/mock_embedding_provider.h`

- **MockVectorStore**
  - 헤더: `include/mock_vector_store.h`

---

## Entry Point
- **main**: `src/main.cpp`
  - 설정 파일 + CLI 파싱
  - 검색 유스케이스 실행
  - 출력 포맷팅

---

## Tests (현재 위치 기준)
- **Unit Tests**: `tests/`
  - Google Test 기반
  - 114개 테스트 (모두 통과)
- **Integration Tests**: `test/integrationtest/*.sh`
  - 40개 쉘 스크립트

---

## GUI Architecture (Qt 6 - Future Design)

> **현재 상태**: GUI는 아직 구현되지 않았습니다. 이 섹션은 향후 구현을 위한 설계 문서입니다.

### Framework Selection

**선택**: Qt 6 (LTS)

**선택 이유**:
- ✅ 크로스 플랫폼 (Linux, macOS, Windows)
- ✅ 성능 최적화된 위젯 (QTreeView, QTableView)
- ✅ 비동기 UI 업데이트 (Signals & Slots)
- ✅ CMake 통합 용이
- ✅ C++ 네이티브, Clean Architecture 적용 가능

### Clean Architecture Mapping

```
┌────────────────────────────────────────────────┐
│  Infrastructure Layer (GUI)                    │
│  • QtApplication (main event loop)             │
│  • QtSettingsManager (QSettings)               │
└────────────────────────────────────────────────┘
          ↓ depends on
┌────────────────────────────────────────────────┐
│  Interface Adapters Layer (GUI)                │
│  • MainWindow (View)                           │
│  • SearchPresenter (Presenter)                 │
│  • FileListViewModel (ViewModel)               │
│  • FilterPanelController (Controller)          │
│  • IndexingStatusWidget (Custom Widget)        │
└────────────────────────────────────────────────┘
          ↓ depends on
┌────────────────────────────────────────────────┐
│  Application Layer (CLI와 공유)                 │
│  • FileScanner, SemanticSearcher               │
│  • PatternMatcher, ContentSearcher             │
│  • IgnorePatterns                              │
└────────────────────────────────────────────────┘
          ↓ depends on
┌────────────────────────────────────────────────┐
│  Domain Layer (CLI와 공유)                      │
│  • FileInfo, SearchResult, SearchCriteria      │
└────────────────────────────────────────────────┘
```

**핵심**: CLI와 GUI는 **동일한 Application/Domain 레이어를 공유**합니다.

### Component Design (MVP Pattern)

#### 1. MainWindow (View)

```cpp
위치: include/adapters/gui/main_window.h (예정)
      src/adapters/gui/main_window.cpp

책임:
- 메인 UI 렌더링 (QMainWindow)
- 사용자 입력 수신 (검색어, 버튼 클릭)
- Presenter에게 이벤트 전달

주요 구성:
- QLineEdit: 검색어 입력
- QTableView: 검색 결과 리스트
- QToolBar: 필터 버튼
- QStatusBar: 검색 통계 + 인덱싱 상태 (2개 섹션)
- QDockWidget: 고급 필터 패널
```

#### 2. SearchPresenter (Presenter)

```cpp
위치: include/adapters/gui/search_presenter.h (예정)
      src/adapters/gui/search_presenter.cpp

책임:
- View와 Application Layer 중재
- 사용자 입력을 SearchCriteria로 변환
- FileScanner/SemanticSearcher 호출
- SearchResult를 ViewModel로 변환

주요 메서드:
- onSearchTextChanged(QString)
- onFilterChanged(FilterOptions)
- onFileOpen(QString)
- onStartIndexing()
- onIndexingProgress(int current, int total)
```

#### 3. FileListViewModel (ViewModel)

```cpp
위치: include/adapters/gui/file_list_view_model.h (예정)
      src/adapters/gui/file_list_view_model.cpp

책임:
- SearchResult를 Qt Model로 변환 (QAbstractTableModel)
- 정렬, 필터링 로직
- 컬럼 포맷팅 (크기, 날짜 표시 형식)
```

#### 4. IndexingStatusWidget (Custom Widget)

```cpp
위치: include/adapters/gui/indexing_status_widget.h (예정)
      src/adapters/gui/indexing_status_widget.cpp

책임:
- 인덱싱 진행 상황 시각화
- 상태바 오른쪽에 표시
- 실시간 통계 업데이트 (QProgressBar)
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
├─────────────────────────────────────────────────────────────────────────────────┤
│ 1,234 files found (456 shown) | Search: 0.089s │ ⚡ Indexing: 3,421/5,678 (60%) │
└─────────────────────────────────────────────────────────────────────────────────┘
```

### Performance Optimization

#### 1. Virtual Scrolling (QTableView)
- 대량 결과 처리 (100만 개 파일도 가능)
- QAbstractItemModel의 lazy loading
- 보이는 행만 렌더링 (메모리 효율)

#### 2. Debounced Search
- 200ms 디바운스 (타이핑 중 검색 방지)
- QTimer::singleShot 사용

#### 3. Incremental Updates
- 검색 중간 결과 표시 (1000개 단위)
- 인덱싱 실시간 진행 상황 (100ms 간격)
- 취소 가능한 검색/인덱싱

#### 4. Icon Caching
- QFileIconProvider 캐싱
- 확장자별 아이콘 미리 로드

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

#### CMake 설정 (예정)
```cmake
option(BUILD_GUI "Build GUI application" OFF)

if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS
        Core Widgets Concurrent
    )
    
    add_executable(find_my_files_gui
        src/adapters/gui/main_window.cpp
        src/adapters/gui/search_presenter.cpp
        src/adapters/gui/file_list_view_model.cpp
        src/infrastructure/gui/qt_application.cpp
        src/gui_main.cpp
    )
    
    target_link_libraries(find_my_files_gui
        PRIVATE
        find_my_files_core  # Domain + Application + Infrastructure 공유
        Qt6::Core Qt6::Widgets Qt6::Concurrent
    )
endif()
```

### Testing Strategy

#### 1. Unit Tests
- SearchPresenter 로직 (Mock View 사용)
- FileListViewModel 변환 로직
- FilterPanelController 상태 관리
- Google Test + Qt Test (QTest)

#### 2. Integration Tests
- MainWindow + SearchPresenter 통합
- 실제 Qt 이벤트 루프
- 키보드/마우스 시뮬레이션
- QTest::qWait(), QTest::keyClick()

#### 3. Manual Testing
- 다양한 해상도에서 UI 레이아웃
- 다크/라이트 테마 전환
- 대량 결과 스크롤 성능
- 메모리 누수 (Qt Creator Profiler)

### Future Enhancements (GUI Phases)

#### Phase 1: Basic GUI (MVP)
- [ ] Qt 프로젝트 설정
- [ ] MainWindow 기본 레이아웃
- [ ] 단순 파일 이름 검색
- [ ] 결과 테이블 표시

#### Phase 2: Advanced Features
- [ ] 고급 필터 패널
- [ ] 정규식/컨텐츠 검색 지원
- [ ] 컨텍스트 메뉴
- [ ] 키보드 단축키

#### Phase 3: Polish
- [ ] 아이콘 및 테마
- [ ] 애니메이션 
- [ ] 프리뷰 패널
- [ ] 드래그앤드롭

#### Phase 4: Semantic Search Integration
- [ ] Semantic Search 모드
- [ ] 유사 파일 찾기 UI
- [ ] 인덱싱 진행 상황 표시 (IndexingStatusWidget)
- [ ] 벡터 시각화 (선택적)

---

## Notes

### 현재 코드 상태
- 현재 코드는 `include/`, `src/`에 flat 구조로 존재합니다.
- 레이어별 폴더 구조로의 리팩터링 계획/원칙/절차는
  `.github/instructions/architecture.instructions.md`에 정리되어 있습니다.

### GUI 구현 계획
- GUI는 Phase 12에서 구현 예정입니다.
- GUI는 CLI와 동일한 Core 레이어(Domain + Application)를 공유합니다.
- MVP 패턴과 Clean Architecture 원칙을 따릅니다.

### 관련 문서
- **설계 원칙 및 가이드라인**: `.github/instructions/architecture.instructions.md`
- **코딩 표준**: `.github/instructions/cpp-standards.instructions.md`
- **TDD 사이클**: `.github/instructions/tdd-cycle.instructions.md`
- **테스팅 가이드**: `.github/instructions/testing.instructions.md`
