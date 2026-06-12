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

---

## [2026-06-12] Feature-01 시료 관리 구현

### 작업 내용
- PoC 재확인 (ConsoleMVC / DataPersistence / DataMonitor / DummyDataGenerator)
  - ConsoleMVC: `avgProductionTime`, `ISampleRepository::add()` 등 인터페이스 설계 확인
  - DataPersistence: nlohmann/json CRUD 패턴, 주문번호 채번 방식 확인
  - PoC 간 필드명 통일 (`avgProductionTime` 사용, `orderNo` 통일)
- 공통 인프라 구현
  - `model/`: Sample, Order, OrderStatus (ConsoleMVC 구조 재사용)
  - `util/`: IClock / SystemClock (시간 추상화)
  - `repository/`: ISampleRepository, IOrderRepository 인터페이스
  - `repository/`: JsonSampleRepository, JsonOrderRepository (DataPersistence 패턴 적용)
- Feature-01 구현
  - `view/`: ISampleView 인터페이스, SampleView (ostream 주입으로 테스트 가능 구조)
  - `controller/`: IController, SampleController (등록/목록/검색)
  - `main.cpp`: 메인 메뉴 스켈레톤 (Feature-01 동작, 2-6 미구현 표시)
- 테스트 프로젝트 구성
  - `SampleOrderSystemTest.vcxproj` 신규 생성 (ConsoleMVCTest 구조 참조)
  - `unit/SampleControllerTest.cpp`: 14개 테스트 (InMemorySampleRepository 활용)
  - `unit/JsonSampleRepositoryTest.cpp`: 12개 테스트 (임시 파일 기반 영속성 검증)
- 모든 .cpp/.h 파일 UTF-8 with BOM 저장 (`/utf-8` 컴파일러 옵션 추가)
- nlohmann/json v3.11.3 DataPersistence PoC에서 복사

### 커밋
- `f50de63` [AI-Feature] Feature-01 시료 관리 구현 (모델/리포지토리/뷰/컨트롤러)
- `17b06d8` [AI-Test] Feature-01 단위 테스트 추가 (SampleController, JsonSampleRepository)
- `6f10f47` [AI-Chore] VS가 slnx에 프로젝트 GUID 자동 추가

### 리뷰 요청
- Visual Studio에서 `SampleOrderSystem.slnx` 열어 **Debug|x64** 빌드 및 실행 가능 여부 확인 부탁드립니다.
- `SampleOrderSystemTest` 프로젝트 빌드 후 테스트 실행 결과 확인 부탁드립니다.
- Feature-01 동작(시료 등록/조회/검색) 정상 여부 확인 후 Feature-02(시료 주문) 진행 지시 부탁드립니다.

---
### 리뷰 (by User)
- 구현 내용에서 빌드 불가

### 다음 작업 지시
- 직접 MSVC를 사용해서 빌드하여 원인 파악 수정

---

## [2026-06-12] Feature-01 빌드 오류 수정

### 작업 내용
- 빌드 오류 원인: `SampleView.h`에서 기본 인자 `std::cout` 참조 시 `<iostream>` 미포함
- `SampleView.h`에 `#include <iostream>` 추가하여 해결
- 메인 프로젝트 빌드 성공 확인 (`SampleOrderSystem.exe`)
- 테스트 프로젝트 빌드 성공 확인 (`SampleOrderSystemTest.exe`)
- 27개 테스트 전부 PASSED (SampleControllerTest 14개 + JsonSampleRepositoryTest 13개)

### 커밋
- `4cb6fa1` [AI-Fix] SampleView.h에 <iostream> 누락으로 인한 빌드 오류 수정

### 리뷰 요청
- 빌드 및 테스트 27/27 통과 확인되었습니다.
- Visual Studio에서 직접 실행하여 Feature-01 동작(시료 등록/조회/검색) 확인 부탁드립니다.
- 이상 없으면 Feature-02(시료 주문) 진행 지시 부탁드립니다.

---
### 리뷰 (by User)
- 정상 빌드 및 출력 정상 확인
- Feature1 기능 구현 확인
- binary 종료 후 재실행시 Feature1 (시료) 영속성 확인

### 다음 작업 지시
- Negative TC 구현 및 테스트

---

## [2026-06-12] Negative TC 구현 및 테스트

### 작업 내용
- `SampleControllerTest.cpp`에 7개 Negative/Edge-case TC 추가
  - `NegativeAvgTimeShowsOutOfRangeAndRetries`: 음수 평균생산시간 → 오류 후 재입력
  - `YieldExactlyZeroShowsOutOfRangeError`: 수율 0.0 경계값 → 범위 오류
  - `YieldExactlyOneSucceeds`: 수율 1.0 경계값 → 등록 성공 (포함 경계)
  - `EmptyIdRepromptsUntilValidIdEntered`: 빈 ID 입력 → 루프 재입력
  - `NonNumericAvgTimeTriggersRetryAndOutOfRangeError`: 비숫자 평균시간 → stod 예외 → 0.0 → 오류 후 재입력
  - `NonNumericYieldShowsOutOfRangeError`: 비숫자 수율 → stod 예외 → 0.0 → 범위 오류
  - `SearchByYieldWithHighThresholdShowsNoResults`: 임계값보다 낮은 수율만 존재 → 검색 결과 없음
