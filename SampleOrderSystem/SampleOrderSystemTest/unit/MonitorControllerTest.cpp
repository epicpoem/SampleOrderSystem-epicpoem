#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include "SampleOrderSystem/controller/MonitorController.h"
#include "SampleOrderSystem/view/IMonitorView.h"
#include "SampleOrderSystem/repository/ISampleRepository.h"
#include "SampleOrderSystem/repository/IOrderRepository.h"
#include "SampleOrderSystem/service/StockService.h"
#include "SampleOrderSystem/util/FakeClock.h"

using ::testing::_;
using ::testing::NiceMock;

namespace {

class MonSampleRepo : public ISampleRepository {
    std::vector<Sample> data_;
public:
    bool add(const Sample& s) override {
        if (exists(s.id)) return false;
        data_.push_back(s);
        return true;
    }
    std::vector<Sample> findAll() const override { return data_; }
    std::optional<Sample> findById(const std::string& id) const override {
        for (const auto& s : data_) if (s.id == id) return s;
        return std::nullopt;
    }
    bool exists(const std::string& id) const override { return findById(id).has_value(); }
    bool update(const Sample& s) override {
        for (auto& x : data_) if (x.id == s.id) { x = s; return true; }
        return false;
    }
    bool decreaseStock(const std::string& id, int amount) override {
        for (auto& s : data_)
            if (s.id == id && s.stock >= amount) { s.stock -= amount; return true; }
        return false;
    }
};

class MonOrderRepo : public IOrderRepository {
    std::vector<Order> data_;
public:
    bool add(const Order& o) override { data_.push_back(o); return true; }
    std::vector<Order> findAll() const override { return data_; }
    std::vector<Order> findByStatus(OrderStatus s) const override {
        std::vector<Order> r;
        for (const auto& o : data_) if (o.status == s) r.push_back(o);
        return r;
    }
    std::optional<Order> findByNo(const std::string& no) const override {
        for (const auto& o : data_) if (o.orderNo == no) return o;
        return std::nullopt;
    }
    bool update(const Order& order) override {
        for (auto& o : data_) if (o.orderNo == order.orderNo) { o = order; return true; }
        return false;
    }
    std::string generateOrderNo(const std::string& date) override {
        return "ORD-" + date + "-0001";
    }
};

static Order makeOrder(const std::string& no, const std::string& sid,
                       int qty, OrderStatus st) {
    Order o;
    o.orderNo    = no;
    o.sampleId   = sid;
    o.quantity   = qty;
    o.status     = st;
    return o;
}

} // namespace

class MockMonitorView : public IMonitorView {
public:
    MOCK_METHOD(void, showMenu, (), (override));
    MOCK_METHOD(void, showOrderStats, (int, int, int, int), (override));
    MOCK_METHOD(void, showStockTableHeader, (), (override));
    MOCK_METHOD(void, showStockRow,
                (const Sample&, int, const std::string&), (override));
    MOCK_METHOD(void, showNoSamples, (), (override));
    MOCK_METHOD(void, showProductionCompleted, (const std::string&), (override));
};

class MonitorControllerTest : public ::testing::Test {
protected:
    MonSampleRepo sampleRepo;
    MonOrderRepo  orderRepo;
    FakeClock     clock{0, "20260612"};
    StockService  stockService{sampleRepo, orderRepo, clock};

    void SetUp() override {
        sampleRepo.add({"S-001", "실리콘 웨이퍼", 1.0, 0.9, 100});
    }
};

// ── 기본 통계 ────────────────────────────────────────────────────────────────

TEST_F(MonitorControllerTest, NoOrdersShowsZeroStats) {
    NiceMock<MockMonitorView> view;
    std::istringstream in("");
    MonitorController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showOrderStats(0, 0, 0, 0)).Times(1);
    ctrl.run();
}

TEST_F(MonitorControllerTest, OrderStatsCountedCorrectlyByStatus) {
    orderRepo.add(makeOrder("O-1", "S-001", 10, OrderStatus::RESERVED));
    orderRepo.add(makeOrder("O-2", "S-001", 10, OrderStatus::CONFIRMED));
    orderRepo.add(makeOrder("O-3", "S-001", 10, OrderStatus::CONFIRMED));
    // PRODUCING: totalProductionTimeMin > 0 으로 설정해야 checkAndCompleteProduction 에서 완료 처리되지 않음
    Order prodOrder = makeOrder("O-4", "S-001", 10, OrderStatus::PRODUCING);
    prodOrder.totalProductionTimeMin = 999.0;
    orderRepo.add(prodOrder);
    orderRepo.add(makeOrder("O-5", "S-001", 10, OrderStatus::RELEASE));
    orderRepo.add(makeOrder("O-6", "S-001", 10, OrderStatus::RELEASE));
    orderRepo.add(makeOrder("O-7", "S-001", 10, OrderStatus::RELEASE));

    NiceMock<MockMonitorView> view;
    std::istringstream in("");
    MonitorController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showOrderStats(1, 2, 1, 3)).Times(1);
    ctrl.run();
}

