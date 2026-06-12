#include "ApprovalController.h"
#include <string>
#include <cmath>
#include <algorithm>

ApprovalController::ApprovalController(std::istream& in, IApprovalView& view,
                                        ISampleRepository& sampleRepo,
                                        IOrderRepository& orderRepo,
                                        IClock& clock)
    : in_(in), view_(view), sampleRepo_(sampleRepo),
      orderRepo_(orderRepo), clock_(clock) {}

void ApprovalController::checkAndCompleteProduction() {
    std::time_t now = clock_.now();
    for (const auto& o : orderRepo_.findAll()) {
        if (o.status != OrderStatus::PRODUCING) continue;
        double elapsedMin = std::difftime(now, o.productionStartTime) / 60.0;
        if (elapsedMin >= o.totalProductionTimeMin) {
            Order completed = o;
            completed.status = OrderStatus::CONFIRMED;
            orderRepo_.update(completed);

            auto sampleOpt = sampleRepo_.findById(o.sampleId);
            if (sampleOpt.has_value()) {
                Sample updated = *sampleOpt;
                updated.stock += o.actualProduction;
                sampleRepo_.update(updated);
            }

            view_.showProductionCompleted(o.orderNo);
        }
    }
}

double ApprovalController::calcPhysicalStock(const Sample& sample) {
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

void ApprovalController::run() {
    checkAndCompleteProduction();

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
    auto sampleOpt = sampleRepo_.findById(order.sampleId);
    if (!sampleOpt.has_value()) {
        view_.showOrderNotFound();
        return;
    }
    const Sample& sample = *sampleOpt;

    double physStock = calcPhysicalStock(sample);
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
            updated.productionStartTime = clock_.now();
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
