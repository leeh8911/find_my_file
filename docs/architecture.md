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

## Tests (현재 위치 기준)
- **Unit Tests**: `tests/`
- **Integration Tests**: `test/integrationtest/*.sh`

---

## Notes
- 현재 코드는 `include/`, `src/`에 flat 구조로 존재합니다.
- 레이어별 폴더 구조로의 리팩터링 계획/원칙/절차는
  `.github/instructions/architecture.instructions.md`에 정리되어 있습니다.
