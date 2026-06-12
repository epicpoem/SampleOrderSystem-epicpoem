#pragma once
#include <string>
#include <vector>
#include "../model/Order.h"

class IReleaseView {
public:
    virtual ~IReleaseView() = default;
    virtual void showMenu() = 0;
    virtual void showNoConfirmedOrders() = 0;
    virtual void showConfirmedOrderList(const std::vector<Order>& orders,
                                        const std::vector<std::string>& sampleNames) = 0;
    virtual void showOrderSelectPrompt() = 0;
    virtual void showOrderNotFound() = 0;
    virtual void showReleaseCompleted(const Order& order, const std::string& dateStr) = 0;
    virtual void showProductionCompleted(const std::string& orderNo) = 0;
    virtual void showPressEnterPrompt() {}
};
