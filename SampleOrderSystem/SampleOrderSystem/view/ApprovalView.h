#pragma once
#include "IApprovalView.h"
#include <iostream>
#include <ostream>

class ApprovalView : public IApprovalView {
public:
    explicit ApprovalView(std::ostream& out = std::cout);

    void showApprovalMenu() override;
    void showNoReservedOrders() override;
    void showReservedOrderList(const std::vector<Order>& orders) override;
    void showOrderSelectPrompt() override;
    void showOrderNotFound() override;
    void showStockSufficient(double physStock, int quantity) override;
    void showStockInsufficient(double physStock, int quantity,
                               double shortage, int actualProd,
                               double totalProdTimeMin) override;
    void showApprovePrompt() override;
    void showApprovedAsConfirmed(const Order& order) override;
    void showApprovedAsProducing(const Order& order) override;
    void showRejected(const Order& order) override;
    void showProductionCompleted(const std::string& orderNo) override;
    void showPressEnterPrompt() override;

private:
    std::ostream& out_;
};
