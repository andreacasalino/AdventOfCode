#include <utils.h>

#include <vector>
#include <unordered_set>

using Cell = std::pair<int, int>;

struct Hasher {
    std::size_t operator()(const Cell& c) const {
        std::uint64_t res = static_cast<std::uint64_t>(c.first);
        res = res | static_cast<std::uint64_t>(c.second) << 32;
        static std::hash<std::uint64_t> h;
        return h(res);
    }
};

using Cells = std::unordered_set<Cell, Hasher>;

struct Grid {
    Grid(const std::filesystem::path& src) {
        utils::forEachFileLine(src, [&](std::string_view line) {
            slots.emplace_back(line.data(), line.size());
        });
        rows = slots.size();
        cols = slots.begin()->size();
    }

    int rows;
    int cols;
    std::vector<std::string> slots;
};

std::uint64_t phase1_explore(const Grid& grid, const Cell& start) {
    Cells reached{start}, update;
    char next = '1';
    for (int h=0; h<9 && !reached.empty(); ++h, ++next) {
        update.clear();
        auto visit = [&](int r, int c) {
            if(r < 0 || c < 0 || grid.rows <= r || grid.cols <= c ||
               grid.slots[r][c] != next) {
                return;
            }
            update.emplace(std::make_pair(r, c));
        };

        for(const auto& prev : reached) {
            visit(prev.first + 1, prev.second);
            visit(prev.first, prev.second + 1);
            visit(prev.first - 1, prev.second);
            visit(prev.first, prev.second - 1);
        }
        reached = std::move(update);
    }
    return reached.empty() ? 0 : reached.size();
}

#include <unordered_map>

std::uint64_t phase2_explore(const Grid& grid, const Cell& start) {
    std::unordered_map<Cell, std::uint64_t, Hasher> reached, update;
    reached.emplace(start, 1);
    char next = '1';
    for (int h=0; h<9 && !reached.empty(); ++h, ++next) {
        update.clear();
        auto visit = [&](int r, int c, std::uint64_t delta) {
            if(r < 0 || c < 0 || grid.rows <= r || grid.cols <= c ||
               grid.slots[r][c] != next) {
                return;
            }
            Cell to_add = std::make_pair(r, c);
            auto it = update.find(to_add);
            if(it == update.end()) {
                update.emplace(to_add, delta);
            }
            else {
                it->second += delta;
            }
        };

        for(const auto& [prev, how_many] : reached) {
            visit(prev.first + 1, prev.second, how_many);
            visit(prev.first, prev.second + 1, how_many);
            visit(prev.first - 1, prev.second, how_many);
            visit(prev.first, prev.second - 1, how_many);
        }
        reached = std::move(update);
    }
    if(reached.empty()) {
        return 0;
    }
    std::uint64_t res = 0;
    for(const auto& [_, c] : reached) {
        res += c;
    }
    return res;
}

template<typename Pred>
std::uint64_t forEachTrail(Grid&& grid, Pred scoreEval) {
    std::uint64_t res{0};
    for(int r =0 ;r<grid.rows; ++r) {
    for(int c =0 ;c<grid.cols; ++c) {
        if(grid.slots[r][c] == '0') {
            res += scoreEval(grid, std::make_pair(r, c));
        }
    }
    }
    return res;
}

#include <iostream>

int main() {
    std::cout << forEachTrail(Grid{"sample.txt"}, &phase1_explore) << std::endl;
    std::cout << forEachTrail(Grid{"input.txt"}, &phase2_explore) << std::endl;

    return 0;
}
