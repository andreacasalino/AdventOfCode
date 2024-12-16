#include <utils.h>

#include <array>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct State {
  int row;
  int col;
  std::uint8_t dir;

  bool operator==(const State &o) const {
    return row == o.row && col == o.col && dir == o.dir;
  }
};

struct Hash {
  std::size_t operator()(const State &s) const {
    std::uint32_t res = static_cast<std::uint32_t>(s.dir);
    res = res | static_cast<std::uint8_t>(s.row) << 2;
    res = res | static_cast<std::uint8_t>(s.col) << 10;
    static std::hash<std::uint32_t> h;
    return h(res);
  }
};

struct OpenSet {
  OpenSet() = default;

  bool empty() const { return map.empty(); }

  auto extractTop() {
    auto it = map.begin();
    auto top = *map.begin();
    table.erase(it->second);
    map.erase(it);
    return top;
  }

  void update(std::uint64_t cost, const State &reached) {
    auto it_table = table.find(reached);
    if (it_table == table.end()) {
      table[reached] = map.emplace(cost, reached);
      return;
    }
    auto it = it_table->second;
    if (cost < it->first) {
      map.erase(it);
      table[reached] = map.emplace(cost, reached);
    }
  }

private:
  using Map = std::multimap<std::uint64_t, State>;
  std::unordered_map<State, typename Map::iterator, Hash> table;
  Map map;
};

struct Grid {
  static Grid parse(const std::filesystem::path &src) {
    Grid res;
    std::string buffer;
    utils::forEachFileLine(src, [&res, &buffer](const std::string_view line) {
      buffer = {line.data(), line.size()};
      if (auto index = line.find('S'); index != std::string::npos) {
        res.start = std::make_pair(res.data.size(), index);
        buffer[index] = '.';
      }
      if (auto index = line.find('E'); index != std::string::npos) {
        res.end = std::make_pair(res.data.size(), index);
        buffer[index] = '.';
      }
      res.data.emplace_back(std::move(buffer));
    });
    return res;
  }

  template <typename Pred>
  void forEachNext(const State &from, std::uint64_t fromCost, Pred pred) const {
    static std::array<Info, 4> alternatives{Info{0}, Info{1}, Info{2}, Info{3}};
    for (const auto &next : alternatives[from.dir].next) {
      State n = from;
      n.row += next.delta_r;
      n.col += next.delta_c;
      if (n.row < 0 || data.size() <= n.row || n.col < 0 ||
          data.front().size() <= n.col || data[n.row][n.col] == '#') {
        continue;
      }
      n.dir = next.dir_after;
      std::uint64_t cost = fromCost + 1;
      if (n.dir != from.dir) {
        cost += 1000;
      }
      pred(n, cost);
    }
  }

  template <typename Pred> void forEachPrev(const State &to, Pred pred) const {
    State prev = to;
    switch (to.dir) {
    case 0: {
      --prev.col;
      if (prev.col < 0) {
        return;
      }
      pred(prev);
      prev.dir = 1;
      pred(prev);
      prev.dir = 3;
      pred(prev);
    } break;
    case 2: {
      ++prev.col;
      if (data.front().size() <= prev.col) {
        return;
      }
      pred(prev);
      prev.dir = 1;
      pred(prev);
      prev.dir = 3;
      pred(prev);
    } break;

    case 1: {
      ++prev.row;
      if (data.size() <= prev.row) {
        return;
      }
      pred(prev);
      prev.dir = 0;
      pred(prev);
      prev.dir = 2;
      pred(prev);
    } break;
    default: {
      --prev.row;
      if (prev.row < 0) {
        return;
      }
      pred(prev);
      prev.dir = 0;
      pred(prev);
      prev.dir = 2;
      pred(prev);
    } break;
    }
  }

