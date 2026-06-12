#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "SampleOrderSystem/repository/JsonOrderRepository.h"

namespace fs = std::filesystem;

class JsonOrderRepositoryTest : public ::testing::Test {
protected:
    std::string testFilePath;

    void SetUp() override {
        testFilePath = "test_orders_" + std::to_string(
            std::hash<std::string>{}(::testing::UnitTest::GetInstance()->current_test_info()->name())
        ) + ".json";
        if (fs::exists(testFilePath)) fs::remove(testFilePath);
    }

    void TearDown() override {
        if (fs::exists(testFilePath)) fs::remove(testFilePath);
    }

    Order makeOrder(const std::string& no, const std::string& sampleId,
                    const std::string& customer, int qty,
                    OrderStatus status = OrderStatus::RESERVED) {
        Order o;
        o.orderNo      = no;
        o.sampleId     = sampleId;
        o.customerName = customer;
        o.quantity     = qty;
        o.status       = status;
        return o;
    }
};

// ── 기본 CRUD ─────────────────────────────────────────────────────────────

TEST_F(JsonOrderRepositoryTest, AddOrderAndFindAll) {
    JsonOrderRepository repo(testFilePath);
    repo.add(makeOrder("ORD-20260612-0001", "S-001", "삼성전자", 100));

    auto all = repo.findAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].orderNo, "ORD-20260612-0001");
    EXPECT_EQ(all[0].sampleId, "S-001");
    EXPECT_EQ(all[0].quantity, 100);
    EXPECT_EQ(all[0].status, OrderStatus::RESERVED);
}

TEST_F(JsonOrderRepositoryTest, FindByNoReturnsCorrectOrder) {
    JsonOrderRepository repo(testFilePath);
    repo.add(makeOrder("ORD-20260612-0001", "S-001", "고객A", 50));
    repo.add(makeOrder("ORD-20260612-0002", "S-002", "고객B", 80));

    auto result = repo.findByNo("ORD-20260612-0002");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->customerName, "고객B");
    EXPECT_EQ(result->quantity, 80);
}

TEST_F(JsonOrderRepositoryTest, FindByNoReturnsNulloptWhenNotFound) {
    JsonOrderRepository repo(testFilePath);

    auto result = repo.findByNo("ORD-99999999-0001");
    EXPECT_FALSE(result.has_value());
}

TEST_F(JsonOrderRepositoryTest, FindByStatusFiltersCorrectly) {
    JsonOrderRepository repo(testFilePath);
    repo.add(makeOrder("ORD-20260612-0001", "S-001", "고객A", 100, OrderStatus::RESERVED));
    repo.add(makeOrder("ORD-20260612-0002", "S-001", "고객B", 200, OrderStatus::CONFIRMED));
    repo.add(makeOrder("ORD-20260612-0003", "S-001", "고객C", 300, OrderStatus::RESERVED));

    auto reserved = repo.findByStatus(OrderStatus::RESERVED);
    EXPECT_EQ(reserved.size(), 2u);

    auto confirmed = repo.findByStatus(OrderStatus::CONFIRMED);
    EXPECT_EQ(confirmed.size(), 1u);
    EXPECT_EQ(confirmed[0].orderNo, "ORD-20260612-0002");
}

TEST_F(JsonOrderRepositoryTest, UpdateOrderModifiesStatus) {
    JsonOrderRepository repo(testFilePath);
    repo.add(makeOrder("ORD-20260612-0001", "S-001", "고객A", 100, OrderStatus::RESERVED));

    Order updated = makeOrder("ORD-20260612-0001", "S-001", "고객A", 100, OrderStatus::CONFIRMED);
    EXPECT_TRUE(repo.update(updated));

    auto result = repo.findByNo("ORD-20260612-0001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, OrderStatus::CONFIRMED);
}

TEST_F(JsonOrderRepositoryTest, UpdateNonExistentOrderReturnsFalse) {
    JsonOrderRepository repo(testFilePath);

    Order o = makeOrder("ORD-99999999-0001", "S-001", "고객A", 100);
    EXPECT_FALSE(repo.update(o));
}

TEST_F(JsonOrderRepositoryTest, DataPersistsAcrossRepositoryInstances) {
    {
        JsonOrderRepository repo(testFilePath);
        repo.add(makeOrder("ORD-20260612-0001", "S-001", "영속성고객", 150, OrderStatus::CONFIRMED));
    }
    {
        JsonOrderRepository repo(testFilePath);
        auto result = repo.findByNo("ORD-20260612-0001");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->customerName, "영속성고객");
        EXPECT_EQ(result->status, OrderStatus::CONFIRMED);
    }
}

