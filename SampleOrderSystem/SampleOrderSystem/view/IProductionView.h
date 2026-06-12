#pragma once
#include "../model/Order.h"
#include <string>

class IProductionView {
public:
    virtual ~IProductionView() = default;
    virtual void showMenu() = 0;
    virtual void showNoProducing() = 0;
    virtual void showCurrentProduction(const Order& order, const std::string& sampleName,
                                       double progressPct, double remainingMin) = 0;
    virtual void showQueueHeader(int count) = 0;
    virtual void showQueueItem(int rank, const Order& order, const std::string& sampleName,
                               double remainingMin) = 0;
    virtual void showQueueEmpty() = 0;
    virtual void showProductionCompleted(const std::string& orderNo) = 0;
};
