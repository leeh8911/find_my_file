# Agent Instruction (TDD + Auto Commit + SOLID)

## 0. Purpose
이 문서는 본 프로젝트에서 AI Agent가 수행하는 **모든 개발 작업의 운영 규칙(working agreement)** 을 정의한다.  
Agent는 어떤 작업을 수행하든 **반드시 본 문서를 먼저 읽고**, 여기 정의된 방식대로만 작업한다.

---

## 0.1 Task Management via todos.md (Mandatory)

### Rule
- 본 프로젝트의 **모든 작업은 `todos.md` 파일에 기록되고 관리된다.**
- Agent는 작업을 시작하기 전에 **항상 `todos.md`를 읽고**,  
  다음에 수행할 작업을 그 파일에서 결정한다.
- `todos.md`에 명시되지 않은 작업은 **임의로 수행하지 않는다.**

### Operating Policy
- 새로운 작업이 필요하다고 판단되면:
  1) 즉시 코드를 작성하지 않는다.
  2) 먼저 `todos.md`에 작업 항목을 추가한다.
  3) 해당 항목을 기준으로 TDD 사이클(RED → GREEN → REFACTOR)을 시작한다.
- 작업이 완료되면:
  - `todos.md`에서 해당 항목을 완료 처리한다.
  - 필요 시 후속 작업을 새로운 항목으로 추가한다.

### Versioning & Commits for todos.md
- `todos.md` 변경은 **항상 커밋 대상**이다.
- `todos.md` 변경 커밋은 반드시 `docs:` prefix로 **분리**한다.
  - 예: `docs: add todos for phase bootstrap`
  - 예: `docs: mark TASK-003 done`
- 코드 변경과 `todos.md` 변경을 **같은 커밋에 섞지 않는다.**

### Versioning & Commits for architecture.md
- `architecture.md` 변경은 **항상 커밋 대상**이다.
- `architecture.md` 변경 커밋은 반드시 `docs:` prefix로 **분리**한다.
  - 예: `docs: update architecture for indexing pipeline`
  - 예: `docs: revise component responsibilities`
- 코드 변경과 `architecture.md` 변경을 **같은 커밋에 섞지 않는다.**
- 아키텍처 변경 제안이 있을 경우:
  1) 먼저 `architecture.md`를 수정한다.
  2) `docs:` 커밋으로 변경 이력을 남긴다.
  3) 이후에 해당 변경을 반영하는 todo를 `todos.md`에 추가한다.


### Task Origin (Architecture / Human Input)

#### Architecture-driven tasks
- Agent는 작업 수행 시 **`architecture.md`를 참조하여 작업을 수행할 수 있다.**
- `architecture.md`에서 다음이 발견되면:
  - 정의만 있고 구현되지 않은 컴포넌트
  - 누락된 책임, 경계, 추상화
- Agent는 이를 **새로운 todo 항목으로 `todos.md`에 먼저 기록**한다.
- 이 경우에도:
  - 코드를 바로 작성하지 않는다.
  - `todos.md` 반영 → `docs:` 커밋 → 이후 작업 진행 순서를 따른다.

#### Human-added tasks
- 사람(사용자/리뷰어)은 언제든지 `todos.md`에 직접 작업을 추가할 수 있다.
- Agent는 사람이 추가한 작업을 **동일한 규칙**으로 처리한다.
- 사람의 입력은 **요구사항의 최상위 권한(source of intent)** 으로 간주한다.

### Authority Order
작업 해석 및 수행 시 우선순위는 다음을 따른다:
1) `todos.md`
2) `architecture.md`
3) 사용자(사람)의 명시적 지시
4) Agent의 추론/판단

충돌이 발생할 경우:
- Agent는 임의로 판단하지 않는다.
- `todos.md`에 정리하고 사람의 확인을 기다린다.

### Single Source of Truth
- `todos.md`는 작업 상태에 대한 **단일 진실 공급원(Single Source of Truth)** 이다.
- 코드, 커밋 메시지, 대화 내용보다 `todos.md`의 내용이 우선한다.

---

## 0.2 Branching Model by Todo Phase (Mandatory)

### Concept
- 각 todo 항목은 **phase 단위로 그룹화**된다.
- 각 phase는 **전용 브랜치**에서 작업하고 완료 시 `main`으로 병합한다.
- phase는 “의미 있는 통합 단위”이며 내부에 여러 todo item을 포함할 수 있다.

### Phase Naming
- 브랜치 이름은 `phase/<slug>` 형식을 사용한다.
  - 예: `phase/bootstrap`
  - 예: `phase/core-indexer`
  - 예: `phase/ui-shell`

### Workflow
1) `main`에서 다음 phase를 `todos.md` 기준으로 결정한다.
2) `phase/<slug>` 브랜치를 생성한다.
3) phase 브랜치에서 todo item들을 **TDD + 단계별 커밋**으로 수행한다.
4) todo 상태 변경은 항상 `docs:` 커밋으로 분리한다.
5) phase 범위의 todo가 완료되면 **통합 테스트를 수행**한다.
6) 모든 조건을 만족하면 `main`으로 병합한다.
7) 병합 후 `main`에서 `todos.md`를 정리하고 `docs:` 커밋을 남긴다.

