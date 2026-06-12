#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include <vector>
#include "SampleOrderSystem/controller/SampleController.h"
#include "SampleOrderSystem/view/ISampleView.h"
#include "SampleOrderSystem/repository/ISampleRepository.h"

using ::testing::_;
using ::testing::NiceMock;

// ISampleRepository 인메모리 구현 (테스트용)
class InMemorySampleRepository : public ISampleRepository {
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
    bool update(const Sample& sample) override {
        for (auto& s : samples_)
            if (s.id == sample.id) { s = sample; return true; }
        return false;
    }
    bool decreaseStock(const std::string& id, int amount) override {
        for (auto& s : samples_)
            if (s.id == id && s.stock >= amount) { s.stock -= amount; return true; }
        return false;
    }
};

// ISampleView Mock
class MockSampleView : public ISampleView {
public:
    MOCK_METHOD(void, showMenu, (), (override));
    MOCK_METHOD(void, showInvalidInput, (), (override));
    MOCK_METHOD(void, showRegisterPrompt, (), (override));
    MOCK_METHOD(void, showNamePrompt, (), (override));
    MOCK_METHOD(void, showTimePrompt, (), (override));
    MOCK_METHOD(void, showYieldPrompt, (), (override));
    MOCK_METHOD(void, showYieldOutOfRange, (), (override));
    MOCK_METHOD(void, showTimeOutOfRange, (), (override));
    MOCK_METHOD(void, showDuplicateId, (), (override));
    MOCK_METHOD(void, showRegisterSuccess, (const Sample&), (override));
    MOCK_METHOD(void, showSampleList, (const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showSearchMenu, (), (override));
    MOCK_METHOD(void, showSearchPrompt, (), (override));
    MOCK_METHOD(void, showSearchResult, (const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showNoResults, (), (override));
};

class SampleControllerTest : public ::testing::Test {
protected:
    InMemorySampleRepository repo;
};

TEST_F(SampleControllerTest, InputZeroReturnsToMain) {
    NiceMock<MockSampleView> view;
    std::istringstream in("0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showMenu()).Times(1);
    ctrl.run();
}

TEST_F(SampleControllerTest, InvalidInputShowsError) {
    NiceMock<MockSampleView> view;
    std::istringstream in("9\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showInvalidInput()).Times(1);
    ctrl.run();
}

TEST_F(SampleControllerTest, RegisterSampleSuccessWithValidInput) {
    NiceMock<MockSampleView> view;
    std::istringstream in("1\nS-001\nTest Sample\n0.5\n0.92\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showRegisterSuccess(_)).Times(1);
    ctrl.run();
    EXPECT_TRUE(repo.exists("S-001"));
}

TEST_F(SampleControllerTest, RegisterDuplicateIdShowsError) {
    repo.add({"S-001", "기존 시료", 0.5, 0.9, 0});
    NiceMock<MockSampleView> view;
    // 중복 ID 입력 후 새 ID로 재입력, 나머지 입력 완성 후 0으로 종료
    std::istringstream in("1\nS-001\nS-002\nNew Sample\n0.5\n0.9\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showDuplicateId()).Times(1);
    ctrl.run();
}

TEST_F(SampleControllerTest, YieldBelowZeroShowsOutOfRangeError) {
    NiceMock<MockSampleView> view;
    std::istringstream in("1\nS-001\nTest Sample\n0.5\n-0.1\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showYieldOutOfRange()).Times(1);
    ctrl.run();
    EXPECT_FALSE(repo.exists("S-001"));
}

TEST_F(SampleControllerTest, YieldAboveOneShowsOutOfRangeError) {
    NiceMock<MockSampleView> view;
    std::istringstream in("1\nS-001\nTest Sample\n0.5\n1.5\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showYieldOutOfRange()).Times(1);
    ctrl.run();
    EXPECT_FALSE(repo.exists("S-001"));
}

TEST_F(SampleControllerTest, ZeroAvgTimeShowsOutOfRangeError) {
    NiceMock<MockSampleView> view;
    // avgTime=0.0 -> 오류 -> 0.5로 재입력
    std::istringstream in("1\nS-001\nTest\n0.0\n0.5\n0.9\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showTimeOutOfRange()).Times(1);
    ctrl.run();
    EXPECT_TRUE(repo.exists("S-001"));
}

TEST_F(SampleControllerTest, ListShowsSampleList) {
    NiceMock<MockSampleView> view;
    std::istringstream in("2\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showSampleList(_)).Times(1);
    ctrl.run();
}

TEST_F(SampleControllerTest, SearchShowsSearchMenu) {
    NiceMock<MockSampleView> view;
    std::istringstream in("3\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showSearchMenu()).Times(1);
    ctrl.run();
}

TEST_F(SampleControllerTest, SearchByIdFindsMatch) {
    repo.add({"S-001", "실리콘 웨이퍼", 0.5, 0.92, 100});
    NiceMock<MockSampleView> view;
    std::istringstream in("3\n1\nS-001\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showSearchResult(_)).Times(1);
    ctrl.run();
}

TEST_F(SampleControllerTest, SearchByNameFindsMatch) {
    repo.add({"S-001", "실리콘 웨이퍼", 0.5, 0.92, 100});
    NiceMock<MockSampleView> view;
    std::istringstream in("3\n2\n실리콘\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showSearchResult(_)).Times(1);
    ctrl.run();
}

TEST_F(SampleControllerTest, SearchByYieldFindsMatch) {
    repo.add({"S-001", "실리콘 웨이퍼", 0.5, 0.92, 100});
    NiceMock<MockSampleView> view;
    std::istringstream in("3\n3\n0.9\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showSearchResult(_)).Times(1);
    ctrl.run();
}

TEST_F(SampleControllerTest, SearchNoResultsShowsNoResults) {
    NiceMock<MockSampleView> view;
    std::istringstream in("3\n1\nX-999\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showNoResults()).Times(1);
    ctrl.run();
}

TEST_F(SampleControllerTest, SearchWithInvalidCriteriaShowsError) {
    NiceMock<MockSampleView> view;
    std::istringstream in("3\n9\n0\n");
    SampleController ctrl(in, view, repo);

    EXPECT_CALL(view, showInvalidInput()).Times(1);
    ctrl.run();
}
