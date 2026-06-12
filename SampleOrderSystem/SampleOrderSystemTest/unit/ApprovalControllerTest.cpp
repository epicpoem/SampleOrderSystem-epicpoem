#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include <cmath>
#include "SampleOrderSystem/controller/ApprovalController.h"
#include "SampleOrderSystem/view/IApprovalView.h"
#include "SampleOrderSystem/repository/ISampleRepository.h"
#include "SampleOrderSystem/repository/IOrderRepository.h"
#include "SampleOrderSystem/service/StockService.h"
#include "SampleOrderSystem/util/FakeClock.h"

using ::testing::_;
using ::testing::NiceMock;

// ── 테스트용 인메모리 구현체 ──────────────────────────────────────────────

namespace {

class ApprSampleRepo : public ISampleRepository {
    std::vector<Sample> samples_;
public:
    bool add(const Sample& s) override {
        if (exists(s.id)) return false;
        samples_.push_back(s);
        return true;
    }
    std::vector<Sample> findAll() const override { return samples_; }
    std::optional<Sample> findById(const std::string& id) const override {
        for (const auto& s : samples_)
            if (s.id == id) return s;
        return std::nullopt;
    }
    bool exists(const std::string& id) const override {
        return findById(id).has_value();
    }
    bool update(const Sample& s) override {
        for (auto& x : samples_)
            if (x.id == s.id) { x = s; return true; }
        return false;
    }
    bool decreaseStock(const std::string& id, int amount) override {
        for (auto& s : samples_)
            if (s.id == id && s.stock >= amount) { s.stock -= amount; return true; }
        return false;
    }
};

class ApprOrderRepo : public IOrderRepository {
    std::vector<Order> orders_;
public:
    bool add(const Order& o) override { orders_.push_back(o); return true; }
    std::vector<Order> findAll() const override { return orders_; }
    std::vector<Order> findByStatus(OrderStatus s) const override {
        std::vector<Order> r;
        for (const auto& o : orders_)
            if (o.status == s) r.push_back(o);
        return r;
    }
    std::optional<Order> findByNo(const std::string& no) const override {
        for (const auto& o : orders_)
            if (o.orderNo == no) return o;
        return std::nullopt;
    }
    bool update(const Order& order) override {
        for (auto& o : orders_)
            if (o.orderNo == order.orderNo) { o = order; return true; }
        return false;
    }
    std::string generateOrderNo(const std::string& date) override {
        return "ORD-" + date + "-0001";
    }
};

} // namespace

// ── MockApprovalView ──────────────────────────────────────────────────────

class MockApprovalView : public IApprovalView {
public:
    MOCK_METHOD(void, showApprovalMenu, (), (override));
    MOCK_METHOD(void, showNoReservedOrders, (), (override));
    MOCK_METHOD(void, showReservedOrderList, (const std::vector<Order>&), (override));
    MOCK_METHOD(void, showOrderSelectPrompt, (), (override));
    MOCK_METHOD(void, showOrderNotFound, (), (override));
    MOCK_METHOD(void, showStockSufficient, (double, int), (override));
    MOCK_METHOD(void, showStockInsufficient,
                (double, int, double, int, double), (override));
    MOCK_METHOD(void, showApprovePrompt, (), (override));
    MOCK_METHOD(void, showApprovedAsConfirmed, (const Order&), (override));
    MOCK_METHOD(void, showApprovedAsProducing, (const Order&), (override));
    MOCK_METHOD(void, showRejected, (const Order&), (override));
    MOCK_METHOD(void, showProductionCompleted, (const std::string&), (override));
};

// ── 헬퍼: 간단한 주문 생성 ────────────────────────────────────────────────

static Order makeReserved(const std::string& no, const std::string& sampleId,
                           const std::string& customer, int qty) {
    Order o;
    o.orderNo      = no;
    o.sampleId     = sampleId;
    o.customerName = customer;
    o.quantity     = qty;
    o.status       = OrderStatus::RESERVED;
    return o;
}

// ── Fixture ───────────────────────────────────────────────────────────────

class ApprovalControllerTest : public ::testing::Test {
protected:
    ApprSampleRepo sampleRepo;
    ApprOrderRepo  orderRepo;
    FakeClock      clock{0, "20260612"};
    StockService   stockService{sampleRepo, orderRepo, clock};

