#include "OrderController.h"
#include <string>

OrderController::OrderController(std::istream& in, IOrderView& view,
                                 ISampleRepository& sampleRepo,
                                 IOrderRepository& orderRepo,
                                 IClock& clock)
    : in_(in), view_(view), sampleRepo_(sampleRepo),
      orderRepo_(orderRepo), clock_(clock) {}

// 빈 입력 시 취소 확인. true → 취소 의사 확인됨
bool OrderController::confirmCancelOnEmpty() {
    view_.showCancelConfirmPrompt();
    std::string ans;
    if (!std::getline(in_, ans)) return true;
    return (ans == "Y" || ans == "y");
}

void OrderController::run() {
    view_.showOrderForm();

    std::string sampleId;
    while (true) {
        view_.showSampleIdPrompt();
        if (!std::getline(in_, sampleId)) return;
        if (sampleId.empty()) {
            if (confirmCancelOnEmpty()) { view_.showOrderCancelled(); return; }
            continue;
        }
        if (sampleRepo_.exists(sampleId)) break;
        view_.showInvalidSampleId();
    }

    std::string customerName;
    while (true) {
        view_.showCustomerNamePrompt();
        if (!std::getline(in_, customerName)) return;
        if (customerName.empty()) {
            if (confirmCancelOnEmpty()) { view_.showOrderCancelled(); return; }
            continue;
        }
        break;
    }

    int qty = 0;
    while (true) {
        view_.showQuantityPrompt();
        std::string qtyStr;
        if (!std::getline(in_, qtyStr)) return;
        if (qtyStr.empty()) {
            if (confirmCancelOnEmpty()) { view_.showOrderCancelled(); return; }
            continue;
        }
        try { qty = std::stoi(qtyStr); } catch (...) { qty = 0; }
        if (qty > 0) break;
        view_.showInvalidQuantity();
    }

    view_.showOrderConfirmation(sampleId, customerName, qty);
    view_.showConfirmPrompt();
    std::string confirm;
    if (!std::getline(in_, confirm)) return;
    if (confirm != "Y" && confirm != "y") {
        view_.showOrderCancelled();
        return;
    }

    Order order;
    order.orderNo      = orderRepo_.generateOrderNo(clock_.today());
    order.sampleId     = sampleId;
    order.customerName = customerName;
    order.quantity     = qty;
    order.status       = OrderStatus::RESERVED;

    orderRepo_.add(order);
    view_.showOrderSuccess(order);
}
