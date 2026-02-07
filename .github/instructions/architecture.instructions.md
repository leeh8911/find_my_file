# Architecture Instructions

> 📖 **Related Docs**: [architecture.md](../../docs/architecture.md), [requirements.md](../../docs/requirements.md)

---

## Overview

본 프로젝트는 **SOLID 원칙**을 기반으로 설계된다.

아키텍처 결정(요약)은 `docs/architecture.md` 또는 별도 ADR 문서에 기록한다.

아키텍처 설계 방법론(원칙, 패턴, 의존성/데이터 흐름 등)은 본 문서에 통합되어 있으며,\n+코드의 **실제 구조(파일 배치, 컴포넌트 위치)**는 `docs/architecture.md`에서 확인한다.

---

## Clean Architecture Principles

### Dependency Rule
**핵심 규칙: 의존성은 항상 안쪽(내부)을 향해야 한다.**

```
Infrastructure → Adapters → Application → Domain
(바깥)                                    (안쪽)
```

- 외부 레이어는 내부 레이어에 의존 가능
- 내부 레이어는 외부 레이어를 알 수 없음
- Domain은 어떤 레이어에도 의존하지 않음

### Layer Structure (Conceptual)

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

## SOLID Principles

### S - Single Responsibility Principle (SRP)

**정의:** 클래스는 하나의 책임만 가져야 한다.

#### ✅ Good Example
```cpp
// 책임: 파일 스캔
class FileScanner {
    std::vector<FileInfo> scan(const std::string& path);
};

// 책임: 패턴 매칭
class PatternMatcher {
    bool matches(const std::string& text, const std::string& pattern);
};
```

#### ❌ Bad Example
```cpp
// 너무 많은 책임
class FileManager {
    std::vector<FileInfo> scan();  // 스캔
    bool matches();                // 매칭
    void output();                 // 출력
    void log();                    // 로깅
};
```

---

### O - Open/Closed Principle (OCP)

**정의:** 확장에는 열려 있고 수정에는 닫혀 있어야 한다.

#### ✅ Good Example
```cpp
// Interface for extension
class IEmbeddingProvider {
public:
    virtual std::vector<float> generateEmbedding(const std::string& text) = 0;
    virtual ~IEmbeddingProvider() = default;
};

// Extend without modifying existing code
class OpenAIProvider : public IEmbeddingProvider { /*...*/ };
class LocalModelProvider : public IEmbeddingProvider { /*...*/ };
```

#### ❌ Bad Example
```cpp
class SearchEngine {
    void search() {
        if (type == "keyword") {
            // keyword search
        } else if (type == "semantic") {
            // semantic search
        }
        // Adding new type requires modifying this class
    }
};
```

---

### L - Liskov Substitution Principle (LSP)

**정의:** 자식 클래스는 부모 클래스를 대체할 수 있어야 한다.

#### ✅ Good Example
```cpp
class IVectorStore {
public:
    virtual std::vector<SemanticResult> search(
        const std::vector<float>& query, size_t topK) = 0;
};

// Both can replace IVectorStore without breaking contracts
class MockVectorStore : public IVectorStore { /*...*/ };
class FAISSVectorStore : public IVectorStore { /*...*/ };
```

#### ❌ Bad Example
```cpp
class Bird {
    virtual void fly() = 0;
};

class Penguin : public Bird {
    void fly() override {
        throw std::runtime_error("Penguins can't fly!");
        // Violates LSP: can't substitute Bird with Penguin
    }
};
```

---

### I - Interface Segregation Principle (ISP)

**정의:** 클라이언트는 사용하지 않는 인터페이스에 의존하면 안 된다.

#### ✅ Good Example
```cpp
// Small, focused interfaces
class IEmbeddingProvider {
    virtual std::vector<float> generateEmbedding(const std::string& text) = 0;
};

class IVectorStore {
    virtual std::vector<SemanticResult> search(...) = 0;
};
```

#### ❌ Bad Example
```cpp
// Fat interface
class ISearchEngine {
    virtual void keywordSearch() = 0;
    virtual void semanticSearch() = 0;
    virtual void indexFiles() = 0;
    virtual void exportResults() = 0;
    virtual void configureLogging() = 0;
    // Client may only need keywordSearch but must implement all
};
```

---

### D - Dependency Inversion Principle (DIP)

