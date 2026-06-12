#include "MonitorView.h"
#include <iomanip>

MonitorView::MonitorView(std::ostream& out) : out_(out) {}

void MonitorView::showMenu() {
    out_ << "\n===========================\n";
    out_ << "     [4] 모니터링\n";
    out_ << "===========================\n";
}

void MonitorView::showOrderStats(int reserved, int confirmed, int producing, int released) {
    out_ << "\n[주문 현황]\n";
    out_ << "  RESERVED  : " << reserved  << "건\n";
    out_ << "  CONFIRMED : " << confirmed << "건\n";
    out_ << "  PRODUCING : " << producing << "건\n";
    out_ << "  RELEASE   : " << released  << "건\n";
}

void MonitorView::showStockTableHeader() {
    out_ << "\n[시료별 재고 현황]\n";
    out_ << std::left
         << std::setw(10) << "ID"
         << std::setw(26) << "시료명"
         << std::setw(12) << "현재 재고"
         << std::setw(12) << "주문 수량"
         << "상태\n";
    out_ << std::string(68, '-') << "\n";
}

void MonitorView::showStockRow(const Sample& sample, int totalOrderedQty,
                               const std::string& statusLabel) {
    std::string stockStr  = std::to_string(sample.stock) + " ea";
    std::string orderStr  = std::to_string(totalOrderedQty) + " ea";
    out_ << std::left
         << std::setw(10) << sample.id
         << std::setw(26) << sample.name
         << std::setw(12) << stockStr
         << std::setw(12) << orderStr
         << statusLabel << "\n";
}

void MonitorView::showNoSamples() {
    out_ << "  등록된 시료가 없습니다.\n";
}

void MonitorView::showProductionCompleted(const std::string& orderNo) {
    out_ << "[생산완료] " << orderNo << " \xE2\x86\x92 CONFIRMED\n";
}
