#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include <vector>
#include "SampleOrderSystem/controller/OrderController.h"
#include "SampleOrderSystem/view/IOrderView.h"
#include "SampleOrderSystem/repository/ISampleRepository.h"
#include "SampleOrderSystem/repository/IOrderRepository.h"
#include "SampleOrderSystem/util/FakeClock.h"

using ::testing::_;
using ::testing::NiceMock;

// ── 테스트용 인메모리 Repository ──────────────────────────────────────────

class InMemorySampleRepo2 : public ISampleRepository {
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

class InMemoryOrderRepo : public IOrderRepository {
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
        std::string prefix = "ORD-" + date + "-";
        int maxSeq = 0;
        for (const auto& o : orders_) {
            if (o.orderNo.rfind(prefix, 0) == 0) {
                try {
                    int seq = std::stoi(o.orderNo.substr(prefix.size()));
                    if (seq > maxSeq) maxSeq = seq;
                } catch (...) {}
            }
        }
        char buf[5];
        std::snprintf(buf, sizeof(buf), "%04d", maxSeq + 1);
        return prefix + buf;
    }
};

// ── MockOrderView ─────────────────────────────────────────────────────────

class MockOrderView : public IOrderView {
public:
    MOCK_METHOD(void, showOrderForm, (), (override));
    MOCK_METHOD(void, showSampleIdPrompt, (), (override));
    MOCK_METHOD(void, showCustomerNamePrompt, (), (override));
    MOCK_METHOD(void, showQuantityPrompt, (), (override));
    MOCK_METHOD(void, showOrderConfirmation,
                (const std::string&, const std::string&, int), (override));
    MOCK_METHOD(void, showConfirmPrompt, (), (override));
    MOCK_METHOD(void, showOrderSuccess, (const Order&), (override));
    MOCK_METHOD(void, showOrderCancelled, (), (override));
    MOCK_METHOD(void, showInvalidSampleId, (), (override));
    MOCK_METHOD(void, showInvalidQuantity, (), (override));
};

// ── Fixture ───────────────────────────────────────────────────────────────

class OrderControllerTest : public ::testing::Test {
protected:
    InMemorySampleRepo2 sampleRepo;
    InMemoryOrderRepo   orderRepo;
    FakeClock           clock{0, "20260612"};

    void SetUp() override {
        sampleRepo.add({"S-001", "실리콘 웨이퍼", 0.5, 0.92, 0});
    }
};

// ── 정상 케이스 ───────────────────────────────────────────────────────────

TEST_F(OrderControllerTest, PlaceOrderSuccessWithYesConfirmation) {
    NiceMock<MockOrderView> view;
    std::istringstream in("S-001\n삼성전자\n100\nY\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);

    EXPECT_CALL(view, showOrderSuccess(_)).Times(1);
    ctrl.run();

    auto orders = orderRepo.findAll();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].sampleId, "S-001");
    EXPECT_EQ(orders[0].customerName, "삼성전자");
    EXPECT_EQ(orders[0].quantity, 100);
    EXPECT_EQ(orders[0].status, OrderStatus::RESERVED);
}

TEST_F(OrderControllerTest, OrderNoUsesClockDate) {
    NiceMock<MockOrderView> view;
    std::istringstream in("S-001\n고객A\n50\nY\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);
    ctrl.run();

    auto orders = orderRepo.findAll();
    ASSERT_FALSE(orders.empty());
    EXPECT_EQ(orders[0].orderNo, "ORD-20260612-0001");
}

TEST_F(OrderControllerTest, SecondOrderOnSameDayHasIncrementedSeq) {
    NiceMock<MockOrderView> view;
    std::istringstream in1("S-001\n고객A\n10\nY\n");
    OrderController ctrl1(in1, view, sampleRepo, orderRepo, clock);
    ctrl1.run();

    std::istringstream in2("S-001\n고객B\n20\nY\n");
    OrderController ctrl2(in2, view, sampleRepo, orderRepo, clock);
    ctrl2.run();

    auto orders = orderRepo.findAll();
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_EQ(orders[0].orderNo, "ORD-20260612-0001");
    EXPECT_EQ(orders[1].orderNo, "ORD-20260612-0002");
}

// ── 취소 케이스 ───────────────────────────────────────────────────────────

TEST_F(OrderControllerTest, PlaceOrderCancelledWithUpperCaseN) {
    NiceMock<MockOrderView> view;
    std::istringstream in("S-001\n고객A\n50\nN\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);

    EXPECT_CALL(view, showOrderCancelled()).Times(1);
    ctrl.run();

    EXPECT_TRUE(orderRepo.findAll().empty());
}

TEST_F(OrderControllerTest, PlaceOrderCancelledWithLowerCaseN) {
    NiceMock<MockOrderView> view;
    std::istringstream in("S-001\n고객A\n50\nn\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);

    EXPECT_CALL(view, showOrderCancelled()).Times(1);
    ctrl.run();

    EXPECT_TRUE(orderRepo.findAll().empty());
}

// ── Negative / Edge-case TCs ──────────────────────────────────────────────

TEST_F(OrderControllerTest, InvalidSampleIdShowsErrorAndRetries) {
    NiceMock<MockOrderView> view;
    // 없는 ID → 오류 → 유효한 ID 재입력
    std::istringstream in("X-999\nS-001\n고객A\n50\nY\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);

    EXPECT_CALL(view, showInvalidSampleId()).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findAll().size(), 1u);
}

TEST_F(OrderControllerTest, ZeroQuantityShowsErrorAndRetries) {
    NiceMock<MockOrderView> view;
    std::istringstream in("S-001\n고객A\n0\n50\nY\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);

    EXPECT_CALL(view, showInvalidQuantity()).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findAll()[0].quantity, 50);
}

TEST_F(OrderControllerTest, NegativeQuantityShowsErrorAndRetries) {
    NiceMock<MockOrderView> view;
    std::istringstream in("S-001\n고객A\n-10\n30\nY\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);

    EXPECT_CALL(view, showInvalidQuantity()).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findAll()[0].quantity, 30);
}

TEST_F(OrderControllerTest, NonNumericQuantityShowsErrorAndRetries) {
    NiceMock<MockOrderView> view;
    std::istringstream in("S-001\n고객A\nabc\n20\nY\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);

    EXPECT_CALL(view, showInvalidQuantity()).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findAll()[0].quantity, 20);
}

TEST_F(OrderControllerTest, EmptySampleIdRepromptsUntilValid) {
    NiceMock<MockOrderView> view;
    std::istringstream in("\n\nS-001\n고객A\n10\nY\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);

    EXPECT_CALL(view, showOrderSuccess(_)).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findAll().size(), 1u);
}

TEST_F(OrderControllerTest, LowerCaseYAlsoConfirmsOrder) {
    NiceMock<MockOrderView> view;
    std::istringstream in("S-001\n고객A\n50\ny\n");
    OrderController ctrl(in, view, sampleRepo, orderRepo, clock);

    EXPECT_CALL(view, showOrderSuccess(_)).Times(1);
    ctrl.run();

    EXPECT_EQ(orderRepo.findAll().size(), 1u);
}