    void SetUp() override {
        // 기본 시료: yield=0.9, avgProdTime=1.0 min/ea, stock=100
        sampleRepo.add({"S-001", "실리콘 웨이퍼", 1.0, 0.9, 100});
    }
};

// ── 기본 흐름 ─────────────────────────────────────────────────────────────

TEST_F(ApprovalControllerTest, NoReservedOrdersShowsEmptyMessage) {
    NiceMock<MockApprovalView> view;
    std::istringstream in("");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showNoReservedOrders()).Times(1);
    ctrl.run();
}

TEST_F(ApprovalControllerTest, ApproveWithSufficientStockBecomesConfirmed) {
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 50));
    NiceMock<MockApprovalView> view;
    // stock=100 >= qty=50 → sufficient
    std::istringstream in("1\nY\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showStockSufficient(_, _)).Times(1);
    EXPECT_CALL(view, showApprovedAsConfirmed(_)).Times(1);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260612-0001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::CONFIRMED);
}

TEST_F(ApprovalControllerTest, ApproveWithInsufficientStockBecomesProducing) {
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 200));
    NiceMock<MockApprovalView> view;
    // stock=100 < qty=200 → insufficient
    std::istringstream in("1\nY\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showStockInsufficient(_, _, _, _, _)).Times(1);
    EXPECT_CALL(view, showApprovedAsProducing(_)).Times(1);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260612-0001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::PRODUCING);
    EXPECT_GT(updated->actualProduction, 0);
    EXPECT_GT(updated->totalProductionTimeMin, 0.0);
}

TEST_F(ApprovalControllerTest, RejectSufficientStockBecomesRejected) {
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 50));
    NiceMock<MockApprovalView> view;
    std::istringstream in("1\nN\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showRejected(_)).Times(1);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260612-0001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::REJECTED);
}

TEST_F(ApprovalControllerTest, RejectInsufficientStockBecomesRejected) {
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 200));
    NiceMock<MockApprovalView> view;
    std::istringstream in("1\nN\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showRejected(_)).Times(1);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260612-0001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::REJECTED);
}

TEST_F(ApprovalControllerTest, InputZeroReturnsWithoutAction) {
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 50));
    NiceMock<MockApprovalView> view;
    std::istringstream in("0\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showApprovePrompt()).Times(0);
    ctrl.run();

    // status unchanged
    auto o = orderRepo.findByNo("ORD-20260612-0001");
    EXPECT_EQ(o->status, OrderStatus::RESERVED);
}

TEST_F(ApprovalControllerTest, InvalidIndexShowsOrderNotFound) {
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 50));
    NiceMock<MockApprovalView> view;
    std::istringstream in("99\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showOrderNotFound()).Times(1);
    ctrl.run();
}

// ── 생산완료 자동 감지 ────────────────────────────────────────────────────

TEST_F(ApprovalControllerTest, ProductionCompletedOnMenuEntry) {
    // PRODUCING 주문: totalProdTime=10min, startTime=0, now=600+s (elapsed >= 10min)
    Order prod;
    prod.orderNo              = "ORD-20260612-0001";
    prod.sampleId             = "S-001";
    prod.customerName         = "고객A";
    prod.quantity             = 100;
    prod.status               = OrderStatus::PRODUCING;
    prod.actualProduction     = 112;
    prod.totalProductionTimeMin = 10.0;
    prod.productionStartTime  = 0;
    orderRepo.add(prod);

    // clock.now() = 601초 → elapsed = 601/60 ≈ 10.017min >= 10min → 완료
    clock.setNow(601);

    NiceMock<MockApprovalView> view;
    std::istringstream in("");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showProductionCompleted(_)).Times(1);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260612-0001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::CONFIRMED);
}

TEST_F(ApprovalControllerTest, ProductionNotCompletedWhenElapsedInsufficient) {
    Order prod;
    prod.orderNo              = "ORD-20260612-0001";
    prod.sampleId             = "S-001";
    prod.customerName         = "고객A";
    prod.quantity             = 100;
    prod.status               = OrderStatus::PRODUCING;
    prod.actualProduction     = 112;
    prod.totalProductionTimeMin = 10.0;
    prod.productionStartTime  = 0;
    orderRepo.add(prod);

    // elapsed = 300/60 = 5min < 10min → 아직 미완료
    clock.setNow(300);

    NiceMock<MockApprovalView> view;
    std::istringstream in("");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showProductionCompleted(_)).Times(0);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260612-0001");
    EXPECT_EQ(updated->status, OrderStatus::PRODUCING);
}