- `JsonSampleRepositoryTest.cpp`에 4개 Negative/Edge-case TC 추가
  - `MissingFileReturnsEmptyList`: 파일 없음 → 빈 목록 반환 (크래시 없음)
  - `CorruptedJsonFileReturnsEmptyAndDoesNotCrash`: 손상된 JSON → 복구 후 빈 목록
  - `DecreaseStockByExactAmountSucceeds`: 재고 정확히 일치 수량 차감 → 성공, 재고 0
  - `DecreaseStockByZeroAmountSucceeds`: 0 수량 차감 → 성공, 재고 변화 없음

### 커밋
- `d46b81b` [AI-Test] Negative TC 추가 - 경계값/예외 입력/파일 손상 케이스 (38/38 PASS)

### 리뷰 요청
- 기존 27개 + 신규 11개 = 총 38개 테스트 전부 PASS 확인되었습니다.
- 신규 TC 항목 및 커버리지 수준이 적절한지 확인 부탁드립니다.
- 이상 없으면 Feature-02(시료 주문) 구현 진행 지시 부탁드립니다.

---
### 리뷰 (by User)
- Positive/Negative TC 구현 확인, 커버리지 적절한 것으로 판단
- PoC 구현 구조대로 초안 구현된 것 코드리뷰 완료
- TC 직접 실행하여 확인, 정상 테스트 확인 

### 다음 작업 지시
- Feature-02 구현 지시
---

## [2026-06-12] Feature-02 시료 주문 구현

### 작업 내용
- `util/FakeClock.h` 신규 생성: 테스트용 시간 고정 구현체 (IClock 인터페이스 준수)
- `view/IOrderView.h` 신규 생성: 주문 뷰 인터페이스 (Y/N 확인 메서드 포함)
- `view/OrderView.h/.cpp` 신규 생성: 콘솔 출력 구현 (ostream 주입으로 테스트 가능)
- `controller/OrderController.h/.cpp` 신규 생성
  - 시료 ID 유효성 검증 (미등록 ID → 재입력 루프)
  - 수량 유효성 검증 (0 이하, 비숫자 → 재입력 루프)
  - Y/N 확인 단계 포함 (N/n → 취소)
  - IClock 주입으로 주문번호 날짜 채번 (ORD-YYYYMMDD-XXXX)
- `main.cpp`: Feature-02 연결 (orderCtrl.run() 호출)
- `unit/OrderControllerTest.cpp` 신규 생성: 11개 테스트 (FakeClock 활용)
  - 정상 주문, Y/N 취소, 잘못된 시료ID, 0/음수/비숫자 수량, 빈 ID 루프, 당일 순번 증가

### 커밋
- `a399263` [AI-Feature] Feature-02 시료 주문 구현 (OrderController/View/FakeClock)

### 리뷰 요청
- 빌드 및 테스트 49/49 통과 확인되었습니다.
- Visual Studio에서 실행하여 [2] 시료 주문 기능 동작 확인 부탁드립니다.
  - 시료 등록(1번 메뉴) → 시료 주문(2번 메뉴) → 주문번호 ORD-YYYYMMDD-0001 생성 확인
  - 프로그램 재시작 후 두 번째 주문 시 ORD-YYYYMMDD-0002 순번 증가 확인
- 이상 없으면 Feature-03(주문 승인/거절) 진행 지시 부탁드립니다.

---
### 리뷰 (by User)
- orders.json에 주문 추가 되는 것 확인
- 모니터링이 구현되지 않아 프로그램 내 확인은 불가
- 주문 순번 증가 확인
- 구현 코드 확인

### 다음 작업 지시
- 시료 주문중 취소하는 방법 필요 (빈 str 입력시 시료 주문을 취소하겠습니까 출력 등)

---

## [2026-06-12] 시료 주문 빈 입력 취소 기능 추가

### 작업 내용
- `IOrderView`에 `showCancelConfirmPrompt()` 메서드 추가
- `OrderController::run()` 수정: 시료 ID / 고객명 / 수량 각 입력 단계에서 빈 문자열 입력 시 "주문을 취소하시겠습니까? [Y/N]" 출력
  - Y/y → 주문 취소 후 메인 메뉴로 복귀
  - N/n → 해당 항목 재입력
- 기존 `EmptySampleIdRepromptsUntilValid` TC를 새 동작에 맞게 수정
- 빈 입력 취소 관련 TC 4개 신규 추가
  - `EmptySampleIdThenConfirmYCancelsOrder`
  - `EmptyCustomerNameThenConfirmYCancelsOrder`
  - `EmptyQuantityThenConfirmYCancelsOrder`
  - `EmptyQuantityDeclineCancelThenSucceeds`

### 커밋
- `7c5fe61` [AI-Fix] 시료 주문 중 빈 입력 시 취소 확인 기능 추가 (53/53 PASS)

