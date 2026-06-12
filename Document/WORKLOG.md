# WORKLOG - SampleOrderSystem

> **작성 규칙**
> - Agent는 커밋 후 아래 템플릿으로 작업 내용을 기록하고 WORKLOG를 커밋한다.
> - 리뷰 요청 섹션을 작성 후 사용자에게 리뷰를 요청한다.
> - 사용자는 리뷰 섹션에 피드백/요청사항을 작성 후 커밋한다.
> - Agent는 다음 작업 시작 전 반드시 최신 WORKLOG를 확인한다.

---

## 템플릿

```
## [YYYY-MM-DD] {작업 제목}

### 작업 내용
- 

### 커밋
- `{hash}` {커밋 메시지}

### 리뷰 요청
- 

---
### 리뷰 (by User)
- 

### 다음 작업 지시
- 
```

---

<!-- 실제 로그는 아래에 최신순으로 작성 -->

## [2026-06-12] 프로젝트 착수 준비 - PoC 검토 및 스펙 분석

### 작업 내용
- PoC 4개 프로젝트 구현 상태 전수 확인
  - ConsoleMVC: 완료 (54 TC 통과) — MVC 3계층, IClock, 6개 Controller 패턴 확립
  - DataPersistence: 완료 (49 TC 통과) — JSON CRUD, Repository 인터페이스/구현 분리 패턴 확립
  - DataMonitor: 완료 (57 TC 통과) — 읽기 전용 Repository, View ostream 주입 패턴 확립
  - DummyDataGenerator: 완료 (34 TC 통과) — 시드 기반 더미 데이터, SampleOrderSystem 호환 JSON 스키마 확립
- spec_raw.txt / spec_reference.txt 정독 및 개발 범위 파악
  - 6개 메뉴 + 종료 구현 범위 확정
  - 물리적 재고 실시간 계산 공식, 재고 차감 시점, 생산 완료 감지 방식 등 핵심 정책 파악
  - 아키텍처 구조 (MVC + Repository + util/Clock) 확정

### 커밋
- (없음 — 분석 작업)

### 리뷰 요청
- 스펙 해석 이슈 없음. 다음 작업(PRD.md + FEATURES 문서 작성)을 사용자 지시에 따라 시작 예정

---
### 리뷰 (by User)
- PoC 프로젝트 구현 현황 확인함

### 다음 작업 지시
- PoC와 SPEC 문서 기반으로 PRD.md 및 Features 작성

---

## [2026-06-12] PRD.md 및 FEATURES 문서 작성

### 작업 내용
- `Document/PRD.md` 작성: 사용자 관점 요구사항 기술 (6개 기능 + 비기능 요구사항 + 상태 전이 요약)
- `Document/FEATURES/` 작성: 기능별 상세 요구사항 6개 파일
  - FEATURE-01: 시료 관리 (등록/목록조회/검색)
  - FEATURE-02: 시료 주문 (주문 접수, 주문번호 채번 정책)
  - FEATURE-03: 주문 승인/거절 (물리적 재고 계산, 생산량 공식, 결정 시점 고정 정책)
  - FEATURE-04: 모니터링 (주문량 확인, 재고량 확인, 여유/부족/고갈 기준)
  - FEATURE-05: 생산라인 조회 (FIFO, 생산완료 감지, FakeClock 구조)
  - FEATURE-06: 출고 처리 (CONFIRMED 의미, 재고 차감 시점)

### 커밋
- `c0e37fb` [AI-Docs] Add PRD.md and FEATURES documents

### 리뷰 요청
- PRD 및 FEATURES 문서 전반 검토 부탁드립니다.
- 특히 FEATURE-03(주문 승인)의 물리적 재고 계산 정책 및 FEATURE-05(생산라인)의 완료 감지 방식이 스펙 의도와 일치하는지 확인 부탁드립니다.
- 승인 후 코드 작업을 시작할 예정입니다.

---
### 리뷰 (by User)
- 구현 Feature 확인

### 다음 작업 지시
- PoC 프로젝트 중 MVC 스켈레톤과 Feature 맞지 않는 것 없는지 재확인
- 코드 작업 시작 (Feature-01)
- 코드 작업에는 ConsoleMVC PoC 에서 정의한 구조를 참조
- 코드 작업시 데이터 영속성 (프로그램 재실행 시에도 마지막 상태를 유지) 보유하도록 DataPersistence PoC에서 구현한 CRUD를 참조
- PoC간 충돌나는게 없도록 구현