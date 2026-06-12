#pragma once
#include "IController.h"
#include "../view/IProductionView.h"
#include "../repository/ISampleRepository.h"
#include "../repository/IOrderRepository.h"
#include "../service/StockService.h"
#include <istream>

class ProductionController : public IController {
public:
    ProductionController(std::istream& in, IProductionView& view,
                         ISampleRepository& sampleRepo,
                         IOrderRepository& orderRepo,
                         StockService& stockService);
    void run() override;

private:
    std::istream& in_;
    IProductionView& view_;
    ISampleRepository& sampleRepo_;
    IOrderRepository& orderRepo_;
    StockService& stockService_;
};
