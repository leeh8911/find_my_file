# Find My Files - TODO List

> 완료된 Phase는 [todos_done.md](./todos_done.md)를 참조하세요.

## Phase 4: 성능 최적화 (부분 완료)
- [x] 멀티스레딩 ✅ **완료**
  - [x] 병렬 디렉토리 탐색
  - [x] 스레드 풀 구현
  - [x] 동기화 및 스레드 안전성
- [ ] 캐싱 시스템
  - [ ] 파일 시스템 스캔 결과 캐싱
  - [ ] 캐시 무효화 전략
  - [ ] 증분 업데이트
- [ ] 메모리 최적화
  - [ ] 대용량 디렉토리 처리
  - [ ] 메모리 사용량 모니터링
- [ ] 조기 종료 최적화
  - [ ] 최대 결과 수 제한
  - [ ] 타임아웃 설정

---

## Phase 7: 테스트 (대부분 완료)
- [x] 유닛 테스트 ✅
  - [x] Google Test 통합 (완료)
  - [x] 핵심 기능 테스트 커버리지 (114 unit tests)
  - [ ] Mock 파일 시스템 (선택적 - 추후 개선)
- [x] 통합 테스트 ✅
  - [x] 전체 워크플로우 테스트 (40 integration tests)
  - [x] 엣지 케이스 테스트
- [ ] 성능 테스트
  - [ ] 벤치마크 스위트 (선택적 - 추후 작업)
  - [ ] 대용량 디렉토리 테스트 (선택적 - 추후 작업)
- [x] CI/CD 설정 ✅
  - [x] GitHub Actions 워크플로우 (.github/workflows/ci.yml)

---

## Phase 10: Semantic Search (LLM 기반 의미론적 검색) 🔴 우선순위
### 목표
파일 내용의 의미를 이해하고 자연어 쿼리로 관련 파일을 찾는 기능 추가

### 1. 인터페이스 설계 및 추상화 ✅ **완료**
- [x] IEmbeddingProvider 인터페이스 정의
  - [x] generateEmbedding(text) → vector<float>
  - [x] batchGenerate(texts) → vector<vector<float>>
  - [x] getDimension() → size_t
- [x] IVectorStore 인터페이스 정의
  - [x] add(id, vector, metadata)
  - [x] search(queryVector, topK) → vector<SearchResult>
  - [x] remove(id)
  - [x] save(path) / load(path)
- [x] SemanticResult 구조체 정의
  - [x] FileInfo, relevanceScore, matchedChunks
- [x] 단위 테스트: 인터페이스 mock 구현
- 커밋: fc5d9a1 "feat: implement semantic search interfaces"

### 2. Mock Provider 구현 (테스트용) ✅ **완료**
- [x] MockEmbeddingProvider 클래스
  - [x] 고정된 테스트 벡터 반환
  - [x] 결정론적 동작 보장
- [x] MockVectorStore 클래스
  - [x] In-memory map 기반 저장
  - [x] 단순 cosine similarity 검색
- [x] 단위 테스트: Mock을 사용한 기본 시나리오
- 커밋: 70fc10e "feat: extract mock classes to reusable headers"

### 3. FAISS 기반 VectorStore
- [ ] FAISSStore 클래스 구현
  - [ ] FAISS 라이브러리 통합 (CMake)
  - [ ] Index 생성 및 관리
  - [ ] Persistence (save/load)
- [ ] CMake 옵션: -DENABLE_FAISS=ON
- [ ] 단위 테스트: FAISS 기본 동작
- [ ] 통합 테스트: 대량 벡터 인덱싱/검색

### 4. LocalEmbeddingProvider (로컬 모델 기반) 🔴 우선순위
- [x] LocalEmbeddingProvider 클래스 구현 ✅ **완료**
  - [x] 기본 구조 (Pimpl idiom)
  - [x] IEmbeddingProvider 인터페이스 구현
  - [x] generateEmbedding(text) 구현 (placeholder)
  - [x] batchGenerate(texts) 최적화
  - [x] Model 파일 관리 (~/.fmf/models/)
  - [ ] ONNX Runtime 통합 (TODO)
  - [ ] Sentence-BERT 모델 로드 (all-MiniLM-L6-v2)
