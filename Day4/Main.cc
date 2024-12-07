#include <utils.h>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <math.h>

std::vector<int> parseVals(std::string_view line) {
    std::vector<int> res;
    while (!line.empty()) {
        for(; line.front() == ' '; line = std::string_view{line.data() + 1, line.size() - 1}) {
        }
        res.push_back(std::atoi(line.data()));
        auto sep = line.find(' ');
        if(sep == std::string::npos) {
            break;
        }
        line = std::string_view{line.data() + sep, line.size() - sep};
    }
    return res;
}

struct Line {
    static Line parse(std::string_view line) {
        auto pipe = line.find('|');
        std::string_view first{line.data(), pipe -1};
        std::string_view second{line.data() + pipe + 1};

        auto comma = first.find(':');
        first = std::string_view{first.data() + comma + 1, first.size() - comma - 1};

        Line res;
        auto w = parseVals(first);
        res.winners = {w.begin(), w.end()};
        res.val = parseVals(second);
        return res;
    }

    std::unordered_set<int> winners;
    std::vector<int> val;
};

std::uint64_t phase1(const std::filesystem::path& src) {
    std::uint64_t res{0};
    utils::forEachFileLine(src, [&res](std::string_view line) {
        auto [winners, values] = Line::parse(line);
        std::size_t w = std::count_if(values.begin(), values.end(), [&winners](int v) {
            return winners.contains(v);
        });
        if(0 < w) {
            res += pow(2, w - 1);
        }
    });
    return res;
}

#include <deque>

std::uint64_t phase2(const std::filesystem::path& src) {
    std::vector<int> cache;

    int r = 0;
    std::deque<int> open;
    utils::forEachFileLine(src, [&](std::string_view line) {
        auto [winners, values] = Line::parse(line);
        auto w_count =  static_cast<int>(std::count_if(values.begin(), values.end(), [&winners](int v) {
            return winners.contains(v);
        }));
        cache.push_back(w_count);
        open.push_back(r);
        ++r;
    });

    std::uint64_t collected{0};
    while (!open.empty()) {
        ++collected;
        auto front = open.front();
        open.pop_front();
        auto next = cache[front];
        for(int k =0 ; k< next && front + k + 1 < cache.size(); ++k) {
            open.push_back(front + k + 1);
        }
    }
    return collected;
}

#include <iostream>

int main() {
    // std::cout << phase1("input.txt") << std::endl;
    std::cout << phase2("input.txt") << std::endl;

    return 0;
}
