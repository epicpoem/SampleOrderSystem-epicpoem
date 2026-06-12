#include "StockService.h"
#include <cmath>
#include <algorithm>

StockService::StockService(ISampleRepository& sampleRepo,
                           IOrderRepository& orderRepo,
                           IClock& clock)
    : sampleRepo_(sampleRepo), orderRepo_(orderRepo), clock_(clock) {}

std::vector<std::string> StockService::checkAndCompleteProduction() {
    std::vector<std::string> completed;
    std::time_t now = clock_.now();
    for (const auto& o : orderRepo_.findAll()) {
        if (o.status != OrderStatus::PRODUCING) continue;
        double elapsedMin = std::difftime(now, o.productionStartTime) / 60.0;
        if (o.totalProductionTimeMin <= 0.0) continue;
        if (elapsedMin >= o.totalProductionTimeMin) {
            Order done = o;
            done.status = OrderStatus::CONFIRMED;
            orderRepo_.update(done);

            auto sOpt = sampleRepo_.findById(o.sampleId);
            if (sOpt.has_value()) {
                Sample updated = *sOpt;
                updated.stock += o.actualProduction;
                sampleRepo_.update(updated);
            }

            completed.push_back(o.orderNo);
        }
    }
    return completed;
}

double StockService::calcPhysicalStock(const Sample& sample) const {
    double physStock = sample.stock;
    std::time_t now = clock_.now();
    for (const auto& o : orderRepo_.findAll()) {
        if (o.status != OrderStatus::PRODUCING) continue;
        if (o.sampleId != sample.id) continue;
        if (o.totalProductionTimeMin <= 0.0) continue;
        double elapsedMin = std::difftime(now, o.productionStartTime) / 60.0;
        double ratio = std::min(1.0, elapsedMin / o.totalProductionTimeMin);
        physStock += o.actualProduction * ratio;
    }
    return physStock;
}

std::optional<Sample> StockService::findSampleById(const std::string& id) const {
    return sampleRepo_.findById(id);
}

std::time_t StockService::now() const {
    return clock_.now();
}

std::string StockService::today() const {
    return clock_.today();
}
