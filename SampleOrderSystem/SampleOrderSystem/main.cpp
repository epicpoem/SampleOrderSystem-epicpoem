#include <windows.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include "controller/SampleController.h"
#include "controller/OrderController.h"
#include "controller/ApprovalController.h"
#include "controller/MonitorController.h"
#include "controller/ProductionController.h"
#include "controller/ReleaseController.h"
#include "repository/JsonSampleRepository.h"
#include "repository/JsonOrderRepository.h"
#include "service/StockService.h"
#include "view/SampleView.h"
#include "view/OrderView.h"
#include "view/ApprovalView.h"
#include "view/MonitorView.h"
#include "view/ProductionView.h"
#include "view/ReleaseView.h"
#include "util/SystemClock.h"

static void showMainMenu(JsonSampleRepository& sampleRepo,
                         const JsonOrderRepository& orderRepo,
                         StockService& stockService) {
    system("cls");

    auto samples = sampleRepo.findAll();
    auto orders  = orderRepo.findAll();

    double totalPhysStock = 0.0;
    for (const auto& s : samples)
        totalPhysStock += stockService.calcPhysicalStock(s);

    int producingCount = 0;
    for (const auto& o : orders)
        if (o.status == OrderStatus::PRODUCING) ++producingCount;

    // 현재 시간
    std::time_t now = std::time(nullptr);
    std::tm lt{};
    localtime_s(&lt, &now);
    char timeBuf[24];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &lt);

    // ASCII 배너
    std::cout << "\n";
    std::cout << "   ____       ____                  _ \n";
    std::cout << "  / ___|     / ___|  ___ _ __ ___ (_)\n";
    std::cout << "  \\___ \\     \\___ \\ / _ \\ '_ ` _ \\| |\n";
    std::cout << "   ___) |     ___) |  __/ | | | | | |\n";
    std::cout << "  |____/     |____/ \\___|_| |_| |_|_|\n";
    std::cout << "\n";
    std::cout << "           반도체 시료 생산주문관리 시스템\n";
    std::cout << std::string(68, '=') << "\n";
    std::cout << "  시스템 현황   " << timeBuf << "\n";
    std::cout << "\n";
    std::cout << "  등록 시료   " << std::setw(4) << samples.size() << "종"
              << "        총 재고    " << (long long)totalPhysStock << " ea\n";
    std::cout << "  전체 주문   " << std::setw(4) << orders.size() << "건"
              << "        생산라인   " << producingCount << "건 대기\n";
    std::cout << std::string(68, '-') << "\n";
    std::cout << "  [1] 시료 관리               [2] 시료 주문\n";
    std::cout << "  [3] 주문 승인/거절           [4] 모니터링\n";
    std::cout << "  [5] 생산라인 조회            [6] 출고 처리\n";
    std::cout << "  [0] 종료\n";
    std::cout << std::string(68, '-') << "\n";
    std::cout << "선택 > ";
    std::cout.flush();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    JsonSampleRepository sampleRepo("data/samples.json");
    JsonOrderRepository  orderRepo("data/orders.json");
    SystemClock          clock;
    StockService         stockService(sampleRepo, orderRepo, clock);

    SampleView       sampleView;
    SampleController sampleCtrl(std::cin, sampleView, sampleRepo);

    OrderView        orderView;
    OrderController  orderCtrl(std::cin, orderView, sampleRepo, orderRepo, clock);

    ApprovalView       approvalView;
    ApprovalController approvalCtrl(std::cin, approvalView, orderRepo, stockService);

    MonitorView        monitorView;
    MonitorController  monitorCtrl(std::cin, monitorView, sampleRepo, orderRepo, stockService);

    ProductionView       productionView;
    ProductionController productionCtrl(std::cin, productionView, sampleRepo, orderRepo, stockService);

    ReleaseView       releaseView(std::cout);
    ReleaseController releaseCtrl(std::cin, releaseView, sampleRepo, orderRepo, stockService);

    while (true) {
        for (const auto& no : stockService.checkAndCompleteProduction())
            std::cout << "[생산완료] " << no << " \xE2\x86\x92 CONFIRMED\n";

        showMainMenu(sampleRepo, orderRepo, stockService);

        std::string line;
        if (!std::getline(std::cin, line)) break;

        if      (line == "0") break;
        else if (line == "1") { system("cls"); sampleCtrl.run(); }
        else if (line == "2") { system("cls"); orderCtrl.run(); }
        else if (line == "3") { system("cls"); approvalCtrl.run(); }
        else if (line == "4") { system("cls"); monitorCtrl.run(); }
        else if (line == "5") { system("cls"); productionCtrl.run(); }
        else if (line == "6") { system("cls"); releaseCtrl.run(); }
        else                  std::cout << "  유효하지 않은 선택입니다.\n";
    }

    std::cout << "\n시스템을 종료합니다.\n";
    return 0;
}
