#include "ReleaseView.h"
#include <iomanip>

ReleaseView::ReleaseView(std::ostream& out) : out_(out) {}

void ReleaseView::showMenu() {
    out_ << "\n===========================\n";
    out_ << "   [6] 출고 처리\n";
    out_ << "===========================\n";
}

void ReleaseView::showNoConfirmedOrders() {
    out_ << "  출고 가능한 주문(CONFIRMED)이 없습니다.\n";
}

void ReleaseView::showConfirmedOrderList(const std::vector<Order>& orders) {
    out_ << "\n[출고 대기 목록]\n";
    out_ << std::left
         << std::setw(6)  << " 번호"
         << std::setw(24) << " 주문번호"
         << std::setw(16) << " 시료ID"
         << std::setw(14) << " 고객명"
         << "수량\n";
    out_ << std::string(64, '-') << "\n";
    for (int i = 0; i < (int)orders.size(); ++i) {
        const auto& o = orders[i];
        out_ << std::left
             << std::setw(6)  << (" " + std::to_string(i + 1))
             << std::setw(24) << (" " + o.orderNo)
             << std::setw(16) << (" " + o.sampleId)
             << std::setw(14) << (" " + o.customerName)
             << " " << o.quantity << " ea\n";
    }
}

void ReleaseView::showOrderSelectPrompt() {
    out_ << "출고할 주문 번호 (0: 취소) > ";
}

void ReleaseView::showOrderNotFound() {
    out_ << "  [오류] 해당 번호의 주문을 찾을 수 없습니다.\n";
}

void ReleaseView::showReleaseCompleted(const Order& order, const std::string& dateStr) {
    out_ << "\n[출고 완료]\n";
    out_ << "  주문번호 : " << order.orderNo << "\n";
    out_ << "  출고수량 : " << order.quantity << " ea\n";
    out_ << "  처리일시 : " << dateStr << "\n";
    out_ << "  상태전환 : CONFIRMED \xE2\x86\x92 RELEASE\n";
}

void ReleaseView::showProductionCompleted(const std::string& orderNo) {
    out_ << "[생산완료] " << orderNo << " \xE2\x86\x92 CONFIRMED\n";
}