### 리뷰 요청
- 빌드 및 테스트 53/53 통과 확인되었습니다.
- Visual Studio에서 실행하여 시료 주문 중 빈 입력 시 취소 확인 문구 동작 확인 부탁드립니다.
- 이상 없으면 Feature-03(주문 승인/거절) 진행 지시 부탁드립니다.

---
### 리뷰 (by User)
- 휘소 동작 확인

### 다음 작업 지시
- NegativeTC 추가 및 테스트

---

## [2026-06-12] Negative TC 추가 (Feature-02 OrderController + JsonOrderRepository)

### 작업 내용
- `OrderControllerTest.cpp`에 5개 Negative/Edge-case TC 추가
  - `QuantityExactlyOneSucceeds`: 수량 최솟값 1 경계값 → 성공
  - `MultipleInvalidSampleIdsBeforeValidSucceeds`: 잘못된 시료 ID 복수 입력 후 유효 ID → 성공
  - `MultipleInvalidQuantitiesBeforeValidSucceeds`: 0, 음수, 비숫자 연속 입력 후 유효 수량 → 성공
  - `CancelConfirmWithNonYNInputTreatedAsNo`: 취소 확인에 Y/N 외 입력 → 취소 안 됨 (재입력)
  - `FinalConfirmWithEmptyInputCancels`: 최종 확인에 빈 입력 → 취소 처리
- `JsonOrderRepositoryTest.cpp` 신규 생성: 14개 테스트
  - CRUD: add/findAll, findByNo, findByStatus, update, 영속성
  - 주문번호 채번: 첫 주문 0001, 동일날 순번 증가, 다른 날짜 독립
  - Negative: 파일 없음, 손상된 JSON, 빈 저장소 상태 조회, 전체 상태 타입 저장/조회

### 커밋
- `d020f15` [AI-Test] Negative TC 추가 - OrderController 경계값/연속오류/취소확인, JsonOrderRepository 전체 검증 (72/72 PASS)

### 리뷰 요청
- 기존 53개 + 신규 19개 = 총 72개 테스트 전부 PASS 확인되었습니다.
- TC 항목 및 커버리지 수준 확인 부탁드립니다.
- 이상 없으면 Feature-03(주문 승인/거절) 구현 진행 지시 부탁드립니다.

---
### 리뷰 (by User)
- NegativeTC 확인, 커버리지 확인. 이견 없음

### 다음 작업 지시
- Feature-03 구현 진행

## [2026-06-12] Feature-03 주문 승인/거절 구현

### 작업 내용
- `ApprovalController` 구현: 물리적 재고 계산, RESERVED→CONFIRMED/PRODUCING/REJECTED 전이
- `checkAndCompleteProduction()`: 메뉴 진입 시 경과시간 체크 → 완료 시 CONFIRMED 전환 + stock 증가
- `calcPhysicalStock()`: `sample.stock + Σ(PRODUCING.actualProd * min(1.0, elapsed/total))`
- `IApprovalView` / `ApprovalView` 구현 (12개 메서드)
- `main.cpp`에 ApprovalController 연결 (메뉴 "3")
- `ApprovalControllerTest.cpp`: 15개 테스트 (87/87 전체 통과)
- 버그 수정: 생산 완료 전환 시 `sample.stock += actualProduction` 누락 → 수정 후 테스트 통과

### 커밋
- `4e0ab07` [AI-Feature] Feature-03 주문 승인/거절 구현 (물리적재고/생산량계산/완료감지)

### 리뷰 요청
- 물리적 재고 계산 공식 (`physStock = stock + Σ(actualProd * ratio)`) 구현 확인 요청
- 생산 완료 시 `sample.stock` 업데이트 처리 확인 요청 (PRODUCING→CONFIRMED 전환과 동시에 재고 반영)
- RESERVED→PRODUCING 전환 시 `actualProduction`, `totalProductionTimeMin`, `productionStartTime` 필드 기록 확인
- 전체 87개 테스트 통과 확인

---
### 리뷰 (by User)
- 물리적 재고 계산 구현 확인
- 생산 완료시 업데이트는 처리되나, PRODUCING 상태에서 재고 시간이 흐름에 따라 계속 물리적 재고에 반영되어야 함.
현재는 생산 완료시에만 처리하는 것으로 확인
- 메뉴가 바뀌거나, 첫 화면이 다시 출력될 때마다 재고 실시간 업데이트 되어야 함 (PRODUCING 상태인 주문이 있을 때 첫 화면에 출력되는 재고도 첫 화면 재출력시마다 물리적 재고 반영되어 늘어나야 함)

### 다음 작업 지시
- 리뷰 의견 반영하여 구현 지시

## [2026-06-12] 물리적 재고 실시간 반영 수정 (리뷰 반영)

### 작업 내용
- `StockService` 신규 생성 (`service/StockService.h/.cpp`):
  - `checkAndCompleteProduction()`: 생산 완료 감지 + 주문 상태 + 시료 stock 업데이트, 완료된 주문번호 목록 반환
  - `calcPhysicalStock(sample)`: `stock + Σ(actualProd * min(1.0, elapsed/total))`
  - `findSampleById()`, `now()` 위임 메서드