### Merge Policy
- 병합은 **phase 단위로만** 수행한다.
- 병합 전 조건:
  - 단위 테스트 통과 (`test/unittest/`)
  - 통합 테스트 스크립트 통과 (`test/integrationtest/`)
  - clang-format / clang-tidy / cpplint 통과
  - `todos.md`에서 phase 완료 처리됨
- 병합 커밋 메시지 권장 형식:
  - `merge: phase <slug>`

---

## 1. Non-negotiables (절대 규칙)
1) 개발은 **TDD의 RED → GREEN → REFACTOR** 사이클을 따른다.  
2) 각 단계마다 **커밋을 반드시 남긴다.**  
3) 아키텍처는 **SOLID 원칙을 만족**해야 한다.  
4) 요구사항이 불명확하면 테스트로 행동을 고정한다.  
5) 더러운 코드는 REFACTOR 단계에서 반드시 정리한다.

---

## 2. Workflow: RED → GREEN → REFACTOR

### 2.1 RED
- 실패하는 테스트를 먼저 작성한다.
- 테스트는 요구사항을 가장 명확하게 표현해야 한다.

**Commit**
- `test: add failing test for <feature>`

---

### 2.2 GREEN
- 가장 단순한 구현으로 테스트를 통과시킨다.
- 구조적 파괴는 허용하지 않는다.

**Commit**
- `feat: implement <feature> to pass tests`

---

### 2.3 REFACTOR
- 동작 변경 없이 구조를 개선한다.
- SOLID, 책임 분리, 의존성 역전을 적용한다.

**Commit**
- `refactor: improve <component> structure`

---

## 3. Automatic Commit Policy
- RED / GREEN / REFACTOR 각 단계 완료 시 즉시 커밋한다.
- 문서 변경(`todos.md`, `architecture.md` 포함)은 항상 `docs:` 커밋으로 분리한다.

---

## 4. Architecture & SOLID Rules

### SOLID Checklist
- SRP, OCP, LSP, ISP, DIP를 모두 고려한다.

### Dependency Direction
- UI → Application → Domain
- Infrastructure → (implements) abstractions
- Domain은 프레임워크에 의존하지 않는다.

### Interface Boundaries
- 외부 시스템 접근은 Port / Adapter 패턴으로 감싼다.

---

## 5. Testing Rules

### 5.1 Unit Testing
- 단위 테스트는 `test/unittest/` 아래에 위치한다.
- 테스트는 “구현”이 아니라 “행동”을 검증한다.
- Domain → Application → UI 순으로 우선한다.
- flakey 테스트 금지.

---

## 5.2 Integration Testing as Scripts (Mandatory)

### Directory Convention
- 통합 테스트는 `test/integrationtest/` 아래에 **스크립트 형태로** 작성한다.

### Core Principles
- Agent는 GUI를 직접 테스트하지 않는다.
- 통합 테스트는 **CLI 기반 유스케이스 실행**을 통해 수행한다.
- GUI는 테스트 대상이 아니라, **검증된 유스케이스의 표현 계층**이다.

### Script Rules
- 각 통합 테스트는:
  - 실행 가능해야 한다.
  - 성공 시 exit code `0`, 실패 시 `!= 0`
  - stdout / stderr 또는 생성된 artifact로 결과를 검증한다.
- 스크립트는 독립적으로 실행 가능해야 한다.

### Naming
- 파일명은 유스케이스 중심으로 명명한다.
  - 예: `uc_create_project_ok.py`
  - 예: `uc_index_roundtrip.sh`

### Test Runner
- `test/integrationtest/run_all.*` 형태의 단일 실행 진입점을 유지한다.
- phase 병합 전에는 반드시 해당 runner를 실행한다.

---

## 6. Agent Operating Procedure
1) `todos.md`를 읽는다.
2) 가장 작은 todo부터 RED를 시작한다.
3) 단계별 커밋을 남긴다.
4) 완료 후 `todos.md`를 갱신한다.

---

## 7. Definition of Done
- 단위 테스트 통과 (`test/unittest/`)
- 통합 테스트 스크립트 통과 (`test/integrationtest/`)
- SOLID 위반 없음
- 불필요한 코드 제거
- 문서 갱신 완료

---

## 8. Notes
본 문서가 다른 지침과 충돌할 경우 **본 문서가 우선**한다.  
Agent는 코드 작성자이기 이전에 **문서(`architecture.md`, `todos.md`)의 실행자**이다.
아키텍처 변경은 코드 변경이 아니라 문서 변경으로 시작한다.


---

## 9. C++ Style & Static Analysis

### clang-format
- REFACTOR 단계 전 반드시 적용

### clang-tidy
- 버그/UB 경고는 반드시 수정
- 억제는 최소 범위 + 사유 명시

### cpplint
- 형식 일관성 보조
- 충돌 시 clang-format 우선

### Tooling DoD
- format / tidy / lint 통과
