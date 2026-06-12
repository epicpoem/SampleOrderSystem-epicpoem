#include "ProductionController.h"
#include <algorithm>
#include <cmath>

ProductionController::ProductionController(std::istream& in, IProductionView& view,
                                           ISampleRepository& sampleRepo,
                                           IOrderRepository& orderRepo,
                                           StockService& stockService)
    : in_(in), view_(view), sampleRepo_(sampleRepo),
      orderRepo_(orderRepo), stockService_(stockService) {}

void ProductionController::run() {
    for (const auto& no : stockService_.checkAndCompleteProduction())
        view_.showProductionCompleted(no);

    view_.showMenu();

    // PRODUCING 주문을 productionStartTime 기준 FIFO 정렬
    auto producing = orderRepo_.findByStatus(OrderStatus::PRODUCING);
    if (producing.empty()) {
        view_.showNoProducing();
        return;
    }

    std::sort(producing.begin(), producing.end(),
              [](const Order& a, const Order& b) {
                  return a.productionStartTime < b.productionStartTime;
              });

    std::time_t now = stockService_.now();

    auto sampleName = [&](const std::string& sid) -> std::string {
        auto opt = sampleRepo_.findById(sid);
        return opt.has_value() ? opt->name : sid;
    };

    auto calcProgress = [&](const Order& o) -> std::pair<double, double> {
        if (o.totalProductionTimeMin <= 0.0) return {100.0, 0.0};
        double totalSec    = o.totalProductionTimeMin * 60.0;
        double elapsedSec  = std::difftime(now, o.productionStartTime);
        double pct         = std::min(100.0, elapsedSec / totalSec * 100.0);
        double remainMin   = std::max(0.0, (totalSec - elapsedSec) / 60.0);
        return {pct, remainMin};
    };

    // 현재 생산 중 (FIFO 첫 번째)
    const Order& current = producing[0];
    auto [pct, rem] = calcProgress(current);
    view_.showCurrentProduction(current, sampleName(current.sampleId), pct, rem);

    // 대기 큐 (나머지)
    int queueCount = (int)producing.size() - 1;
    view_.showQueueHeader(queueCount);
    if (queueCount == 0) {
        view_.showQueueEmpty();
    } else {
        for (int i = 1; i < (int)producing.size(); ++i) {
            const Order& o = producing[i];
            auto [p2, r2] = calcProgress(o);
            view_.showQueueItem(i, o, sampleName(o.sampleId), r2);
        }
    }
}
