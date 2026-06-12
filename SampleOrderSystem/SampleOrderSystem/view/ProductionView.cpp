#include "ProductionView.h"
#include <iomanip>
#include <sstream>
#include <ctime>

ProductionView::ProductionView(std::ostream& out) : out_(out) {}

void ProductionView::showMenu() {
    out_ << std::string(70, '=') << "\n";
    out_ << "  [5] \xEC\x83\x9D\xEC\x82\xB0\xEB\x9D\xBC\xEC\x9D\xB8 \xEC\xA1\xB0\xED\x9A\x8C   FIFO \xEB\xB0\xA9\xEC\x8B\x9D\n";
    out_ << std::string(70, '-') << "\n";
    out_ << "  \xEC\x83\x9D\xEC\x82\xB0\xEB\x9D\xBC\xEC\x9D\xB8  1\xEA\xB0\x9C (\xEB\x8B\xA8\xEC\x9D\xBC \xEB\x9D\xBC\xEC\x9D\xB8)\n";
    out_ << std::string(70, '-') << "\n";
}

void ProductionView::showNoProducing() {
    out_ << "  \xED\x98\x84\xEC\x9E\xAC \xEC\x83\x9D\xEC\x82\xB0 \xEC\xA4\x91\xEC\x9D\xB8 \xEC\xA3\xBC\xEB\xAC\xB8\xEC\x9D\xB4 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
}

void ProductionView::showCurrentProduction(const Order& order, const std::string& sampleName,
                                           double progressPct, double remainingMin) {
    out_ << "\n[\xED\x98\x84\xEC\x9E\xAC \xEC\xB2\x98\xEB\xA6\xAC \xEC\xA4\x91]\n";
    out_ << "  \xEC\xA3\xBC\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8  " << order.orderNo
         << "   \xEC\x8B\x9C\xEB\xA3\x8C  " << sampleName << "\n";
    out_ << "  \xEC\xA3\xBC\xEB\xAC\xB8\xEB\x9F\x89    " << order.quantity
         << " ea   \xEB\xB6\x80\xEC\xA1\xB1  " << order.shortageQty
         << " ea   \xEC\x8B\xA4\xEC\x83\x9D\xEC\x82\xB0\xEB\x9F\x89  " << order.actualProduction
         << " ea  (\xEC\xB4\x9D " << std::fixed << std::setprecision(0) << order.totalProductionTimeMin << " min)\n";

    int filled = (int)(progressPct / 10.0);
    if (filled > 10) filled = 10;
    std::string bar;
    for (int i = 0; i < filled; ++i)  bar += "\xE2\x96\x88";
    for (int i = filled; i < 10; ++i) bar += "\xE2\x96\x91";

    std::string completionStr;
    if (remainingMin <= 0.0) {
        completionStr = "\xEC\x99\x84\xEB\xA3\x8C \xEB\x8C\x80\xEA\xB8\xB0";
    } else {
        std::time_t now = std::time(nullptr);
        std::time_t ct = now + (std::time_t)(remainingMin * 60.0);
        std::tm lt{};
        localtime_s(&lt, &ct);
        char buf[8];
        std::strftime(buf, sizeof(buf), "%H:%M", &lt);
        completionStr = buf;
    }

    out_ << "  \xEC\xA7\x84\xED\x96\x89      [" << bar << "] "
         << std::fixed << std::setprecision(1) << progressPct
         << "%   \xEC\x99\x84\xEB\xA3\x8C \xEC\x98\x88\xEC\xA0\x95 " << completionStr << "\n";
}

void ProductionView::showQueueHeader(int count) {
    out_ << "\n[\xEB\x8C\x80\xEA\xB8\xB0 \xEC\xA4\x91\xEC\x9D\xB8 \xEC\xA3\xBC\xEB\xAC\xB8  (FIFO \xEC\x88\x9C)]  " << count << "\xEA\xB1\xB4\n";
    out_ << std::left
         << std::setw(6)  << "\xEC\x88\x9C\xEC\x84\x9C"
         << std::setw(22) << "\xEC\xA3\xBC\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8"
         << std::setw(26) << "\xEC\x8B\x9C\xEB\xA3\x8C"
         << std::setw(10) << "\xEC\xA3\xBC\xEB\xAC\xB8\xEB\x9F\x89"
         << std::setw(10) << "\xEB\xB6\x80\xEC\xA1\xB1\xEB\xB6\x84"
         << std::setw(10) << "\xEC\x8B\xA4\xEC\x83\x9D\xEC\x82\xB0\xEB\x9F\x89"
         << "\xEC\x98\x88\xEC\x83\x81 \xEC\x99\x84\xEB\xA3\x8C\n";
    out_ << std::string(84, '-') << "\n";
}

void ProductionView::showQueueItem(int rank, const Order& order, const std::string& sampleName,
                                   int shortageQty, const std::string& estimatedCompletion) {
    out_ << std::left
         << std::setw(6)  << rank
         << std::setw(22) << order.orderNo
         << std::setw(26) << sampleName
         << std::setw(10) << (std::to_string(order.quantity) + " ea")
         << std::setw(10) << (std::to_string(shortageQty) + " ea")
         << std::setw(10) << (std::to_string(order.actualProduction) + " ea")
         << estimatedCompletion << "\n";
}

void ProductionView::showQueueEmpty() {
    out_ << "  \xEB\x8C\x80\xEA\xB8\xB0 \xEC\xA4\x91\xEC\x9D\xB8 \xEC\xA3\xBC\xEB\xAC\xB8\xEC\x9D\xB4 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
}

void ProductionView::showProductionCompleted(const std::string& orderNo) {
    out_ << "[\xEC\x83\x9D\xEC\x82\xB0\xEC\x99\x84\xEB\xA3\x8C] " << orderNo << " \xE2\x86\x92 CONFIRMED\n";
}

void ProductionView::showPressEnterPrompt() {
    out_ << "\n* \xEB\xB6\x80\xEC\xA1\xB1\xEB\xB6\x84 = \xEC\xA3\xBC\xEB\xAC\xB8\xEB\x9F\x89 - \xEC\x9E\xAC\xEA\xB3\xA0,  "
         << "\xEC\x8B\xA4\xEC\x83\x9D\xEC\x82\xB0\xEB\x9F\x89 = ceil(\xEB\xB6\x80\xEC\xA1\xB1\xEB\xB6\x84 / (\xEC\x88\x98\xEC\x9C\xA8 * 0.9))\n";
    out_ << "* FIFO(\xEC\x84\xA0\xEC\x9E\x85\xEC\x84\xA0\xEC\xB6\x9C) \xEB\xB0\xA9\xEC\x8B\x9D\xEC\x9C\xBC\xEB\xA1\x9C \xEC\xB2\x98\xEB\xA6\xAC\xEB\x90\xA9\xEB\x8B\x88\xEB\x8B\xA4.\n";
    out_ << "\xEC\x84\xa0\xED\x83\x9D > ";
    out_.flush();
}
