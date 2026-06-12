# SampleOrderSystem - 반도체 시료 생산주문관리 시스템 (미션2 메인)

## 역할
전체 기능 명세를 구현하는 **메인 프로젝트**.
PoC(ConsoleMVC, DataPersistence, DataMonitor, DummyDataGenerator)에서 검증된 패턴을 기반으로
Agentic Engineering을 도입하여 고품질 코드를 개발한다.

## 상위 Orchestrator
`../CLAUDE.md` 참조. **스펙 문서: `../Document/spec/spec_reference.txt` (반드시 정독)**

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
2. **Harness** 도입
3. **Test**: 단위 테스트 / 통합 테스트 (MockClock 활용)
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

## 개발 워크플로우 (WORKLOG 기반)

모든 작업은 아래 사이클을 반드시 따른다.
WORKLOG 파일 위치: `Document/WORKLOG.md`

```
1. 작업 시작 전  → Document/WORKLOG.md 열어 최신 리뷰/지시사항 확인
2. 작업 및 커밋  → 기능 단위로 커밋
3. WORKLOG 업데이트 → 작업 내용 요약, 커밋 해시, 리뷰 요청사항 기록
4. WORKLOG 커밋  → "docs: update WORKLOG" 메시지로 커밋 및 푸시
5. 사용자 리뷰   → 사용자가 WORKLOG에 피드백/다음 지시 작성 후 커밋
6. 1번으로 반복
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
