# Testing Instructions

> 📖 **Related Docs**: [tdd-cycle.instructions.md](tdd-cycle.instructions.md), [architecture.instructions.md](architecture.instructions.md)

---

## Overview

본 프로젝트는 **Test-Driven Development (TDD)** 방식을 따른다.

테스트는 "구현"이 아니라 "행동"을 검증한다.

---

## Test Directory Structure

```
tests/
├── test_file_info.cpp           # Unit test for FileInfo
├── test_file_scanner.cpp        # Unit test for FileScanner
├── test_pattern_matcher.cpp     # Unit test for PatternMatcher
├── test_semantic_searcher.cpp   # Unit test for SemanticSearcher
└── ...
```

---

## Unit Testing Guidelines

### What to Test

#### ✅ Test
- Public interface behavior
- Edge cases and boundaries
- Error conditions
- Business logic
- Return values
- State changes

#### ❌ Don't Test
- Private methods (test through public interface)
- getters/setters (unless logic exists)
- Third-party libraries
- Language features
- Trivial code

### Test Framework

본 프로젝트는 **Google Test (gtest)** 를 사용한다.

```cpp
#include <gtest/gtest.h>

TEST(TestSuiteName, TestName) {
    // Arrange
    // Act
    // Assert
}
```

---

## Writing Good Unit Tests

### 1. AAA Pattern (Arrange-Act-Assert)

```cpp
TEST(FileScanner, ScansDirectoryRecursively) {
    // Arrange: Setup test data
    FileScanner scanner;
    std::string testDir = "/tmp/test";
    setupTestDirectory(testDir);
    
    // Act: Execute the behavior
    auto results = scanner.scan(testDir, true);
    
    // Assert: Verify the outcome
    EXPECT_EQ(results.size(), 3);
    EXPECT_TRUE(containsFile(results, "file1.txt"));
}
```

### 2. One Behavior Per Test

✅ **Good:**
```cpp
TEST(PatternMatcher, MatchesExactString) {
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.matches("hello", "hello"));
}

TEST(PatternMatcher, MatchesWildcard) {
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.matches("hello world", "hello*"));
}
```

❌ **Bad:**
```cpp
TEST(PatternMatcher, AllMatchingTests) {
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.matches("hello", "hello"));
    EXPECT_TRUE(matcher.matches("hello world", "hello*"));
    EXPECT_FALSE(matcher.matches("bye", "hello"));
    // Too many unrelated assertions
}
```

### 3. Descriptive Test Names

테스트 이름은 **검증하려는 행동**을 명확히 표현해야 한다.

**Naming Convention:**
```
TEST(ComponentName, BehaviorUnderTest_ExpectedOutcome)
```

**Examples:**
```cpp
TEST(FileScanner, ScanDirectoryNonRecursive)
TEST(PatternMatcher, MatchesWildcardStar)
TEST(SemanticSearcher, SearchReturnsEmptyWhenNoMatches)
TEST(ConfigFile, LoadNonExistentFileReturnsFalse)
```

### 4. Test Independence

각 테스트는 **독립적**이어야 한다.

```cpp
class FileScannerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create fresh test environment
        testDir = "/tmp/test_" + std::to_string(getpid());
        std::filesystem::create_directory(testDir);
    }
    
    void TearDown() override {
        // Clean up after each test
        std::filesystem::remove_all(testDir);
    }
    
    std::string testDir;
};
```

### 5. Fast Tests

- No file I/O (use in-memory or mock)
- No network calls
- No sleep/wait
- Run in parallel

**Exception:** 테스트 대상이 실제 파일 시스템인 경우 (FileScanner 등)

---

## Test Fixtures

공통 setup/teardown이 필요한 경우 Test Fixture 사용:

```cpp
class SemanticSearcherTest : public ::testing::Test {
protected:
    void SetUp() override {
        provider = std::make_unique<MockEmbeddingProvider>();
        store = std::make_unique<MockVectorStore>();
        searcher = std::make_unique<SemanticSearcher>(
            std::move(provider), std::move(store));
    }
    
    void TearDown() override {
        // Cleanup if needed
    }
    
    std::unique_ptr<MockEmbeddingProvider> provider;
    std::unique_ptr<MockVectorStore> store;
    std::unique_ptr<SemanticSearcher> searcher;
};

TEST_F(SemanticSearcherTest, IndexSingleFile) {
    // Use fixture members
    EXPECT_TRUE(searcher->indexFile("test.txt"));
}
```

