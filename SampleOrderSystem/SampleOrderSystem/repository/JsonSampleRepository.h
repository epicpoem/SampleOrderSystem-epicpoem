#pragma once
#include "ISampleRepository.h"
#include <string>

class JsonSampleRepository : public ISampleRepository {
public:
    explicit JsonSampleRepository(const std::string& filePath);

    bool add(const Sample& sample) override;
    std::vector<Sample> findAll() const override;
    std::optional<Sample> findById(const std::string& id) const override;
    bool exists(const std::string& id) const override;
    bool update(const Sample& sample) override;
    bool decreaseStock(const std::string& id, int amount) override;

private:
    std::string filePath_;
    std::vector<Sample> loadFromFile() const;
    void saveToFile(const std::vector<Sample>& samples) const;
};