- [x] 토크나이저 통합 ✅ **완료**
  - [x] SimpleTokenizer 구현 (whitespace 기반)
  - [x] Lowercase conversion
  - [x] Punctuation stripping
  - [x] Special tokens ([CLS], [SEP], [PAD], [UNK])
  - [x] Max sequence length (512)
  - [ ] Proper BPE/WordPiece tokenizer (TODO)
  - [ ] Vocabulary 로드 (TODO)
- [ ] 추론 최적화
  - [ ] ONNX Runtime 통합
  - [ ] Batch processing 최적화
  - [ ] CPU 최적화 (OpenMP, BLAS)
  - [ ] 옵션: GPU 지원 (CUDA/ROCm)
- [ ] Model 다운로드 자동화
  - [ ] Hugging Face Hub 통합 (선택적)
  - [ ] Model 파일 검증 (SHA256)
  - [ ] 처음 실행 시 자동 다운로드
- [ ] 대안: llama.cpp 통합
  - [ ] GGUF 모델 지원
  - [ ] llama.cpp 빌드 통합
  - [ ] Embedding 추출 API
- [x] CMake 옵션: -DENABLE_LOCAL_EMBEDDING=ON ✅
- [x] 단위 테스트: 26개 테스트 (LocalEmbeddingProvider 13 + SimpleTokenizer 13) ✅
- [ ] 통합 테스트: End-to-end 로컬 검색
- 커밋: 
  * def98a3 "feat: implement LocalEmbeddingProvider"
  * 5880b17 "feat: add SimpleTokenizer with improved tokenization"

### 5. SemanticSearcher 코어 로직 ✅ **완료**
- [x] SemanticSearcher 클래스 구현
  - [x] indexFile(filepath) - 파일 인덱싱
  - [x] indexDirectory(dirpath) - 디렉토리 인덱싱
  - [x] search(query, topK) - 검색 수행
  - [x] searchSimilar(filepath, topK) - 유사 파일 찾기
- [x] Chunking 전략
  - [x] FixedSizeChunker (기본)
  - [x] Configurable chunk size and overlap
- [x] File type detection (extension-based)
- [x] 단위 테스트: 각 메서드별 테스트
- 커밋: a0d19af "feat: implement SemanticSearcher core logic"
- [ ] Index 관리
  - [ ] .fmf_index/ 디렉토리 구조
  - [ ] metadata.json (파일 경로, 타임스탬프)
  - [ ] Incremental update (변경 감지)
- [ ] 단위 테스트: 각 메서드별 테스트
- [ ] 통합 테스트: End-to-end 시나리오

### 6. CLI 통합 (SemanticSearcher 연동)
- [ ] CommandLineParser 확장
  - [ ] --semantic-search QUERY
  - [ ] --similar FILE
  - [ ] --index (인덱싱만 수행)
  - [ ] --rebuild-index
  - [ ] --embedding-provider [openai|local]
  - [ ] --top-k N
- [ ] ApplicationConfig 확장
  - [ ] semanticSearchQuery
  - [ ] similarFile
  - [ ] embeddingProvider
  - [ ] topK
- [ ] main.cpp 통합
  - [ ] Semantic search 모드 분기
  - [ ] Index 자동 생성/업데이트
  - [ ] 결과 출력 (relevance score 포함)
- [ ] 통합 테스트: CLI 시나리오 (uc_semantic_search.sh)

### 7. Hybrid Search (선택적)
- [ ] QueryProcessor 클래스
  - [ ] Keyword search (ContentSearcher) + Semantic search 병합
  - [ ] Score normalization
  - [ ] Result ranking algorithm
- [ ] --hybrid-search 옵션
- [ ] 통합 테스트: Hybrid search 시나리오

### 8. 인덱스 Persistence
- [ ] Index 저장/로드 최적화
  - [ ] 압축 (zlib/zstd)
  - [ ] 증분 저장
- [ ] Cache 관리 (.fmf_cache/)
  - [ ] Embedding cache (중복 계산 방지)
  - [ ] LRU eviction
- [ ] 성능 벤치마크
  - [ ] 인덱싱 속도
  - [ ] 검색 속도
  - [ ] 메모리 사용량

### 9. OpenAI Embedding Provider (선택적, 낮은 우선순위)
- [ ] OpenAIProvider 클래스 구현
  - [ ] HTTP client 통합 (libcurl or cpp-httplib)
  - [ ] API 키 관리 (환경변수/config)
  - [ ] Rate limiting 처리
  - [ ] Retry logic with exponential backoff