---

## Mocking and Dependency Injection

### When to Mock

Mock을 사용해야 하는 경우:
- 외부 의존성 (API, DB, File system)
- 느린 작업
- 비결정적 동작 (random, time)
- 에러 조건 시뮬레이션

### Mock Example

```cpp
// Interface
class IEmbeddingProvider {
public:
    virtual std::vector<float> generateEmbedding(const std::string& text) = 0;
};

// Mock implementation for testing
class MockEmbeddingProvider : public IEmbeddingProvider {
public:
    std::vector<float> generateEmbedding(const std::string& text) override {
        // Return deterministic test data
        return std::vector<float>(128, 1.0f);
    }
};

// Test using mock
TEST(SemanticSearcher, UsesEmbeddingProvider) {
    auto mockProvider = std::make_unique<MockEmbeddingProvider>();
    auto mockStore = std::make_unique<MockVectorStore>();
    SemanticSearcher searcher(std::move(mockProvider), std::move(mockStore));
    
    // Test with predictable mock behavior
    auto results = searcher.search("test query", 5);
    EXPECT_GE(results.size(), 0);
}
```

---

## Assertions

### Common Assertions

```cpp
// Boolean
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Equality
EXPECT_EQ(actual, expected);
EXPECT_NE(actual, expected);

// Comparison
EXPECT_LT(val1, val2);  // Less than
EXPECT_LE(val1, val2);  // Less than or equal
EXPECT_GT(val1, val2);  // Greater than
EXPECT_GE(val1, val2);  // Greater than or equal

// Floating point
EXPECT_FLOAT_EQ(val1, val2);
EXPECT_DOUBLE_EQ(val1, val2);

// Strings
EXPECT_STREQ(str1, str2);

// Exceptions
EXPECT_THROW(statement, exception_type);
EXPECT_NO_THROW(statement);
```

### EXPECT vs ASSERT

- **EXPECT_**: 실패해도 테스트 계속 실행
- **ASSERT_**: 실패 시 즉시 테스트 중단

```cpp
TEST(Example, MultipleChecks) {
    auto result = someFunction();
    
    ASSERT_NE(result, nullptr);  // nullptr이면 다음 검사 불가능
    EXPECT_EQ(result->getValue(), 42);
    EXPECT_TRUE(result->isValid());
}
```

---

## Testing Best Practices

### 1. Test Behavior, Not Implementation

✅ **Good:**
```cpp
TEST(Calculator, AddTwoNumbers) {
    Calculator calc;
    EXPECT_EQ(calc.add(2, 3), 5);  // Testing public behavior
}
```

❌ **Bad:**
```cpp
TEST(Calculator, InternalStateAfterAdd) {
    Calculator calc;
    calc.add(2, 3);
    EXPECT_EQ(calc.getInternalBuffer(), 5);  // Testing implementation detail
}
```

### 2. Use Given-When-Then Comments

```cpp
TEST(FileScanner, RecursiveMode) {
    // Given: A directory with nested files
    setupTestDirectory("/tmp/test");
    FileScanner scanner;
    
    // When: Scanning recursively
    auto results = scanner.scan("/tmp/test", true);
    
    // Then: All nested files are found
    EXPECT_EQ(results.size(), 5);
}
```

### 3. Test Edge Cases

```cpp
TEST(PatternMatcher, EmptyPattern) {
    PatternMatcher matcher;
    EXPECT_FALSE(matcher.matches("text", ""));
}

TEST(PatternMatcher, EmptyText) {
    PatternMatcher matcher;
    EXPECT_FALSE(matcher.matches("", "pattern"));
}

TEST(PatternMatcher, BothEmpty) {
    PatternMatcher matcher;
    EXPECT_TRUE(matcher.matches("", ""));
}
```

### 4. Test Error Conditions

```cpp
TEST(FileScanner, NonExistentDirectory) {
    FileScanner scanner;
    EXPECT_THROW(scanner.scan("/nonexistent"), std::runtime_error);
}

TEST(SemanticSearcher, EmptyQuery) {
    SemanticSearcher searcher(provider, store);
    auto results = searcher.search("", 5);
    EXPECT_TRUE(results.empty());
}
```

