#pragma once
#include "../model/Order.h"
#include <string>

class IOrderView {
public:
    virtual ~IOrderView() = default;
    virtual void showOrderForm() = 0;
    virtual void showSampleIdPrompt() = 0;
    virtual void showCustomerNamePrompt() = 0;
    virtual void showQuantityPrompt() = 0;
    virtual void showOrderConfirmation(const std::string& sampleId,
                                       const std::string& customerName,
                                       int quantity) = 0;
    virtual void showConfirmPrompt() = 0;
    virtual void showOrderSuccess(const Order& order) = 0;
    virtual void showOrderCancelled() = 0;
    virtual void showInvalidSampleId() = 0;
    virtual void showInvalidQuantity() = 0;
    virtual void showCancelConfirmPrompt() = 0;
};
