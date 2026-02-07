# C++ Standards and Static Analysis

> 📖 **Related Docs**: [commit-style.instructions.md](commit-style.instructions.md)

---

## Overview

본 프로젝트는 **C++17** 표준을 사용한다.

코드 품질을 위해 다음 도구들을 사용:
- **clang-format**: 코드 포맷팅
- **clang-tidy**: 정적 분석 및 linting
- **cpplint**: Google 스타일 가이드 검사

---

## C++ Standard

### Version
- **C++17** (required)
- C++20 features not allowed
- Backward compatibility not needed

### Key C++17 Features Used

```cpp
// Filesystem
#include <filesystem>
std::filesystem::path p = "/some/path";

// Optional
#include <optional>
std::optional<int> value = getValue();

// String view
#include <string_view>
void process(std::string_view text);

// Structured bindings
auto [key, value] = map.find(...);

// if with initializer
if (auto result = calculate(); result > 0) {
    // use result
}
```

---

## clang-format

### Purpose
자동 코드 포맷팅 - **일관된 스타일 유지**

### Configuration

`.clang-format` 파일을 프로젝트 루트에 배치:
```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 80
```

### When to Apply
- **REFACTOR 단계 전**: 반드시 적용
- 새 파일 작성 후
- 기존 파일 수정 후

### Usage

```bash
# Single file
clang-format -i src/my_file.cpp

# All source files
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Check without modifying
clang-format --dry-run --Werror src/my_file.cpp
```

### Commit Message
```
style: apply clang-format to all source files
```

---

## clang-tidy

### Purpose
정적 분석 - **버그, UB, 코드 냄새 탐지**

### Key Checks

#### Modernize
- Use `auto` where appropriate
- Use `nullptr` instead of `NULL`
- Use range-based for loops
- Use `override` keyword

#### Performance
- Pass by const reference
- Avoid unnecessary copies
- Use `std::move` for transfers

#### Readability
- Consistent naming
- Function complexity
- Magic numbers

#### Bug-prone patterns
- Null pointer dereference
- Resource leaks
- Uninitialized variables

### Configuration

`.clang-tidy` 파일:
```yaml
Checks: >
  -*,
  bugprone-*,
  modernize-*,
  performance-*,
  readability-*,
  -modernize-use-trailing-return-type

CheckOptions:
  - key: readability-identifier-naming.ClassCase
    value: CamelCase
  - key: readability-identifier-naming.FunctionCase
    value: camelCase
  - key: readability-identifier-naming.VariableCase
    value: camelBack
```

### Usage

```bash
# Check single file
clang-tidy src/my_file.cpp -- -I include -std=c++17

# Check all files
find src -name "*.cpp" | xargs clang-tidy -p build

# Auto-fix (use with caution)
clang-tidy -fix src/my_file.cpp -- -I include -std=c++17
```

### Suppression

필요한 경우에만 경고를 억제 (주석으로 이유 명시):

```cpp
// NOLINT reason: <why suppression is needed>
auto* ptr = reinterpret_cast<int*>(address);  // NOLINT: hardware access

// NOLINTNEXTLINE reason: <why>
legacy_function();
```

### Handling Warnings

#### 1. Fix the Issue (권장)
```cpp
// Before (clang-tidy warning)
for (int i = 0; i < vec.size(); i++) {
    process(vec[i]);
}

// After (fixed)
for (const auto& item : vec) {
    process(item);
}
```

#### 2. Suppress with Justification
```cpp
// External API requires this pattern
void* raw_ptr = malloc(size);  // NOLINT: C API requirement
```

---

## cpplint

### Purpose
Google C++ 스타일 가이드 준수 체크

### Key Style Rules

#### Header Guards
```cpp
#ifndef PROJECT_PATH_FILE_H
#define PROJECT_PATH_FILE_H
// ...
#endif  // PROJECT_PATH_FILE_H
```

#### Include Order
```cpp
// Related header (if .cpp file)
#include "my_class.h"

// C++ standard library
#include <vector>
#include <string>

// Other libraries
#include <gtest/gtest.h>

// Project headers
#include "another_class.h"
```

#### Naming Conventions
```cpp
// Classes: UpperCamelCase
class MyClass { };

// Functions: lowerCamelCase
void doSomething();

// Variables: snake_case or camelCase (consistent within file)
int my_variable;

// Constants: kCamelCase
const int kMaxSize = 100;

// Namespaces: lowercase
namespace myproject { }
```

