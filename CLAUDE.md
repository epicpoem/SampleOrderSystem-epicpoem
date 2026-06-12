# SampleOrderSystem - 반도체 시료 생산주문관리 시스템 (미션2 메인)

## 역할
전체 기능 명세를 구현하는 **메인 프로젝트**.
PoC(ConsoleMVC, DataPersistence, DataMonitor, DummyDataGenerator)에서 검증된 패턴을 기반으로
Agentic Engineering을 도입하여 고품질 코드를 개발한다.

## 상위 Orchestrator
`../CLAUDE.md` 참조. **스펙 문서: `../Document/spec/spec_reference.txt` (반드시 정독)**

---

## 참조 PoC 프로젝트 (완료됨)

아래 4개의 PoC 프로젝트가 **모두 완료**되었다.
SampleOrderSystem 개발 시작 전에 각 PoC 코드를 반드시 읽고, 검증된 패턴을 재사용한다.
코드를 중복 작성하지 말고 PoC에서 확립된 설계를 이어받아 구현한다.

| PoC 프로젝트 | 로컬 경로 | GitHub |
|---|---|---|
| ConsoleMVC | [`../ConsoleMVC/`](../ConsoleMVC/) | https://github.com/epicpoem/ConsoleMVC-epicpoem |
| DataPersistence | [`../DataPersistence/`](../DataPersistence/) | https://github.com/epicpoem/DataPersistence-epicpoem |
| DataMonitor | [`../DataMonitor/`](../DataMonitor/) | https://github.com/epicpoem/DataMonitor-epicpoem |
| DummyDataGenerator | [`../DummyDataGenerator/`](../DummyDataGenerator/) | https://github.com/epicpoem/DummyDataGenerator-epicpoem |

### PoC별 재사용 대상

#### ConsoleMVC (`../ConsoleMVC/`)
- MVC 3계층 패키지 구조 (model/, controller/, view/)
- `ISampleRepository` / `IOrderRepository` 인터페이스 설계
- `IClock` / `SystemClock` / `FakeClock` 시간 추상화 패턴
- 6개 Controller 구현 패턴 (SampleController, OrderController 등)
- 콘솔 한글 인코딩 처리 (`SetConsoleOutputCP(CP_UTF8)`)
- MSVC UTF-8 소스 빌드 옵션 (`/utf-8`)

#### DataPersistence (`../DataPersistence/`)
- JSON 파일 저장/불러오기 구현체 (`JsonSampleRepository`)
- Repository 인터페이스와 JSON 구현체 분리 패턴
- ID 중복 방지 처리 방식

#### DataMonitor (`../DataMonitor/`)
- 전체 데이터 콘솔 조회 Tool 구조
- 읽기 전용 Repository 활용 패턴

#### DummyDataGenerator (`../DummyDataGenerator/`)
- SampleOrderSystem JSON 스키마 및 초기 데이터 포맷
- 테스트용 Dummy 데이터 생성 Tool (개발/테스트 시 활용)

---

## 시스템 개요

- 플랫폼: 콘솔 기반 CLI
- 생산 라인: 단일 라인, FIFO 스케줄링
- 데이터 저장: JSON
- 아키텍처: MVC 패턴

---

## 도메인 모델

### 엔티티
```
Sample     : id, name, avgProdTime, yield, stock
Order      : orderId, sampleId, customerName, quantity, status, createdAt
Production : orderId, actualQty, totalProdTime, startedAt, status
```

### 주문 상태 전이
```
RESERVED → REJECTED
RESERVED → CONFIRMED (재고 충분)
RESERVED → PRODUCING (재고 부족) → CONFIRMED → RELEASE
```

---

## 기능 명세 (메뉴별)

스펙 전체는 `../Document/spec/spec_reference.txt` 참조.

| 메뉴 | 주요 구현 포인트 |
|------|-----------------|
| [1] 시료 관리 | 등록(초기재고 0, ID중복 방지), 조회, 다속성 선택 검색 |
| [2] 시료 주문 | RESERVED 생성, 주문번호 날짜별 순번 채번 |
| [3] 주문 승인/거절 | 물리적 재고(PRODUCING 실시간 반영) 기준 자동 분기 |
| [4] 모니터링 | 상태별 주문 수, CONFIRMED+PRODUCING 대비 재고 상태 |
| [5] 생산라인 조회 | FIFO 큐, 진행률, 완료 예정시간 |
| [6] 출고 처리 | CONFIRMED → RELEASE, 재고 차감 |
| [0] 종료 | 데이터 저장 후 종료 |

