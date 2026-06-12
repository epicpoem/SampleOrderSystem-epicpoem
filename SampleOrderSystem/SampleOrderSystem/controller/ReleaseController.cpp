#include "ReleaseController.h"

ReleaseController::ReleaseController(std::istream& in, IReleaseView& view,
                                     ISampleRepository& sampleRepo,
                                     IOrderRepository& orderRepo,
                                     StockService& stockService)
    : in_(in), view_(view), sampleRepo_(sampleRepo),
      orderRepo_(orderRepo), stockService_(stockService) {}

void ReleaseController::run() {
    for (const auto& no : stockService_.checkAndCompleteProduction())
        view_.showProductionCompleted(no);

    view_.showMenu();

    auto confirmed = orderRepo_.findByStatus(OrderStatus::CONFIRMED);
    if (confirmed.empty()) {
        view_.showNoConfirmedOrders();
        return;
    }

    view_.showConfirmedOrderList(confirmed);
    view_.showOrderSelectPrompt();

    std::string input;
    if (!std::getline(in_, input)) return;
    if (input == "0") return;

    int idx = -1;
    try { idx = std::stoi(input) - 1; } catch (...) {}
    if (idx < 0 || idx >= (int)confirmed.size()) {
        view_.showOrderNotFound();
        return;
    }

    Order order = confirmed[idx];
    sampleRepo_.decreaseStock(order.sampleId, order.quantity);
    order.status = OrderStatus::RELEASE;
    orderRepo_.update(order);

    view_.showReleaseCompleted(order, stockService_.today());
}
