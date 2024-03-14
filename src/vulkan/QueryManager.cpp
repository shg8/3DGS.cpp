#include "QueryManager.h"

#include <functional>
#include <iostream>

uint32_t QueryManager::registerQuery(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!registry.contains(name)) {
        registry[name] = nextId++;
    }
    return registry[name];
}

uint32_t QueryManager::getQueryId(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex);
    if (registry.contains(name)) {
        return registry.at(name);
    }
    return 0;
}

std::unordered_map<std::string, uint64_t> QueryManager::parseResults(const std::vector<uint64_t>& results) {
    // all names end with _start or _end
    // calculate the time between the two
    // push the results to the results map
    // print every 1 seconds
    std::lock_guard<std::mutex> lock(mutex);
    std::unordered_map<std::string, uint64_t> resultsMap;
    for (auto& [name, id] : registry) {
        if (name.ends_with("_start")) {
            auto endName = name.substr(0, name.size() - 5) + "end";
            if (registry.contains(endName)) {
                auto start = results[id];
                auto end = results[registry[endName]];
                auto diff = end - start;
                auto truncated = name.substr(0, name.size() - 6);
                resultsMap[truncated] = diff;
            }
        }
    }
    return resultsMap;
    // auto now = std::chrono::high_resolution_clock::now();
    // if (now - lastPrint > std::chrono::seconds(1)) {
    //     lastPrint = now;
    //     for (auto& [name, result] : this->results) {
    //         auto truncated = name.substr(0, name.size() - 6);
    //         std::cout << truncated << ": ";
    //         // calculate average
    //         uint64_t sum = 0;
    //         for (auto& r : result) {
    //             sum += r;
    //         }
    //         std::cout << sum / result.size() / 1000000.0 << "ms" << std::endl;
    //     }
    // }
}
