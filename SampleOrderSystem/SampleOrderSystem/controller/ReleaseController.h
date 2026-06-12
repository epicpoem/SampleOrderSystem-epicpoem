#pragma once
#include <istream>
#include "../view/IReleaseView.h"
#include "../repository/ISampleRepository.h"
#include "../repository/IOrderRepository.h"
#include "../service/StockService.h"

class ReleaseController {
    std::istream&       in_;
    IReleaseView&       view_;
    ISampleRepository&  sampleRepo_;
    IOrderRepository&   orderRepo_;
    StockService&       stockService_;
public:
    ReleaseController(std::istream& in, IReleaseView& view,
                      ISampleRepository& sampleRepo,
                      IOrderRepository& orderRepo,
                      StockService& stockService);
    void run();
};
