# Commit Style Instructions

> 📖 **Related Docs**: [tdd-cycle.instructions.md](tdd-cycle.instructions.md), [workflow.instructions.md](workflow.instructions.md)

---

## Overview

본 프로젝트는 **Conventional Commits** 스타일을 사용한다.

모든 커밋 메시지는 다음 형식을 따른다:
```
<type>: <description>

[optional body]

[optional footer]
```

---

## Commit Types

### `test:` - Test Code
**When:** RED 단계에서 실패하는 테스트 작성 시

**Examples:**
```
test: add failing test for file scanner recursive mode

test: add failing test for semantic search similarity

test: add integration test for end-to-end indexing
```

### `feat:` - New Feature
**When:** GREEN 단계에서 새로운 기능 구현 시

**Examples:**
```
feat: implement file scanner recursive directory traversal

feat: implement semantic search with embedding provider

feat: add CLI flag for case-insensitive search
```

### `fix:` - Bug Fix
**When:** 버그 수정 시 (GREEN 또는 별도 fix)

**Examples:**
```
fix: correct pattern matching for wildcard characters

fix: resolve memory leak in file scanner

fix: handle empty query in semantic search
```

### `refactor:` - Code Refactoring
**When:** REFACTOR 단계에서 구조 개선 시

**Examples:**
```
refactor: extract file type detection to separate class

refactor: apply dependency injection for embedding provider

refactor: simplify search result aggregation logic
```

### `docs:` - Documentation
**When:** 문서 변경 시 (특히 todos.md, architecture.md)

**Examples:**
```
docs: add todos for phase bootstrap

docs: mark Task 5 as complete in todos.md

docs: update architecture for semantic search pipeline

docs: add API documentation for SemanticSearcher
```

**Important:** 
- `docs/todos.md` 변경은 **항상 별도 커밋**
- `docs/architecture.md` 변경은 **항상 별도 커밋**
- 코드 변경과 문서 변경을 **같은 커밋에 섞지 않음**

### `style:` - Code Style
**When:** 포맷팅, 세미콜론 등 (동작에 영향 없음)

**Examples:**
```
style: apply clang-format to all source files

style: fix indentation in semantic_searcher.cpp
```

### `perf:` - Performance Improvement
**When:** 성능 최적화 시

**Examples:**
```
perf: optimize vector similarity calculation with SIMD

perf: add caching for repeated embedding generation
```

### `build:` - Build System
**When:** CMake, 의존성, 빌드 설정 변경 시

**Examples:**
```
build: add FAISS library dependency to CMakeLists.txt

build: configure C++17 standard in CMake

build: add optional semantic search feature flag
```

### `ci:` - CI/CD
**When:** GitHub Actions, CI 설정 변경 시

**Examples:**
```
ci: add clang-tidy check to workflow

ci: configure test coverage reporting
```

### `chore:` - Maintenance
**When:** 기타 도구, 설정 변경 (코드/테스트 무관)

**Examples:**
```
chore: update .gitignore for build artifacts

chore: add .clang-format configuration
```

---

## Commit Message Structure

### Description (필수)
- 현재형, 명령형 사용 ("add" not "added" or "adds")
- 첫 글자 소문자
- 마침표 없음
- 50자 이내 권장

✅ Good:
```
feat: implement semantic search query processing
```

❌ Bad:
```
Feat: Implemented the semantic search query processing feature.
```

### Body (선택)
- 72자마다 줄바꿈
- "무엇을" 보다 "왜"와 "어떻게"에 집중
- 빈 줄로 description과 분리

Example:
```
feat: implement semantic search query processing

The query processor normalizes text, generates embeddings,
and searches the vector store for similar documents.

This enables users to find files using natural language
instead of exact keyword matching.
```

### Footer (선택)
- Breaking changes
- Issue references

Example:
```
feat: change search API return type

BREAKING CHANGE: search() now returns SemanticResult instead of FileInfo

Closes #123
```

---

## TDD Cycle Commit Pattern

각 TDD 사이클은 **3개의 커밋**을 생성한다:

```
test: add failing test for user authentication
↓
feat: implement user authentication with JWT
↓
refactor: extract token validation to separate class
```

---

## Document Change Commit Pattern

문서 변경은 **항상 별도 커밋**:

```bash
# 1. Code implementation
git commit -m "feat: implement semantic searcher core"

# 2. Update todos.md (separate commit)
git commit -m "docs: mark Task 5 as complete"

# 3. Update architecture.md if needed (separate commit)
git commit -m "docs: add SemanticSearcher component to architecture"
```

---

## Multi-file Commit Guidelines

### ✅ Same Logical Change
여러 파일이 하나의 논리적 변경에 속하면 하나의 커밋:
```
feat: implement semantic search

- include/semantic_searcher.h
- src/semantic_searcher.cpp
- tests/test_semantic_searcher.cpp
```

