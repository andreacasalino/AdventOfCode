#include <utils.h>

#include <iostream>
#include <unordered_set>
#include <vector>

using Cell = std::pair<int, int>;

struct Hash {
  std::size_t operator()(const Cell &cell) const {
    std::uint16_t res = static_cast<std::uint16_t>(cell.first);
    res = res | static_cast<std::uint16_t>(cell.second) << 8;
    static std::hash<std::uint16_t> h;
    return h(res);
  }
};

struct Obstacles {
  Obstacles(const std::filesystem::path &src) {
    utils::forEachFileLine(src, [&](std::string_view line) {
      auto sep = line.find(',');
      Cell cell;
      cell.second = std::atoi(line.data());
      cell.first = std::atoi(line.data() + sep + 1);
      obstacles.emplace_back(cell);
    });
  }

  std::pair<int, Cell> next() {
    if (index == obstacles.size()) {
      throw std::runtime_error{"no more obstacles"};
    }
    const auto &res = obstacles[index++];
    return std::make_pair(index - 1, res);
  }

private:
  int index = 0;
  std::vector<Cell> obstacles;
};

struct Grid {
  Grid(Obstacles &ob, std::size_t fallen, int size) : rows{size}, cols{size} {
    for (std::size_t k = 0; k < fallen; ++k) {
      auto [_, obst] = ob.next();
      obstacles.emplace(obst);
    }
  }

  void add(const Cell &ob) { obstacles.emplace(ob); }

  bool check(const Cell &cell) const {
    auto [r, c] = cell;
    return (0 <= r) && (0 <= c) && (r < rows) && (c < cols) &&
           !obstacles.contains(cell);
  }

  Cell target() const { return std::make_pair(rows - 1, cols - 1); }

private:
  int rows;
  int cols;
  std::unordered_set<Cell, Hash> obstacles;
};

int find_best_path_len(Grid &grid) {
  auto trgt = grid.target();
  int steps = 0;
  std::unordered_set<Cell, Hash> reached, update, visited;
  reached.emplace(std::make_pair(0, 0));
  while (!reached.empty() && !reached.contains(trgt)) {
    ++steps;
    visited.insert(reached.begin(), reached.end());
    update.clear();
    for (const auto &prev : reached) {
      auto expl = [&](const Cell &next) {
        if (grid.check(next) && !visited.contains(next)) {
          update.emplace(next);
        }
      };

      expl(std::make_pair(prev.first + 1, prev.second));
      expl(std::make_pair(prev.first, prev.second + 1));
      expl(std::make_pair(prev.first - 1, prev.second));
      expl(std::make_pair(prev.first, prev.second - 1));
    }
    reached = std::move(update);
  }
  if (reached.empty()) {
    return -1;
  }
  return steps;
}

int phase1_sample() {
  Obstacles ob{"sample.txt"};
  Grid grid{ob, 12, 7};
  return find_best_path_len(grid);
}

int phase1() {
  Obstacles ob{"input.txt"};
  Grid grid{ob, 1024, 71};
  return find_best_path_len(grid);
}

std::string find_first_breaking(Obstacles &ob, Grid &grid) {
  std::string res;
  while (true) {
    auto [_, obst] = ob.next();
    grid.add(obst);
    if (find_best_path_len(grid) == -1) {
      res += std::to_string(obst.second);
      res += ',';
      res += std::to_string(obst.first);
      break;
    }
  }
  return res;
}

std::string phase2_sample() {
  Obstacles ob{"sample.txt"};
  Grid grid{ob, 12, 7};
  return find_first_breaking(ob, grid);
}

std::string phase2() {
  Obstacles ob{"input.txt"};
  Grid grid{ob, 1024, 71};
  return find_first_breaking(ob, grid);
}

int main() {
  // std::cout << phase1_sample() << std::endl;
  // std::cout << phase1() << std::endl;

  // std::cout << phase2_sample() << std::endl;
  std::cout << phase2() << std::endl;

  return 0;
}