- `ApprovalController` 리팩토링: `StockService` 주입 방식으로 변경 (sampleRepo/clock 직접 의존 제거)
- `main.cpp` 메인 루프: 메뉴 출력 직전마다 `stockService.checkAndCompleteProduction()` 호출
- `showMainMenu`: `sample.stock` 단순 합산 → `stockService.calcPhysicalStock(s)` 합산으로 변경 (물리적 재고 실시간 표시)
- 87/87 테스트 전부 통과

### 커밋
- `a1d0836` [AI-Fix] StockService 추출 - 물리적 재고 실시간 반영 (메뉴 진입마다 갱신)

### 리뷰 요청
- 메인 화면 `총 재고` 수치가 PRODUCING 주문의 진행분을 반영하여 실시간으로 변하는 동작 확인 요청
- 메뉴 진입마다 `checkAndCompleteProduction` 호출 → 생산 완료 시 알림 출력 동작 확인 요청
- ApprovalController에서도 동일 StockService를 사용하므로 이중 호출 시 멱등성 보장 확인 요청
- 이상 없으면 Feature-04(모니터링) 구현 지시 부탁드립니다.

---
### 리뷰 (by User)
- 기능 확인
- 생산 시간이 최소 1min으로 구현됨 확인

### 다음 작업 지시
- 생산시간 소숫점 단위 입력 가능하도록 리팩토링 (0.05min/ea 등)

## [2026-06-12] 생산시간 소수점 입력 지원 (리뷰 반영)

### 작업 내용
- 코드 확인: `avgProductionTime`은 이미 `double`, 입력도 `stod()` 사용 → 소수점 입력 자체는 원래 동작
- `SampleView.showTimePrompt()`: 프롬프트에 "(소수점 입력 가능)" 힌트 추가
- `SampleView.showRegisterSuccess()` / `showSampleList()`: avgProductionTime 표시에 `std::fixed << std::setprecision(4)` 적용 → 0.0500 min/ea 형식으로 명확하게 표시
- `SampleControllerTest`: `RegisterSampleWithSmallDecimalAvgTimeSucceeds` (0.05), `RegisterSampleWithTinyDecimalAvgTimeSucceeds` (0.001) TC 추가
- `ApprovalControllerTest`: `SmallDecimalAvgTimeProductionTimeIsCorrect` (생산량 계산), `SmallDecimalAvgTimeCompletesQuickly` (0.5min 생산 완료 31초 경과) TC 추가
- 91/91 테스트 전부 통과

### 커밋
- `628718b` [AI-Fix] 생산시간 소수점 입력 지원 명시 (프롬프트 힌트, 표시 형식 개선) + [AI-Test] 소수점 TC 추가

### 리뷰 요청
- 프롬프트에 "(소수점 입력 가능)" 힌트 추가 및 `0.0500 min/ea` 형식 표시 확인 요청
- 0.05, 0.001 등 소수점 avgTime 등록/생산계산/완료감지 시나리오 TC 확인 요청
- 이상 없으면 Feature-04(모니터링) 구현 지시 부탁드립니다.

---
### 리뷰 (by User)
- 생산시간 정상 표시 확인
- TC 시나리오 및 TC 결과 정상 확인

### 다음 작업 지시
- 유사한 기능인 Feature-04와 Feature-05 동시 구현 
- PoC 프로젝트 중 DataMonitor 구현 참조하여 개발

## [2026-06-12] Feature-04 모니터링 + Feature-05 생산라인 조회 구현

### 작업 내용
- `MonitorController` / `MonitorView` / `IMonitorView` 신규 구현 (Feature-04)
  - 주문 상태별 수 집계 (RESERVED/CONFIRMED/PRODUCING/RELEASE), REJECTED 제외
  - 시료별 재고 상태: CONFIRMED+PRODUCING 주문량 합산 vs stock 비교 → 여유/부족/고갈
  - 메뉴 진입 시 생산 완료 자동 감지 표시
- `ProductionController` / `ProductionView` / `IProductionView` 신규 구현 (Feature-05)
  - FIFO 큐: PRODUCING 주문을 productionStartTime 오름차순 정렬, 첫 번째가 현재 생산 중
  - 진행률(%), 잔여시간(min) 실시간 계산 표시
  - 대기 큐: 순서/주문번호/시료명/잔여시간 표 형식 출력
- `StockService.checkAndCompleteProduction()`: `totalProductionTimeMin <= 0.0` 방어 가드 추가
- `main.cpp`: 메뉴 [4] 모니터링, [5] 생산라인 조회 연결
- 테스트: 3건 실패 수정 후 **108/108 전부 통과**
  - `OrderStatsCountedCorrectly`: PRODUCING 테스트 주문에 `totalProdTimeMin=999` 설정
  - `StockStatusDepleted`: S-001/"여유" + S-002/"고갈" 양쪽 EXPECT_CALL 명시
  - `ProgressCapped → ElapsedExceedsTotal`: elapsed >= total 시 CONFIRMED 전환 동작 검증으로 변경

### 커밋
- `6f9550b` [AI-Feature] Feature-04/05 모니터링 및 생산라인 조회 구현
- `339bd1d` [AI-Test] Feature-04/05 MonitorController 및 ProductionController 테스트 추가

