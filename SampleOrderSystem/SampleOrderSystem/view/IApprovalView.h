#pragma once
#include "../model/Order.h"
#include <vector>
#include <string>

class IApprovalView {
public:
    virtual ~IApprovalView() = default;
    virtual void showApprovalMenu() = 0;
    virtual void showNoReservedOrders() = 0;
    virtual void showReservedOrderList(const std::vector<Order>& orders) = 0;
    virtual void showOrderSelectPrompt() = 0;
    virtual void showOrderNotFound() = 0;
    virtual void showStockSufficient(double physStock, int quantity) = 0;
    virtual void showStockInsufficient(double physStock, int quantity,
                                       double shortage, int actualProd,
                                       double totalProdTimeMin) = 0;
    virtual void showApprovePrompt() = 0;
    virtual void showApprovedAsConfirmed(const Order& order) = 0;
    virtual void showApprovedAsProducing(const Order& order) = 0;
    virtual void showRejected(const Order& order) = 0;
    virtual void showProductionCompleted(const std::string& orderNo) = 0;
};
