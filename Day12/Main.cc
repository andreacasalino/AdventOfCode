#include <utils.h>

#include <vector>
#include <unordered_set>

using Cell = std::pair<int, int>;
struct Hash {
    size_t operator()(const Cell& cell) const {
        std::uint64_t res = static_cast<std::uint64_t>(cell.first);
        res  = res | static_cast<std::uint64_t>(cell.second) << 32;
        static std::hash<std::uint64_t> h;
        return h(res);
    }
};
using Cells = std::unordered_set<Cell, Hash>;

struct Grid {
    Grid(const std::filesystem::path& src) {
        utils::forEachFileLine(src, [&](std::string_view line) {
            ++rows;
            cells.emplace_back(line.data(), line.size());
        });
        cols = cells.front().size();
    }

    int rows{0};
    int cols;
    std::vector<std::string> cells;
};

Cells explore_island(int r0, int c0, const Grid& grid) {
    Cells res;
    char sym = grid.cells[r0][c0];
    Cells step, update;
    step.emplace(std::make_pair(r0, c0));
    while (!step.empty()) {
        res.insert(step.begin(), step.end());
        update.clear();
        for(const auto& prev : step) {
            auto expl = [&](int r, int c) {
                Cell cell = std::make_pair(r, c);
                if(r < 0 || c < 0 || grid.rows <= r || grid.cols <= c ||
                   grid.cells[r][c] != sym || res.contains(cell)) {
                    return;
                }
                update.emplace(cell);
            };
            expl(prev.first + 1, prev.second);
            expl(prev.first - 1, prev.second);
            expl(prev.first, prev.second + 1);
            expl(prev.first, prev.second - 1);
        }
        step = std::move(update);
    }    
    return res;
}

template<typename Pred>
void forEachIsland(const Grid& grid, Pred pred) {
    Cells visited;
    for(int r=0; r<grid.rows; ++r) {
    for(int c=0; c<grid.cols; ++c) {
        if(visited.contains(std::make_pair(r, c))) {
            continue;
        }
        auto island = explore_island(r, c, grid);
        pred(island);
        visited.insert(island.begin(), island.end());
    }
    }
}

template<typename Pred>
void forEachPerimeterSide(const Cells& cells, Pred pred) {
    enum class PerimeterSide { R, U, L, D };
    for(const auto& cell : cells) {
        auto expl = [&](Cell cell, PerimeterSide side) {
            if(cells.contains(cell)) {
                return;
            }
            switch (side) {
            case PerimeterSide::D:
                --cell.first;
                pred(std::make_pair(cell.first + 1, cell.second), std::make_pair(cell.first + 1, cell.second + 1));
                break;
            case PerimeterSide::U:
                ++cell.first;
                pred(std::make_pair(cell.first, cell.second), std::make_pair(cell.first, cell.second + 1));
                break;
            case PerimeterSide::R:
                --cell.second;
                pred(std::make_pair(cell.first, cell.second + 1), std::make_pair(cell.first + 1, cell.second + 1));
                break;
            case PerimeterSide::L:
                ++cell.second;
                pred(std::make_pair(cell.first, cell.second), std::make_pair(cell.first + 1, cell.second));
                break;
            }
        };
        expl(std::make_pair(cell.first + 1, cell.second), PerimeterSide::D);
        expl(std::make_pair(cell.first - 1, cell.second), PerimeterSide::U);
        expl(std::make_pair(cell.first, cell.second + 1), PerimeterSide::R);
        expl(std::make_pair(cell.first, cell.second - 1), PerimeterSide::L);
    }
}

std::uint64_t phase1(Grid&& grid) {
    std::uint64_t res{0};

    auto perimeter_len = [](const Cells& cells) {
        std::uint64_t res = 0;
        forEachPerimeterSide(cells, [&res] (const Cell& , const Cell& ){
            ++res;
        });
        return res;
    };

    forEachIsland(grid, [&](const Cells& island) {
        auto area = island.size();
        auto perimeter = perimeter_len(island);
        res += area * perimeter;
    });
    return res;   
}

#include <unordered_map>
#include <algorithm>    

std::uint64_t phase2_perimeter_len(const Cells& island) {
    struct EdgeProximity {
        std::uint8_t horz{0};
        std::uint8_t vert{0};
    };
    std::unordered_map<Cell, EdgeProximity, Hash> points;
    forEachPerimeterSide(island, [&](const Cell& a, const Cell& b) {
        if(a.first == b.first) {
            ++points[a].horz;
            ++points[b].horz;
        }
        else {
            ++points[a].vert;
            ++points[b].vert;
        }
    });
    std::uint64_t res = 0;
    for(const auto& [_, info] : points) {
        if(info.horz == 1 && info.vert == 1) {
            ++res;
        }
        else if(info.horz == 2 && info.vert == 2) {
            res += 2;
        }
    }
    return res;
}

std::uint64_t phase2(Grid&& grid) {
    std::uint64_t res{0};

    forEachIsland(grid, [&](const Cells& island) {
        auto area = island.size();
        auto perimeter = phase2_perimeter_len(island);
        res += area * perimeter;
    });
    return res;   
}

#include <iostream>

int main() {
    // std::cout << phase1(Grid{"input.txt"}) << std::endl;
    std::cout << phase2(Grid{"input.txt"}) << std::endl;

    return 0;
}
