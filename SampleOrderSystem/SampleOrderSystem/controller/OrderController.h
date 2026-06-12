#pragma once
#include "IController.h"
#include "../view/IOrderView.h"
#include "../repository/ISampleRepository.h"
#include "../repository/IOrderRepository.h"
#include "../util/IClock.h"
#include <istream>

class OrderController : public IController {
public:
    OrderController(std::istream& in, IOrderView& view,
                    ISampleRepository& sampleRepo,
                    IOrderRepository& orderRepo,
                    IClock& clock);
    void run() override;

private:
    bool confirmCancelOnEmpty();

    std::istream& in_;
    IOrderView& view_;
    ISampleRepository& sampleRepo_;
    IOrderRepository& orderRepo_;
    IClock& clock_;
};
