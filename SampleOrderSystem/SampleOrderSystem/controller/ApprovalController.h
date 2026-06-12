#pragma once
#include "IController.h"
#include "../view/IApprovalView.h"
#include "../repository/IOrderRepository.h"
#include "../service/StockService.h"
#include <istream>

class ApprovalController : public IController {
public:
    ApprovalController(std::istream& in, IApprovalView& view,
                       IOrderRepository& orderRepo,
                       StockService& stockService);
    void run() override;

private:
    std::istream& in_;
    IApprovalView& view_;
    IOrderRepository& orderRepo_;
    StockService& stockService_;
};
