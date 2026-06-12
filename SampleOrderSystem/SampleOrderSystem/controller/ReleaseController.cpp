#include "ReleaseController.h"
#include <ctime>

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
        view_.showPressEnterPrompt();
        std::string dummy;
        std::getline(in_, dummy);
        return;
    }

    // 시료명 목록 구성
    std::vector<std::string> sampleNames;
    for (const auto& o : confirmed) {
        auto opt = sampleRepo_.findById(o.sampleId);
        sampleNames.push_back(opt.has_value() ? opt->name : o.sampleId);
    }

    view_.showConfirmedOrderList(confirmed, sampleNames);
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

    // 처리일시 계산
    std::time_t now = stockService_.now();
    std::tm lt{};
    localtime_s(&lt, &now);
    char dateBuf[24];
    std::strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d %H:%M:%S", &lt);

    view_.showReleaseCompleted(order, std::string(dateBuf));

    view_.showPressEnterPrompt();
    std::string dummy;
    std::getline(in_, dummy);
}
