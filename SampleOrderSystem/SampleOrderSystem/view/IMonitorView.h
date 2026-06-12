#pragma once
#include "../model/Sample.h"
#include <string>

class IMonitorView {
public:
    virtual ~IMonitorView() = default;
    virtual void showMenu(const std::string& timestamp) = 0;
    virtual void showOrderStats(int reserved, int confirmed, int producing, int released) = 0;
    virtual void showStockTableHeader() = 0;
    virtual void showStockRow(const Sample& sample, double physStock,
                              int totalOrderedQty,
                              const std::string& statusLabel) = 0;
    virtual void showNoSamples() = 0;
    virtual void showProductionCompleted(const std::string& orderNo) = 0;
    virtual void showPressEnterPrompt() {}
};
