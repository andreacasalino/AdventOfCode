#include <utils.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

using Cell = std::pair<int, int>;

struct Hasher {
    std::size_t operator()(const Cell& c) const {
        std::uint64_t res = static_cast<std::uint64_t>(c.first);
        res = res | static_cast<std::uint64_t>(c.second) << 32;
        static std::hash<std::uint64_t> h;
        return h(res);
    }
};

struct Grid {
    Grid(const std::filesystem::path& src) 
    : rows{0} { 
        utils::forEachFileLine(src, [&](std::string_view line) {
            for(int c=0; c<line.size(); ++c) {
                if(line[c] != '.') {
                    antennasAll.emplace(Cell{rows, c});   
                    antennasTable[line[c]].push_back(Cell{rows, c});
                }
            }            
            cols = line.size();    
            ++rows;
        });
    }

    template<typename Pred>
    void forEachPair(Pred pred) {
        for(const auto& [_, cells] : antennasTable) {
            for(auto it_a = cells.begin(); it_a!=cells.end(); ++it_a) {
            for(auto it_b = it_a + 1; it_b!=cells.end(); ++it_b) {
                pred(*it_a, *it_b);
            }
            }
        }
    }

    bool isInside(const Cell& cell) const {
        return 0 <= cell.first && cell.first < rows &&
               0 <= cell.second && cell.second < cols;
    }

    int rows;
    int cols;
    std::unordered_map<char, std::vector<Cell>> antennasTable;
    std::unordered_set<Cell, Hasher> antennasAll;
};

void print(const Grid& grid, const std::unordered_set<Cell, Hasher>& antinodes) {
    std::vector<std::string> tmp;
    for(int r=0; r<grid.rows; ++r) {
    auto& l = tmp.emplace_back();
    for(int c=0; c<grid.cols; ++c) {
        l += '.';
    }
    }
    for(const auto& [sym, cells] :grid.antennasTable) {
    for(auto [r,c] : cells) {
        tmp[r][c] = sym;
    }
    }
    for(auto [r,c] : antinodes) {
        tmp[r][c] = '#';
    }
    for(const auto& l : tmp) {
        std::cout << l << std::endl;
    }
}

std::size_t phase1(const std::filesystem::path& src) {
    Grid grid(src);
    std::unordered_set<Cell, Hasher> antinodes;
    auto add = [&](const Cell& toAdd) {
        if(!grid.isInside(toAdd)) {
            return;
        }
        antinodes.emplace(toAdd);
    };
    grid.forEachPair([&](const Cell& a, const Cell& b) {
        int delta_r = b.first - a.first;
        int delta_c = b.second - a.second;
        add(std::make_pair(b.first + delta_r, b.second + delta_c));
        add(std::make_pair(a.first - delta_r, a.second - delta_c));
    });
    print(grid, antinodes);
    return antinodes.size();
}

std::size_t phase2(const std::filesystem::path& src) {
    Grid grid(src);
    std::unordered_set<Cell, Hasher> antinodes;
    auto addTillOut = [&](int delta_r, int delta_c, Cell prev) {
        Cell next;
        while (true) {
            next.first = prev.first + delta_r;
            next.second = prev.second + delta_c;
            if(!grid.isInside(next)) {
                break;
            }
            if(!grid.antennasAll.contains(next)) {
                antinodes.emplace(next);
            }
            prev = next;
        }
    };
    grid.forEachPair([&](const Cell& a, const Cell& b) {
        int delta_r = b.first - a.first;
        int delta_c = b.second - a.second;
        addTillOut(delta_r, delta_c, b);
        addTillOut(-delta_r, -delta_c, a);
    });
    print(grid, antinodes);
    return antinodes.size() + grid.antennasAll.size();
}

int main() {
    // std::cout << phase1("input.txt") << std::endl;
    std::cout << phase2("input.txt") << std::endl;

    return 0;
}