**정의:** 고수준 모듈은 저수준 모듈에 의존하면 안 되고, 둘 다 추상화에 의존해야 한다.

#### ✅ Good Example
```cpp
// High-level module depends on abstraction
class SemanticSearcher {
public:
    SemanticSearcher(
        std::unique_ptr<IEmbeddingProvider> provider,
        std::unique_ptr<IVectorStore> store)
        : provider_(std::move(provider)),
          store_(std::move(store)) {}
private:
    std::unique_ptr<IEmbeddingProvider> provider_;  // Abstraction
    std::unique_ptr<IVectorStore> store_;           // Abstraction
};
```

#### ❌ Bad Example
```cpp
// High-level module depends on concrete implementation
class SemanticSearcher {
public:
    SemanticSearcher() {
        provider_ = new OpenAIProvider();  // Concrete dependency
        store_ = new FAISSStore();         // Concrete dependency
    }
private:
    OpenAIProvider* provider_;
    FAISSStore* store_;
};
```

---

## Dependency Direction

### Layered Architecture

```
┌─────────────────────────┐
│      Application        │  (CLI, Config)
│    (main, parsers)      │
└───────────┬─────────────┘
            │ depends on
            ▼
┌─────────────────────────┐
│        Domain           │  (Core Business Logic)
│  (SemanticSearcher,     │
│   FileScanner, etc.)    │
└───────────┬─────────────┘
            │ depends on
            ▼
┌─────────────────────────┐
│      Abstractions       │  (Interfaces)
│  (IEmbeddingProvider,   │
│   IVectorStore, etc.)   │
└─────────────────────────┘
            ▲
            │ implements
┌───────────┴─────────────┐
│    Infrastructure       │  (External Dependencies)
│  (OpenAIProvider,       │
│   FAISSStore, etc.)     │
└─────────────────────────┘
```

### Rules
1. **Application → Domain**: 허용
2. **Domain → Infrastructure**: ❌ 금지
3. **Domain → Abstractions**: ✅ 필수
4. **Infrastructure → Abstractions**: ✅ 필수 (구현)
5. **Domain은 프레임워크에 의존하지 않음**

---

## Dependency Flow (Detailed)

```
main.cpp (최외곽 - 의존성 조립)
    ↓
Infrastructure Layer
    ├─ Logger
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
    │   ├─ PatternMatcher
    │   ├─ ContentSearcher
    │   └─ IgnorePatterns
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
   - ThreadPool은 생성자/setter로 주입 (DI)
   - ThreadPool 없이도 순차 스캔 동작 가능

2. **SemanticSearcher → IEmbeddingProvider/IVectorStore**
   - 구현체가 아닌 Port(인터페이스)에 의존
   - 구현체는 Infrastructure Layer에 위치
   - Mock 구현으로 테스트 가능

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

### Semantic Search Flow
```
1. main() → semantic search 옵션 감지

2. SemanticSearcher 초기화 (Use Case)
   ├─ EmbeddingProvider 주입 (DI, Port)
   └─ VectorStore 주입 (DI, Port)

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

### Port/Adapter Pattern (Hexagonal Architecture)

외부 시스템 접근은 Port(Interface) / Adapter(Implementation)로 감싼다.

```cpp
// Port (Interface)
class IEmbeddingProvider {
public:
    virtual std::vector<float> generateEmbedding(const std::string& text) = 0;
};

// Adapter (OpenAI implementation)
class OpenAIProvider : public IEmbeddingProvider {
    std::vector<float> generateEmbedding(const std::string& text) override {
        // Call external OpenAI API
    }
};

// Adapter (Mock for testing)
class MockEmbeddingProvider : public IEmbeddingProvider {
    std::vector<float> generateEmbedding(const std::string& text) override {
        // Return deterministic test data
    }
};
```

**Benefits:**
- 테스트 용이성 (Mock 사용 가능)
- 외부 의존성 교체 용이
- Domain logic 독립성

---

### Repository Pattern

데이터 저장/조회를 추상화한다.

```cpp
// Repository interface
class IVectorStore {
public:
    virtual void add(const std::string& id, 
                     const std::vector<float>& vector,
                     const VectorMetadata& metadata) = 0;
    virtual std::vector<SemanticResult> search(
        const std::vector<float>& query, size_t topK) = 0;
};

// In-memory implementation
class MockVectorStore : public IVectorStore { /*...*/ };

// FAISS implementation
class FAISSVectorStore : public IVectorStore { /*...*/ };
```

