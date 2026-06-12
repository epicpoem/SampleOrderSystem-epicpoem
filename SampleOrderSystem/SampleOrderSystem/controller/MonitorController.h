#pragma once
#include "IController.h"
#include "../view/IMonitorView.h"
#include "../repository/ISampleRepository.h"
#include "../repository/IOrderRepository.h"
#include "../service/StockService.h"
#include <istream>

class MonitorController : public IController {
public:
    MonitorController(std::istream& in, IMonitorView& view,
                      ISampleRepository& sampleRepo,
                      IOrderRepository& orderRepo,
                      StockService& stockService);
    void run() override;

private:
    std::istream& in_;
    IMonitorView& view_;
    ISampleRepository& sampleRepo_;
    IOrderRepository& orderRepo_;
    StockService& stockService_;
};