---

## Integration Testing

### Not Implemented Yet

본 프로젝트는 현재 통합 테스트를 구현하지 않았습니다.

향후 필요 시 다음 방식으로 구현할 계획:
- CLI 기반 스크립트 테스트
- End-to-end 시나리오 검증
- 별도 디렉토리 관리

---

## Test Coverage Guidelines

### Prioritize Tests

1. **Core Business Logic** (highest priority)
   - SemanticSearcher
   - FileScanner
   - PatternMatcher

2. **Data Structures**
   - FileInfo
   - SearchResult
   - SemanticResult

3. **Utilities**
   - OutputFormatter
   - Logger

4. **UI/CLI** (lowest priority)
   - CommandLineParser
   - main.cpp

### Coverage Goals

- **Core components**: 90%+ coverage
- **Overall project**: 80%+ coverage
- **Critical paths**: 100% coverage

---

## Running Tests

### Build and Run All Tests

```bash
cd build
cmake ..
make
./tests
```

### Run Specific Test Suite

```bash
./tests --gtest_filter="FileScanner*"
./tests --gtest_filter="SemanticSearcherTest.*"
```

### Run Single Test

```bash
./tests --gtest_filter="FileScanner.ScanDirectoryRecursive"
```

### Verbose Output

```bash
./tests --gtest_print_time=1
```

---

## Test-Driven Development Workflow

### TDD Cycle

```
1. Write failing test (RED)
   → commit "test: add failing test for X"

2. Write minimal code to pass (GREEN)
   → commit "feat: implement X to pass tests"

3. Refactor code (REFACTOR)
   → commit "refactor: improve X structure"

4. Run all tests
   → ensure everything still passes

5. Repeat for next feature
```

### Example Session

```bash
# Start with failing test
git commit -m "test: add failing test for semantic search"

# Make it pass
git commit -m "feat: implement semantic search indexing"

# Refactor
git commit -m "refactor: extract chunking logic to helper"

# Run all tests
./build/tests

# All green? Move to next feature
```

---

## Common Testing Mistakes

### ❌ Testing Private Methods
```cpp
// Don't do this
TEST(MyClass, PrivateHelperMethod) {
    // Testing internal implementation
}
```

**Fix:** Test through public interface

### ❌ Flaky Tests
```cpp
// Flaky: depends on timing
TEST(Cache, Expiration) {
    cache.set("key", "value", 1s);
    sleep(2);  // Unreliable
    EXPECT_FALSE(cache.has("key"));
}
```

**Fix:** Use deterministic time or mock

### ❌ Test Interdependence
```cpp
// Test B depends on Test A running first
static int counter = 0;

TEST(Counter, FirstTest) {
    counter = 1;
}

TEST(Counter, SecondTest) {
    EXPECT_EQ(counter, 1);  // Breaks if run alone
}
```

**Fix:** Make tests independent

### ❌ Testing Too Much
```cpp
TEST(Everything, KitchenSink) {
    // 50 assertions testing multiple components
}
```

**Fix:** One behavior per test

---

## Definition of Done (Testing)

Phase 완료 전 확인사항:

- [ ] 모든 새 코드에 대한 단위 테스트 작성
- [ ] 모든 기존 테스트 통과
- [ ] 새 테스트 통과
- [ ] Edge cases 테스트
- [ ] Error conditions 테스트
- [ ] No flaky tests
- [ ] Tests run < 1 second (단위 테스트)

---

## 📌 Quick Reference

### Test Structure
```cpp
TEST(ComponentName, DescriptiveBehaviorName) {
    // Arrange: Setup
    // Act: Execute
    // Assert: Verify
}
```

### Common Patterns
```cpp
// Test fixture
class MyTest : public ::testing::Test {
protected:
    void SetUp() override { /* ... */ }
    void TearDown() override { /* ... */ }
};

// Mocking
auto mock = std::make_unique<MockClass>();

// Running tests
./tests --gtest_filter="MyTest.*"
```

### Best Practices
- ✅ Test behavior, not implementation
- ✅ One behavior per test
- ✅ Descriptive names
- ✅ Fast and independent
- ✅ Use mocks for dependencies