- [ ] 에러 처리
  - [ ] Network error → fallback to local provider
  - [ ] API error → 명확한 에러 메시지
- [ ] CMake 옵션: -DENABLE_OPENAI_PROVIDER=ON
- [ ] 단위 테스트: HTTP mock
- [ ] 통합 테스트: 실제 API 호출 (CI skip 가능)

### 10. 문서화 및 예제
- [ ] README.md 업데이트
  - [ ] Semantic Search 섹션
  - [ ] 사용 예제
  - [ ] 설치 가이드 (optional dependencies)
- [ ] examples/semantic_search_demo.sh
- [ ] ConfigFile 확장 (.findmyfilesrc)
  - [ ] [semantic] 섹션
  - [ ] embedding_provider, model, api_key_env

### Phase 10 Definition of Done
- [ ] 모든 단위 테스트 통과
- [ ] 모든 통합 테스트 통과
- [ ] SOLID 원칙 준수
- [ ] clang-format / clang-tidy / cpplint 통과
- [ ] Architecture.md 업데이트 완료
- [ ] README.md 업데이트 완료
- [ ] 성능 벤치마크 결과 문서화

---

## Phase 12: GUI Implementation (Everything UI Style)
### 목표
Everything 스타일의 빠르고 직관적인 데스크톱 GUI 제공. CLI와 Domain/Application 레이어를 공유하여 코드 재사용 극대화.

### Why: GUI의 필요성
- **사용성**: 일반 사용자를 위한 직관적 인터페이스
- **생산성**: 실시간 검색, 빠른 필터링, 마우스 조작
- **시각화**: 파일 아이콘, 색상 코딩, 프리뷰
- **통합**: 파일 관리자, 외부 앱과 연동 (드래그앤드롭, 컨텍스트 메뉴)

### Architecture Principles
- **Clean Architecture**: CLI와 동일한 Domain/Application 레이어 공유
- **MVP Pattern**: MainWindow (View), SearchPresenter (Presenter), FileListViewModel (ViewModel)
- **Async UI**: UI 스레드 블로킹 방지 (QtConcurrent, std::async)
- **Cross-platform**: Linux, macOS, Windows 지원 (Qt 6)

### Phase 12.1: Qt 프로젝트 설정 및 기본 구조
- [ ] CMake 설정
  - [ ] BUILD_GUI 옵션 추가
  - [ ] Qt6 의존성 (Core, Widgets, Concurrent)
  - [ ] find_my_files_core 라이브러리 생성 (공유 코드)
  - [ ] find_my_files_gui 실행 파일
- [ ] 디렉토리 생성
  - [ ] include/adapters/gui/
  - [ ] include/infrastructure/gui/
  - [ ] src/adapters/gui/
  - [ ] src/infrastructure/gui/
  - [ ] tests/gui/
- [ ] 기본 파일
  - [ ] src/gui_main.cpp (Qt entry point)
  - [ ] QtApplication 클래스 (QApplication 래퍼)
  - [ ] 빌드 및 실행 검증 (빈 창)
- [ ] 문서화
  - [ ] README.md에 GUI 빌드 방법 추가
  - [ ] Qt 설치 가이드

### Phase 12.2: MainWindow UI 레이아웃 (Everything 스타일)
- [ ] MainWindow 클래스 (QMainWindow)
  - [ ] 메뉴 바 (File, Edit, View, Tools, Help)
  - [ ] 검색 바 (QLineEdit, 포커스 우선)
  - [ ] 인덱스 버튼 (툴바 우측, 수동 인덱싱 트리거)
  - [ ] 경로 선택 (QComboBox + Browse 버튼)
  - [ ] 빠른 필터 (QCheckBox: Match Case, Regex, Content, Semantic)
  - [ ] 결과 테이블 (QTableView: Name, Path, Size, Modified, Relevance)
  - [ ] 상태 바 (QStatusBar: 2개 섹션)
    * 왼쪽: 검색 통계 (N files found, search time, total size)
    * 오른쪽: IndexingStatusWidget (인덱싱 진행률, 남은 파일)