// ── 물리적 재고 실시간 반영 ───────────────────────────────────────────────

TEST_F(ApprovalControllerTest, PhysicalStockIncludesProducingProgress) {
    // stock=0, PRODUCING: actualProd=100, totalProdTime=100min, elapsed=50min → 진행분=50
    // physStock = 0 + 50 = 50
    // 새 주문 qty=40 → sufficient (50 >= 40) → CONFIRMED
    sampleRepo.add({"S-002", "GaN", 1.0, 0.9, 0});

    Order prod;
    prod.orderNo              = "ORD-20260611-0001";
    prod.sampleId             = "S-002";
    prod.customerName         = "고객B";
    prod.quantity             = 100;
    prod.status               = OrderStatus::PRODUCING;
    prod.actualProduction     = 100;
    prod.totalProductionTimeMin = 100.0;
    prod.productionStartTime  = 0;
    orderRepo.add(prod);

    orderRepo.add(makeReserved("ORD-20260612-0001", "S-002", "고객A", 40));

    // elapsed = 3000초 = 50분 → ratio = 50/100 = 0.5 → 진행분 = 50
    clock.setNow(3000);

    NiceMock<MockApprovalView> view;
    std::istringstream in("1\nY\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showStockSufficient(_, _)).Times(1);
    EXPECT_CALL(view, showApprovedAsConfirmed(_)).Times(1);
    ctrl.run();
}

TEST_F(ApprovalControllerTest, PhysicalStockWithFullyCompletedProducingIsCapped) {
    // elapsed >= totalProdTime → ratio=1.0 → 진행분=actualProd 전체
    sampleRepo.add({"S-002", "GaN", 1.0, 0.9, 0});

    Order prod;
    prod.orderNo              = "ORD-20260611-0001";
    prod.sampleId             = "S-002";
    prod.customerName         = "고객B";
    prod.quantity             = 50;
    prod.status               = OrderStatus::PRODUCING;
    prod.actualProduction     = 60;
    prod.totalProductionTimeMin = 60.0;
    prod.productionStartTime  = 0;
    orderRepo.add(prod);

    orderRepo.add(makeReserved("ORD-20260612-0001", "S-002", "고객A", 55));

    // elapsed >> totalProdTime → ratio capped to 1.0 → 진행분 = 60
    clock.setNow(99999);

    NiceMock<MockApprovalView> view;
    std::istringstream in("1\nY\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showStockSufficient(_, _)).Times(1);
    ctrl.run();
}

// ── 생산량 계산 정확성 ────────────────────────────────────────────────────

TEST_F(ApprovalControllerTest, ProductionQuantityCalculationIsCorrect) {
    // stock=10, qty=110, shortage=100
    // actualProd = ceil(100 / (0.9 * 0.9)) = ceil(100/0.81) = ceil(123.46) = 124
    // totalProdTime = 1.0 * 124 = 124 min
    sampleRepo.add({"S-002", "GaN", 1.0, 0.9, 10});
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-002", "고객A", 110));

    NiceMock<MockApprovalView> view;
    std::istringstream in("1\nY\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260612-0001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::PRODUCING);
    // shortage = 100, yield=0.9: ceil(100 / (0.9*0.9)) = ceil(123.457) = 124
    EXPECT_EQ(updated->actualProduction, 124);
    EXPECT_DOUBLE_EQ(updated->totalProductionTimeMin, 124.0);
}

// ── Negative / Edge-case TCs ──────────────────────────────────────────────

TEST_F(ApprovalControllerTest, ExactlyEqualStockBecomesConfirmed) {
    // physStock == qty → sufficient (경계값)
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 100));
    NiceMock<MockApprovalView> view;
    std::istringstream in("1\nY\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showStockSufficient(_, _)).Times(1);
    EXPECT_CALL(view, showApprovedAsConfirmed(_)).Times(1);
    ctrl.run();
}

TEST_F(ApprovalControllerTest, NonNumericIndexShowsOrderNotFound) {
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 50));
    NiceMock<MockApprovalView> view;
    std::istringstream in("abc\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showOrderNotFound()).Times(1);
    ctrl.run();
}

TEST_F(ApprovalControllerTest, MultipleReservedOrdersPickSecond) {
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 50));
    orderRepo.add(makeReserved("ORD-20260612-0002", "S-001", "고객B", 30));
    NiceMock<MockApprovalView> view;
    std::istringstream in("2\nY\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showApprovedAsConfirmed(_)).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0001")->status, OrderStatus::RESERVED);
    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0002")->status, OrderStatus::CONFIRMED);
}