### 리뷰 요청
- [4] 모니터링 메뉴: 상태별 주문 수 및 재고 상태(여유/부족/고갈) 표시 정상 동작 확인 요청
- [5] 생산라인 조회 메뉴: FIFO 순서, 진행률, 잔여시간 표시 정상 동작 확인 요청
- PRODUCING 주문이 완료 시간 경과 후 메뉴 진입 시 자동으로 CONFIRMED 전환 동작 확인 요청
- 이상 없으면 Feature-06(출고 처리) 구현 지시 부탁드립니다.

---
### 리뷰 (by User)
- Feature 6 구현 중 리뷰/테스트 진행 예정
- 리뷰/테스트 의견 발생시 다음 Worklog에 작성

### 다음 작업 지시
- Feature 3,4,5 Negative TC 추가 후 테스트
- 테스트 성공시 Feature 6 구현 지시

## [2026-06-12] Feature-03/04/05 Negative TC 추가 + Feature-06 출고 처리 구현

### 작업 내용
**Negative TC (Feature-03/04/05)**
- ApprovalControllerTest 3개 추가: `StockOneShortGoesToProducing`(경계값), `NegativeIndexShowsOrderNotFound`, `SampleNotFoundForOrderShowsNotFound`
- MonitorControllerTest 2개 추가: `ReleaseOrderExcludedFromStockCalculation`, `StockBoundaryOneLessThanOrdered`
- ProductionControllerTest 3개 추가: `ProgressNearHundredPercent`(99.83% 미완료), `UnknownSampleIdFallsBackToId`, `ThreeOrdersFifoQueueOrdering`
- 116/116 전부 통과 확인

**Feature-06 출고 처리 구현**
- `IReleaseView.h` / `ReleaseView.h/.cpp` / `ReleaseController.h/.cpp` 신규 생성
- `StockService.today()` 추가 (처리 일시 표시용)
- CONFIRMED 주문 목록 표시 → 선택 → RELEASE 전환 + 재고 차감 (decreaseStock)
- 메뉴 진입 시 생산 완료 자동 감지 (checkAndCompleteProduction)
- `main.cpp`: 메뉴 [6] 출고 처리 연결
- `ReleaseControllerTest.cpp` 9개 테스트: 기본 흐름 4개 + Negative 5개
- **125/125 전부 통과**

### 커밋
- `d6dc0a0` [AI-Test] Feature-03/04/05 Negative TC 추가 (116/116 통과)
- `4321d51` [AI-Feature] Feature-06 출고 처리 구현
- `85ec029` [AI-Test] Feature-06 ReleaseController 테스트 추가 (125/125 통과)

### 리뷰 요청
- Feature-03/04/05 Negative TC 추가 내용 확인 요청
- [6] 출고 처리 메뉴: CONFIRMED 주문 선택 → RELEASE 전환 및 재고 차감 동작 확인 요청
- PRODUCING → CONFIRMED 자동 전환 후 즉시 출고 가능한 동작 확인 요청
- 전체 6개 메뉴 + 종료 기능 최종 완료. 추가 리팩토링/보완 사항 지시 부탁드립니다.

---
### 리뷰 (by User)
- 빌드 되지 않았으나 수정 후 정상 빌드 확인
- 출고 기능 정상 동작 확인

### 다음 작업 지시
- Feature 4. 모니터링에서 현재 재고에 PRODUCTION에서 생산중인 수량도 물리적 재고에 실시간 반영되어야 함
   → Monitoring 실행할때마다 Production 중인 수량은 생산된 량만큼 물리적 재고/화면에 실시간 업데이트
- Feature 4. 모니터링에서 잔여율 그래프 있어야 함. feature 스펙에 추가
- Feature 6에 대해 Negative TC 추가 및 테스트

---

## [2026-06-12] Feature-04 물리적 재고 실시간 반영 및 잔여율 그래프, Feature-06 Negative TC

### 작업 내용

**Feature-04 모니터링 물리적 재고 실시간 반영**
- `IMonitorView::showStockRow`: `double physStock` 파라미터 추가 (3인자 → 4인자)
- `MonitorController`: `s.stock` 대신 `stockService_.calcPhysicalStock(s)` 사용
  - PRODUCING 주문 진행분 비례 생산량이 재고 판단에 실시간 반영됨
  - 상태 판단 기준: `physStock <= 0 → 고갈`, `physStock >= totalQty → 여유`, else `부족`
- `MonitorView::showStockRow`: 물리적 재고(실시간) 표시 + 잔여율 막대 그래프 추가

**Feature-04 잔여율 막대 그래프**
- 10칸 막대 그래프: █ (채움) / ░ (비움), 1칸 = 10%
- `잔여율 = min(100, physStock / CONFIRMED+PRODUCING 주문량 * 100)`, 주문량=0이면 100%
- `FEATURE-04-monitoring.md`: 물리적 재고 정책 및 잔여율 그래프 섹션 추가

