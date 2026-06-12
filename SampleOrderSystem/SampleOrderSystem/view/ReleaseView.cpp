#include "ReleaseView.h"
#include <iomanip>

ReleaseView::ReleaseView(std::ostream& out) : out_(out) {}

void ReleaseView::showMenu() {
    out_ << std::string(70, '=') << "\n";
    out_ << "  [6] \xEC\xB6\x9C\xEA\xB3\xA0 \xEC\xB2\x98\xEB\xA6\xAC\n";
    out_ << std::string(70, '-') << "\n";
}

void ReleaseView::showNoConfirmedOrders() {
    out_ << "  \xEC\xB6\x9C\xEA\xB3\xA0 \xEA\xB0\x80\xEB\x8A\xA5\xED\x95\x9C \xEC\xA3\xBC\xEB\xAC\xB8(CONFIRMED)\xEC\x9D\xB4 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
}

void ReleaseView::showConfirmedOrderList(const std::vector<Order>& orders,
                                          const std::vector<std::string>& sampleNames) {
    out_ << "\xEC\xB6\x9C\xEA\xB3\xA0 \xEA\xB0\x80\xEB\x8A\xA5 \xEC\xA3\xBC\xEB\xAC\xB8  (CONFIRMED)\n";
    out_ << std::left
         << std::setw(8)  << "\xEB\xB2\x88\xED\x98\xB8"
         << std::setw(22) << "\xEC\xA3\xBC\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8"
         << std::setw(18) << "\xEA\xB3\xA0\xEA\xB0\x9D"
         << std::setw(26) << "\xEC\x8B\x9C\xEB\xA3\x8C"
         << "\xEC\x88\x98\xEB\x9F\x89\n";
    out_ << std::string(70, '-') << "\n";
    for (int i = 0; i < (int)orders.size(); ++i) {
        const auto& o = orders[i];
        std::string nameStr = (i < (int)sampleNames.size()) ? sampleNames[i] : o.sampleId;
        out_ << std::left
             << std::setw(8)  << ("[" + std::to_string(i + 1) + "]")
             << std::setw(22) << o.orderNo
             << std::setw(18) << o.customerName
             << std::setw(26) << nameStr
             << o.quantity << " ea\n";
    }
}

void ReleaseView::showOrderSelectPrompt() {
    out_ << "\xEC\xB6\x9C\xEA\xB3\xA0\xED\x95\xA0 \xEB\xB2\x88\xED\x98\xB8 > ";
    out_.flush();
}

void ReleaseView::showOrderNotFound() {
    out_ << "  [\xEC\x98\xA4\xEB\xA5\x98] \xED\x95\xB4\xEB\x8B\xB9 \xEB\xB2\x88\xED\x98\xB8\xEC\x9D\x98 \xEC\xA3\xBC\xEB\xAC\xB8\xEC\x9D\x84 \xEC\xB0\xBE\xEC\x9D\x84 \xEC\x88\x98 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
}

void ReleaseView::showReleaseCompleted(const Order& order, const std::string& dateStr) {
    out_ << "\n\xEC\xB6\x9C\xEA\xB3\xA0 \xEC\xB2\x98\xEB\xA6\xAC \xEC\x99\x84\xEB\xA3\x8C.\n";
    out_ << "\n";
    out_ << "\xEC\xA3\xBC\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8    " << order.orderNo << "\n";
    out_ << "\xEC\xB6\x9C\xEA\xB3\xA0\xEC\x88\x98\xEB\x9F\x89    " << order.quantity << " ea\n";
    out_ << "\xEC\xB2\x98\xEB\xA6\xAC\xEC\x9D\xBC\xEC\x8B\x9C    " << dateStr << "\n";
    out_ << "\xEC\x83\x81\xED\x83\x9C        CONFIRMED \xE2\x86\x92 RELEASE\n";
    out_ << "\n";
}

void ReleaseView::showProductionCompleted(const std::string& orderNo) {
    out_ << "[\xEC\x83\x9D\xEC\x82\xB0\xEC\x99\x84\xEB\xA3\x8C] " << orderNo << " \xE2\x86\x92 CONFIRMED\n";
}

void ReleaseView::showPressEnterPrompt() {
    out_ << "\xEC\x84\xa0\xED\x83\x9D > ";
    out_.flush();
}
