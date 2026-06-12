#include "ProductionView.h"
#include <iomanip>
#include <sstream>

ProductionView::ProductionView(std::ostream& out) : out_(out) {}

void ProductionView::showMenu() {
    out_ << "\n===========================\n";
    out_ << "   [5] 생산라인 조회\n";
    out_ << "===========================\n";
}

void ProductionView::showNoProducing() {
    out_ << "  현재 생산 중인 주문이 없습니다.\n";
}

void ProductionView::showCurrentProduction(const Order& order, const std::string& sampleName,
                                           double progressPct, double remainingMin) {
    out_ << "\n[현재 생산 중]\n";
    out_ << "  주문번호 : " << order.orderNo << "\n";
    out_ << "  시료명   : " << sampleName << "\n";
    out_ << "  주문수량 : " << order.quantity << " ea\n";
    out_ << "  실생산량 : " << order.actualProduction << " ea\n";
    out_ << "  진행률   : " << std::fixed << std::setprecision(1)
         << progressPct << " %\n";
    if (remainingMin <= 0.0)
        out_ << "  잔여시간 : 완료 대기 중\n";
    else
        out_ << "  잔여시간 : " << std::fixed << std::setprecision(1)
             << remainingMin << " min\n";
}

void ProductionView::showQueueHeader(int count) {
    out_ << "\n[대기 큐]  (" << count << "건)\n";
    out_ << std::left
         << std::setw(6)  << " 순서"
         << std::setw(24) << " 주문번호"
         << std::setw(26) << " 시료명"
         << std::setw(12) << " 실생산량"
         << "잔여시간\n";
    out_ << std::string(74, '-') << "\n";
}

void ProductionView::showQueueItem(int rank, const Order& order, const std::string& sampleName,
                                   double remainingMin) {
    std::string rankStr = std::to_string(rank);
    std::string prodStr = std::to_string(order.actualProduction) + " ea";
    std::string remStr;
    if (remainingMin <= 0.0)
        remStr = "완료 대기";
    else {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << remainingMin << " min";
        remStr = oss.str();
    }
    out_ << std::left
         << std::setw(6)  << (" " + rankStr)
         << std::setw(24) << (" " + order.orderNo)
         << std::setw(26) << (" " + sampleName)
         << std::setw(12) << (" " + prodStr)
         << " " << remStr << "\n";
}

void ProductionView::showQueueEmpty() {
    out_ << "  대기 중인 주문이 없습니다.\n";
}

void ProductionView::showProductionCompleted(const std::string& orderNo) {
    out_ << "[생산완료] " << orderNo << " \xE2\x86\x92 CONFIRMED\n";
}
