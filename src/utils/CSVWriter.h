#ifndef CSVWRITER_H
#define CSVWRITER_H

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cstdint>
#include <ranges>

class CSVWriter {
private:
    std::vector<std::string> columns;
    std::ofstream file;

public:
    // Constructor that takes a vector of column names
    explicit CSVWriter(const std::string& filename) {
        // assert that the file does not exist
        if (std::ifstream(filename)) {
            throw std::runtime_error("File already exists");
        }

        file.open(filename, std::ios::out | std::ios::trunc);
    }

    CSVWriter(const CSVWriter &other) = delete;

    CSVWriter(CSVWriter &&other) noexcept
        : columns(std::move(other.columns)),
          file(std::move(other.file)) {
    }

    CSVWriter & operator=(const CSVWriter &other) = delete;

    CSVWriter & operator=(CSVWriter &&other) noexcept {
        if (this == &other)
            return *this;
        columns = std::move(other.columns);
        file = std::move(other.file);
        return *this;
    }

    // Log function to write a row of data
    void log(const std::vector<std::string>& rowData) {
        if (!file.is_open()) {
            throw std::runtime_error("File is not open");
        }
        // Write the row data
        for (size_t i = 0; i < rowData.size(); ++i) {
            file << rowData[i];
            if (i < rowData.size() - 1) file << ",";
        }
        file << "\n";
    }

    void log(const std::unordered_map<std::string, uint64_t>& row) {
        if (columns.empty()) {
            for (const auto& columnName : std::ranges::views::keys(row)) {
                columns.push_back(columnName);
            }
            log(columns);
        }

        if (!file.is_open()) {
            throw std::runtime_error("File is not open");
        }

        // Write the row data
        for (size_t i = 0; i < columns.size(); ++i) {
            file << row.at(columns[i]);
            if (i < columns.size() - 1) file << ",";
        }
        file << "\n";
    }

    // Destructor to close the file if it's open
    ~CSVWriter() {
        if (file.is_open()) {
            file.close();
        }
    }
};



#endif //CSVWRITER_H