- [ ] IndexingStatusWidget 클래스
  - [ ] QProgressBar: 인덱싱 진행률 (0-100%)
  - [ ] QLabel: 상태 텍스트 ("Indexing: 1,234 / 5,678 (22%)")
  - [ ] QPushButton: 취소 버튼 (인덱싱 중단)
  - [ ] 상태 아이콘: 대기/진행/완료/에러/취소
  - [ ] 진행률 바 색상 (0-30% 노란, 31-70% 파란, 71-100% 초록)
- [ ] 스타일시트
  - [ ] Everything 스타일 CSS (미니멀, 깔끔)
  - [ ] 다크/라이트 테마 지원
- [ ] 아이콘
  - [ ] 파일 타입 아이콘 (QFileIconProvider)
  - [ ] 툴바 아이콘 (Material Icons or Feather Icons)
- [ ] 레이아웃 테스트
  - [ ] 다양한 해상도 (1920x1080, 1366x768)
  - [ ] 윈도우 리사이즈 동작

### Phase 12.3: SearchPresenter 및 ViewModel
- [ ] SearchPresenter 클래스
  - [ ] onSearchTextChanged(QString) - 디바운스 200ms
  - [ ] onPathChanged(QString)
  - [ ] onFilterChanged(FilterOptions)
  - [ ] performSearch() - 비동기 검색 (std::async)
  - [ ] onSearchComplete(SearchResult)
  - [ ] cancelSearch() - 검색 취소
  - [ ] onStartIndexing() - 인덱싱 시작
  - [ ] onIndexingProgress(int current, int total) - 진행 상황 업데이트
  - [ ] onIndexingComplete() - 인뎁싱 완료
  - [ ] cancelIndexing() - 인뎁싱 취소
- [ ] FileListViewModel 클래스 (QAbstractTableModel)
  - [ ] rowCount(), columnCount()
  - [ ] data(index, role) - DisplayRole, UserRole, DecorationRole
  - [ ] headerData() - 컬럼 헤더
  - [ ] setSearchResult(SearchResult)
  - [ ] sort(column, order)
- [ ] 의존성 주입
  - [ ] SearchPresenter → FileScanner (Application Layer)
  - [ ] MainWindow → SearchPresenter
- [ ] 단위 테스트
  - [ ] SearchPresenter 로직 (Mock View)
  - [ ] FileListViewModel 변환

### Phase 12.4: 기본 검색 기능 통합 (CLI 재사용)
- [ ] 파일 이름 검색
  - [ ] 검색어 입력 → SearchCriteria 생성
  - [ ] FileScanner 호출
  - [ ] 결과 테이블 업데이트
- [ ] 와일드카드 패턴
  - [ ] "*.cpp", "test_*.h"
- [ ] 대소문자 구분 옵션
- [ ] 정규식 옵션
- [ ] 통합 테스트
  - [ ] QTest::keyClick() 시뮬레이션
  - [ ] 검색 결과 검증

### Phase 12.5: 고급 필터 패널 (QDockWidget)
- [ ] FilterPanelController 클래스
  - [ ] 파일 타입 (File, Folder, Symlink)
  - [ ] 크기 범위 (QSlider: 0B ~ 100GB)
  - [ ] 날짜 범위 (QDateEdit: Modified Time)
  - [ ] 확장자 다중 선택 (QListWidget)
  - [ ] reset() - 필터 초기화
- [ ] 도킹 가능한 패널
  - [ ] 기본 위치: 오른쪽
  - [ ] 숨기기/보이기 토글 (View 메뉴)
- [ ] SearchCriteria 통합
  - [ ] FilterPanel → SearchCriteria 변환
  - [ ] Presenter에 전달

### Phase 12.6: 실시간 검색 및 성능 최적화
- [ ] Debounced Search
  - [ ] QTimer::singleShot(200ms)
  - [ ] 타이핑 중 검색 방지
- [ ] 비동기 검색
  - [ ] QtConcurrent::run() 또는 std::async
  - [ ] UI 스레드 블로킹 방지
  - [ ] QMetaObject::invokeMethod() for UI update
- [ ] 취소 가능한 검색
  - [ ] Cancel 버튼
  - [ ] std::atomic<bool> m_searchCancelled
- [ ] Virtual Scrolling
  - [ ] QTableView 기본 가상화 활용
  - [ ] 대량 결과 (10만+ 파일) 테스트
- [ ] Incremental Updates
  - [ ] 1000개 단위 중간 결과 표시
  - [ ] QProgressBar (상태 바)

