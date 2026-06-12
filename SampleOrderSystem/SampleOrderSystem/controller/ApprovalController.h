#pragma once
#include "IController.h"
#include "../view/IApprovalView.h"
#include "../repository/ISampleRepository.h"
#include "../repository/IOrderRepository.h"
#include "../util/IClock.h"
#include "../model/Sample.h"
#include <istream>

class ApprovalController : public IController {
public:
    ApprovalController(std::istream& in, IApprovalView& view,
                       ISampleRepository& sampleRepo,
                       IOrderRepository& orderRepo,
                       IClock& clock);
    void run() override;

private:
    void checkAndCompleteProduction();
    double calcPhysicalStock(const Sample& sample);

    std::istream& in_;
    IApprovalView& view_;
    ISampleRepository& sampleRepo_;
    IOrderRepository& orderRepo_;
    IClock& clock_;
};
