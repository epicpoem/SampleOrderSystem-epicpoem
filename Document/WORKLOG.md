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
