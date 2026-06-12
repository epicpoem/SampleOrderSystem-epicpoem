#include "MonitorView.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

MonitorView::MonitorView(std::ostream& out) : out_(out) {}

void MonitorView::showMenu(const std::string& timestamp) {
    out_ << std::string(70, '=') << "\n";
    out_ << "  [4] \xEB\xAA\xA8\xEB\x8B\x88\xED\x84\xB0\xEB\xA7\x81   " << timestamp << "\n";
    out_ << std::string(70, '-') << "\n";
}

void MonitorView::showOrderStats(int reserved, int confirmed, int producing, int released) {
    out_ << "\n[\xEC\xB3\xBC\xEA\xB2\x8C \xEC\xA3\xBC\xEB\xAC\xB8 \xED\x98\x84\xED\x99\xA9]\n";
    out_ << "  RESERVED  : " << reserved  << "\xEA\xB1\xB4\n";
    out_ << "  CONFIRMED : " << confirmed << "\xEA\xB1\xB4\n";
    out_ << "  PRODUCING : " << producing << "\xEA\xB1\xB4"
         << (producing > 0 ? "  \xE2\x86\x90 \xEC\x83\x9D\xEC\x82\xB0\xEB\x9D\xBC\xEC\x9D\xB8 \xEB\x8C\x80\xEA\xB8\xB0" : "")
         << "\n";
    out_ << "  RELEASE   : " << released  << "\xEA\xB1\xB4\n";
}

void MonitorView::showStockTableHeader() {
    out_ << "\n[\xEC\x9E\xAC\xEA\xB3\xA0 \xED\x98\x84\xED\x99\xA9]\n";
    out_ << std::left
         << std::setw(10) << "ID"
         << std::setw(32) << "\xEC\x8B\x9C\xEB\xA3\x8C\xEB\xAA\x85"
         << std::setw(12) << "\xEC\x9E\xAC\xEA\xB3\xA0"
         << std::setw(8)  << "\xEC\x83\x81\xED\x83\x9C"
         << "\xEC\x9E\x94\xEC\x97\xAC\xEC\x9C\xA8\n";
    out_ << std::string(70, '-') << "\n";
}

void MonitorView::showStockRow(const Sample& sample, double physStock,
                               int totalOrderedQty,
                               const std::string& statusLabel) {
    std::string stockStr = std::to_string((long long)physStock) + " ea";

    int ratio = 100;
    if (totalOrderedQty > 0) {
        double r = physStock / (double)totalOrderedQty * 100.0;
        ratio = (int)std::min(100.0, std::max(0.0, r));
    }
    int filled = ratio / 10;

    std::string bar;
    for (int i = 0; i < filled; ++i)  bar += "\xE2\x96\x88";
    for (int i = filled; i < 10; ++i) bar += "\xE2\x96\x91";

    out_ << std::left
         << std::setw(10) << sample.id
         << std::setw(32) << sample.name
         << std::setw(12) << stockStr
         << std::setw(8)  << statusLabel
         << "[" << bar << "] " << std::setw(3) << ratio << "%\n";
}

void MonitorView::showNoSamples() {
    out_ << "  \xEB\xB3\xB5\xEB\xA1\x9D\xEB\x90\x9C \xEC\x8B\x9C\xEB\xA3\x8C\xEA\xB0\x80 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
}

void MonitorView::showProductionCompleted(const std::string& orderNo) {
    out_ << "[\xEC\x83\x9D\xEC\x82\xB0\xEC\x99\x84\xEB\xA3\x8C] " << orderNo << " \xE2\x86\x92 CONFIRMED\n";
}

void MonitorView::showPressEnterPrompt() {
    out_ << "\n\xEC\x84\xa0\xED\x83\x9D > ";
    out_.flush();
}
