#include <queue>
#include <cstdint>
#include <utils.h>
#include <iostream>

using Queue = std::priority_queue<int, std::vector<int>, std::greater<int>>;

template<typename Pred>
void forEachPair(const std::filesystem::path& src, Pred pred) {
    utils::forEachFileLine(src, [&pred](std::string_view buffer) {
        auto sep = buffer.find("   ");
        std::string_view first{buffer.data(), sep};
        std::string_view second{buffer.data() + sep + 3};
        pred(std::atoll(first.data()), std::atoll(second.data()));
    });
}

std::pair<Queue, Queue> parseInputHeaps(const std::filesystem::path& src) {
    std::pair<Queue, Queue> res;
    forEachPair(src, [&res](int first, int second) {
        res.first.emplace(first);
        res.second.emplace(second);
    });
    return res;
}

std::uint64_t phase1() {
    auto&& [first, second]  =parseInputHeaps("input.txt");
    std::uint64_t res{0};
    int a,b;
    while (!first.empty()) {
        a = first.top();
        first.pop();
        b = second.top();
        second.pop();
        if(b < a) {
            std::swap(a, b);
        }
        res += b - a;
    }
    return res;
}

#include <unordered_map>

using Table = std::unordered_map<int, int>;

void add(Table& t, int val) {
    auto it = t.find(val);
    if(it == t.end()) {
        t.emplace(val, 1);
    }
    else {
        ++it->second;
    }
}

std::pair<Table, Table> parseInputTables(const std::filesystem::path& src) {
    std::pair<Table, Table> res;
    forEachPair(src, [&res](int first, int second) {
        add(res.first, first);
        add(res.second, second);
    });
    return res;
}

std::uint64_t phase2() {
    auto&& [first, second]  = parseInputTables("input.txt");
    std::uint64_t res{0};
    for(auto [val, freq] : first) {
        auto it = second.find(val);
        if(it != second.end()) {
            res += freq * val * it->second;
        }
    }
    return res;
}

int main() {
    std::cout << phase1() << std::endl;
    std::cout << phase2() << std::endl;

    return 0;
}
