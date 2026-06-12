#pragma once
#include "ISampleView.h"
#include <iostream>
#include <ostream>

class SampleView : public ISampleView {
public:
    explicit SampleView(std::ostream& out = std::cout);

    void showMenu() override;
    void showInvalidInput() override;
    void showRegisterPrompt() override;
    void showNamePrompt() override;
    void showTimePrompt() override;
    void showYieldPrompt() override;
    void showYieldOutOfRange() override;
    void showTimeOutOfRange() override;
    void showDuplicateId() override;
    void showRegisterSuccess(const Sample& sample) override;
    void showSampleList(const std::vector<Sample>& samples) override;
    void showSearchMenu() override;
    void showSearchPrompt() override;
    void showSearchResult(const std::vector<Sample>& samples) override;
    void showNoResults() override;

private:
    std::ostream& out_;
};
