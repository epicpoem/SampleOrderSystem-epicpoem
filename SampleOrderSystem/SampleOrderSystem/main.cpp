#include <windows.h>
#include <iostream>
#include "controller/SampleController.h"
#include "repository/JsonSampleRepository.h"
#include "repository/JsonOrderRepository.h"
#include "view/SampleView.h"
#include "util/SystemClock.h"

static void showMainMenu(const JsonSampleRepository& sampleRepo,
                         const JsonOrderRepository& orderRepo) {
    auto samples = sampleRepo.findAll();
    auto orders  = orderRepo.findAll();

    int totalStock = 0;
    for (const auto& s : samples) totalStock += s.stock;

    int producingCount = 0;
    for (const auto& o : orders)
        if (o.status == OrderStatus::PRODUCING) ++producingCount;

    std::cout << "\n===================================================================\n";
    std::cout << "        반도체 시료 생산주문관리 시스템  (S-Semi)\n";
    std::cout << "===================================================================\n";
    std::cout << "  등록 시료  " << samples.size() << "종"
              << "       총 재고    " << totalStock << " ea\n";
    std::cout << "  전체 주문  " << orders.size() << "건"
              << "       생산라인   " << producingCount << "건 대기\n";
    std::cout << "-------------------------------------------------------------------\n";
    std::cout << " [1] 시료 관리          [2] 시료 주문\n";
    std::cout << " [3] 주문 승인/거절     [4] 모니터링\n";
    std::cout << " [5] 생산라인 조회      [6] 출고 처리\n";
    std::cout << " [0] 종료\n";
    std::cout << "-------------------------------------------------------------------\n";
    std::cout << "선택 > ";
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    JsonSampleRepository sampleRepo("data/samples.json");
    JsonOrderRepository  orderRepo("data/orders.json");
    SystemClock          clock;

    SampleView       sampleView;
    SampleController sampleCtrl(std::cin, sampleView, sampleRepo);

    while (true) {
        showMainMenu(sampleRepo, orderRepo);

        std::string line;
        if (!std::getline(std::cin, line)) break;

        if      (line == "0") break;
        else if (line == "1") sampleCtrl.run();
        else if (line == "2") std::cout << "  [미구현] 시료 주문\n";
        else if (line == "3") std::cout << "  [미구현] 주문 승인/거절\n";
        else if (line == "4") std::cout << "  [미구현] 모니터링\n";
        else if (line == "5") std::cout << "  [미구현] 생산라인 조회\n";
        else if (line == "6") std::cout << "  [미구현] 출고 처리\n";
        else                  std::cout << "  유효하지 않은 선택입니다.\n";
    }

    std::cout << "\n시스템을 종료합니다.\n";
    return 0;
}