### Phase 12.7: 컨텍스트 메뉴 및 파일 작업
- [ ] 파일 우클릭 메뉴
  - [ ] Open (기본 앱)
  - [ ] Open With > (앱 선택)
  - [ ] Copy Path / Copy Name
  - [ ] Show in File Manager (Nautilus/Finder/Explorer)
  - [ ] Open Terminal Here
  - [ ] Properties (상세 정보 다이얼로그)
  - [ ] Delete (확인 후 삭제)
- [ ] 파일 열기 로직
  - [ ] QDesktopServices::openUrl()
  - [ ] 플랫폼별 처리 (Linux: xdg-open, macOS: open, Windows: start)
- [ ] 드래그앤드롭
  - [ ] 파일 테이블 → 외부 앱 (파일 경로 전달)

### Phase 12.8: 키보드 단축키
- [ ] 검색 단축키
  - [ ] Ctrl+F: 검색 바 포커스
  - [ ] F3 / Enter: 다음 결과
  - [ ] Shift+F3: 이전 결과
  - [ ] Escape: 검색 취소
- [ ] 네비게이션
  - [ ] ↑/↓: 결과 이동
  - [ ] Page Up/Down: 페이지 이동
  - [ ] Home/End: 처음/마지막
  - [ ] Enter: 파일 열기
- [ ] 필터 토글
  - [ ] Ctrl+M: Match Case
  - [ ] Ctrl+R: Regex
  - [ ] Ctrl+Shift+F: Content Search
- [ ] 앱 제어
  - [ ] Ctrl+Q: 종료
  - [ ] Ctrl+,: 설정
  - [ ] F1: 도움말

### Phase 12.9: 설정 및 프리셋
- [ ] QtSettingsManager 클래스 (QSettings 래퍼)
  - [ ] 윈도우 크기/위치 저장/복원
  - [ ] 최근 검색어 저장 (max 20개)
  - [ ] 최근 경로 저장 (max 10개)
  - [ ] 필터 프리셋 (저장된 검색)
- [ ] 설정 다이얼로그 (Ctrl+,)
  - [ ] 테마 선택 (다크/라이트/시스템)
  - [ ] 기본 검색 경로
  - [ ] 스레드 수
  - [ ] 로그 레벨
- [ ] ConfigFile 통합
  - [ ] .findmyfilesrc → 기본 설정 로드
  - [ ] GUI 설정 → .findmyfilesrc 저장

### Phase 12.10: 테마 및 UI 폴리시
- [ ] 다크 모드
  - [ ] Qt 스타일시트 (Fusion 스타일)
  - [ ] 색상 팔레트 (배경, 텍스트, 강조)
- [ ] 라이트 모드
  - [ ] 기본 Qt 테마
- [ ] 시스템 테마 감지
  - [ ] QGuiApplication::styleHints()
- [ ] 아이콘 통일
  - [ ] Material Icons or Feather Icons
  - [ ] SVG 형식 (스케일링)
- [ ] 애니메이션
  - [ ] 검색 중 로딩 스피너
  - [ ] 결과 테이블 페이드인
- [ ] 폰트 설정
  - [ ] 고정폭 폰트 (경로 컬럼)
  - [ ] 가독성 최적화

### Phase 12.11: 컨텐츠 및 Semantic 검색 통합
- [ ] Content Search 모드
  - [ ] ContentSearcher 통합
  - [ ] 검색 진행 표시 (파일 수 카운터)
- [ ] Semantic Search 모드 (Phase 10 선행)
  - [ ] SemanticSearcher 통합
  - [ ] 인뎁싱 진행 상태 표시
    * IndexingStatusWidget에 실시간 업데이트 (100ms 간격)
    * 진행률 바 (0-100%)
    * 현재/전체 파일 수 ("3,421 / 5,678 files")
    * 남은 퍼센트 ("%")
    * 예상 남은 시간 (선택적, ETA 계산)
  - [ ] 인뎁싱 세부 정보 다이얼로그
    * 인뎁싱된 파일 목록 (QListWidget)
    * 건너뚴 파일 (바이너리, 너무 큼, 권한 없음)
    * 에러 파일 목록 (빨간색 표시)
    * 인뎁스 크기 (MB/GB)
    * 소요 시간 (12.4s)
  - [ ] 인뎁싱 상태 지속성
    * 마지막 인뎁싱 시간 표시
    * 인뎁스 유효성 표시 (최신/오래됨)
    * 자동 증분 업데이트 (변경된 파일만 재인뎁싱)
  - [ ] "Similar files" 버튼 (선택한 파일의 유사 파일 찾기)
