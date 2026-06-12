#include "ApprovalView.h"
#include <iomanip>

ApprovalView::ApprovalView(std::ostream& out) : out_(out) {}

void ApprovalView::showApprovalMenu() {
    out_ << "\n===========================\n";
    out_ << "   [3] 주문 승인/거절\n";
    out_ << "===========================\n";
}

void ApprovalView::showNoReservedOrders() {
    out_ << "  승인 대기 중인 주문이 없습니다.\n";
}

void ApprovalView::showReservedOrderList(const std::vector<Order>& orders) {
    out_ << "\n[승인 대기 주문 목록]\n";
    out_ << std::left
         << std::setw(5)  << " No"
         << std::setw(22) << "  주문번호"
         << std::setw(10) << " 시료ID"
         << std::setw(20) << " 고객명"
         << "수량\n";
    out_ << std::string(68, '-') << "\n";
    for (int i = 0; i < (int)orders.size(); ++i) {
        const auto& o = orders[i];
        out_ << std::setw(5)  << ("  " + std::to_string(i + 1))
             << std::setw(22) << ("  " + o.orderNo)
             << std::setw(10) << (" " + o.sampleId)
             << std::setw(20) << (" " + o.customerName)
             << " " << o.quantity << " ea\n";
    }
    out_ << " [0] 뒤로\n";
}

void ApprovalView::showOrderSelectPrompt() {
    out_ << "처리할 번호 선택 > ";
}

void ApprovalView::showOrderNotFound() {
    out_ << "[오류] 유효하지 않은 번호입니다.\n";
}

void ApprovalView::showStockSufficient(double physStock, int quantity) {
    out_ << "\n  물리적 재고: " << std::fixed << std::setprecision(1)
         << physStock << " ea  (주문 수량: " << quantity << " ea) → 재고 충분\n";
    out_ << "  승인 시 즉시 CONFIRMED 전환됩니다.\n";
}

void ApprovalView::showStockInsufficient(double physStock, int quantity,
                                          double shortage, int actualProd,
                                          double totalProdTimeMin) {
    out_ << "\n  물리적 재고: " << std::fixed << std::setprecision(1)
         << physStock << " ea  (주문 수량: " << quantity << " ea) → 재고 부족\n";
    out_ << "  부족분: " << std::fixed << std::setprecision(1) << shortage
         << " ea  |  실생산량: " << actualProd
         << " ea  |  생산시간: " << std::fixed << std::setprecision(0)
         << totalProdTimeMin << " min\n";
    out_ << "  승인 시 생산라인에 등록되어 PRODUCING 전환됩니다.\n";
}

void ApprovalView::showApprovePrompt() {
    out_ << "승인하시겠습니까? [Y/N] > ";
}

void ApprovalView::showApprovedAsConfirmed(const Order& order) {
    out_ << "\n[완료] 주문 " << order.orderNo << " → CONFIRMED 전환되었습니다.\n";
}

void ApprovalView::showApprovedAsProducing(const Order& order) {
    out_ << "\n[완료] 주문 " << order.orderNo << " → PRODUCING 전환되었습니다.\n";
    out_ << "  실생산량: " << order.actualProduction
         << " ea  |  총 생산시간: " << std::fixed << std::setprecision(0)
         << order.totalProductionTimeMin << " min\n";
}

void ApprovalView::showRejected(const Order& order) {
    out_ << "\n[완료] 주문 " << order.orderNo << " → REJECTED 처리되었습니다.\n";
}

void ApprovalView::showProductionCompleted(const std::string& orderNo) {
    out_ << "[생산완료] " << orderNo << " 생산 완료 → CONFIRMED 전환\n";
}
