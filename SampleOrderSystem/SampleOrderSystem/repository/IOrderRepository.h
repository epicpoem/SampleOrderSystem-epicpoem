#pragma once
#include <vector>
#include <optional>
#include <string>
#include "../model/Order.h"

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;
    virtual bool add(const Order& order) = 0;
    virtual std::vector<Order> findAll() const = 0;
    virtual std::vector<Order> findByStatus(OrderStatus status) const = 0;
    virtual std::optional<Order> findByNo(const std::string& orderNo) const = 0;
    virtual bool update(const Order& order) = 0;
    virtual std::string generateOrderNo(const std::string& date) = 0;
};
