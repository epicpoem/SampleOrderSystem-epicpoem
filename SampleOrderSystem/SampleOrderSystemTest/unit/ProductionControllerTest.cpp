#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include "SampleOrderSystem/controller/ProductionController.h"
#include "SampleOrderSystem/view/IProductionView.h"
#include "SampleOrderSystem/repository/ISampleRepository.h"
#include "SampleOrderSystem/repository/IOrderRepository.h"
#include "SampleOrderSystem/service/StockService.h"
#include "SampleOrderSystem/util/FakeClock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DoubleNear;

namespace {

class ProdSampleRepo : public ISampleRepository {
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

class ProdOrderRepo : public IOrderRepository {
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

static Order makeProducing(const std::string& no, const std::string& sid,
                            int qty, int actualProd, double totalMin,
                            std::time_t startTime) {
    Order o;
    o.orderNo                = no;
    o.sampleId               = sid;
    o.quantity               = qty;
    o.status                 = OrderStatus::PRODUCING;
    o.actualProduction       = actualProd;
    o.totalProductionTimeMin = totalMin;
    o.productionStartTime    = startTime;
    return o;
}

} // namespace

class MockProductionView : public IProductionView {
public:
    MOCK_METHOD(void, showMenu, (), (override));
    MOCK_METHOD(void, showNoProducing, (), (override));
    MOCK_METHOD(void, showCurrentProduction,
                (const Order&, const std::string&, double, double), (override));
    MOCK_METHOD(void, showQueueHeader, (int), (override));
    MOCK_METHOD(void, showQueueItem,
                (int, const Order&, const std::string&, double), (override));
    MOCK_METHOD(void, showQueueEmpty, (), (override));
    MOCK_METHOD(void, showProductionCompleted, (const std::string&), (override));
};

class ProductionControllerTest : public ::testing::Test {
protected:
    ProdSampleRepo sampleRepo;
    ProdOrderRepo  orderRepo;
    FakeClock      clock{0, "20260612"};
    StockService   stockService{sampleRepo, orderRepo, clock};

    void SetUp() override {
        sampleRepo.add({"S-001", "실리콘 웨이퍼", 1.0, 0.9, 0});
        sampleRepo.add({"S-002", "GaN 에피택셀",  0.3, 0.8, 0});
    }
};

// ── 기본 흐름 ─────────────────────────────────────────────────────────────

TEST_F(ProductionControllerTest, NoProducingOrdersShowsMessage) {
    NiceMock<MockProductionView> view;
    std::istringstream in("");
    ProductionController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showNoProducing()).Times(1);
    EXPECT_CALL(view, showCurrentProduction(_, _, _, _)).Times(0);
    ctrl.run();
}

TEST_F(ProductionControllerTest, OneProducingOrderShowsCurrentProduction) {
    orderRepo.add(makeProducing("O-1", "S-001", 100, 112, 100.0, 0));
    // elapsed=3000초=50분 → progress=50%, remaining=50min
    clock.setNow(3000);

    NiceMock<MockProductionView> view;
    std::istringstream in("");
    ProductionController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showCurrentProduction(_, _, DoubleNear(50.0, 0.01),
                                             DoubleNear(50.0, 0.01))).Times(1);
    EXPECT_CALL(view, showQueueEmpty()).Times(1);
    ctrl.run();
}

TEST_F(ProductionControllerTest, MultipleOrdersFirstIsCurrentRestIsQueue) {
    // startTime이 작은 순서가 FIFO 앞
    orderRepo.add(makeProducing("O-1", "S-001", 100, 112, 100.0, 0));
    orderRepo.add(makeProducing("O-2", "S-002",  50,  70,  60.0, 10));
    orderRepo.add(makeProducing("O-3", "S-001",  30,  40,  40.0, 20));
    clock.setNow(0);

    NiceMock<MockProductionView> view;
    std::istringstream in("");
    ProductionController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showCurrentProduction(_, _, _, _)).Times(1);
    EXPECT_CALL(view, showQueueHeader(2)).Times(1);
    EXPECT_CALL(view, showQueueItem(_, _, _, _)).Times(2);
    ctrl.run();
}

// ── 진행률 계산 ──────────────────────────────────────────────────────────

TEST_F(ProductionControllerTest, ProgressIsZeroAtStart) {
    orderRepo.add(makeProducing("O-1", "S-001", 100, 100, 100.0, 0));
    clock.setNow(0);  // elapsed=0

    NiceMock<MockProductionView> view;
    std::istringstream in("");
    ProductionController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showCurrentProduction(_, _, DoubleNear(0.0, 0.01), _)).Times(1);
    ctrl.run();
}

TEST_F(ProductionControllerTest, ElapsedExceedsTotalCompletesProduction) {
    // elapsed >= totalProdTime 이면 checkAndCompleteProduction 이 CONFIRMED 로 전환
    // → showProductionCompleted 호출 후 showNoProducing 표시
    orderRepo.add(makeProducing("O-1", "S-001", 100, 100, 10.0, 0));
    clock.setNow(99999);  // elapsed >> totalProdTime

    NiceMock<MockProductionView> view;
    std::istringstream in("");
    ProductionController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showProductionCompleted(_)).Times(1);
    EXPECT_CALL(view, showNoProducing()).Times(1);
    ctrl.run();
}

TEST_F(ProductionControllerTest, FifoOrderingByStartTime) {
    // O-2가 먼저 시작했으므로 현재 생산 중이어야 함
    orderRepo.add(makeProducing("O-1", "S-001", 100, 100, 100.0, 1000));
    orderRepo.add(makeProducing("O-2", "S-002",  50,  70,  60.0,    0));
    clock.setNow(0);

    NiceMock<MockProductionView> view;
    std::istringstream in("");
    ProductionController ctrl(in, view, sampleRepo, orderRepo, stockService);

    // showCurrentProduction은 O-2(startTime=0)에 대해 호출되어야 함
    EXPECT_CALL(view, showCurrentProduction(_, std::string("GaN 에피택셀"), _, _)).Times(1);
    ctrl.run();
}

TEST_F(ProductionControllerTest, ProductionCompletedOnMenuEntry) {
    Order prod = makeProducing("O-PROD", "S-001", 50, 60, 10.0, 0);
    orderRepo.add(prod);
    clock.setNow(601);  // elapsed >= 10min → 완료

    NiceMock<MockProductionView> view;
    std::istringstream in("");
    ProductionController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showProductionCompleted(_)).Times(1);
    EXPECT_CALL(view, showNoProducing()).Times(1);
    ctrl.run();
}

TEST_F(ProductionControllerTest, QueueEmpty_WhenOnlyOneProducing) {
    orderRepo.add(makeProducing("O-1", "S-001", 100, 100, 100.0, 0));
    clock.setNow(0);

    NiceMock<MockProductionView> view;
    std::istringstream in("");
    ProductionController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showQueueHeader(0)).Times(1);
    EXPECT_CALL(view, showQueueEmpty()).Times(1);
    ctrl.run();
}
