#include "MonitorView.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

MonitorView::MonitorView(std::ostream& out) : out_(out) {}

void MonitorView::showMenu() {
    out_ << "\n===========================\n";
    out_ << "     [4] \xEB\xaa\xA8\xEB\x8B\x88\xED\x84\xB0\xEB\xA7\x81\n";
    out_ << "===========================\n";
}

void MonitorView::showOrderStats(int reserved, int confirmed, int producing, int released) {
    out_ << "\n[\xEC\xA3\xBC\xEB\xAC\xB8 \xED\x98\x84\xED\x99\xA9]\n";
    out_ << "  RESERVED  : " << reserved  << "\xEA\xB1\xB4\n";
    out_ << "  CONFIRMED : " << confirmed << "\xEA\xB1\xB4\n";
    out_ << "  PRODUCING : " << producing << "\xEA\xB1\xB4\n";
    out_ << "  RELEASE   : " << released  << "\xEA\xB1\xB4\n";
}

void MonitorView::showStockTableHeader() {
    out_ << "\n[\xEC\x8B\x9C\xEB\xA3\x8C\xEB\xB3\x84 \xEC\x9E\xAC\xEA\xB3\xA0 \xED\x98\x84\xED\x99\xA9]\n";
    out_ << std::left
         << std::setw(10) << "ID"
         << std::setw(26) << "\xEC\x8B\x9C\xEB\xA3\x8C\xEB\xBA\x85"
         << std::setw(16) << "\xEB\xB3\xB4\xEC\x9C\xA0\xEC\x9E\xAC\xEA\xB3\xA0(\xEC\x8B\xA4\xEC\x8B\x9C\xEA\xB0\x84)"
         << std::setw(10) << "\xEC\xA3\xBC\xEB\xAC\xB8\xEC\x88\x98\xEB\x9F\x89"
         << std::setw(6)  << "\xEC\x83\x81\xED\x83\x9C"
         << "  \xEC\x9E\x94\xEC\x97\xAC\xEC\x9C\xA8\n";
    out_ << std::string(80, '-') << "\n";
}

void MonitorView::showStockRow(const Sample& sample, double physStock,
                               int totalOrderedQty,
                               const std::string& statusLabel) {
    // physStock string (2 decimal places)
    std::ostringstream physStr;
    physStr << std::fixed << std::setprecision(2) << physStock << " ea";

    std::string orderStr = std::to_string(totalOrderedQty) + " ea";

    // 잔여율 계산 (0~100, cap at 100)
    int ratio = 100;
    if (totalOrderedQty > 0) {
        double r = physStock / (double)totalOrderedQty * 100.0;
        ratio = (int)std::min(100.0, std::max(0.0, r));
    }
    int filled = ratio / 10;

    // 막대 그래프: █ = E2 96 88, ░ = E2 96 91
    std::string bar;
    for (int i = 0; i < filled; ++i)      bar += "\xE2\x96\x88";
    for (int i = filled; i < 10; ++i)     bar += "\xE2\x96\x91";

    out_ << std::left
         << std::setw(10) << sample.id
         << std::setw(26) << sample.name
         << std::setw(16) << physStr.str()
         << std::setw(10) << orderStr
         << std::setw(6)  << statusLabel
         << "  [" << bar << "] " << std::setw(3) << ratio << "%\n";
}

void MonitorView::showNoSamples() {
    out_ << "  \xEB\xB3\xB5\xEB\xA1\x9D\xEB\x90\x9C \xEC\x8B\x9C\xEB\xA3\x8C\xEA\xB0\x80 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
}

void MonitorView::showProductionCompleted(const std::string& orderNo) {
    out_ << "[\xEC\x83\x9D\xEC\x82\xB0\xEC\x99\x84\xEB\xA3\x8C] " << orderNo << " \xE2\x86\x92 CONFIRMED\n";
}