---

## 핵심 정책 (구현 필수 반영)

### 재고 계산
```
물리적 재고(실시간) = 보유 재고 + 각 PRODUCING 주문의 진행분 비례 합산
진행분(1건)        = 실생산량 * (경과시간 / 총생산시간)
부족분             = 주문수량 - 물리적 재고(실시간)
```

### 생산 공식
```
실 생산량    = ceil(부족분 / (수율 * 0.9))
총 생산 시간 = 평균생산시간(min/ea) * 실 생산량
```

### 재고 차감 시점
- CONFIRMED 전환 시: 차감 없음
- RELEASE 처리 시: 주문 수량 차감
- PRODUCING 중: 진행분 비례로 물리적 재고에 실시간 반영

### 생산 완료 감지 (Q4 정책)
- 메뉴 진입 시 경과 시간 체크하여 완료 처리 (별도 스레드 불필요)

### 모니터링 재고 상태 기준 (Q5 정책)
- CONFIRMED + PRODUCING 주문 총량과 비교 (RESERVED 제외)

### 시간 처리
- 실시간 처리, 단 테스트 시 MockClock 주입 가능 구조 필수
  ```
  Clock (interface)
    ├── SystemClock  (실제 환경)
    └── FakeClock    (테스트 환경)
  ```

---

## 아키텍처 요구사항

```
src/
  model/        # 도메인 엔티티, 비즈니스 규칙
  controller/   # 입력 처리, 흐름 제어
  view/         # 콘솔 출력 전담
  repository/   # 저장소 인터페이스 + JSON 구현체
  service/      # 비즈니스 로직 (선택적 분리)
  util/         # Clock 추상화 등
data/           # JSON 저장 파일
```

---

## 평가 기준 (미션2)

1. **CLAUDE.md, PRD.md** 등 문서 관리
2. **Harness** 도입: gmock/gtest 기반 테스트 하네스, ctest로 일괄 실행 가능
3. **Test**: 단위 테스트(unit/) + 통합 테스트(integration/), MockClock 활용
4. **CleanCode**: 명확한 네이밍, 단일 책임, 불필요 주석 없음
5. **Commit 이력**: 의미있는 메시지, 기능 단위 커밋

---

## 제출 기준

- [ ] 전체 메뉴 동작 (6개 메뉴 + 종료)
- [ ] 주문 상태 전이 정상 동작
- [ ] 재고 실시간 반영 및 차감 정책 준수
- [ ] JSON 데이터 영속성 (재시작 후 복원)
- [ ] MockClock 주입 구조 포함
- [ ] 단위/통합 테스트 존재
- [ ] CLAUDE.md, PRD.md 존재
- [ ] 의미있는 커밋 이력

---

## 작업 시작 전 필수 문서 작성

코드 작업 시작 전 아래 두 가지 문서를 반드시 먼저 작성한다.

### 1. Document/PRD.md (Product Requirements Document)
- **사용자 관점**에서 필요한 기능과 요구사항을 기술
- 기술적인 구현 방법, 클래스명, 라이브러리 등 기술 내용은 다루지 않는다
- "사용자는 ~할 수 있어야 한다" 형식으로 작성
- 커밋: `[AI-Docs] Add PRD.md`

### 2. Document/FEATURES/ (기능별 상세 요구사항)
- 각 기능마다 별도의 `.md` 파일로 작성
- 파일명 예시: `FEATURE-01-sample-management.md`, `FEATURE-02-order.md`
- 내용: 기능 목적, 사용자 시나리오, 입력/출력, 예외 처리 기준
- 기술 구현 내용 제외, 요구사항 수준으로 기술
- 커밋: `[AI-Docs] Add FEATURES documents`

### 문서 작성 순서
```
PRD.md 작성 → FEATURES/ 각 파일 작성 → 커밋 → 사용자 리뷰 요청 → 승인 후 코드 작업 시작
```

---

## 개발 환경 및 테스트 정책

### 개발 언어
- **C++** (C++17 이상 권장)
- 빌드 시스템: CMake 사용 권장

### 테스트 프레임워크
- **Google Mock (gmock) / Google Test (gtest)**
- CMakeLists.txt에 테스트 타겟을 별도 구성
- 테스트 파일 위치: `test/` 디렉토리