#### Comments
```cpp
// Single line comment

/**
 * @brief Multi-line documentation comment
 *
 * Detailed description
 *
 * @param name Parameter description
 * @return Return value description
 */
```

### Usage

```bash
# Check single file
cpplint src/my_file.cpp

# Check all files
cpplint --recursive src include

# With filters
cpplint --filter=-whitespace/indent src/*.cpp
```

### Common Issues

```cpp
// Issue: Line too long (> 80 chars)
// Fix: Break line
std::string long_name = 
    "This is a very long string that exceeds 80 characters";

// Issue: Missing header guard
// Fix: Add proper guard

// Issue: Include order
// Fix: Sort includes by category
```

---

## Coding Standards

### General Guidelines

#### 1. Const Correctness
```cpp
// Use const where applicable
void process(const std::string& input);

const auto& getReference() const;
```

#### 2. RAII (Resource Acquisition Is Initialization)
```cpp
// Good: Automatic cleanup
std::unique_ptr<Resource> resource = acquireResource();

// Bad: Manual cleanup
Resource* resource = new Resource();
// ... might forget to delete
```

#### 3. Prefer Standard Library
```cpp
// Good
std::vector<int> numbers;

// Bad
int* numbers = new int[size];
```

#### 4. Avoid Raw Pointers
```cpp
// Good
std::unique_ptr<Widget> widget = std::make_unique<Widget>();
std::shared_ptr<Config> config = std::make_shared<Config>();

// Bad
Widget* widget = new Widget();
```

#### 5. Use Explicit
```cpp
// Prevent implicit conversions
class MyClass {
public:
    explicit MyClass(int value);
};
```

---

## Static Analysis Workflow

### During Development

```bash
# 1. Write code
vim src/my_feature.cpp

# 2. Format
clang-format -i src/my_feature.cpp

# 3. Check for issues
clang-tidy src/my_feature.cpp -- -I include -std=c++17

# 4. Fix warnings
# (edit code)

# 5. Verify style
cpplint src/my_feature.cpp

# 6. Build and test
cd build && make && ./tests
```

### Before Commit

```bash
# Run all checks
./scripts/check-style.sh  # if available

# Or manually
clang-format -i $(find src include -name "*.cpp" -o -name "*.h")
clang-tidy -p build src/*.cpp
cpplint src/*.cpp include/*.h

# Then commit
git add .
git commit -m "feat: implement new feature"
```

---

## Tooling Definition of Done

Phase 병합 전 필수 조건:

- [ ] **clang-format**
  - 모든 소스 파일에 적용
  - 일관된 포맷팅
  
- [ ] **clang-tidy**
  - 버그 관련 경고 없음
  - UB(Undefined Behavior) 없음
  - 필요한 경우에만 억제
  
- [ ] **cpplint**
  - 스타일 가이드 준수
  - 헤더 가드 올바름
  - Include 순서 올바름

---

## Tool Installation

### Ubuntu/Debian
```bash
sudo apt-get install clang-format clang-tidy
pip install cpplint
```

### macOS
```bash
brew install clang-format
pip install cpplint
```

### Verification
```bash
clang-format --version
clang-tidy --version
cpplint --version
```

---

## Common Patterns and Anti-patterns

### ✅ Good Patterns

```cpp
// 1. Smart pointers
std::unique_ptr<T> ptr = std::make_unique<T>();

// 2. Range-based for
for (const auto& item : collection) { }

// 3. Const references
void process(const std::string& text);

// 4. auto with clear type
auto result = std::make_unique<Widget>();

// 5. nullptr
if (ptr != nullptr) { }
```

### ❌ Anti-patterns

```cpp
// 1. Raw pointers (ownership)
T* ptr = new T();  // Use unique_ptr instead

// 2. Manual memory management
delete ptr;  // Use RAII/smart pointers

// 3. C-style casts
int value = (int)floatValue;  // Use static_cast<int>

// 4. NULL
if (ptr == NULL)  // Use nullptr

// 5. Magic numbers
if (size > 100)  // Define const int kMaxSize = 100
```

---

## 📌 Quick Reference

### Format Code
```bash
clang-format -i <file>
```

### Check Code Quality
```bash
clang-tidy <file> -- -I include -std=c++17
```

### Check Style
```bash
cpplint <file>
```

### Before Committing
```bash
# Format all
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Check all
clang-tidy -p build src/*.cpp
cpplint src/*.cpp include/*.h

# Build and test
cd build && make && ./tests
```

### Commit Style Changes
```
style: apply clang-format to all source files

style: fix clang-tidy warnings in semantic_searcher.cpp
```