---

### Strategy Pattern

알고리즘을 런타임에 교체 가능하게 만든다.

```cpp
class IPatternMatcher {
public:
    virtual bool matches(const std::string& text, 
                        const std::string& pattern) = 0;
};

class WildcardMatcher : public IPatternMatcher { /*...*/ };
class RegexMatcher : public IPatternMatcher { /*...*/ };

class FileSearcher {
public:
    void setMatcher(std::unique_ptr<IPatternMatcher> matcher) {
        matcher_ = std::move(matcher);
    }
private:
    std::unique_ptr<IPatternMatcher> matcher_;
};
```

---

## Interface Design Guidelines

### When to Create an Interface

✅ **Create Interface When:**
- 여러 구현이 있을 가능성
- 외부 의존성 (API, DB, File system)
- 테스트 시 Mock 필요
- 의존성 역전 필요

❌ **Don't Create Interface When:**
- 단 하나의 구현만 있을 것이 확실
- Pure data structure (DTO, VO)
- Utility functions

### Interface Naming
- Prefix `I` for interfaces: `IEmbeddingProvider`
- Or suffix with role: `EmbeddingProviderInterface`

Project standard: **Use `I` prefix**

---

## Component Boundaries

각 컴포넌트는 명확한 책임과 경계를 가진다.

### Example: Semantic Search Components

```
SemanticSearcher
├── Responsibilities:
│   ├── Index files/directories
│   ├── Perform semantic search
│   └── Manage search lifecycle
├── Dependencies:
│   ├── IEmbeddingProvider (abstraction)
│   ├── IVectorStore (abstraction)
│   └── FileInfo (domain model)
└── Does NOT:
    ├── Know how embeddings are generated
    ├── Know how vectors are stored
    └── Parse command-line arguments
```

---

## Error Handling Strategy

### Exceptions vs Return Codes

**Use Exceptions for:**
- Unexpected errors (network failure, file not found)
- Errors that can't be handled locally
- Constructor failures

**Use Return Codes for:**
- Expected failures (search returns no results)
- Performance-critical paths
- Validation errors

### Example
```cpp
class SemanticSearcher {
public:
    // Exception for unexpected error
    void SemanticSearcher(/* constructor params */) {
        if (!provider) {
            throw std::invalid_argument("Provider cannot be null");
        }
    }
    
    // Return value for expected case
    std::vector<SemanticResult> search(const std::string& query) {
        // Returns empty vector if no results (not an exception)
        return results;
    }
};
```

---

## Testability Guidelines

### Design for Testability

1. **Constructor Injection:** 의존성을 생성자로 주입
2. **Interface-based:** 추상화에 의존
3. **Pure Functions:** 가능한 곳에서는 순수 함수
4. **No Hidden State:** 전역 변수 금지

### Example
```cpp
// Testable design
class SemanticSearcher {
public:
    // Dependencies injected via constructor
    SemanticSearcher(
        std::unique_ptr<IEmbeddingProvider> provider,
        std::unique_ptr<IVectorStore> store);
};

// In tests
TEST(SemanticSearcherTest, SearchQuery) {
    auto mockProvider = std::make_unique<MockEmbeddingProvider>();
    auto mockStore = std::make_unique<MockVectorStore>();
    SemanticSearcher searcher(std::move(mockProvider), std::move(mockStore));
    // Easy to test with mocks
}
```

---

## Checklist for New Components

Before creating a new component, verify:

- [ ] Single Responsibility: 하나의 명확한 책임
- [ ] Open/Closed: 확장 가능한 설계
- [ ] Dependency Inversion: 추상화에 의존
- [ ] Interface Segregation: 작고 집중된 인터페이스
- [ ] Testability: Mock 가능한 의존성
- [ ] Documentation: `docs/architecture.md`에 기록

---

## Architecture Review Questions

새로운 설계를 리뷰할 때 질문:

1. **SRP**: 이 클래스가 변경되어야 하는 이유가 여러 개인가?
2. **OCP**: 새로운 기능 추가 시 기존 코드 수정이 필요한가?
3. **LSP**: 자식 클래스가 부모 클래스를 완전히 대체할 수 있는가?
4. **ISP**: 클라이언트가 사용하지 않는 메서드에 의존하는가?
5. **DIP**: 고수준 모듈이 구체적인 구현에 의존하는가?

