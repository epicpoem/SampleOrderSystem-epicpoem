#include "SampleView.h"
#include <iostream>
#include <iomanip>
#include <sstream>

SampleView::SampleView(std::ostream& out) : out_(out) {}

void SampleView::showMenu() {
    out_ << std::string(70, '=') << "\n";
    out_ << "  [1] 시료 관리\n";
    out_ << std::string(70, '-') << "\n";
    out_ << "  [1] 시료 등록   [2] 시료 목록   [3] 시료 검색   [0] 뒤로\n";
    out_ << std::string(70, '-') << "\n";
    out_ << "선택 > ";
    out_.flush();
}

void SampleView::showInvalidInput() {
    out_ << "[오류] 유효하지 않은 입력입니다.\n";
}

void SampleView::showRegisterPrompt() {
    out_ << std::string(70, '-') << "\n";
    out_ << "[시료 등록]\n";
    out_ << "시료 ID             > ";
}

void SampleView::showNamePrompt() {
    out_ << "이름                > ";
}

void SampleView::showTimePrompt() {
    out_ << "평균 생산시간(min/ea, 소수점 입력 가능) > ";
}

void SampleView::showYieldPrompt() {
    out_ << "수율(0.0~1.0)       > ";
}

void SampleView::showYieldOutOfRange() {
    out_ << "[오류] 수율은 0.0 초과 1.0 이하여야 합니다.\n";
}

void SampleView::showTimeOutOfRange() {
    out_ << "[오류] 평균 생산시간은 0 초과여야 합니다.\n";
}

void SampleView::showDuplicateId() {
    out_ << "[오류] 이미 존재하는 시료 ID입니다. 다시 입력해 주세요.\n";
}

void SampleView::showRegisterSuccess(const Sample& sample) {
    out_ << "\n[완료] 시료가 등록되었습니다.\n";
    out_ << "  ID: " << sample.id
         << "  이름: " << sample.name
         << "  평균생산시간: " << std::fixed << std::setprecision(4) << sample.avgProductionTime << " min/ea"
         << "  수율: " << std::defaultfloat << sample.yield << "\n";
}

void SampleView::showSampleList(const std::vector<Sample>& samples) {
    out_ << std::string(70, '-') << "\n";
    out_ << "등록 시료 목록  (총 " << samples.size() << "종)\n";
    if (samples.empty()) {
        out_ << "  등록된 시료가 없습니다.\n";
        return;
    }
    out_ << "\n" << std::left
         << std::setw(10) << "ID"
         << std::setw(32) << "시료명"
         << std::setw(26) << "평균 생산시간"
         << std::setw(10) << "수율"
         << "현재 재고\n";
    out_ << std::string(70, '-') << "\n";
    for (const auto& s : samples) {
        std::ostringstream timeStr;
        timeStr << std::fixed << std::setprecision(1) << s.avgProductionTime << " min/ea";
        out_ << std::setw(10) << s.id
             << std::setw(32) << s.name
             << std::setw(21) << timeStr.str()
             << std::setw(10) << s.yield
             << s.stock << " ea\n";
    }
}

void SampleView::showSearchMenu() {
    out_ << std::string(70, '-') << "\n";
    out_ << "[시료 검색] 검색 기준을 선택하세요.\n";
    out_ << "  [1] ID로 검색   [2] 이름으로 검색   [3] 수율로 검색 (이상)   [0] 뒤로\n";
    out_ << "선택 > ";
    out_.flush();
}

void SampleView::showSearchPrompt() {
    out_ << "검색어 > ";
}

void SampleView::showSearchResult(const std::vector<Sample>& samples) {
    out_ << "[검색 결과]  " << samples.size() << "건\n";
    showSampleList(samples);
}

void SampleView::showNoResults() {
    out_ << "  검색 결과가 없습니다.\n";
}

void SampleView::showCancelConfirmPrompt() {
    out_ << "\xEC\x8B\x9C\xEB\xA3\x8C \xEB\x93\xB1\xEB\xA1\x9D\xEC\x9D\x84 \xEC\xB7\xA8\xEC\x86\x8C\xED\x95\x98\xEC\x8B\x9C\xEA\xB2\xA0\xEC\x8A\xB5\xEB\x8B\x88\xEA\xB9\x8C? [Y/N] > ";
    out_.flush();
}

void SampleView::showRegisterCancelled() {
    out_ << "[\xEC\xB7\xA8\xEC\x86\x8C] \xEC\x8B\x9C\xEB\xA3\x8C \xEB\x93\xB1\xEB\xA1\x9D\xEC\x9D\xB4 \xEC\xB7\xA8\xEC\x86\x8C\xEB\x90\x98\xEC\x97\x88\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
}
