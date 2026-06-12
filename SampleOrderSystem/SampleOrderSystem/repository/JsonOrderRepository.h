#pragma once
#include "IOrderRepository.h"
#include <string>

class JsonOrderRepository : public IOrderRepository {
public:
    explicit JsonOrderRepository(const std::string& filePath);

    bool add(const Order& order) override;
    std::vector<Order> findAll() const override;
    std::vector<Order> findByStatus(OrderStatus status) const override;
    std::optional<Order> findByNo(const std::string& orderNo) const override;
    bool update(const Order& order) override;
    std::string generateOrderNo(const std::string& date) override;

private:
    std::string filePath_;
    std::vector<Order> loadFromFile() const;
    void saveToFile(const std::vector<Order>& orders) const;
};