**Feature-06 Negative TC 추가**
- `ReservedOrderNotShownInReleaseList`: RESERVED 주문만 있을 때 출고 목록 비어 있어야 함
- `ProducingOrderNotShownInReleaseList`: PRODUCING 주문만 있을 때 (아직 완료 안 됨) 비어야 함
- `ReleasedOrderNotShownInReleaseList`: 이미 RELEASE된 주문은 목록에 미노출
- `StockExactlyZeroAfterRelease`: stock=qty 출고 시 재고가 정확히 0이 되는 경계값 테스트

**MonitorControllerTest 업데이트**
- mock `showStockRow` 시그니처: 3인자 → 4인자
- 기존 EXPECT_CALL 전체를 `(_, _, totalQty, status)` 형식으로 변경
- `PhysicalStockFromProducingProgressIsReflected` TC 추가

### 커밋
- `96bb644` [AI-Feature] Feature-04 모니터링: 물리적 재고 실시간 반영 및 잔여율 그래프 추가
- `7cd7c83` [AI-Test] Feature-04/06 테스트 업데이트: physStock 파라미터 반영 및 Negative TC 추가

### 리뷰 요청
- 130/130 전체 통과 확인
- 모니터링 화면([4])에서 PRODUCING 주문이 있을 때 물리적 재고가 진행분 비례로 표시되는지 확인 요청
  - 예) stock=0, PRODUCING actualProd=100 중 50% 진행 → 물리적재고 50.00 ea 표시
- 잔여율 그래프 막대([██████░░░░] 60%) 출력 형식 확인 요청
- Feature-06 Negative TC: RESERVED/PRODUCING/RELEASE 주문이 출고 목록에 노출되지 않는지 확인 요청

---
### 리뷰 (by User)
- 잔여율 그래프 정상 출력 확인
- TC 범위, Negative TC 확인

- 모니터링시 시료별 재고 현황에서, 재고는 정수여야 함. (ea임)
- 생산라인 조회시 실 생산량은 정수여야 함 (ea임)

- SPEC 문서 (PDF 파일, @Document/spec/[CRA_AI] Day3_개인과제_반도체시료관리.pdf) 11페이지의 예시 UI 화면이 Main 화면에 그대로 구현되지 않음
 . 로딩될때는 화면을 클리어 하고 다시 출력되어야 함 
 . 상단 아스키 아트가 빠져 있음
 . 시스템 현황 + 시간이 빠져 있음
 . 아스키 아트, 시스템 현황, 메뉴 1~0, 선택> 사이에 가로바로 구분이 되어 있어야 함

- SPEC 문서 (PDF 파일, @Document/spec/[CRA_AI] Day3_개인과제_반도체시료관리.pdf) 13페이지의 예시 UI 화면이 시료관리 화면에 그대로 구현되지 않음
 . 로딩될때는 화면을 클리어 하고 다시 출력되어야 함
 . 맨 위 "[1] 시료 관리" 출력
 . 시료 등록, 시료 목록, 시료 검색, 뒤로는 가로로 출력
 . 시료 관리, 메뉴, 이후 입력과 출력 사이에는 입출력마다 가로바로 구분
 . 평균 생산시간과 수율이 너무 가까움
 . 데이터 출력과 아래 실제 데이터가 빈칸 간격이 맞지 않음

### 다음 작업 지시
- 리뷰 내용 보완 개발

---

## [2026-06-12] UI 개선: 재고 정수 표시, 메인/시료관리 화면 스펙 보완

### 작업 내용

**재고 정수 표시 수정**
- `MonitorView::showStockRow`: physStock을 소수점 2자리(`100.00 ea`) → 정수(`100 ea`) 표시로 변경
- `actualProduction`은 원래 `int`형으로 ProductionView에서 이미 정수 표시 확인 (변경 불필요)

**메인 메뉴 화면 스펙 보완 (PDF 11페이지)**
- `showMainMenu()`: `system("cls")` 추가 (매 메인메뉴 진입 시 화면 클리어)
- ASCII 배너 추가 (S-Semi 로고, 5행 ASCII 아트 + "반도체 시료 생산주문관리 시스템")
- 현재 시간 표시 추가 (`localtime_s` 사용, `YYYY-MM-DD HH:MM:SS` 형식)
- 2컬럼 메뉴 레이아웃: `[1] 시료 관리               [2] 시료 주문` 형식
- 가로바 구분선 (`=` 68칸, `-` 68칸) 추가로 영역 구분
- 서브메뉴 진입 시 `system("cls")` 추가 (각 컨트롤러 run() 호출 전)

**시료 관리 화면 스펙 보완 (PDF 13페이지)**
- `SampleView::showMenu()`: 가로 메뉴 레이아웃 구현
  `[1] 시료 등록   [2] 시료 목록   [3] 시료 검색   [0] 뒤로`
- 화면 구조: `=70` 구분선 → `[1] 시료 관리` 타이틀 → `-70` 구분선 → 메뉴 → `-70` 구분선 → `선택 > `
- `showSampleList()`: `-70` 구분선 추가 (콘텐츠 영역 분리), `avgProductionTime` 4자리→1자리 표시
- `showRegisterPrompt()`: `-70` 구분선 추가 (등록 섹션 시작 구분)
- `showSearchMenu()`: 가로 메뉴 레이아웃 구현

### 커밋
- `d145149` [AI-Fix] UI 개선: 재고 정수 표시, 메인/시료관리 화면 스펙 보완

