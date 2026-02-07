# Find My Files - Code Architecture (Actual Code Structure)

## Overview
이 문서는 **현재 코드베이스의 실제 구조**와 주요 구성요소를 정리합니다.
아키텍처 설계 방법론, 원칙, 패턴, 데이터/의존성 흐름 등은
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

## Logical Layer Map (현재 파일 위치 기준)

> 아래 “레이어”는 역할 구분을 위한 **논리적 분류**이며,
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