TEST_F(MonitorControllerTest, RejectedOrdersExcludedFromStats) {
    orderRepo.add(makeOrder("O-1", "S-001", 10, OrderStatus::REJECTED));
    orderRepo.add(makeOrder("O-2", "S-001", 10, OrderStatus::RESERVED));

    NiceMock<MockMonitorView> view;
    std::istringstream in("");
    MonitorController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showOrderStats(1, 0, 0, 0)).Times(1);
    ctrl.run();
}

// ── 재고 상태 ────────────────────────────────────────────────────────────────

TEST_F(MonitorControllerTest, StockStatusExcess_WhenStockCoversAll) {
    // stock=100, CONFIRMED qty=50 → 여유
    orderRepo.add(makeOrder("O-1", "S-001", 50, OrderStatus::CONFIRMED));

    NiceMock<MockMonitorView> view;
    std::istringstream in("");
    MonitorController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showStockRow(_, 50, std::string("여유"))).Times(1);
    ctrl.run();
}

TEST_F(MonitorControllerTest, StockStatusShortage_WhenStockLessThanOrdered) {
    // stock=100, CONFIRMED+PRODUCING=150 → 부족
    orderRepo.add(makeOrder("O-1", "S-001", 100, OrderStatus::CONFIRMED));
    orderRepo.add(makeOrder("O-2", "S-001",  50, OrderStatus::PRODUCING));

    NiceMock<MockMonitorView> view;
    std::istringstream in("");
    MonitorController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showStockRow(_, 150, std::string("부족"))).Times(1);
    ctrl.run();
}

TEST_F(MonitorControllerTest, StockStatusDepleted_WhenStockIsZero) {
    // S-001 (fixture, stock=100, no orders) → "여유"
    // S-002 (stock=0, CONFIRMED qty=50)     → "고갈"
    sampleRepo.add({"S-002", "GaN", 1.0, 0.9, 0});
    orderRepo.add(makeOrder("O-1", "S-002", 50, OrderStatus::CONFIRMED));

    NiceMock<MockMonitorView> view;
    std::istringstream in("");
    MonitorController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showStockRow(_, 0,  std::string("여유"))).Times(1);
    EXPECT_CALL(view, showStockRow(_, 50, std::string("고갈"))).Times(1);
    ctrl.run();
}

TEST_F(MonitorControllerTest, StockStatus_ReservedOrderExcluded) {
    // RESERVED는 재고 계산에서 제외: stock=100, RESERVED=200 → 여유
    orderRepo.add(makeOrder("O-1", "S-001", 200, OrderStatus::RESERVED));

    NiceMock<MockMonitorView> view;
    std::istringstream in("");
    MonitorController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showStockRow(_, 0, std::string("여유"))).Times(1);
    ctrl.run();
}

TEST_F(MonitorControllerTest, NoSamplesShowsNoSamplesMessage) {
    MonSampleRepo emptySampleRepo;
    StockService  emptyService{emptySampleRepo, orderRepo, clock};

    NiceMock<MockMonitorView> view;
    std::istringstream in("");
    MonitorController ctrl(in, view, emptySampleRepo, orderRepo, emptyService);

    EXPECT_CALL(view, showNoSamples()).Times(1);
    ctrl.run();
}

TEST_F(MonitorControllerTest, ProductionCompletedOnMenuEntry) {
    Order prod;
    prod.orderNo                = "O-PROD";
    prod.sampleId               = "S-001";
    prod.quantity               = 50;
    prod.status                 = OrderStatus::PRODUCING;
    prod.actualProduction       = 60;
    prod.totalProductionTimeMin = 10.0;
    prod.productionStartTime    = 0;
    orderRepo.add(prod);

    clock.setNow(601);  // elapsed >= 10min → 완료

    NiceMock<MockMonitorView> view;
    std::istringstream in("");
    MonitorController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showProductionCompleted(_)).Times(1);
    ctrl.run();
}
