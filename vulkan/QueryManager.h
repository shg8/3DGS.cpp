#ifndef QUERYMANAGER_H
#define QUERYMANAGER_H
#include <mutex>
#include <unordered_map>
#include <sys/types.h>
#include <vector>
#include <chrono>
#include <string>


class QueryManager {
public:
    uint32_t registerQuery(const std::string &name);
    [[nodiscard]] uint32_t getQueryId(const std::string &name);

    std::unordered_map<std::string, uint64_t> parseResults(const std::vector<uint64_t>& results);
    int nextId = 0;
private:
    std::mutex mutex;
    std::unordered_map<std::string, uint32_t> registry;
    std::unordered_map<std::string, std::vector<uint64_t>> results;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPrint;
};



#endif //QUERYMANAGER_H
