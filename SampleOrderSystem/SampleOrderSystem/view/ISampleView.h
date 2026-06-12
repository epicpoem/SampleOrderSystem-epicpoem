#pragma once
#include <vector>
#include "../model/Sample.h"

class ISampleView {
public:
    virtual ~ISampleView() = default;
    virtual void showMenu() = 0;
    virtual void showInvalidInput() = 0;
    virtual void showRegisterPrompt() = 0;
    virtual void showNamePrompt() = 0;
    virtual void showTimePrompt() = 0;
    virtual void showYieldPrompt() = 0;
    virtual void showYieldOutOfRange() = 0;
    virtual void showTimeOutOfRange() = 0;
    virtual void showDuplicateId() = 0;
    virtual void showRegisterSuccess(const Sample& sample) = 0;
    virtual void showSampleList(const std::vector<Sample>& samples) = 0;
    virtual void showSearchMenu() = 0;
    virtual void showSearchPrompt() = 0;
    virtual void showSearchResult(const std::vector<Sample>& samples) = 0;
    virtual void showNoResults() = 0;
    virtual void showCancelConfirmPrompt() {}
    virtual void showRegisterCancelled() {}
};
