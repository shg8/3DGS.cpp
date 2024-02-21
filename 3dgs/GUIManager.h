#ifndef GUIMANAGER_H
#define GUIMANAGER_H
#include <unordered_map>
#include <vector>
#include <string>


#include "imgui.h"
#include "implot/implot.h"

template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t size = 600) : buffer(size), start(0), end(0), isFull(false) {}

    void push(T value) {
        buffer[end] = value;
        end = (end + 1) % buffer.size();

        if (isFull) {
            start = (start + 1) % buffer.size();
        }

        isFull = end == start;
    }

    T pop() {
        if (isEmpty()) {
            throw std::runtime_error("Buffer is empty");
        }

        T value = buffer[start];
        start = (start + 1) % buffer.size();
        isFull = false;

        return value;
    }

    bool isEmpty() const {
        return !isFull && (end == start);
    }

    bool isBufferFull() const {
        return isFull;
    }

    size_t capacity() const {
        return buffer.size();
    }

    size_t size() const {
        size_t size = buffer.size();

        if (!isFull) {
            if (end >= start) {
                size = end - start;
            } else {
                size = buffer.size() + end - start;
            }
        }

        return size;
    }

    T lookup(size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("Index out of range");
        }

        return buffer[(start + index) % buffer.size()];
    }

private:
    std::vector<T> buffer;
    size_t start;
    size_t end;
    bool isFull;
};

struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = 2000) {
        MaxSize = max_size;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void addPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void clear() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};

class GUIManager {
public:
    GUIManager();

    static void init();

    static void buildGui();

    static void pushMetric(const std::string& name, float value);

    static void pushMetric(const std::unordered_map<std::string, float>& name);

};

#endif //GUIMANAGER_H
