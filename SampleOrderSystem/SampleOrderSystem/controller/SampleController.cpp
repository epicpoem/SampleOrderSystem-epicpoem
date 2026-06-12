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

bool SampleController::askCancelRegister() {
    view_.showCancelConfirmPrompt();
    std::string ans;
    if (!std::getline(in_, ans)) return true;
    if (ans == "Y" || ans == "y") {
        view_.showRegisterCancelled();
        return true;
    }
    return false;
}

void SampleController::handleRegister() {
    std::string id;
    while (true) {
        view_.showRegisterPrompt();
        if (!std::getline(in_, id)) return;
        if (id.empty()) {
            if (askCancelRegister()) return;
            continue;
        }
        if (!repo_.exists(id)) break;
        view_.showDuplicateId();
    }

    std::string name;
    while (true) {
        view_.showNamePrompt();
        if (!std::getline(in_, name)) return;
        if (!name.empty()) break;
        if (askCancelRegister()) return;
    }

    double avgTime = 0.0;
    while (true) {
        view_.showTimePrompt();
        std::string timeStr;
        if (!std::getline(in_, timeStr)) return;
        if (timeStr.empty()) {
            if (askCancelRegister()) return;
            continue;
        }
        try { avgTime = std::stod(timeStr); } catch (...) { avgTime = 0.0; }
        if (avgTime > 0.0) break;
        view_.showTimeOutOfRange();
    }

    double yield = 0.0;
    while (true) {
        view_.showYieldPrompt();
        std::string yieldStr;
        if (!std::getline(in_, yieldStr)) return;
        if (yieldStr.empty()) {
            if (askCancelRegister()) return;
            continue;
        }
        try { yield = std::stod(yieldStr); } catch (...) { yield = 0.0; }
        if (yield > 0.0 && yield <= 1.0) break;
        view_.showYieldOutOfRange();
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