### 리뷰 요청
- 130/130 테스트 통과 확인
- 메인 메뉴 화면: ASCII 배너, 현재 시간, 2컬럼 메뉴, 가로바 구분선 확인 요청
- 시료 관리 화면: 가로 메뉴, 구분선, 시료 목록 컬럼 간격(평균생산시간↔수율) 확인 요청
- 모니터링: 재고 표시 정수 여부 확인 요청
- 추가 보완이 필요한 화면이 있으면 지시 부탁드립니다.

---
### 리뷰 (by User)
- 생산라인 조회 화면에서 대기중인 주문 제대로 출력되지 않음
- 3 메뉴 들어갔을 때 승인할 주문이 없을 경우, 에러메시지 출력 후 엔터 입력받아 메인 메뉴로 복귀 (바로 복귀하여 에러메시지 보이지 않음)
- 4,5 메뉴 들어갔을 때 엔터 입력받아 메인 메뉴로 복귀 (바로 복귀하여 모니터링 보이지 않음)

- SPEC 문서 (PDF 파일, @Document/spec/[CRA_AI] Day3_개인과제_반도체시료관리.pdf) 15페이지의 예시 UI 화면이 시료 주문 화면에 그대로 구현되지 않음
 . 시료 주문 화면 로딩될때는 화면을 클리어 하고 다시 출력되어야 함
 . 맨 위 "[2] 시료 주문" 출력
 . [주문 내용 확인]이 아니라 [입력 내용 확인] 임
 . 확인시 시료 ID가 아니라 시료명이 표시되고, 시료 ID는 뒤에 (시료ID)형태로 출력되어야 함
 . 예약 접수하시겠습니까? [Y/N] 이 아니라 수량 이후 한줄 띄고, "[Y] 예약 접수 [N] 취소" 다음줄 "선택 >" 형태가 되어야 함.
 . Y 이후 출력 메시지 "[완료] 주문이 접수되었습니다." 가 아니라 "예약 접수 완료"
 . 상태 이후 "※ 재고 확인은 [3] 승인 메뉴에서 직접 진행하세요" 출력, 이후 "선택>"출력
 . 상위 메뉴 이동은 빈 입력시 상위 메뉴로 이동할지 물어보고 이동함

- SPEC 문서 (PDF 파일, @Document/spec/[CRA_AI] Day3_개인과제_반도체시료관리.pdf) 19페이지의 예시 UI 화면이 모니터링 화면에 그대로 구현되지 않음
 . 모니터링 화면 로딩될때는 화면을 클리어 하고 다시 출력되어야 함
 . 나머지 화면 출력 PDF 파일과 같은 방식으로 출력

- SPEC 문서 (PDF 파일, @Document/spec/[CRA_AI] Day3_개인과제_반도체시료관리.pdf) 21페이지의 예시 UI 화면이 생산라인 조회 화면에 그대로 구현되지 않음
 . 로딩될때는 화면을 클리어 하고 다시 출력되어야 함
 . 나머지 화면 출력 PDF 파일과 같은 방식으로 출력

- SPEC 문서 (PDF 파일, @Document/spec/[CRA_AI] Day3_개인과제_반도체시료관리.pdf) 23페이지의 예시 UI 화면이 출고 처리 화면에 그대로 구현되지 않음
 . 로딩될때는 화면을 클리어 하고 다시 출력되어야 함
 . 나머지 화면 출력 PDF 파일과 같은 방식으로 출력

### 다음 작업 지시
- 리뷰 의견 반영
- SPEC PDF 문서의 15,19,21,23 페이지 그림 재확인, 최대한 비슷하게 구현 Update

---

## [2026-06-12] PDF 스펙 기반 UI 개선 및 생산라인 조회 버그 수정

### 작업 내용

**시료 주문 화면 (PDF 15p)**
- `IOrderView::showOrderConfirmation`: sampleId 단독 → (sampleId, sampleName, customerName, qty) 4인자로 변경
- `OrderView::showOrderConfirmation`: 시료명(시료ID) 형식으로 표시, "[입력 내용 확인]" 헤더
- `OrderView::showConfirmPrompt`: "[Y] 예약 접수   [N] 취소 / 선택 >" 형식
- `OrderView::showOrderSuccess`: "예약 접수 완료." + 주문번호/상태/※ 재고 확인 안내
- `OrderView::showPressEnterPrompt`: 예약 접수 완료 후 "선택 >" 대기 후 복귀
- `OrderController`: showOrderConfirmation에 sampleName 조회 후 전달

**모니터링 화면 (PDF 19p)**
- `IMonitorView::showMenu`: timestamp 인자 추가
- `MonitorController`: `stockService_.now()` + `localtime_s`로 타임스탬프 생성 → showMenu 전달
- `MonitorView::showMenu`: `[4] 모니터링   YYYY-MM-DD HH:MM:SS` 형식 헤더
- `MonitorView::showOrderStats`: PRODUCING > 0 시 "← 생산라인 대기" 표시
- 화면 표시 후 "선택 >" Enter 대기 후 메인 복귀

