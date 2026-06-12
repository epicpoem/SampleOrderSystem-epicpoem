#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include "SampleOrderSystem/controller/ReleaseController.h"
#include "SampleOrderSystem/view/IReleaseView.h"
#include "SampleOrderSystem/repository/ISampleRepository.h"
#include "SampleOrderSystem/repository/IOrderRepository.h"
#include "SampleOrderSystem/service/StockService.h"
#include "SampleOrderSystem/util/FakeClock.h"

using ::testing::_;
using ::testing::NiceMock;

namespace {

class RelSampleRepo : public ISampleRepository {
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

class RelOrderRepo : public IOrderRepository {
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

static Order makeConfirmed(const std::string& no, const std::string& sid, int qty) {
    Order o;
    o.orderNo      = no;
    o.sampleId     = sid;
    o.customerName = "CustA";
    o.quantity     = qty;
    o.status       = OrderStatus::CONFIRMED;
    return o;
}

} // namespace

class MockReleaseView : public IReleaseView {
public:
    MOCK_METHOD(void, showMenu, (), (override));
    MOCK_METHOD(void, showNoConfirmedOrders, (), (override));
    MOCK_METHOD(void, showConfirmedOrderList, (const std::vector<Order>&), (override));
    MOCK_METHOD(void, showOrderSelectPrompt, (), (override));
    MOCK_METHOD(void, showOrderNotFound, (), (override));
    MOCK_METHOD(void, showReleaseCompleted, (const Order&, const std::string&), (override));
    MOCK_METHOD(void, showProductionCompleted, (const std::string&), (override));
};

class ReleaseControllerTest : public ::testing::Test {
protected:
    RelSampleRepo sampleRepo;
    RelOrderRepo  orderRepo;
    FakeClock     clock{0, "20260612"};
    StockService  stockService{sampleRepo, orderRepo, clock};

    void SetUp() override {
        sampleRepo.add({"S-001", "Si", 1.0, 0.9, 200});
    }
};

// -- Basic Flow ---------------------------------------------------------------

TEST_F(ReleaseControllerTest, NoConfirmedOrdersShowsEmptyMessage) {
    NiceMock<MockReleaseView> view;
    std::istringstream in("");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showNoConfirmedOrders()).Times(1);
    ctrl.run();
}

TEST_F(ReleaseControllerTest, ReleaseConfirmedOrderChangesStatusToRelease) {
    orderRepo.add(makeConfirmed("ORD-20260612-0001", "S-001", 50));
    NiceMock<MockReleaseView> view;
    std::istringstream in("1\n");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showReleaseCompleted(_, _)).Times(1);
    ctrl.run();

    auto updated = orderRepo.findByNo("ORD-20260612-0001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::RELEASE);
}

TEST_F(ReleaseControllerTest, StockDecreasedByOrderQuantityOnRelease) {
    // stock=200, release qty=70 -> stock=130
    orderRepo.add(makeConfirmed("ORD-20260612-0001", "S-001", 70));
    NiceMock<MockReleaseView> view;
    std::istringstream in("1\n");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);
    ctrl.run();

    auto s = sampleRepo.findById("S-001");
    ASSERT_TRUE(s.has_value());
    EXPECT_EQ(s->stock, 130);
}

TEST_F(ReleaseControllerTest, ZeroInputCancelsWithoutRelease) {
    orderRepo.add(makeConfirmed("ORD-20260612-0001", "S-001", 50));
    NiceMock<MockReleaseView> view;
    std::istringstream in("0\n");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showReleaseCompleted(_, _)).Times(0);
    ctrl.run();

    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0001")->status, OrderStatus::CONFIRMED);
}

// -- Negative / Edge-case TCs -------------------------------------------------

TEST_F(ReleaseControllerTest, InvalidIndexShowsOrderNotFound) {
    orderRepo.add(makeConfirmed("ORD-20260612-0001", "S-001", 50));
    NiceMock<MockReleaseView> view;
    std::istringstream in("99\n");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showOrderNotFound()).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0001")->status, OrderStatus::CONFIRMED);
}

TEST_F(ReleaseControllerTest, NegativeIndexShowsOrderNotFound) {
    orderRepo.add(makeConfirmed("ORD-20260612-0001", "S-001", 50));
    NiceMock<MockReleaseView> view;
    std::istringstream in("-1\n");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showOrderNotFound()).Times(1);
    ctrl.run();
}

TEST_F(ReleaseControllerTest, NonNumericInputShowsOrderNotFound) {
    orderRepo.add(makeConfirmed("ORD-20260612-0001", "S-001", 50));
    NiceMock<MockReleaseView> view;
    std::istringstream in("abc\n");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showOrderNotFound()).Times(1);
    ctrl.run();
}

