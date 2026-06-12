#include "ApprovalController.h"
#include <string>
#include <cmath>

ApprovalController::ApprovalController(std::istream& in, IApprovalView& view,
                                       IOrderRepository& orderRepo,
                                       StockService& stockService)
    : in_(in), view_(view), orderRepo_(orderRepo), stockService_(stockService) {}

void ApprovalController::run() {
    for (const auto& orderNo : stockService_.checkAndCompleteProduction())
        view_.showProductionCompleted(orderNo);

    auto reserved = orderRepo_.findByStatus(OrderStatus::RESERVED);
    if (reserved.empty()) {
        view_.showNoReservedOrders();
        return;
    }

    view_.showApprovalMenu();
    view_.showReservedOrderList(reserved);
    view_.showOrderSelectPrompt();

    std::string input;
    if (!std::getline(in_, input)) return;
    if (input == "0") return;

    int idx = -1;
    try { idx = std::stoi(input) - 1; } catch (...) {}
    if (idx < 0 || idx >= (int)reserved.size()) {
        view_.showOrderNotFound();
        return;
    }

    const Order& order = reserved[idx];
    auto sampleOpt = stockService_.findSampleById(order.sampleId);
    if (!sampleOpt.has_value()) {
        view_.showOrderNotFound();
        return;
    }
    const Sample& sample = *sampleOpt;

    double physStock = stockService_.calcPhysicalStock(sample);
    bool sufficient = (physStock >= order.quantity);

    if (sufficient) {
        view_.showStockSufficient(physStock, order.quantity);
    } else {
        double shortage = order.quantity - physStock;
        int actualProd = (int)std::ceil(shortage / (sample.yield * 0.9));
        double totalProdTime = sample.avgProductionTime * actualProd;
        view_.showStockInsufficient(physStock, order.quantity, shortage,
                                    actualProd, totalProdTime);
    }

    view_.showApprovePrompt();
    std::string confirm;
    if (!std::getline(in_, confirm)) return;

    if (confirm == "Y" || confirm == "y") {
        if (sufficient) {
            Order updated = order;
            updated.status = OrderStatus::CONFIRMED;
            orderRepo_.update(updated);
            view_.showApprovedAsConfirmed(updated);
        } else {
            double shortage = order.quantity - physStock;
            int actualProd = (int)std::ceil(shortage / (sample.yield * 0.9));
            double totalProdTime = sample.avgProductionTime * actualProd;
            Order updated = order;
            updated.status = OrderStatus::PRODUCING;
            updated.actualProduction = actualProd;
            updated.totalProductionTimeMin = totalProdTime;
            updated.productionStartTime = stockService_.now();
            orderRepo_.update(updated);
            view_.showApprovedAsProducing(updated);
        }
    } else {
        Order updated = order;
        updated.status = OrderStatus::REJECTED;
        orderRepo_.update(updated);
        view_.showRejected(updated);
    }
}
