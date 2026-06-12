#pragma once
#include "IMonitorView.h"
#include <iostream>
#include <ostream>

class MonitorView : public IMonitorView {
public:
    explicit MonitorView(std::ostream& out = std::cout);
    void showMenu() override;
    void showOrderStats(int reserved, int confirmed, int producing, int released) override;
    void showStockTableHeader() override;
    void showStockRow(const Sample& sample, double physStock,
                      int totalOrderedQty,
                      const std::string& statusLabel) override;
    void showNoSamples() override;
    void showProductionCompleted(const std::string& orderNo) override;
private:
    std::ostream& out_;
};
