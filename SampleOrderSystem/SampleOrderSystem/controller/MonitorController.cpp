#include "MonitorController.h"

MonitorController::MonitorController(std::istream& in, IMonitorView& view,
                                     ISampleRepository& sampleRepo,
                                     IOrderRepository& orderRepo,
                                     StockService& stockService)
    : in_(in), view_(view), sampleRepo_(sampleRepo),
      orderRepo_(orderRepo), stockService_(stockService) {}

void MonitorController::run() {
    for (const auto& no : stockService_.checkAndCompleteProduction())
        view_.showProductionCompleted(no);

    view_.showMenu();

    // 상태별 주문 건수 집계 (REJECTED 제외)
    int reserved = 0, confirmed = 0, producing = 0, released = 0;
    for (const auto& o : orderRepo_.findAll()) {
        switch (o.status) {
        case OrderStatus::RESERVED:  ++reserved;  break;
        case OrderStatus::CONFIRMED: ++confirmed; break;
        case OrderStatus::PRODUCING: ++producing; break;
        case OrderStatus::RELEASE:   ++released;  break;
        default: break;
        }
    }
    view_.showOrderStats(reserved, confirmed, producing, released);

    // 시료별 재고 상태
    auto samples = sampleRepo_.findAll();
    if (samples.empty()) {
        view_.showNoSamples();
        return;
    }

    view_.showStockTableHeader();
    auto orders = orderRepo_.findAll();

    for (const auto& s : samples) {
        int totalQty = 0;
        for (const auto& o : orders) {
            if (o.sampleId != s.id) continue;
            if (o.status == OrderStatus::CONFIRMED || o.status == OrderStatus::PRODUCING)
                totalQty += o.quantity;
        }

        double physStock = stockService_.calcPhysicalStock(s);

        std::string statusLabel;
        if (physStock <= 0.0)               statusLabel = "\xEA\xB3\xA0\xEA\xB0\x88";
        else if (physStock >= (double)totalQty) statusLabel = "\xEC\x97\xAC\xEC\x9C\xA0";
        else                                statusLabel = "\xEB\xB6\x80\xEC\xA1\xB1";

        view_.showStockRow(s, physStock, totalQty, statusLabel);
    }
}
