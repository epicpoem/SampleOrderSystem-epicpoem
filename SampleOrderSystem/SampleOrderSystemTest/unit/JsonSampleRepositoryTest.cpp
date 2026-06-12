#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "SampleOrderSystem/repository/JsonSampleRepository.h"

namespace fs = std::filesystem;

class JsonSampleRepositoryTest : public ::testing::Test {
protected:
    std::string testFilePath;

    void SetUp() override {
        testFilePath = "test_samples_" + std::to_string(
            std::hash<std::string>{}(::testing::UnitTest::GetInstance()->current_test_info()->name())
        ) + ".json";
        if (fs::exists(testFilePath)) fs::remove(testFilePath);
    }

    void TearDown() override {
        if (fs::exists(testFilePath)) fs::remove(testFilePath);
    }
};

TEST_F(JsonSampleRepositoryTest, AddNewSampleReturnsTrueAndPersists) {
    JsonSampleRepository repo(testFilePath);
    Sample s{"S-001", "실리콘 웨이퍼", 0.5, 0.92, 0};

    EXPECT_TRUE(repo.add(s));

    auto all = repo.findAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id, "S-001");
    EXPECT_EQ(all[0].name, "실리콘 웨이퍼");
}

TEST_F(JsonSampleRepositoryTest, AddDuplicateIdReturnsFalse) {
    JsonSampleRepository repo(testFilePath);
    Sample s{"S-001", "실리콘 웨이퍼", 0.5, 0.92, 0};

    EXPECT_TRUE(repo.add(s));
    EXPECT_FALSE(repo.add(s));

    EXPECT_EQ(repo.findAll().size(), 1u);
}

TEST_F(JsonSampleRepositoryTest, FindByIdReturnsCorrectSample) {
    JsonSampleRepository repo(testFilePath);
    Sample s{"S-002", "GaN 에피택셜", 0.3, 0.78, 220};
    repo.add(s);

    auto result = repo.findById("S-002");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "GaN 에피택셜");
    EXPECT_DOUBLE_EQ(result->yield, 0.78);
    EXPECT_EQ(result->stock, 220);
}

TEST_F(JsonSampleRepositoryTest, FindByIdReturnsNulloptWhenNotFound) {
    JsonSampleRepository repo(testFilePath);

    auto result = repo.findById("NONEXISTENT");
    EXPECT_FALSE(result.has_value());
}

TEST_F(JsonSampleRepositoryTest, ExistsReturnsTrueForRegistered) {
    JsonSampleRepository repo(testFilePath);
    repo.add({"S-001", "테스트", 0.5, 0.9, 0});

    EXPECT_TRUE(repo.exists("S-001"));
}

TEST_F(JsonSampleRepositoryTest, ExistsReturnsFalseForUnregistered) {
    JsonSampleRepository repo(testFilePath);

    EXPECT_FALSE(repo.exists("S-999"));
}

TEST_F(JsonSampleRepositoryTest, FindAllReturnsAllSamples) {
    JsonSampleRepository repo(testFilePath);
    repo.add({"S-001", "A", 0.5, 0.9, 0});
    repo.add({"S-002", "B", 0.3, 0.8, 0});
    repo.add({"S-003", "C", 0.8, 0.7, 0});

    EXPECT_EQ(repo.findAll().size(), 3u);
}

TEST_F(JsonSampleRepositoryTest, UpdateModifiesPersisted) {
    JsonSampleRepository repo(testFilePath);
    repo.add({"S-001", "원래이름", 0.5, 0.9, 100});

    Sample updated{"S-001", "바뀐이름", 0.6, 0.85, 200};
    EXPECT_TRUE(repo.update(updated));

    auto result = repo.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "바뀐이름");
    EXPECT_EQ(result->stock, 200);
}

TEST_F(JsonSampleRepositoryTest, UpdateNonExistentReturnsFalse) {
    JsonSampleRepository repo(testFilePath);

    Sample s{"S-999", "없는시료", 0.5, 0.9, 0};
    EXPECT_FALSE(repo.update(s));
}

TEST_F(JsonSampleRepositoryTest, DecreaseStockSucceeds) {
    JsonSampleRepository repo(testFilePath);
    repo.add({"S-001", "테스트", 0.5, 0.9, 100});

    EXPECT_TRUE(repo.decreaseStock("S-001", 30));

    auto result = repo.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->stock, 70);
}

TEST_F(JsonSampleRepositoryTest, DecreaseStockFailsWhenInsufficient) {
    JsonSampleRepository repo(testFilePath);
    repo.add({"S-001", "테스트", 0.5, 0.9, 10});

    EXPECT_FALSE(repo.decreaseStock("S-001", 50));

    auto result = repo.findById("S-001");
    EXPECT_EQ(result->stock, 10);
}

TEST_F(JsonSampleRepositoryTest, DecreaseStockFailsWhenNotFound) {
    JsonSampleRepository repo(testFilePath);

    EXPECT_FALSE(repo.decreaseStock("NONEXISTENT", 10));
}

TEST_F(JsonSampleRepositoryTest, DataPersistsAcrossRepositoryInstances) {
    {
        JsonSampleRepository repo(testFilePath);
        repo.add({"S-001", "영속성 테스트", 0.5, 0.9, 50});
    }
    {
        JsonSampleRepository repo(testFilePath);
        auto result = repo.findById("S-001");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->name, "영속성 테스트");
        EXPECT_EQ(result->stock, 50);
    }
}

// ─── Negative / Edge-case TCs ─────────────────────────────────────────────

// 파일 없음 → 빈 목록 반환 (크래시 없음)
TEST_F(JsonSampleRepositoryTest, MissingFileReturnsEmptyList) {
    JsonSampleRepository repo(testFilePath);

    EXPECT_TRUE(repo.findAll().empty());
}

// 손상된 JSON 파일 → 로드 시 빈 목록으로 복구 (예외 전파 없음)
TEST_F(JsonSampleRepositoryTest, CorruptedJsonFileReturnsEmptyAndDoesNotCrash) {
    {
        std::ofstream bad(testFilePath);
        bad << "{ this is NOT valid JSON !!! }}}";
    }

    JsonSampleRepository repo(testFilePath);
    EXPECT_TRUE(repo.findAll().empty());
}

// 재고 정확히 일치하는 수량 차감 → 성공, 재고 0 이 됨 (경계값)
TEST_F(JsonSampleRepositoryTest, DecreaseStockByExactAmountSucceeds) {
    JsonSampleRepository repo(testFilePath);
    repo.add({"S-001", "테스트", 0.5, 0.9, 50});

    EXPECT_TRUE(repo.decreaseStock("S-001", 50));

    auto result = repo.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->stock, 0);
}

// 0 수량 차감 → 성공, 재고 변화 없음 (경계값)
TEST_F(JsonSampleRepositoryTest, DecreaseStockByZeroAmountSucceeds) {
    JsonSampleRepository repo(testFilePath);
    repo.add({"S-001", "테스트", 0.5, 0.9, 30});

    EXPECT_TRUE(repo.decreaseStock("S-001", 0));

    auto result = repo.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->stock, 30);
}
