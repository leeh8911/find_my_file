# Project Workflow Instructions

> 📖 **Related Docs**: [todos.md](../../docs/todos.md), [architecture.md](../../docs/architecture.md)

---

## 1. Task Management via todos.md (Mandatory)

### Rule
- 본 프로젝트의 **모든 작업은 `docs/todos.md` 파일에 기록되고 관리된다.**
- Agent는 작업을 시작하기 전에 **항상 `docs/todos.md`를 읽고**,  
  다음에 수행할 작업을 그 파일에서 결정한다.
- **`docs/todos.md`에 명시되지 않은 작업은 임의로 수행하지 않는다.**

### Operating Policy
새로운 작업이 필요하다고 판단되면:
1. **즉시 코드를 작성하지 않는다**
2. **먼저 `docs/todos.md`에 작업 항목을 추가한다**
3. 해당 항목을 기준으로 TDD 사이클(RED → GREEN → REFACTOR)을 시작한다

작업이 완료되면:
- `docs/todos.md`에서 해당 항목을 완료 처리한다
- 필요 시 후속 작업을 새로운 항목으로 추가한다

### Versioning & Commits for todos.md
- `docs/todos.md` 변경은 **항상 커밋 대상**이다
- `docs/todos.md` 변경 커밋은 반드시 `docs:` prefix로 **분리**한다
  - 예: `docs: add todos for phase bootstrap`
  - 예: `docs: mark TASK-003 done`
- **코드 변경과 `docs/todos.md` 변경을 같은 커밋에 섞지 않는다**

### Versioning & Commits for architecture.md
- `docs/architecture.md` 변경은 **항상 커밋 대상**이다
- `docs/architecture.md` 변경 커밋은 반드시 `docs:` prefix로 **분리**한다
  - 예: `docs: update architecture for indexing pipeline`
  - 예: `docs: revise component responsibilities`
- **코드 변경과 `docs/architecture.md` 변경을 같은 커밋에 섞지 않는다**
- 아키텍처 변경 제안이 있을 경우:
  1. 먼저 `docs/architecture.md`를 수정한다
  2. `docs:` 커밋으로 변경 이력을 남긴다
  3. 이후에 해당 변경을 반영하는 todo를 `docs/todos.md`에 추가한다

---

## 2. Task Origin (Architecture / Human Input)

### Architecture-driven tasks
- Agent는 **`docs/architecture.md`를 참조하여 작업을 수행할 수 있다**
- `docs/architecture.md`에서 다음이 발견되면:
  - 정의만 있고 구현되지 않은 컴포넌트
  - 누락된 책임, 경계, 추상화
- Agent는 이를 **새로운 todo 항목으로 `docs/todos.md`에 먼저 기록**한다
- 이 경우에도:
  - 코드를 바로 작성하지 않는다
  - `docs/todos.md` 반영 → `docs:` 커밋 → 이후 작업 진행 순서를 따른다

### Human-added tasks
- 사람(사용자/리뷰어)은 언제든지 `docs/todos.md`에 직접 작업을 추가할 수 있다
- Agent는 사람이 추가한 작업을 **동일한 규칙**으로 처리한다
- 사람의 입력은 **요구사항의 최상위 권한(source of intent)** 으로 간주한다

---

## 3. Authority Order

작업 해석 및 수행 시 우선순위는 다음을 따른다:
1. **`docs/todos.md`**
2. **`docs/architecture.md`**
3. 사용자(사람)의 명시적 지시
4. Agent의 추론/판단

충돌이 발생할 경우:
- Agent는 임의로 판단하지 않는다
- `docs/todos.md`에 정리하고 사람의 확인을 기다린다

### Single Source of Truth
- **`docs/todos.md`는 작업 상태에 대한 단일 진실 공급원(Single Source of Truth)이다**
- 코드, 커밋 메시지, 대화 내용보다 `docs/todos.md`의 내용이 우선한다

---

## 4. Branching Model by Todo Phase (Mandatory)

### Concept
- 각 todo 항목은 **phase 단위로 그룹화**된다
- 각 phase는 **전용 브랜치**에서 작업하고 완료 시 `main`으로 병합한다
- phase는 "의미 있는 통합 단위"이며 내부에 여러 todo item을 포함할 수 있다

### Phase Naming
- 브랜치 이름은 `feat/<task-id>` 형식을 사용한다
  - 예: `feat/TASK-001`
  - 예: `feat/TODO-042`
  - 예: `feat/PHASE-SEMANTIC-SEARCH`

