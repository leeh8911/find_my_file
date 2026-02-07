# TDD Cycle Instructions

> 📖 **Related Docs**: [testing.instructions.md](testing.instructions.md), [commit-style.instructions.md](commit-style.instructions.md)

---

## Core Principle

개발은 **반드시 TDD의 RED → GREEN → REFACTOR 사이클**을 따른다.

각 단계마다 **커밋을 반드시 남긴다.**

---

## The Three Phases

### 🔴 RED Phase: Failing Test First

#### Goal
- 실패하는 테스트를 먼저 작성한다
- 테스트는 **요구사항을 가장 명확하게 표현**해야 한다

#### Rules
- 구현 코드를 작성하기 **전에** 테스트를 작성
- 테스트는 **명확히 실패**해야 함 (컴파일 실패도 RED 단계)
- 테스트 이름은 **검증하려는 행동**을 표현
- 한 번에 **하나의 행동만** 테스트

#### Checklist
- [ ] 테스트가 실패하는가?
- [ ] 실패 이유가 명확한가?
- [ ] 테스트 이름이 의도를 표현하는가?
- [ ] 필요한 최소한의 테스트인가?

#### Commit
```
test: add failing test for <feature>

- Test verifies <expected behavior>
- Currently fails with <error message>
```

**Examples:**
```
test: add failing test for file scanner recursive mode

test: add failing test for pattern matcher with wildcards

test: add failing test for semantic search result sorting
```

---

### 🟢 GREEN Phase: Make It Pass

#### Goal
- **가장 단순한 구현**으로 테스트를 통과시킨다
- 구조적 파괴는 허용하지 않는다

#### Rules
- 테스트를 통과시키는 **최소한의 코드만** 작성
- 기존 테스트가 깨지면 안 됨
- 아름다움보다 **동작**이 우선
- Hard-coding도 허용 (REFACTOR에서 개선)

#### Anti-patterns
❌ 테스트 없이 추가 기능 구현
❌ 과도한 추상화나 일반화
❌ 여러 클래스를 동시에 작성

#### Checklist
- [ ] 모든 테스트가 통과하는가?
- [ ] 새로운 코드가 기존 테스트를 깨뜨리지 않았는가?
- [ ] 최소한의 구현인가?

#### Commit
```
feat: implement <feature> to pass tests

- Implements <feature description>
- All tests passing
```

**Examples:**
```
feat: implement file scanner recursive directory traversal

feat: implement pattern matcher wildcard support

feat: implement semantic search with mock provider
```

---

### 🔵 REFACTOR Phase: Improve Structure

#### Goal
- **동작 변경 없이** 구조를 개선한다
- SOLID, 책임 분리, 의존성 역전을 적용한다

#### Rules
- 테스트는 **절대 변경하지 않음** (행동이 바뀌지 않으므로)
- 테스트가 계속 통과해야 함
- 리팩토링 중간에도 커밋 가능 (작은 단위)
- 코드 중복 제거
- 이름 개선
- 추상화 도입

#### When to Refactor
- 중복 코드 발견
- 긴 함수/클래스
- SOLID 원칙 위반
- 테스트하기 어려운 구조
- 이해하기 어려운 코드

#### Techniques
- Extract Method/Class
- Introduce Parameter Object
- Replace Conditional with Polymorphism
- Dependency Injection
- Interface Segregation

#### Checklist
- [ ] 모든 테스트가 여전히 통과하는가?
- [ ] 코드가 더 읽기 쉬워졌는가?
- [ ] SOLID 원칙을 더 잘 따르는가?
- [ ] 중복이 제거되었는가?

#### Commit
```
refactor: improve <component> structure

- Extract <responsibility> to separate class
- Apply <SOLID principle>
- Remove code duplication
```

**Examples:**
```
refactor: extract file type detection to separate class

refactor: apply dependency injection for embedding provider

refactor: simplify search result aggregation logic
```

---

## Complete TDD Cycle Example

### Scenario: Implement semantic search feature

#### Step 1: RED
```cpp
// tests/test_semantic_searcher.cpp
TEST(SemanticSearcherTest, SearchReturnsRelevantFiles) {
    SemanticSearcher searcher(mockProvider, mockStore);
    searcher.indexDirectory("/test/data");
    
    auto results = searcher.search("authentication logic", 5);
    
    EXPECT_GT(results.size(), 0);
    EXPECT_EQ(results[0].fileInfo.getName(), "auth.cpp");
}
```
**Commit:** `test: add failing test for semantic search query`