- [ ] Hybrid Search
  - [ ] Keyword + Semantic 병합
  - [ ] Relevance Score 표시 (결과 테이블 컬럼)

### Phase 12.12: 프리뷰 패널 (선택적)
- [ ] 파일 미리보기 (QSplitter)
  - [ ] 텍스트 파일: QTextEdit (읽기 전용)
  - [ ] 이미지: QLabel (QPixmap)
  - [ ] PDF: 외부 라이브러리 (QPdfView) 또는 "Open"으로 대체
- [ ] Syntax Highlighting
  - [ ] QSyntaxHighlighter (간단한 C++ 하이라이팅)
- [ ] 토글 가능 (View 메뉴)

### Phase 12.13: 테스트 및 검증
- [ ] 단위 테스트
  - [ ] SearchPresenter 로직 (Google Test)
  - [ ] FileListViewModel 변환
  - [ ] FilterPanelController 상태 관리
  - [ ] QtSettingsManager 저장/복원
- [ ] Integration Tests (QTest)
  - [ ] MainWindow + SearchPresenter
  - [ ] 키보드/마우스 시뮬레이션 (QTest::keyClick)
  - [ ] 검색 결과 검증 (QSignalSpy)
- [ ] Manual Testing
  - [ ] 다양한 해상도 (4K, FHD, HD)
  - [ ] 다양한 OS (Ubuntu 22.04, macOS 13+, Windows 10+)
  - [ ] 대량 파일 테스트 (10만+ 파일)
  - [ ] 메모리 누수 (Qt Creator Profiler, Valgrind)
  - [ ] CPU 사용률 (검색 중 모니터링)

### Phase 12.14: 문서화 및 배포
- [ ] README.md 업데이트
  - [ ] GUI 스크린샷 추가
  - [ ] GUI 빌드 방법
  - [ ] Qt 6 설치 가이드 (Linux, macOS, Windows)
- [ ] 사용자 가이드
  - [ ] GUI 기능 설명
  - [ ] 키보드 단축키 표
  - [ ] 필터 사용 예제
- [ ] 배포 패키지
  - [ ] AppImage (Linux)
  - [ ] DMG (macOS)
  - [ ] MSI/EXE (Windows)
- [ ] GitHub Release
  - [ ] GUI 바이너리 업로드
  - [ ] Release Notes

### Phase 12 Definition of Done
- [ ] Qt 6 기반 GUI 앱 빌드 성공
- [ ] 모든 기본 검색 기능 동작 (파일 이름, 와일드카드, 정규식, 필터)
- [ ] 실시간 검색 (디바운스, 비동기)
- [ ] 컨텍스트 메뉴 및 파일 열기
- [ ] 키보드 단축키 전체 구현
- [ ] 다크/라이트 테마 지원
- [ ] 윈도우 크기/위치 저장/복원
- [ ] 모든 단위 테스트 통과
- [ ] QTest 통합 테스트 통과
- [ ] 3개 플랫폼 빌드 검증 (Linux, macOS, Windows)
- [ ] 메모리 누수 없음
- [ ] Architecture.md 검토 완료
- [ ] README.md 업데이트 및 스크린샷

### Phase 12 예상 추가
- 예상 코드: +3,500 라인 (GUI 컴포넌트, Qt 통합)
- 예상 테스트: +25 단위 테스트, +15 QTest 통합 테스트
- 예상 외부 의존성: Qt 6 (Core, Widgets, Concurrent)
- 예상 완료율: 98% (Phase 12 완료 시, 전체 프로젝트 거의 완성)

---

## 현재 진행 상황
- **완료된 Phase**: 1, 2, 3, 5, 6, 8, 9, 11 (8개 Phase) ✅
- **부분 완료**: 
  * Phase 4 (멀티스레딩만), 
  * Phase 7 (성능 테스트만 미완), 
  * Phase 10 (인터페이스, Mock, 코어 로직, **LocalEmbeddingProvider, SimpleTokenizer**)
- **진행 중**: Phase 10 (Semantic Search - ONNX 통합, FAISS, CLI 통합) 🔴 최우선
- **설계 완료**: Phase 12 (GUI - architecture.md에 완전한 설계 문서화) ✅