### ❌ Mixing Concerns
서로 다른 관심사는 별도 커밋:
```
# Bad: 섞지 말 것
feat: implement search and update docs

# Good: 분리
feat: implement semantic search functionality
docs: mark semantic search task complete
```

---

## Commit Frequency

### When to Commit

#### During TDD
- RED: 실패하는 테스트 작성 완료 시
- GREEN: 테스트 통과 구현 완료 시
- REFACTOR: 각 리팩토링 단계 완료 시

#### During Development
- 의미있는 단위 작업 완료 시
- 모든 테스트 통과 상태에서
- 빌드가 깨지지 않은 상태에서

### Avoid
❌ 작업 종료 시 한꺼번에 커밋
❌ 빌드 실패 상태 커밋
❌ 테스트 실패 상태 커밋 (RED phase 제외)
❌ 하루 종료 전 "WIP" 커밋

---

## Atomic Commits

각 커밋은 **원자적(atomic)**이어야 한다:

### Atomic Commit Principles
1. **One Purpose**: 하나의 목적만 갖는다
2. **Complete**: 부분적 구현이 아니다
3. **Tested**: 모든 테스트가 통과한다
4. **Building**: 컴파일/빌드된다
5. **Reviewable**: 리뷰하기 쉬운 크기

### Example: Good Atomic Commits
```
feat: add IEmbeddingProvider interface
feat: implement MockEmbeddingProvider
feat: add IVectorStore interface
feat: implement MockVectorStore
feat: implement SemanticSearcher with interfaces
```

### Example: Bad Non-Atomic Commits
```
feat: implement entire semantic search feature

(1000+ lines, multiple components, hard to review)
```

---

## Rewriting History

### Allowed (Before Push)
- `git commit --amend` for typos in last commit
- `git rebase -i` to clean up local commits
- Squashing WIP commits

### Not Allowed (After Push)
- Force push to shared branches
- Rewriting published history
- Amending commits others may have pulled

---

## Branch-Specific Rules

### `main` branch
- Only merge commits from phase branches
- No direct commits
- All tests must pass

### `phase/*` branches
- TDD cycle commits (test → feat → refactor)
- Document commits (`docs:`)
- Incremental, atomic commits

---

## Examples by Feature

### Example 1: Simple Feature
```
test: add failing test for pattern matching with wildcards
feat: implement wildcard pattern matching
refactor: extract pattern parsing logic to helper function
docs: mark pattern matching task complete
```

### Example 2: Complex Feature (Semantic Search)
```
# Phase branch: phase/semantic-search

# Task 1: Interfaces
test: add failing tests for semantic search interfaces
feat: implement IEmbeddingProvider interface
feat: implement IVectorStore interface
feat: implement SemanticResult structure
docs: mark interface design task complete

# Task 2: Mocks
feat: implement MockEmbeddingProvider for testing
feat: implement MockVectorStore for testing
docs: mark mock implementation task complete

# Task 3: Core
test: add failing tests for SemanticSearcher
feat: implement SemanticSearcher index methods
feat: implement SemanticSearcher search methods
refactor: extract text chunking to separate method
refactor: extract file type detection logic
docs: mark SemanticSearcher task complete

# Merge to main
merge: phase/semantic-search - Add semantic search core
```

---

## Commit Message Checklist

Before committing, verify:

- [ ] Correct prefix (`test:`, `feat:`, `fix:`, `refactor:`, `docs:`, etc.)
- [ ] Description in imperative mood ("add" not "added")
- [ ] Description is clear and concise
- [ ] All tests pass (except RED phase)
- [ ] Code compiles/builds
- [ ] No unrelated changes included
- [ ] Document changes in separate commit

---

## 📌 Quick Reference

```bash
# TDD Cycle
git commit -m "test: add failing test for X"
git commit -m "feat: implement X to pass tests"
git commit -m "refactor: improve X structure"

# Documentation
git commit -m "docs: update todos.md for phase Y"
git commit -m "docs: add component Z to architecture"

# Bug Fix
git commit -m "fix: correct edge case in X"

# Style
git commit -m "style: apply clang-format"

# Build
git commit -m "build: add dependency for X"
```

---

## Anti-patterns to Avoid

❌ **Vague messages**
```
git commit -m "fix stuff"
git commit -m "update code"
git commit -m "changes"
```

❌ **Mixing concerns**
```
git commit -m "feat: add feature X and fix bug Y and update docs"
```

❌ **Wrong prefix**
```
git commit -m "feat: fix typo"  # Should be "fix:" or "style:"
git commit -m "fix: add new feature"  # Should be "feat:"
```

❌ **Past tense**
```
git commit -m "feat: implemented search"  # Should be "implement"
git commit -m "test: added test for X"    # Should be "add"
```