**생산라인 조회 화면 (PDF 21p) + 대기 주문 버그 수정**
- `IProductionView::showQueueItem`: shortageQty, estimatedCompletion 인자 추가 (4→5인자)
- `ProductionView::showCurrentProduction`: 주문번호/시료/주문량/부족/실생산량/총생산시간 + 진행률 바 + 완료 예정 HH:MM
- `ProductionView::showQueueItem`: 순서/주문번호/시료/주문량/부족분/실생산량/예상완료 표 형식
- `ProductionView::showPressEnterPrompt`: 부족분/실생산량 공식 안내 + "선택 >" 대기
- **버그 수정**: 대기 주문 예상 완료 시간을 각자 productionStartTime 기준이 아닌 현재 주문 완료 기준 누적 계산으로 수정
- `Order::shortageQty` 필드 추가, `JsonOrderRepository`에 직렬화 반영
- `ApprovalController`: PRODUCING 전환 시 `shortageQty` 저장

**출고 처리 화면 (PDF 23p)**
- `IReleaseView::showConfirmedOrderList`: sampleNames 인자 추가 (1→2인자)
- `ReleaseView::showConfirmedOrderList`: 주문번호/고객/시료명/수량 형식 (시료ID 대신 시료명)
- `ReleaseView::showReleaseCompleted`: "출고 처리 완료." + 주문번호/출고수량/처리일시/상태 표시
- `ReleaseController`: sampleRepo에서 시료명 조회 후 showConfirmedOrderList 전달
- 화면 표시 후 "선택 >" Enter 대기 후 복귀

**주문 승인/거절 (리뷰 반영)**
- `ApprovalController::run()`: showApprovalMenu() 호출 후 빈 목록 체크 → Enter 대기 후 복귀

### 커밋
- `053273b` [AI-Feature] PDF 스펙 기반 UI 개선 및 생산라인 조회 버그 수정

### 리뷰 요청
- 빌드 성공, 130/130 테스트 전부 통과 확인되었습니다.
- [2] 시료 주문: 입력 확인 화면에서 시료명(시료ID) 형식 및 "[Y] 예약 접수 [N] 취소 / 선택 >" 동작 확인 요청
- [3] 주문 승인/거절: 빈 목록 시 헤더 → 에러메시지 → Enter 대기 후 메인 복귀 동작 확인 요청
- [4] 모니터링: 타임스탬프 헤더, PRODUCING 시 "← 생산라인 대기" 표시, Enter 대기 후 복귀 확인 요청
- [5] 생산라인 조회: 현재 주문 진행률 바 + 완료 예정 시각, 대기 주문 누적 완료 시각 확인 요청
- [6] 출고 처리: CONFIRMED 목록에 시료명 표시, 출고 완료 화면, Enter 대기 후 복귀 확인 요청
- 추가 보완 필요한 화면이 있으면 지시 부탁드립니다.

---
### 리뷰 (by User)
- 시료 관리에서 시료 추가에 들어갔을때, 빈 엔터치면 시료 추가를 취소하던 루틴이 사라졌어. 해당 루틴 추가해줘.

### 다음 작업 지시
- 시료 등록 중 빈 입력 취소 확인 기능 복구

---

## [2026-06-12] 시료 등록 중 빈 입력 취소 확인 기능 복구

### 작업 내용
- `ISampleView`: `showCancelConfirmPrompt()` / `showRegisterCancelled()` non-pure 메서드 추가
- `SampleView`: 취소 확인 프롬프트("시료 등록을 취소하시겠습니까? [Y/N] >") 및 취소 완료 메시지 구현
- `SampleController::handleRegister()`: 각 입력 단계(ID / 이름 / 평균생산시간 / 수율)에서 빈 입력 시
  - "시료 등록을 취소하시겠습니까? [Y/N]" 출력 → Y/y: 등록 취소 후 복귀, 기타: 재입력
- 수율 범위 오류 처리: 기존 `return` 방식 → 루프 방식으로 변경 (재입력 가능, 다른 필드와 일관성)
- `SampleControllerTest`: 기존 수율 오류 TC 입력 수정(루프 대응) + 취소 확인 TC 6개 추가
  - `EmptyIdThenConfirmYCancelsRegister`, `EmptyIdDeclineCancelThenSucceeds`
  - `EmptyNameThenConfirmYCancelsRegister`, `EmptyAvgTimeThenConfirmYCancelsRegister`
  - `EmptyYieldThenConfirmYCancelsRegister`, `EmptyYieldDeclineCancelThenSucceeds`

### 커밋
- `6801828` [AI-Fix] 시료 등록 중 빈 입력 취소 확인 기능 추가 (136/136 PASS)

### 리뷰 요청
- 136/136 테스트 전부 통과 확인되었습니다.
- [1] 시료 관리 → [1] 시료 등록: 각 입력 단계에서 빈 엔터 입력 시 취소 확인 문구 동작 확인 요청
  - Y 입력 → "[취소] 시료 등록이 취소되었습니다." 출력 후 시료 관리 메뉴로 복귀
  - N/기타 입력 → 해당 필드 재입력 프롬프트
- 추가 보완 사항 있으면 지시 부탁드립니다.

