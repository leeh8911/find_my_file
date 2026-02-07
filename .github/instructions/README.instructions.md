# Development Instructions Overview

## Purpose
이 디렉토리는 AI Agent와 개발자가 본 프로젝트에서 작업할 때 따라야 하는 **모든 개발 규칙과 가이드라인**을 포함합니다.

작업을 시작하기 전에 **반드시 이 문서들을 읽고** 정의된 방식대로 작업하세요.

---

## 📚 Instruction Files

### 1. [workflow.instructions.md](workflow.instructions.md) ⭐ **필수**
프로젝트 워크플로우의 핵심 규칙
- `todos.md` 기반 작업 관리 (Single Source of Truth)
- `feat/<task-id>` 브랜치 전략
- Agent 작업 절차
- 병합 정책

**먼저 읽어야 할 문서**: 모든 작업의 시작점

### 2. [tdd-cycle.instructions.md](tdd-cycle.instructions.md) ⭐ **필수**
TDD 개발 사이클
- RED → GREEN → REFACTOR 단계별 가이드
- 각 단계의 목표와 제약사항
- 단계별 커밋 메시지 규칙

**Test-Driven Development 방식으로 작업할 때 참조**

### 3. [commit-style.instructions.md](commit-style.instructions.md)
커밋 메시지 작성 규칙
- Conventional Commits 스타일
- 각 prefix의 의미와 사용 시점
- 커밋 분리 원칙

**커밋할 때마다 참조**

### 4. [architecture.instructions.md](architecture.instructions.md)
아키텍처 설계 원칙
- SOLID 원칙 적용 가이드
- 의존성 방향 규칙
- Port/Adapter 패턴
- 계층 구조

**새로운 클래스나 모듈을 설계할 때 참조**

### 5. [testing.instructions.md](testing.instructions.md)
테스팅 표준과 규칙
- 단위 테스트 작성 가이드
- 통합 테스트 (스크립트 기반)
- 테스트 디렉토리 구조
- Definition of Done

**테스트 코드를 작성할 때 참조**

### 6. [cpp-standards.instructions.md](cpp-standards.instructions.md)
C++ 코딩 표준
- clang-format, clang-tidy, cpplint 사용법
- 스타일 가이드
- Static Analysis 통과 기준

**C++ 코드 작성 및 리팩토링 시 참조**

---

## 📖 Project Documentation (docs/)

작업 수행 시 참조해야 하는 프로젝트 문서들:

### [docs/todos.md](../../docs/todos.md) ⭐ **필수**
**작업 항목 관리**
- 현재 진행 중인 모든 작업
- Phase 별 작업 구조
- 작업 완료 상태

**모든 작업은 여기서 시작하고 여기에 기록됨**

### [docs/architecture.md](../../docs/architecture.md) ⭐ **필수**
**시스템 아키텍처 설계서**
- 전체 시스템 구조
- 컴포넌트별 책임
- 의존성 관계
- 설계 결정 사항

**아키텍처 이해 및 설계 변경 시 참조**

### [docs/requirements.md](../../docs/requirements.md)
**기능 요구사항**
- 사용자 스토리
- 기능 명세
- 제약 조건

**새로운 기능 개발 전 참조**

---

## 🎯 Quick Start

### 새 작업을 시작할 때
1. **`docs/todos.md`를 읽고** 다음 작업 확인
2. **작업 브랜치 생성** (`feat/<task-id>`)
3. **TDD 사이클 시작** (RED → GREEN → REFACTOR)
4. **단계별 커밋**
5. **`todos.md` 업데이트** (`docs:` prefix)

### 작업 중 참고 순서
```
todos.md (무엇을) → architecture.md (어떻게) → requirements.md (왜)
```

### 작업 완료 조건 (DoD)
- ✅ 단위 테스트 통과
- ✅ 통합 테스트 통과
- ✅ clang-format/tidy/cpplint 통과
- ✅ SOLID 원칙 준수
- ✅ todos.md 업데이트

---

## 📋 Authority Order (우선순위)

충돌이 발생할 경우 다음 순서로 따름:
1. **`docs/todos.md`** - 작업의 단일 진실 공급원
2. **`docs/architecture.md`** - 설계 원칙
3. **이 instruction 파일들** - 개발 규칙
4. **사용자의 명시적 지시**
5. Agent의 추론/판단

---

## 💡 Core Principles

1. **TDD는 필수** - RED → GREEN → REFACTOR
2. **커밋은 단계별** - 각 단계마다 커밋
3. **SOLID는 지켜야 함** - 아키텍처 원칙
4. **문서가 코드보다 먼저** - todos.md, architecture.md
5. **테스트로 행동 고정** - 요구사항의 명확화

---

## ⚠️ Important Notes

- 본 instruction 파일들이 다른 지침과 충돌할 경우 **이 파일들이 우선**
- Agent는 코드 작성자이기 전에 **문서(todos.md, architecture.md)의 실행자**
- 아키텍처 변경은 **코드 변경이 아니라 문서 변경으로 시작**
- `todos.md`에 없는 작업은 **임의로 수행하지 않음**
