# Refactoring Instructions (Kent Beck Style)

> 📖 **Related Docs**: [tdd-cycle.instructions.md](tdd-cycle.instructions.md), [commit-style.instructions.md](commit-style.instructions.md)

---

## Overview

이 문서는 **켄트 백(Kent Beck)의 리팩터링 철학**을 기반으로,
코드 품질을 유지하면서 안전하게 리팩터링하는 방법을 정리합니다.

핵심 원칙은 다음과 같습니다:
- **작게, 자주** 변경한다.
- **테스트로 안전망**을 확보한다.
- **의미 있는 작은 커밋**으로 히스토리를 남긴다.

---

## Core Principles (Kent Beck)

### 1) 작은 변경 (Small Steps)
- 한 번에 바꾸는 범위를 최소화한다.
- 변경 후 즉시 빌드/테스트로 확인한다.

### 2) 테스트가 먼저 (Safety Net)
- 리팩터링 전후에 테스트가 반드시 통과해야 한다.
- 테스트가 부족하면 먼저 **보강 테스트**를 작성한다.

### 3) 의도 드러내기 (Make Intent Clear)
- 변수/함수/클래스 이름을 명확하게 바꾼다.
- 중복을 제거하고, 코드 구조를 이해하기 쉽게 만든다.

### 4) 행위 보존 (Behavior Preservation)
- **기능(동작)은 그대로**, 구조만 개선한다.
- 결과가 달라졌다면 리팩터링이 아니라 버그 수정/기능 변경이다.

---

## Recommended Refactoring Flow

### Step 1. 현재 테스트 확인
- 기존 테스트가 통과하는지 확인한다.
- 실패한다면 리팩터링 전에 **먼저 원인을 해결**한다.

### Step 2. 테스트 보강 (필요 시)
- 리팩터링 대상이 중요한 로직이라면
  **리팩터링 전에** 보호 테스트를 추가한다.

### Step 3. 작은 단위로 변경
- 한 번에 하나의 의도를 담는다.
  - 예: 함수 추출, 변수명 변경, 책임 분리
- 변경할 때마다 **빌드/테스트**를 돌린다.

### Step 4. 동작 확인
- 테스트가 모두 통과하면 다음 작은 변경으로 이동한다.
- 테스트가 실패하면 바로 되돌리고 원인을 수정한다.

---

## Testing Guidelines (필수)

### 기본 원칙
- 리팩터링 전/후에 **항상 테스트**를 실행한다.
- 테스트가 느리다면 **핵심 범위**만이라도 최소 실행한다.

### 권장 순서
1. 리팩터링 전 전체 테스트 또는 관련 테스트 실행
2. 작은 변경 직후 관련 테스트 실행
3. 최종적으로 전체 테스트 실행

### 테스트 예시
```bash
# 관련 유닛 테스트만 빠르게 확인
ctest -R pattern_matcher

# 전체 테스트
ctest
```

---

## Commit Guidelines (필수)

### 기본 원칙
- **작은 변경마다 커밋**한다.
- 리팩터링 커밋은 `refactor:` prefix를 사용한다.
- 테스트 통과 상태에서만 커밋한다.

### 추천 커밋 흐름
1. (필요 시) 보호 테스트 추가  
   - `test: add regression test for X`
2. 리팩터링 1단계  
   - `refactor: extract helper for X`
3. 리팩터링 2단계  
   - `refactor: rename X to clarify intent`

### 금지 사항
- 여러 의도를 한 커밋에 섞지 않는다.
- 테스트 실패 상태로 커밋하지 않는다.
- 리팩터링 커밋에 기능 변경을 섞지 않는다.

---

## Example: Safe Refactoring Loop

```
1) 테스트 통과 확인
2) 아주 작은 변경
3) 테스트 실행
4) 커밋
5) 반복
```

---

## Quick Checklist

- [ ] 리팩터링 전 테스트 통과 확인
- [ ] 변경 범위를 작게 유지
- [ ] 변경 후 테스트 재실행
- [ ] 동작(기능) 변화 없음
- [ ] refactor: 커밋으로 기록

