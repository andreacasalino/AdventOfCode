#include <utils.h>
#include <vector>
#include <iostream>

struct Grid {
  static Grid parse(const std::filesystem::path& src) {
    Grid res;
    utils::forEachFileLine(src, [&](std::string_view line) {
        res.data.emplace_back(line.data(), line.size());
        ++res.rows;
        res.cols = static_cast<int>(line.size());
    });
    return res;
  }

  int rows{0};
  int cols{0};
  std::vector<std::string> data;
};

bool isSymbol(const Grid& grid, int r, int c) {
    if(r < 0 || grid.rows <= r || c < 0 || grid.cols <= c) {
        return false;
    }
    char sym = grid.data[r][c];
    if('0' <= sym && sym <= '9') {
        return false;
    }
    return sym != '.';
}

template<typename Pred>
void forEachNumber(const std::string_view& src, Pred pred) {
    struct NumberView {
        int start{-1};
        int len{0};
    } view;
    for(int k=0; k<src.size(); ++k) {
        if(!('0' <= src[k] && src[k] <= '9')) {
            if(view.len != 0) {
                pred(view.start, std::string_view{src.data() + view.start, static_cast<std::size_t>(view.len)});
            }
            view = NumberView{};
            continue;
        }
        if(view.len == 0) {
            view.start = k;
        }
        ++view.len;
    }
    if(view.len != 0) {
        pred(view.start, std::string_view{src.data() + view.start, static_cast<std::size_t>(view.len)});
    }
}

std::uint64_t countEngineParts(const Grid& g, const std::string_view& src, int row) {
    std::uint64_t res{0};
    forEachNumber(src, [&](int start, std::string_view str) {
        auto add = [&]() {
            res += std::atoll(str.data());
        };

        for(int k=0; k<3; ++k) {
            if(isSymbol(g, row + k -1, start - 1)) {
                add();
                return;
            }
        }
        for(int k=0; k < static_cast<int>(str.size()); ++k) {
            if(isSymbol(g, row - 1, start + k) || 
               isSymbol(g, row + 1, start + k) ) {
                add();
                return;
            }
        }
        for(int k=0; k<3; ++k) {
            if(isSymbol(g, row + k -1, start + str.size())) {
                add();
                return;
            }
        }
    });
    return res;
}

std::uint64_t phase1(const std::filesystem::path& src) {
    auto grid = Grid::parse(src);
    std::uint64_t res{0};
    for(int r =0 ; r < grid.data.size(); ++r) {
        res += countEngineParts(grid, grid.data[r], r);
    }
    return res;
}

using Cell = std::pair<int, int>;

struct Hasher {
    std::size_t operator()(const Cell& c) const {
        std::uint64_t res = static_cast<std::uint64_t>(c.first);
        res = res | static_cast<std::uint64_t>(c.second) << 32;
        static std::hash<std::uint64_t> h;
        return h(res);
    }
};

#include <unordered_map>

struct Info {
    int number;
    int id;
};
using Numbers = std::unordered_map<Cell, Info, Hasher>;
Numbers findNumbers(const Grid& g) {
    Numbers res;
    int id{-1};
    for(int r = 0; r< g.rows; ++r) {
        forEachNumber(g.data[r], [&](int start, std::string_view str) {
            ++id;
            int val = std::atoi(str.data());
            for(int c = 0; c<str.size(); ++c) {
                res.emplace(std::make_pair(r, c + start), Info{ val , id });
            }
        });
    }
    return res;
}

std::uint64_t phase2(const std::filesystem::path& src) {
    auto grid = Grid::parse(src);
    std::uint64_t res{0};
    auto numbers = findNumbers(grid);
    for(int r = 0; r < grid.rows; ++r) {
    for(int c = 0; c < grid.cols; ++c) {
        if(grid.data[r][c] != '*') {
            continue;
        }
        std::unordered_map<int, int> found;
        auto explore = [&](int r, int c) {
            if(r < 0 || r == grid.rows || c < 0 || c == grid.cols) {
                return;
            }
            auto it = numbers.find(std::make_pair(r, c));
            if(it == numbers.end()) {
                return;
            }
            found.emplace(it->second.id, it->second.number);
        };
        explore(r + 1, c);
        explore(r, c + 1);
        explore(r - 1, c);
        explore(r, c - 1);

        explore(r + 1, c + 1);
        explore(r + 1, c - 1);
        explore(r - 1, c - 1);
        explore(r - 1, c + 1);

        if(found.size() == 2) {
            auto it = found.begin();
            std::uint64_t toAdd = it->second;
            ++it;
            toAdd *= it->second;
            res += toAdd;
        }
    }
    }
    return res;
}

int main() {
    // std::cout << phase1("sample.txt") << std::endl;
    std::cout << phase2("input.txt") << std::endl;

    return 0;
}