// ── 주문번호 채번 ─────────────────────────────────────────────────────────

TEST_F(JsonOrderRepositoryTest, GenerateOrderNoStartsAt0001ForNewDate) {
    JsonOrderRepository repo(testFilePath);

    EXPECT_EQ(repo.generateOrderNo("20260612"), "ORD-20260612-0001");
}

TEST_F(JsonOrderRepositoryTest, GenerateOrderNoIncrementsOnSameDay) {
    JsonOrderRepository repo(testFilePath);
    repo.add(makeOrder("ORD-20260612-0001", "S-001", "고객A", 10));
    repo.add(makeOrder("ORD-20260612-0002", "S-001", "고객B", 20));

    EXPECT_EQ(repo.generateOrderNo("20260612"), "ORD-20260612-0003");
}

TEST_F(JsonOrderRepositoryTest, GenerateOrderNoDifferentDatesAreIndependent) {
    JsonOrderRepository repo(testFilePath);
    repo.add(makeOrder("ORD-20260611-0001", "S-001", "고객A", 10));
    repo.add(makeOrder("ORD-20260611-0002", "S-001", "고객B", 20));

    // 다른 날짜는 0001부터 시작
    EXPECT_EQ(repo.generateOrderNo("20260612"), "ORD-20260612-0001");
}

// ── Negative / Edge-case TCs ──────────────────────────────────────────────

TEST_F(JsonOrderRepositoryTest, MissingFileReturnsEmptyList) {
    JsonOrderRepository repo(testFilePath);

    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(JsonOrderRepositoryTest, CorruptedJsonFileReturnsEmptyAndDoesNotCrash) {
    {
        std::ofstream bad(testFilePath);
        bad << "{ NOT VALID JSON !!!";
    }

    JsonOrderRepository repo(testFilePath);
    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(JsonOrderRepositoryTest, FindByStatusOnEmptyRepoReturnsEmpty) {
    JsonOrderRepository repo(testFilePath);

    EXPECT_TRUE(repo.findByStatus(OrderStatus::RESERVED).empty());
    EXPECT_TRUE(repo.findByStatus(OrderStatus::CONFIRMED).empty());
}

TEST_F(JsonOrderRepositoryTest, AddMultipleOrdersWithDifferentStatuses) {
    JsonOrderRepository repo(testFilePath);
    repo.add(makeOrder("ORD-20260612-0001", "S-001", "A", 10, OrderStatus::RESERVED));
    repo.add(makeOrder("ORD-20260612-0002", "S-001", "B", 20, OrderStatus::PRODUCING));
    repo.add(makeOrder("ORD-20260612-0003", "S-001", "C", 30, OrderStatus::CONFIRMED));
    repo.add(makeOrder("ORD-20260612-0004", "S-001", "D", 40, OrderStatus::RELEASE));
    repo.add(makeOrder("ORD-20260612-0005", "S-001", "E", 50, OrderStatus::REJECTED));

    EXPECT_EQ(repo.findAll().size(), 5u);
    EXPECT_EQ(repo.findByStatus(OrderStatus::PRODUCING).size(), 1u);
    EXPECT_EQ(repo.findByStatus(OrderStatus::RELEASE).size(), 1u);
    EXPECT_EQ(repo.findByStatus(OrderStatus::REJECTED).size(), 1u);
}
