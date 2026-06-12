#pragma once
#include "../repository/ISampleRepository.h"
#include "../repository/IOrderRepository.h"
#include "../util/IClock.h"
#include <vector>
#include <string>
#include <optional>
#include <ctime>

class StockService {
public:
    StockService(ISampleRepository& sampleRepo, IOrderRepository& orderRepo, IClock& clock);

    // 생산 완료 체크: 완료된 주문번호 목록 반환 (주문 상태 + 시료 재고 업데이트)
    std::vector<std::string> checkAndCompleteProduction();

    // 시료의 물리적 재고 (stock + PRODUCING 진행분 비례 합산)
    double calcPhysicalStock(const Sample& sample) const;

    std::optional<Sample> findSampleById(const std::string& id) const;

    std::time_t now() const;

private:
    ISampleRepository& sampleRepo_;
    IOrderRepository& orderRepo_;
    IClock& clock_;
};