### 테스트 하네스 (Harness)
- gmock/gtest 기반 테스트 하네스를 프로젝트 초기에 구성한다.
- 모든 테스트가 `ctest` 또는 단일 실행파일로 일괄 실행 가능해야 한다.
- Mock 객체 활용: `MockClock`, `MockRepository` 등 의존성 주입 방식으로 격리 테스트
- 단위 테스트(Unit)와 통합 테스트(Integration)를 디렉토리로 구분 권장
  ```
  test/
    unit/        # 개별 클래스/함수 단위 테스트
    integration/ # 여러 컴포넌트 연동 테스트 (MockClock 활용)
  ```

### 테스트 정책
- TDD(Test-Driven Development)는 강제하지 않는다.
- 단, 구현한 **각 기능에 대해 반드시 테스트 코드를 작성**한다.
- 기능 구현 커밋(`[AI-Feature]`) 이후 테스트 커밋(`[AI-Test]`)을 함께 남긴다.
- 최소 기준: 정상 동작 케이스 + 경계값/예외 케이스 각 1개 이상

---

## 개발 워크플로우 (WORKLOG 기반)

모든 작업은 아래 사이클을 반드시 따른다.
WORKLOG 파일 위치: `Document/WORKLOG.md`

```
1. 작업 시작 전     → Document/WORKLOG.md 열어 최신 내용 확인 (파일 맨 아래가 최신)
                      사용자가 수정한 내용(리뷰/다음 지시)이 있으면 [USER-Review] 헤더로 즉시 커밋 및 푸시 (별도 요청 불필요)
2. 작업 및 커밋     → 기능 단위로 [AI-xxx] 헤더로 커밋
3. WORKLOG 업데이트 → 새 항목을 파일 맨 아래에 추가
                      작업 내용 요약, 커밋 해시, 리뷰 요청사항 기록 (커밋 안 함)
4. 리뷰 요청        → 사용자에게 WORKLOG 확인 및 리뷰 요청
5. 사용자 리뷰      → 사용자가 WORKLOG에 피드백/다음 지시 작성
6. 1번으로 반복
```

> **규칙**: 작업 시작 시 WORKLOG에 사용자 수정사항이 있으면, 반드시 먼저 `[USER-Review]` 헤더로 커밋 및 푸시한 뒤 작업을 시작한다. **별도 커밋 요청 없이 자동으로 수행한다.**
> **WORKLOG 작성 방향**: 새 항목은 항상 파일 **맨 아래**에 추가한다. 위쪽이 오래된 항목, 아래쪽이 최신 항목.

### 커밋 메시지 컨벤션

| 헤더 | 사용 주체 | 사용 시점 |
|------|-----------|-----------|
| `[AI-Feature]` | Agent | 새 기능 구현 |
| `[AI-Fix]` | Agent | 버그 수정 (리뷰 피드백 반영 포함) |
| `[AI-Test]` | Agent | 테스트 코드 작성/수정 |
| `[AI-Refactoring]` | Agent | 리팩토링 (기능 변경 없음) |
| `[AI-Docs]` | Agent | 문서 작성/수정 (PRD, README 등) |
| `[AI-Chore]` | Agent | 빌드 설정, 의존성 추가, 프로젝트 초기 구성 |
| `[USER-Review]` | Agent | 사용자 WORKLOG 수정 확인 시 자동 커밋 (별도 요청 불필요) |

예시:
```
[AI-Feature] 시료 등록 및 조회 기능 구현
[AI-Fix] 중복 ID 허용되던 버그 수정
[AI-Test] 시료 등록 중복 ID 검증 테스트 추가
[AI-Refactoring] SampleController 메서드 분리
[AI-Docs] PRD.md 작성
[AI-Chore] Gradle 프로젝트 초기 구성
[USER-Review] 사용자 리뷰 반영 - 시료 검색 기능 수정 요청
```

### WORKLOG 작성 형식
```markdown
## [YYYY-MM-DD] {작업 제목}

### 작업 내용
- 구현한 내용 요약

### 커밋
- `{hash}` {커밋 메시지}

### 리뷰 요청
- 확인 또는 판단이 필요한 사항

---
### 리뷰 (by User)
- (사용자 작성)

### 다음 작업 지시
- (사용자 작성)
```

---

## 공통 제약
- 모델: **Sonnet / Effort: Medium** 만 허용 (Opus 사용 금지)
- Repository: Public 유지
- 과제 종료 후 `/logout` 필수
