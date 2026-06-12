#pragma once
#include "IOrderView.h"
#include <iostream>
#include <ostream>

class OrderView : public IOrderView {
public:
    explicit OrderView(std::ostream& out = std::cout);

    void showOrderForm() override;
    void showSampleIdPrompt() override;
    void showCustomerNamePrompt() override;
    void showQuantityPrompt() override;
    void showOrderConfirmation(const std::string& sampleId,
                               const std::string& sampleName,
                               const std::string& customerName,
                               int quantity) override;
    void showConfirmPrompt() override;
    void showOrderSuccess(const Order& order) override;
    void showOrderCancelled() override;
    void showInvalidSampleId() override;
    void showInvalidQuantity() override;
    void showCancelConfirmPrompt() override;
    void showPressEnterPrompt() override;

private:
    std::ostream& out_;
};
