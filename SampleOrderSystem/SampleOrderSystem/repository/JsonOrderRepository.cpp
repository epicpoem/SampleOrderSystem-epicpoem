#include "JsonOrderRepository.h"
#include "../nlohmann/json.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

JsonOrderRepository::JsonOrderRepository(const std::string& filePath)
    : filePath_(filePath) {
    fs::path p(filePath_);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path());
    }
}

std::vector<Order> JsonOrderRepository::loadFromFile() const {
    std::vector<Order> orders;
    if (!fs::exists(filePath_)) return orders;

    std::ifstream file(filePath_);
    if (!file.is_open()) return orders;

    try {
        json j;
        file >> j;
        for (const auto& item : j) {
            Order o;
            o.orderNo               = item.at("orderNo").get<std::string>();
            o.sampleId              = item.at("sampleId").get<std::string>();
            o.customerName          = item.at("customerName").get<std::string>();
            o.quantity              = item.at("quantity").get<int>();
            o.status                = orderStatusFromString(item.at("status").get<std::string>());
            o.actualProduction       = item.at("actualProduction").get<int>();
            o.shortageQty            = item.value("shortageQty", 0);
            o.totalProductionTimeMin = item.at("totalProductionTimeMin").get<double>();
            o.productionStartTime    = static_cast<std::time_t>(
                item.at("productionStartTime").get<long long>());
            orders.push_back(o);
        }
    } catch (...) {}

    return orders;
}

void JsonOrderRepository::saveToFile(const std::vector<Order>& orders) const {
    json j = json::array();
    for (const auto& o : orders) {
        j.push_back({
            {"orderNo",               o.orderNo},
            {"sampleId",              o.sampleId},
            {"customerName",          o.customerName},
            {"quantity",              o.quantity},
            {"status",                orderStatusToString(o.status)},
            {"actualProduction",       o.actualProduction},
            {"shortageQty",            o.shortageQty},
            {"totalProductionTimeMin", o.totalProductionTimeMin},
            {"productionStartTime",   static_cast<long long>(o.productionStartTime)}
        });
    }
    std::ofstream file(filePath_);
    file << j.dump(2);
}

bool JsonOrderRepository::add(const Order& order) {
    auto orders = loadFromFile();
    orders.push_back(order);
    saveToFile(orders);
    return true;
}

std::vector<Order> JsonOrderRepository::findAll() const {
    return loadFromFile();
}

std::vector<Order> JsonOrderRepository::findByStatus(OrderStatus status) const {
    auto all = loadFromFile();
    std::vector<Order> result;
    for (const auto& o : all)
        if (o.status == status) result.push_back(o);
    return result;
}

std::optional<Order> JsonOrderRepository::findByNo(const std::string& orderNo) const {
    auto orders = loadFromFile();
    auto it = std::find_if(orders.begin(), orders.end(),
        [&orderNo](const Order& o) { return o.orderNo == orderNo; });
    if (it != orders.end()) return *it;
    return std::nullopt;
}

bool JsonOrderRepository::update(const Order& order) {
    auto orders = loadFromFile();
    auto it = std::find_if(orders.begin(), orders.end(),
        [&order](const Order& o) { return o.orderNo == order.orderNo; });
    if (it == orders.end()) return false;
    *it = order;
    saveToFile(orders);
    return true;
}

std::string JsonOrderRepository::generateOrderNo(const std::string& date) {
    std::string prefix = "ORD-" + date + "-";
    int maxSeq = 0;
    for (const auto& o : findAll()) {
        if (o.orderNo.rfind(prefix, 0) == 0) {
            try {
                int seq = std::stoi(o.orderNo.substr(prefix.size()));
                if (seq > maxSeq) maxSeq = seq;
            } catch (...) {}
        }
    }
    char seq[5];
    std::snprintf(seq, sizeof(seq), "%04d", maxSeq + 1);
    return prefix + seq;
}
