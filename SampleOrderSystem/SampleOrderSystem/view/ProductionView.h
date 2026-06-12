#pragma once
#include "IProductionView.h"
#include <iostream>
#include <ostream>

class ProductionView : public IProductionView {
public:
    explicit ProductionView(std::ostream& out = std::cout);
    void showMenu() override;
    void showNoProducing() override;
    void showCurrentProduction(const Order& order, const std::string& sampleName,
                               double progressPct, double remainingMin) override;
    void showQueueHeader(int count) override;
    void showQueueItem(int rank, const Order& order, const std::string& sampleName,
                       double remainingMin) override;
    void showQueueEmpty() override;
    void showProductionCompleted(const std::string& orderNo) override;
private:
    std::ostream& out_;
};