#### Step 2: GREEN
```cpp
// src/semantic_searcher.cpp
std::vector<SemanticResult> SemanticSearcher::search(
    const std::string& query, size_t topK) {
    auto queryEmbedding = provider_->generateEmbedding(query);
    return store_->search(queryEmbedding, topK);
}
```
**Commit:** `feat: implement semantic search query functionality`

#### Step 3: REFACTOR
```cpp
// Extracted query processing logic
class QueryProcessor {
public:
    std::vector<float> process(const std::string& query) {
        auto normalized = normalize(query);
        return embedder_->generate(normalized);
    }
private:
    std::string normalize(const std::string& text);
    std::unique_ptr<IEmbeddingProvider> embedder_;
};
```
**Commit:** `refactor: extract query processing logic to separate class`

---

## TDD Best Practices

### 1. Test Behavior, Not Implementation
✅ Good:
```cpp
TEST(Calculator, AddTwoPositiveNumbers) {
    Calculator calc;
    EXPECT_EQ(calc.add(2, 3), 5);
}
```

❌ Bad:
```cpp
TEST(Calculator, CallsInternalAddMethod) {
    // Testing implementation details
}
```

### 2. One Assertion Per Concept
✅ Good:
```cpp
TEST(FileScanner, RecursiveMode) {
    // Test one behavior: recursive scanning
}

TEST(FileScanner, FiltersByExtension) {
    // Test another behavior: filtering
}
```

❌ Bad:
```cpp
TEST(FileScanner, EverythingAtOnce) {
    // Testing multiple unrelated behaviors
}
```

### 3. Arrange-Act-Assert Pattern
```cpp
TEST(Example, TestName) {
    // Arrange: Setup test data
    SomeClass obj(param);
    
    // Act: Execute behavior
    auto result = obj.doSomething();
    
    // Assert: Verify outcome
    EXPECT_EQ(result, expected);
}
```

### 4. Keep Tests Fast
- No file I/O (use mocks)
- No network calls
- No sleep/wait
- Parallel execution safe

### 5. Test Names Should Tell a Story
```cpp
// Good names
TEST(UserService, CreateUserWithValidEmail)
TEST(UserService, CreateUserWithDuplicateEmailThrowsError)
TEST(UserService, CreateUserWithInvalidEmailThrowsError)

// Bad names
TEST(UserService, Test1)
TEST(UserService, CreateUser)
```

---

## Common TDD Mistakes to Avoid

### ❌ Writing Tests After Code
테스트가 구현에 맞춰져서 실제 요구사항 검증 실패

### ❌ Skipping RED Phase
테스트가 제대로 실패하는지 확인하지 않아 false positive

### ❌ Testing Too Much at Once
한 테스트에서 여러 행동을 검증하면 실패 원인 파악이 어려움

### ❌ Not Refactoring
GREEN에서 멈추면 기술 부채 누적

### ❌ Testing Implementation Details
리팩토링 시 테스트가 깨져서 유연성 저하

---

## TDD Flow Diagram

```
┌──────────────────────────────────────────────────┐
│                   Start                          │
└────────────────┬─────────────────────────────────┘
                 │
                 ▼
         ┌───────────────┐
         │   🔴 RED      │
         │               │
         │ Write failing │
         │     test      │
         └───────┬───────┘
                 │
           Commit test:
                 │
                 ▼
         ┌───────────────┐
         │   🟢 GREEN    │
         │               │
         │  Make it pass │
         │  (minimal)    │
         └───────┬───────┘
                 │
          Commit feat:
                 │
                 ▼
         ┌───────────────┐
         │  🔵 REFACTOR  │
         │               │
         │ Improve code  │
         │  structure    │
         └───────┬───────┘
                 │
        Commit refactor:
                 │
                 ▼
        ┌────────────────┐
        │  All tests     │
        │    pass?       │
        └────┬───────┬───┘
             │       │
          Yes│       │No → Fix
             │       └─────┐
             │             │
             ▼             ▼
        Next feature    Debug
```

---

## 📌 Quick Reference

### RED Phase
- Write failing test
- Test describes desired behavior
- Commit: `test: add failing test for X`

### GREEN Phase
- Write minimal code
- Make tests pass
- Commit: `feat: implement X to pass tests`

### REFACTOR Phase
- Improve structure
- Keep tests passing
- Commit: `refactor: improve X structure`

### Remember
- **One phase at a time**
- **One commit per phase**
- **Tests always pass after GREEN and REFACTOR**