---

## Technology Stack

### Core Technologies
- **Language**: C++17
- **Build System**: CMake 3.14+
- **Standard Library**: STL (`std::filesystem`, `std::regex`, `std::optional`)

### Testing
- **Framework**: Google Test (gtest)
- **Unit Tests**: Domain / Application / Adapters / Infrastructure
- **Integration Tests**: bash scripts 기반 시나리오 테스트

### Development Tools
- **Code Style**: clang-format
- **Static Analysis**: clang-tidy, cpplint
- **Compiler Flags**: `-Wall -Wextra -Wpedantic -Werror`

---

## Testing Strategy

### Unit Tests by Layer (예시 분류)
- Domain: `FileInfo`, `SearchResult`, `SearchCriteria`
- Application: `PatternMatcher`, `ContentSearcher`, `IgnorePatterns`, `FileScanner`, `SemanticSearcher`
- Adapters: `CommandLineParser`, `OutputFormatter`
- Infrastructure: `Logger`, `ConfigFile`, `ThreadPool`

### Integration Tests
- CLI 시나리오 기반 테스트(`test/integrationtest/*.sh`)
- 검색/필터/출력/로깅/설정 파일 등의 end-to-end 검증

---

## Migration Plan (Architecture Refactoring)

### 목표
현재 flat 구조를 Clean Architecture 기반 계층 구조로 리팩터링

### 작업 범위
1. 폴더 구조 재편성
2. CMakeLists.txt 업데이트 (include path 변경)
3. `#include` 경로 수정
4. 테스트 파일 이동 및 재편성
5. 문서 업데이트

### 이동 계획 (요약)
- Domain, Application, Adapters, Infrastructure 별로 `include/` 및 `src/` 재배치
- 테스트도 계층별 디렉터리로 분리
- 상세 경로는 `docs/architecture.md`가 아닌 계획 문서에서 관리

### 검증 단계
1. 각 단계 완료 후 빌드 성공 확인
2. 단위/통합 테스트 통과
3. 정적 분석 도구 통과

---

## Performance Considerations
- **멀티스레드 스캔**: ThreadPool 기반 병렬 디렉터리 검색
- **메모리 효율**: 라인 단위 파일 읽기, 10MB 제한
- **패턴 매칭 최적화**: 재귀적 와일드카드 알고리즘
- **바이너리 파일 조기 배제**: Null byte 검사

---

## Security Considerations
- **Path Traversal**: `std::filesystem::canonical()` 사용
- **Symbolic Link Loops**: 기본적으로 심볼릭 링크 추적 비활성화
- **Resource Limits**: 파일 크기/재귀 깊이 제한
- **Input Validation**: Regex 패턴 검증, 파일 존재 여부 확인

---

## Coding Standards
- **Naming**: Classes(PascalCase), Methods/Variables(camelCase), Constants(UPPER_SNAKE_CASE)
- **Indentation**: 4 spaces
- **Line Length**: 80 chars
- **Braces**: Allman style
- **Header Guards**: `#pragma once`
- **Error Handling**: 예외(치명적), 반환값(예상 실패)

---

## Next Steps
- Clean Architecture 기반 폴더 구조 재편성
- Semantic Search 고도화
- Hybrid Search(키워드+의미론) 도입

---

## References
- **Clean Architecture**: Robert C. Martin
- **Hexagonal Architecture**: Alistair Cockburn
- **SOLID Principles**: Robert C. Martin
- **Domain-Driven Design**: Eric Evans

---

## 📌 Quick Reference

### SOLID Checklist
- ✅ SRP: 하나의 책임
- ✅ OCP: 확장 가능
- ✅ LSP: 대체 가능
- ✅ ISP: 작은 인터페이스
- ✅ DIP: 추상화 의존

### Dependency Rules
- ✅ Application → Domain
- ✅ Domain → Abstractions
- ✅ Infrastructure → Abstractions
- ❌ Domain → Infrastructure
- ❌ Abstractions → Concrete

### Design Patterns
- Port/Adapter: 외부 시스템
- Repository: 데이터 접근
- Strategy: 알고리즘 교체
- Factory: 객체 생성

### Before Coding
1. Read `docs/architecture.md`
2. Check SOLID principles
3. Design interfaces first
4. Write tests
5. Implement
