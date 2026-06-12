#include "JsonSampleRepository.h"
#include "../nlohmann/json.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

JsonSampleRepository::JsonSampleRepository(const std::string& filePath)
    : filePath_(filePath) {
    fs::path p(filePath_);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path());
    }
}

std::vector<Sample> JsonSampleRepository::loadFromFile() const {
    std::vector<Sample> samples;
    if (!fs::exists(filePath_)) return samples;

    std::ifstream file(filePath_);
    if (!file.is_open()) return samples;

    try {
        json j;
        file >> j;
        for (const auto& item : j) {
            Sample s;
            s.id                = item.at("id").get<std::string>();
            s.name              = item.at("name").get<std::string>();
            s.avgProductionTime = item.at("avgProductionTime").get<double>();
            s.yield             = item.at("yield").get<double>();
            s.stock             = item.at("stock").get<int>();
            samples.push_back(s);
        }
    } catch (...) {}

    return samples;
}

void JsonSampleRepository::saveToFile(const std::vector<Sample>& samples) const {
    json j = json::array();
    for (const auto& s : samples) {
        j.push_back({
            {"id",                s.id},
            {"name",              s.name},
            {"avgProductionTime", s.avgProductionTime},
            {"yield",             s.yield},
            {"stock",             s.stock}
        });
    }
    std::ofstream file(filePath_);
    file << j.dump(2);
}

bool JsonSampleRepository::add(const Sample& sample) {
    if (exists(sample.id)) return false;
    auto samples = loadFromFile();
    samples.push_back(sample);
    saveToFile(samples);
    return true;
}

std::vector<Sample> JsonSampleRepository::findAll() const {
    return loadFromFile();
}

std::optional<Sample> JsonSampleRepository::findById(const std::string& id) const {
    auto samples = loadFromFile();
    auto it = std::find_if(samples.begin(), samples.end(),
        [&id](const Sample& s) { return s.id == id; });
    if (it != samples.end()) return *it;
    return std::nullopt;
}

bool JsonSampleRepository::exists(const std::string& id) const {
    return findById(id).has_value();
}

bool JsonSampleRepository::update(const Sample& sample) {
    auto samples = loadFromFile();
    auto it = std::find_if(samples.begin(), samples.end(),
        [&sample](const Sample& s) { return s.id == sample.id; });
    if (it == samples.end()) return false;
    *it = sample;
    saveToFile(samples);
    return true;
}

bool JsonSampleRepository::decreaseStock(const std::string& id, int amount) {
    auto samples = loadFromFile();
    auto it = std::find_if(samples.begin(), samples.end(),
        [&id](const Sample& s) { return s.id == id; });
    if (it == samples.end()) return false;
    if (it->stock < amount) return false;
    it->stock -= amount;
    saveToFile(samples);
    return true;
}
