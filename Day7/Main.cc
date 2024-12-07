#include <utils.h>
#include <stack>
#include <vector>
#include <algorithm>
#include <span>
#include <iostream>
#include <math.h>

struct Element {
    int val;
    int digits;
};
std::pair<std::uint64_t, std::vector<Element>> parse_line(std::string_view line) {
    std::pair<std::uint64_t, std::vector<Element>> res;
    res.first = std::atoll(line.data());
    auto sep = line.find(':');
    line = std::string_view(line.data() + sep + 1);
    while (!line.empty()) {
        int val = std::atoi(line.data() + 1);
        sep = line.find(' ', 1);
        if(sep == std::string::npos) {
            res.second.emplace_back(Element{val, static_cast<int>(line.size() - 1)});
            break;
        }
        res.second.emplace_back(Element{val, static_cast<int>(sep - 1)});
        line = std::string_view{line.data() + sep};
    }
    return res;
}

struct Sol {
    Sol(const std::span<Element>& el)
    : remaining{el.begin() + 1, el.end()}
    , tot{ static_cast<std::uint64_t>(el.front().val)} {
    }

    void sumNext() {
        auto next = shift();
        tot += next.val;
    }

    void multNext() {
        auto next = shift();
        tot *= next.val;
    }

    void concatNext() {
        auto next = shift();
        tot = tot * pow(10, next.digits);
        tot += next.val;
    }

    std::span<Element> remaining;
    std::uint64_t tot;

private:
    Element shift() {
        auto next = remaining.front();
        remaining = std::span<Element>{remaining.begin() + 1, remaining.end()};
        return next;
    } 
};

template<bool UseConcat, bool Prune>
bool isObtainable(std::span<Element> elements, std::uint64_t target) {
    std::stack<Sol> open;
    open.emplace(elements);
    while (!open.empty()) {
        auto top = std::move(open.top());
        open.pop();
        if(top.remaining.empty()) {
            if(top.tot == target) {
                return true;
            }
            continue;
        }
        if constexpr (Prune) {
            if(target < top.tot) {
                continue;
            }
        }
        open.emplace(top).sumNext();
        if constexpr (UseConcat) {
            open.emplace(top).concatNext();
        }
        open.emplace(std::move(top)).multNext();
    }
    return false;
}

std::uint64_t phase1(const std::filesystem::path& src) {
    std::uint64_t res{0};
    utils::forEachFileLine(src, [&res](std::string_view line) {
        auto [target, vals] = parse_line(line);
        if(isObtainable<false, true>(std::span<Element>{vals.begin(), vals.end()}, target)) {
            res += target;
        }
    });
    return res;
}

std::uint64_t phase2(const std::filesystem::path& src) {
    std::uint64_t res{0};
    utils::forEachFileLine(src, [&res](std::string_view line) {
        auto [target, vals] = parse_line(line);
        if(isObtainable<true, true>(std::span<Element>{vals.begin(), vals.end()}, target)) {
            res += target;
        }
    });
    return res;
}

int main() {
    // std::cout << phase1("input.txt") << std::endl;
    std::cout << phase2("input.txt") << std::endl;

    return 0;
}
