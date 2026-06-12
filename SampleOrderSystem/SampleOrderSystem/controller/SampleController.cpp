#include "SampleController.h"
#include <string>
#include <algorithm>

SampleController::SampleController(std::istream& in, ISampleView& view, ISampleRepository& repo)
    : in_(in), view_(view), repo_(repo) {}

void SampleController::run() {
    while (true) {
        view_.showMenu();
        std::string line;
        if (!std::getline(in_, line)) return;
        if (line == "0") return;
        if      (line == "1") handleRegister();
        else if (line == "2") handleList();
        else if (line == "3") handleSearch();
        else                  view_.showInvalidInput();
    }
}

void SampleController::handleRegister() {
    std::string id;
    while (true) {
        view_.showRegisterPrompt();
        if (!std::getline(in_, id)) return;
        if (id.empty()) continue;
        if (!repo_.exists(id)) break;
        view_.showDuplicateId();
    }

    view_.showNamePrompt();
    std::string name;
    if (!std::getline(in_, name)) return;

    double avgTime = 0.0;
    while (true) {
        view_.showTimePrompt();
        std::string timeStr;
        if (!std::getline(in_, timeStr)) return;
        try { avgTime = std::stod(timeStr); } catch (...) { avgTime = 0.0; }
        if (avgTime > 0.0) break;
        view_.showTimeOutOfRange();
    }

    view_.showYieldPrompt();
    std::string yieldStr;
    if (!std::getline(in_, yieldStr)) return;
    double yield = 0.0;
    try { yield = std::stod(yieldStr); } catch (...) {}

    if (yield <= 0.0 || yield > 1.0) {
        view_.showYieldOutOfRange();
        return;
    }

    Sample sample{id, name, avgTime, yield, 0};
    repo_.add(sample);
    view_.showRegisterSuccess(sample);
}

void SampleController::handleList() {
    view_.showSampleList(repo_.findAll());
}

void SampleController::handleSearch() {
    view_.showSearchMenu();
    std::string criteria;
    if (!std::getline(in_, criteria)) return;
    if (criteria == "0") return;

    view_.showSearchPrompt();
    std::string term;
    if (!std::getline(in_, term)) return;

    auto all = repo_.findAll();
    std::vector<Sample> result;

    if (criteria == "1") {
        for (const auto& s : all)
            if (s.id.find(term) != std::string::npos) result.push_back(s);
    } else if (criteria == "2") {
        for (const auto& s : all)
            if (s.name.find(term) != std::string::npos) result.push_back(s);
    } else if (criteria == "3") {
        double threshold = 0.0;
        try { threshold = std::stod(term); } catch (...) {}
        for (const auto& s : all)
            if (s.yield >= threshold) result.push_back(s);
    } else {
        view_.showInvalidInput();
        return;
    }

    if (result.empty())
        view_.showNoResults();
    else
        view_.showSearchResult(result);
}