### 프로젝트 통계
- **완료율**: ~78% (핵심 검색 기능 완성, Semantic Search 기반 완료, LocalEmbedding 구현)
- **총 코드**: ~4,600 라인 (헤더 + 구현)
- **총 테스트**: 140 단위 테스트 + 40 통합 테스트 = 180 테스트 (+26 from Phase 10.4)
- **테스트 통과율**: 100%
- **아키텍처**: Clean Architecture 4계층 구조 (Domain, Application, Adapters, Infrastructure)

### 최근 완료 작업 (Phase 10.4)
- ✅ LocalEmbeddingProvider 기본 구조 (Pimpl, placeholder 추론)
- ✅ SimpleTokenizer (lowercase, punctuation, special tokens)
- ✅ CMake 빌드 옵션 (ENABLE_LOCAL_EMBEDDING)
- ✅ 26개 단위 테스트 (LocalEmbedding 13 + Tokenizer 13)
- **커밋**: def98a3, 5880b17

### 다음 작업 우선순위
1. **Phase 10 완료** 🔴 (FAISS VectorStore, OpenAI Provider, CLI 통합)
2. **Phase 12 구현** (Qt GUI - Everything UI 스타일)
3. **Phase 4 나머지** (캐싱, 메모리 최적화, 조기 종료)

### 완료된 Phase 목록
자세한 내용은 [todos_done.md](./todos_done.md)를 참조하세요:
- ✅ Phase 1: 프로젝트 기본 구조 설정
- ✅ Phase 2: 기본 검색 기능
- ✅ Phase 3: 고급 검색 및 필터링
- ✅ Phase 5: CLI 인터페이스
- ✅ Phase 6: 에러 처리 및 로깅
- ✅ Phase 8: 문서화 및 배포
- ✅ Phase 9: 설정 파일 지원
- ✅ Phase 11: Clean Architecture Refactoring (2026-02-07)

### 향후 예상 추가

#### Phase 10 (Semantic Search) 완료 시
- 예상 코드: +2,000 라인 (SemanticSearcher, Providers, VectorStore)
- 예상 테스트: +30 단위 테스트, +10 통합 테스트
- 예상 외부 의존성: FAISS, libcurl, JSON library
- 예상 완료율: 85%

#### Phase 12 (GUI Implementation) 완료 시
- 예상 코드: +3,500 라인 (Everything UI 스타일 GUI)
- 예상 테스트: +25 단위 테스트, +15 QTest 통합 테스트
- 예상 외부 의존성: Qt 6 (Core, Widgets, Concurrent)
- 예상 완료율: 98% (거의 완전한 제품)

---

**마지막 업데이트**: 2026-02-07


### 프로젝트 통계
- **완료율**: ~75% (핵심 검색 기능 완성, Semantic Search 기반 완료, GUI 설계 완료)
- **총 코드**: ~4,300 라인 (헤더 + 구현)
- **총 테스트**: 114 단위 테스트 + 40 통합 테스트 = 154 테스트
- **테스트 통과율**: 100%
- **아키텍처**: Clean Architecture 4계층 구조 (Domain, Application, Adapters, Infrastructure)

### 다음 작업 우선순위
1. **Phase 10 완료** 🔴 (FAISS VectorStore, OpenAI Provider, CLI 통합)
2. **Phase 12 구현** (Qt GUI - Everything UI 스타일)
3. **Phase 4 나머지** (캐싱, 메모리 최적화, 조기 종료)

### 완료된 Phase 목록
#### Phase 10 (Semantic Search)
- 예상 코드: +2,000 라인 (SemanticSearcher, Providers, VectorStore)
- 예상 테스트: +30 단위 테스트, +10 통합 테스트
- 예상 외부 의존성: FAISS, libcurl, JSON library
- 예상 완료율: 95% (Phase 10 완료 시)

#### Phase 12 (GUI Implementation)
- 예상 코드: +3,500 라인 (Everything UI 스타일 GUI)
- 예상 테스트: +25 단위 테스트, +15 QTest 통합 테스트
- 예상 외부 의존성: Qt 6 (Core, Widgets, Concurrent)
- 예상 완료율: 98% (Phase 12 완료 시, 거의 완전한 제품)
---

**마지막 업데이트**: 2026-02-07

