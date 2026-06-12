#include "OrderView.h"

OrderView::OrderView(std::ostream& out) : out_(out) {}

void OrderView::showOrderForm() {
    out_ << "\n===========================\n";
    out_ << "     [2] 시료 주문\n";
    out_ << "===========================\n";
}

void OrderView::showSampleIdPrompt() {
    out_ << "시료 ID    > ";
}

void OrderView::showCustomerNamePrompt() {
    out_ << "고객명     > ";
}

void OrderView::showQuantityPrompt() {
    out_ << "주문 수량  > ";
}

void OrderView::showOrderConfirmation(const std::string& sampleId,
                                      const std::string& customerName,
                                      int quantity) {
    out_ << "\n[주문 내용 확인]\n";
    out_ << "  시료 ID : " << sampleId << "\n";
    out_ << "  고객명  : " << customerName << "\n";
    out_ << "  수량    : " << quantity << " ea\n";
}

void OrderView::showConfirmPrompt() {
    out_ << "예약 접수하시겠습니까? [Y/N] > ";
}

void OrderView::showOrderSuccess(const Order& order) {
    out_ << "\n[완료] 주문이 접수되었습니다.\n";
    out_ << "  주문번호 : " << order.orderNo << "\n";
    out_ << "  상태     : RESERVED\n";
}

void OrderView::showOrderCancelled() {
    out_ << "[취소] 주문이 취소되었습니다.\n";
}

void OrderView::showInvalidSampleId() {
    out_ << "[오류] 등록되지 않은 시료 ID입니다. 다시 입력해 주세요.\n";
}

void OrderView::showInvalidQuantity() {
    out_ << "[오류] 수량은 1 이상이어야 합니다.\n";
}