  std::pair<int, int> start;
  std::pair<int, int> end;
  std::vector<std::string> data;

private:
  struct Info {
    Info(std::uint8_t fromDir) {
      switch (fromDir) {
      case 0:
        next = {Next{3}, Next{0}, Next{1}};
        break;
      case 1:
        next = {Next{0}, Next{1}, Next{2}};
        break;
      case 2:
        next = {Next{1}, Next{2}, Next{3}};
        break;
      default:
        next = {Next{2}, Next{3}, Next{0}};
        break;
      }
    }

    struct Next {
      Next() = default;

      Next(std::uint8_t dir) : dir_after{dir} {
        switch (dir_after) {
        case 0:
          delta_c = 1;
          break;
        case 1:
          delta_r = -1;
          break;
        case 2:
          delta_c = -1;
          break;
        default:
          delta_r = 1;
          break;
        }
      }

      int delta_r{0};
      int delta_c{0};
      std::uint8_t dir_after;
    };
    std::array<Next, 3> next;
  };
};

std::uint64_t phase1(const std::filesystem::path &src) {
  auto grid = Grid::parse(src);
  OpenSet set;
  std::uint64_t res = 0;
  set.update(0, State{grid.start.first, grid.start.second, 0});
  while (!set.empty()) {
    auto [cost, reached] = set.extractTop();
    if (reached.row == grid.end.first && reached.col == grid.end.second) {
      res = cost;
      break;
    }
    grid.forEachNext(reached, cost,
                     [&](const State &next, std::uint64_t next_cost) {
                       set.update(next_cost, next);
                     });
  }
  if (res == 0) {
    throw std::runtime_error{"path not found"};
  }
  return res;
}

std::uint64_t phase2(const std::filesystem::path &src) {
  auto grid = Grid::parse(src);
  std::unordered_map<State, std::uint64_t, Hash> visited;
  OpenSet set;
  State start{grid.start.first, grid.start.second, 0};
  set.update(0, start);
  while (!set.empty()) {
    auto [cost, reached] = set.extractTop();
    if (visited.contains(reached)) {
      continue;
    }
    visited.emplace(reached, cost);
    grid.forEachNext(reached, cost,
                     [&](const State &next, std::uint64_t next_cost) {
                       set.update(next_cost, next);
                     });
  }
  std::uint64_t min_cost = std::numeric_limits<std::uint64_t>::max();
  for (const auto &[state, cost] : visited) {
    if (state.row == grid.end.first && state.col == grid.end.second) {
      min_cost = std::min<std::uint64_t>(min_cost, cost);
    }
  }

  std::unordered_map<int, std::unordered_set<int>> occupied;
  std::unordered_map<State, std::uint64_t, Hash> level, update;
  for (const auto &[state, cost] : visited) {
    if (state.row == grid.end.first && state.col == grid.end.second) {
      if (min_cost == cost) {
        level.emplace(state, 0);
      }
    }
  }
  while (!level.empty()) {
    update.clear();
    for (const auto &[prev, cost] : level) {
      occupied[prev.row].emplace(prev.col);
      if (prev.row == grid.start.first && prev.col == grid.start.second) {
        continue;
      }
      grid.forEachPrev(prev, [&](const State &next) {
        if (update.contains(next)) {
          return;
        }
        std::uint64_t cost_next = cost + 1;
        if (next.dir != prev.dir) {
          cost_next += 1000;
        }
        auto it = visited.find(next);
        if (it == visited.end()) {
          return;
        }
        std::uint64_t cost_tot = it->second + cost_next;
        if (cost_tot == min_cost) {
          update.emplace(next, cost_next);
        }
      });
    }
    level = std::move(update);
  }

  std::uint64_t res = 0;
  for (const auto &[_, elements] : occupied) {
    res += elements.size();
  }
  return res;
}

int main() {
  //   std::cout << phase1("sample.B.txt") << std::endl;
  // std::cout << phase1("sample.B.txt") << std::endl;

  std::cout << phase2("input.txt") << std::endl;

  return 0;
}