// ── 소수점 avgTime 생산시간 계산 ───────────────────────────────────────────

TEST_F(ApprovalControllerTest, SmallDecimalAvgTimeProductionTimeIsCorrect) {
    // avgTime=0.05 min/ea, stock=0, qty=110
    // shortage=110, actualProd = ceil(110 / (0.9*0.9)) = ceil(135.8) = 136
    // totalProdTime = 0.05 * 136 = 6.8 min
    sampleRepo.add({"S-003", "소수점 시료", 0.05, 0.9, 0});
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-003", "고객A", 110));

    NiceMock<MockApprovalView> view;
    std::istringstream in("1\nY\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260612-0001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::PRODUCING);
    EXPECT_EQ(updated->actualProduction, 136);
    EXPECT_DOUBLE_EQ(updated->totalProductionTimeMin, 0.05 * 136);
}

// ── Negative: 경계값 / 오류 입력 / 미등록 시료 ───────────────────────────────

TEST_F(ApprovalControllerTest, StockOneShortGoesToProducing) {
    // 경계값: physStock=100, qty=101 → stock < qty 로 PRODUCING 전환
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 101));
    NiceMock<MockApprovalView> view;
    std::istringstream in("1\nY\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showStockInsufficient(_, _, _, _, _)).Times(1);
    EXPECT_CALL(view, showApprovedAsProducing(_)).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0001")->status, OrderStatus::PRODUCING);
}

TEST_F(ApprovalControllerTest, NegativeIndexShowsOrderNotFound) {
    // 음수 번호 입력 → showOrderNotFound
    orderRepo.add(makeReserved("ORD-20260612-0001", "S-001", "고객A", 50));
    NiceMock<MockApprovalView> view;
    std::istringstream in("-1\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showOrderNotFound()).Times(1);
    ctrl.run();
}

TEST_F(ApprovalControllerTest, SampleNotFoundForOrderShowsNotFound) {
    // 주문이 참조하는 시료가 sampleRepo에 없으면 showOrderNotFound
    Order o = makeReserved("ORD-20260612-0001", "S-999", "고객A", 50);
    orderRepo.add(o);
    NiceMock<MockApprovalView> view;
    std::istringstream in("1\n");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showOrderNotFound()).Times(1);
    ctrl.run();
}

TEST_F(ApprovalControllerTest, SmallDecimalAvgTimeCompletesQuickly) {
    // avgTime=0.05 min/ea, 생산 완료: elapsed=20초 >= totalProdTime=0.05*10=0.5min=30초
    // 0.05min = 3초, actualProd=10 → totalProdTimeMin=0.5min → 30초
    // elapsed=31초 → 완료
    sampleRepo.add({"S-003", "소수점 시료", 0.05, 0.9, 0});

    Order prod;
    prod.orderNo                = "ORD-20260611-0001";
    prod.sampleId               = "S-003";
    prod.customerName           = "고객B";
    prod.quantity               = 10;
    prod.status                 = OrderStatus::PRODUCING;
    prod.actualProduction       = 10;
    prod.totalProductionTimeMin = 0.5;   // 0.05 * 10
    prod.productionStartTime    = 0;
    orderRepo.add(prod);

    // elapsed=31초 → 31/60=0.5167min >= 0.5min → 완료
    clock.setNow(31);

    NiceMock<MockApprovalView> view;
    std::istringstream in("");
    ApprovalController ctrl(in, view, orderRepo, stockService);

    EXPECT_CALL(view, showProductionCompleted(_)).Times(1);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260611-0001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::CONFIRMED);
}
