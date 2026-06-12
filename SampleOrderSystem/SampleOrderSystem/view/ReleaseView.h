#pragma once
#include "IReleaseView.h"
#include <ostream>

class ReleaseView : public IReleaseView {
    std::ostream& out_;
public:
    explicit ReleaseView(std::ostream& out);
    void showMenu() override;
    void showNoConfirmedOrders() override;
    void showConfirmedOrderList(const std::vector<Order>&) override;
    void showOrderSelectPrompt() override;
    void showOrderNotFound() override;
    void showReleaseCompleted(const Order& order, const std::string& dateStr) override;
    void showProductionCompleted(const std::string& orderNo) override;
};
