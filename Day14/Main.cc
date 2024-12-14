#include <algorithm>
#include <array>
#include <iostream>
#include <unordered_set>
#include <utils.h>
#include <vector>

using Cell = std::pair<int, int>;

struct Robot {
  Cell start;
  Cell delta;
};
std::vector<Robot> parse_robots(const std::filesystem::path &src) {
  std::vector<Robot> res;

  utils::forEachFileLine(src, [&res](std::string_view line) {
    auto parse = [](std::string_view line) {
      auto sep = line.find(',');
      int x = std::atoi(line.data());
      int y = std::atoi(line.data() + sep + 1);
      return std::make_pair(x, y);
    };

    auto sep = line.find(' ');
    auto pos = parse(line.data() + 2);
    auto vel = parse(line.data() + sep + 3);

    res.emplace_back(Robot{pos, vel});
  });
  return res;
}

int eval(int pos, int len) {
  int res = pos % len;
  if (res < 0) {
    res += len;
  }
  return res;
}

std::uint64_t phase1(const std::vector<Robot> &robots, int rows, int cols) {
  int x_middle = rows / 2;
  int y_middle = cols / 2;

  std::array<std::uint64_t, 4> res{0, 0, 0, 0};
  for (const auto &robot : robots) {
    int x = robot.start.first + 100 * robot.delta.first;
    int y = robot.start.second + 100 * robot.delta.second;
    x = eval(x, rows);
    y = eval(y, cols);

    if (x_middle == x || y_middle == y) {
      continue;
    }

    if (x < x_middle) {
      if (y < y_middle) {
        ++res[0];
      } else {
        ++res[1];
      }
    } else {
      if (y < y_middle) {
        ++res[2];
      } else {
        ++res[3];
      }
    }
  }
  return res[0] * res[1] * res[2] * res[3];
}

struct Grid {
  Grid(int rows, int cols) : rows_{rows}, cols_{cols} {
    data.resize(rows * cols);
    reset();
  }

  void reset() {
    for (int c = 0; c < data.size(); ++c) {
      data[c] = '.';
    }
  }

  void set(int r, int c) {
    int index = r * cols_ + c;
    data[index] = '#';
  }

  int biggestIsland() const {
    std::vector<int> sizes;
    Cells visited;
    int index = 0;
    for (int r = 0; r < rows_; ++r) {
      for (int c = 0; c < cols_; ++c) {
        auto p = std::make_pair(r, c);
        if (data[index++] != '#' || visited.contains(p)) {
          continue;
        }
        int snap = visited.size();
        exploreIsland(visited, p);
        sizes.push_back(visited.size() - snap);
      }
    }
    return *std::max_element(sizes.begin(), sizes.end());
  }

  void print(std::ostream &s) const {
    int index = 0;
    for (int r = 0; r < rows_; ++r) {
      for (int c = 0; c < cols_; ++c) {
        s << data[index++];
      }
      s << std::endl;
    }
    s << std::endl;
  }

private:
  struct Hash {
    std::size_t operator()(const Cell &c) const {
      std::uint16_t res = static_cast<std::uint16_t>(c.first);
      res = res | static_cast<std::uint16_t>(c.second) << 8;
      static std::hash<std::uint16_t> h;
      return h(res);
    }
  };
  using Cells = std::unordered_set<Cell, Hash>;

  void exploreIsland(Cells &visited, const Cell &start) const {
    Cells reached{start}, update;
    while (!reached.empty()) {
      visited.insert(reached.begin(), reached.end());
      update.clear();
      for (auto prev : reached) {
        auto expl = [&](Cell next) {
          if (next.first < 0 || rows_ <= next.first || next.second < 0 ||
              cols_ <= next.second ||
              data[next.first * cols_ + next.second] != '#' ||
              visited.contains(next)) {
            return;
          }
          update.emplace(next);
        };
        expl(std::make_pair(prev.first + 1, prev.second));
        expl(std::make_pair(prev.first, prev.second + 1));
        expl(std::make_pair(prev.first - 1, prev.second));
        expl(std::make_pair(prev.first, prev.second - 1));
      }
      reached = std::move(update);
    }
  }

  int rows_;
  int cols_;
  std::string data;
};

void phase2(std::vector<Robot> robots, int rows, int cols) {
  int x_middle = rows / 2;
  int y_middle = cols / 2;

  Grid grid{cols, rows};

  for (int k = 0; k < 10000; ++k) {
    grid.reset();
    for (auto &robot : robots) {
      robot.start.first += robot.delta.first;
      robot.start.second += robot.delta.second;
      robot.start.first = eval(robot.start.first, rows);
      robot.start.second = eval(robot.start.second, cols);
      grid.set(robot.start.second, robot.start.first);
    }
    int island = grid.biggestIsland();
    if (100 <= island) {
      std::cout << "===> " << k + 1 << std::endl;
      std::ofstream stream{std::filesystem::path{FOLDER} / "viz"};
      grid.print(stream);
      break;
    }
  }
}

int main() {
  // std::cout << phase1(parse_robots("input.txt"), 101, 103) << std::endl;
  phase2(parse_robots("input.txt"), 101, 103);

  return 0;
}