TEST_F(ReleaseControllerTest, MultipleConfirmedOrdersPickSecond) {
    orderRepo.add(makeConfirmed("ORD-20260612-0001", "S-001", 30));
    orderRepo.add(makeConfirmed("ORD-20260612-0002", "S-001", 50));
    NiceMock<MockReleaseView> view;
    std::istringstream in("2\n");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showReleaseCompleted(_, _)).Times(1);
    ctrl.run();

    // only second order released
    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0001")->status, OrderStatus::CONFIRMED);
    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0002")->status, OrderStatus::RELEASE);
    // stock=200 - 50 = 150
    EXPECT_EQ(sampleRepo.findById("S-001")->stock, 150);
}

TEST_F(ReleaseControllerTest, ReservedOrderNotShownInReleaseList) {
    // RESERVED 주문만 있을 때 출고 목록이 비어야 함
    Order o;
    o.orderNo      = "ORD-20260612-0001";
    o.sampleId     = "S-001";
    o.customerName = "CustA";
    o.quantity     = 50;
    o.status       = OrderStatus::RESERVED;
    orderRepo.add(o);

    NiceMock<MockReleaseView> view;
    std::istringstream in("");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showNoConfirmedOrders()).Times(1);
    EXPECT_CALL(view, showReleaseCompleted(_, _)).Times(0);
    ctrl.run();

    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0001")->status, OrderStatus::RESERVED);
}

TEST_F(ReleaseControllerTest, ProducingOrderNotShownInReleaseList) {
    // PRODUCING 주문만 있을 때 출고 목록이 비어야 함
    Order o;
    o.orderNo                = "ORD-20260612-0001";
    o.sampleId               = "S-001";
    o.customerName           = "CustA";
    o.quantity               = 50;
    o.status                 = OrderStatus::PRODUCING;
    o.actualProduction       = 60;
    o.totalProductionTimeMin = 999.0;  // 아직 완료되지 않음
    o.productionStartTime    = 0;
    orderRepo.add(o);

    NiceMock<MockReleaseView> view;
    std::istringstream in("");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showNoConfirmedOrders()).Times(1);
    EXPECT_CALL(view, showReleaseCompleted(_, _)).Times(0);
    ctrl.run();

    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0001")->status, OrderStatus::PRODUCING);
}

TEST_F(ReleaseControllerTest, ReleasedOrderNotShownInReleaseList) {
    // 이미 RELEASE된 주문만 있을 때 출고 목록이 비어야 함
    Order o;
    o.orderNo      = "ORD-20260612-0001";
    o.sampleId     = "S-001";
    o.customerName = "CustA";
    o.quantity     = 50;
    o.status       = OrderStatus::RELEASE;
    orderRepo.add(o);

    NiceMock<MockReleaseView> view;
    std::istringstream in("");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showNoConfirmedOrders()).Times(1);
    ctrl.run();
}

TEST_F(ReleaseControllerTest, StockExactlyZeroAfterRelease) {
    // stock=50, release qty=50 → stock이 정확히 0이 되어야 함
    sampleRepo.add({"S-002", "GaN", 1.0, 0.9, 50});
    orderRepo.add(makeConfirmed("ORD-20260612-0001", "S-002", 50));

    NiceMock<MockReleaseView> view;
    std::istringstream in("1\n");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);
    ctrl.run();

    auto s = sampleRepo.findById("S-002");
    ASSERT_TRUE(s.has_value());
    EXPECT_EQ(s->stock, 0);
    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0001")->status, OrderStatus::RELEASE);
}

TEST_F(ReleaseControllerTest, ProductionCompletedAutoConvertThenRelease) {
    // PRODUCING order completes on menu entry -> becomes CONFIRMED -> released
    Order prod;
    prod.orderNo                = "ORD-20260612-0001";
    prod.sampleId               = "S-001";
    prod.customerName           = "CustA";
    prod.quantity               = 50;
    prod.status                 = OrderStatus::PRODUCING;
    prod.actualProduction       = 60;
    prod.totalProductionTimeMin = 10.0;
    prod.productionStartTime    = 0;
    orderRepo.add(prod);

    clock.setNow(601);  // elapsed >= 10min -> auto-confirmed

    NiceMock<MockReleaseView> view;
    std::istringstream in("1\n");
    ReleaseController ctrl(in, view, sampleRepo, orderRepo, stockService);

    EXPECT_CALL(view, showProductionCompleted(_)).Times(1);
    EXPECT_CALL(view, showReleaseCompleted(_, _)).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findByNo("ORD-20260612-0001")->status, OrderStatus::RELEASE);
}
