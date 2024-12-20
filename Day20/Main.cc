#include <utils.h>

#include <iostream>
#include <map>
#include <ranges>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Cell = std::pair<int, int>;

struct Hash {
  std::size_t operator()(const Cell &c) const {
    std::uint16_t res = static_cast<std::uint16_t>(c.first);
    res = res | static_cast<std::uint16_t>(c.second) << 8;
    static std::hash<std::uint16_t> h;
    return h(res);
  }
};

struct Input {
  Input(const std::filesystem::path &src) {
    utils::forEachFileLine(src, [&](const std::string_view &line) {
      data.emplace_back(line.data(), line.size());
    });
    for (int r = 0; r < data.size(); ++r) {
      for (int c = 0; c < data.front().size(); ++c) {
        if (data[r][c] == 'S') {
          start = std::make_pair(r, c);
        }
        if (data[r][c] == 'E') {
          end = std::make_pair(r, c);
        }
      }
    }
    findNoCheatPath();
  }

  std::vector<std::string> data;
  Cell start;
  Cell end;

  std::vector<Cell> noCheatPath;

private:
  void findNoCheatPath() {
    struct Path {
      void add(const Cell &cell) { sequence.emplace_back(cell); }

      std::vector<Cell> sequence;
    };
    std::stack<Path> open;
    open.emplace().add(start);
    std::unordered_set<Cell, Hash> all_visited;
    while (!open.empty()) {
      auto top = std::move(open.top());
      open.pop();

      if (top.sequence.back() == end) {
        noCheatPath = std::move(top.sequence);
        break;
      }

      auto expl = [&](const Cell &next) {
        if (next.first < 0 || next.second < 0 || data.size() <= next.first ||
            data.front().size() <= next.second ||
            data[next.first][next.second] == '#' ||
            all_visited.contains(next)) {
          return;
        }
        open.emplace(top).add(next);
        all_visited.emplace(next);
      };

      const auto &last = top.sequence.back();
      expl(std::make_pair(last.first + 1, last.second));
      expl(std::make_pair(last.first - 1, last.second));
      expl(std::make_pair(last.first, last.second + 1));
      expl(std::make_pair(last.first, last.second - 1));
    }
    if (noCheatPath.empty()) {
      throw std::runtime_error{"no solution found"};
    }
  }
};

struct ShortCuts {
  const auto &get() const { return values; }

  virtual void maybe_add(int save, int dist) = 0;

protected:
  void add(int save) {
    if (save <= 0) {
      return;
    }
    auto it = values.find(save);
    if (it == values.end()) {
      values.emplace(save, 1);
    } else {
      ++it->second;
    }
  }

private:
  std::map<int, std::uint64_t> values;
};

template <int Len> struct ShortCutsExactDist : ShortCuts {
  void maybe_add(int save, int dist) override {
    if (Len == dist) {
      add(save);
    }
  }
};

template <typename ShortCutsT> auto computeShortCuts(const Input &input) {
  ShortCutsT cuts_map;
  auto dist = [](const Cell &a, const Cell &b) {
    return abs(a.first - b.first) + abs(a.second - b.second);
  };

  const auto &noCheat = input.noCheatPath;
  for (int k = 0; k < noCheat.size(); ++k) {
    for (int k2 = k + 2; k2 < noCheat.size(); ++k2) {
      int d = dist(noCheat[k], noCheat[k2]);
      int save = k2 - k;
      save -= d;
      cuts_map.maybe_add(save, d);
    }
  }

  return cuts_map.get();
}

std::size_t sumFromVal(const std::map<int, std::uint64_t> &cuts_map,
                       std::size_t from_val) {
  auto it = cuts_map.lower_bound(from_val);
  std::uint64_t res = 0;
  std::for_each(it, cuts_map.end(),
                [&res](const auto &el) { res += el.second; });
  return res;
}

std::uint64_t phase1_sample() {
  Input input{"sample.txt"};
  auto cuts = computeShortCuts<ShortCutsExactDist<2>>(input);
  return sumFromVal(cuts, 0);
}

std::uint64_t phase1() {
  Input input{"input.txt"};
  auto cuts = computeShortCuts<ShortCutsExactDist<2>>(input);
  return sumFromVal(cuts, 100);
}

template <int Len> struct ShortCutsTillDist : ShortCuts {
  void maybe_add(int save, int dist) override {
    if (dist <= Len) {
      add(save);
    }
  }
};

std::uint64_t phase2_sample() {
  Input input{"sample.txt"};
  auto cuts = computeShortCuts<ShortCutsTillDist<20>>(input);
  return sumFromVal(cuts, 50);
}

std::uint64_t phase2() {
  Input input{"input.txt"};
  auto cuts = computeShortCuts<ShortCutsTillDist<20>>(input);
  return sumFromVal(cuts, 100);
}

int main() {
  // std::cout << phase1_sample() << std::endl;
  // std::cout << phase1() << std::endl;

  // std::cout << phase2_sample() << std::endl;
  std::cout << phase2() << std::endl;

  return 0;
}
