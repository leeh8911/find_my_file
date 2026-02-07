# Architecture Instructions

> 📖 **Related Docs**: [architecture.md](../../docs/architecture.md), [requirements.md](../../docs/requirements.md)

---

## Overview

본 프로젝트는 **SOLID 원칙**을 기반으로 설계된다.

모든 아키텍처 결정은 `docs/architecture.md`에 문서화되어야 한다.

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