### Workflow
1. `main`에서 다음 phase를 `docs/todos.md` 기준으로 결정한다
2. `feat/<task-id>` 브랜치를 생성한다
3. feat 브랜치에서 todo item들을 **TDD + 단계별 커밋**으로 수행한다
4. todo 상태 변경은 항상 `docs:` 커밋으로 분리한다
5. phase 범위의 todo가 완료되면 **통합 테스트를 수행**한다
6. 모든 조건을 만족하면 `main`으로 병합한다
7. 병합 후 `main`에서 `docs/todos.md`를 정리하고 `docs:` 커밋을 남긴다

### Merge Policy
병합은 **phase 단위로만** 수행한다.

병합 전 필수 조건:
- ✅ 단위 테스트 통과 (`tests/`)
- ✅ 통합 테스트 스크립트 통과 (있는 경우)
- ✅ clang-format / clang-tidy / cpplint 통과
- ✅ `docs/todos.md`에서 phase 완료 처리됨

병합 커밋 메시지 권장 형식:
```
merge: feat <task-id>

<작업 설명>
- 완료된 주요 기능
- 테스트 결과
```

---

## 5. Agent Operating Procedure

Agent가 작업을 수행하는 표준 절차:

1. **`docs/todos.md`를 읽는다**
   - 다음에 수행할 작업을 확인
   - Phase와 우선순위 파악

2. **관련 문서를 읽는다**
   - `docs/architecture.md` - 설계 이해
   - `docs/requirements.md` - 요구사항 확인

3. **가장 작은 todo부터 시작한다**
   - 한 번에 하나의 todo만 수행
   - 큰 작업은 작은 단위로 분해

4. **TDD 사이클을 따른다**
   - RED: 실패하는 테스트 작성
   - GREEN: 최소 구현으로 통과
   - REFACTOR: 구조 개선

5. **단계별로 커밋한다**
   - RED: `test:` prefix
   - GREEN: `feat:` or `fix:` prefix
   - REFACTOR: `refactor:` prefix

6. **작업 완료 후 `docs/todos.md`를 갱신한다**
   - 완료 항목 체크
   - 새로운 작업 발견 시 추가
   - `docs:` prefix로 커밋

7. **Phase 완료 시 통합 테스트를 실행한다**
   - 모든 테스트 통과 확인
   - Linter 검사 수행

8. **`main`으로 병합한다**
   - Fast-forward 또는 merge commit
   - 병합 후 branch 삭제

---

## 6. Work Breakdown Strategy

큰 작업을 작은 단위로 나누는 방법:

### Horizontal Slicing (계층별)
❌ **비추천** - 통합이 어렵고 증분 가치가 없음
- UI 먼저 → 로직 나중
- DB 먼저 → 비즈니스 로직 나중

### Vertical Slicing (기능별)
✅ **추천** - End-to-end 가치 전달, 테스트 가능
- 작은 기능 단위로 전 계층 구현
- 각 slice가 독립적으로 동작
- 점진적 기능 확장

예시:
```
❌ Todo Phase 1: 모든 도메인 모델
   Todo Phase 2: 모든 리포지토리
   Todo Phase 3: 모든 서비스

✅ Todo Phase 1: 사용자 생성 기능 (모델+repo+서비스+API)
   Todo Phase 2: 사용자 조회 기능 (모델+repo+서비스+API)
   Todo Phase 3: 사용자 수정 기능 (모델+repo+서비스+API)
```

---

## 7. Definition of Done (Phase 완료 조건)

Phase를 완료로 간주하는 조건:

### Code Quality
- ✅ 단위 테스트 통과 (모든 새 코드)
- ✅ 통합 테스트 통과 (E2E 시나리오)
- ✅ SOLID 원칙 위반 없음
- ✅ 불필요한 코드 제거 (주석 포함)

### Static Analysis
- ✅ clang-format 적용
- ✅ clang-tidy 경고 없음
- ✅ cpplint 통과

### Documentation
- ✅ `docs/todos.md` 완료 처리
- ✅ `docs/architecture.md` 업데이트 (필요 시)
- ✅ 코드 주석 (공개 API, 복잡한 로직)

### Git History
- ✅ 의미있는 커밋 메시지
- ✅ TDD 단계별 커밋 분리
- ✅ `docs:` 커밋 분리

---

## 📌 Quick Reference

```bash
# 새 작업 시작
git checkout main
git pull
git checkout -b feat/TASK-001

# 작업 중
# 1. Read docs/todos.md
# 2. Write failing test → commit "test: ..."
# 3. Implement feature → commit "feat: ..."
# 4. Refactor code → commit "refactor: ..."
# 5. Update docs/todos.md → commit "docs: ..."

# Phase 완료
./build/tests  # 모든 테스트 통과 확인
git checkout main
git merge phase/my-feature
git push origin main
git branch -d phase/my-feature
```
