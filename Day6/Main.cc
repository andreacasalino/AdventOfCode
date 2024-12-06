#include <vector>

#include <array>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <utils.h>

using Cell = std::pair<int, int>;

struct Grid {
  Grid(const std::filesystem::path &src) {
    utils::forEachFileLine(src, [this](std::string_view line) {
      auto &ob = obstacles.emplace_back();
      for (int col = 0; col < line.size(); ++col) {
        switch (line[col]) {
        case '#':
          ob.emplace(col);
          break;
        case '^':
          start = Cell{rows, col};
          break;
        default:
          break;
        }
      }
      cols = line.size();
      ++rows;
    });
  }

  Cell start;
  std::vector<std::unordered_set<int>> obstacles;
  int rows{0};
  int cols{0};
};

struct Hasher {
  std::size_t operator()(const Cell &c) const {
    std::uint64_t res = static_cast<std::uint64_t>(c.first);
    res = res | static_cast<std::uint64_t>(c.second) << 32;
    static std::hash<std::uint64_t> h;
    return h(res);
  }
};

struct VisitedDirections : std::array<bool, 4> {
  VisitedDirections() {
    for (int k = 0; k < 4; ++k) {
      (*this)[k] = false;
    }
  }
};

using Cells = std::unordered_map<Cell, VisitedDirections, Hasher>;

bool contains(const Cells &cells, const Cell &pos, int dir) {
  auto it = cells.find(pos);
  return it != cells.end() && it->second.at(dir);
}

enum class Status { VALID, OBSTACLE, EXIT, LOOP_DETECTED };

struct Path {
  Path(Grid &g, Cell start) : grid{g}, position{start} {
    visited[position].at(0) = true;
  }

  static const inline std::array<std::function<Cell(const Cell &)>, 4>
      DIRECTIONS{
          [](const Cell &from) {
            return Cell{from.first - 1, from.second};
          }, // up
          [](const Cell &from) {
            return Cell{from.first, from.second + 1};
          }, // right
          [](const Cell &from) {
            return Cell{from.first + 1, from.second};
          }, // down
          [](const Cell &from) {
            return Cell{from.first, from.second - 1};
          } // left
      };

  std::pair<Cell, int> makeNextPosition() const {
    int dir = static_cast<int>(direction % 4);
    return std::make_pair(DIRECTIONS.at(dir)(position), dir);
  }

  virtual bool isObstacle(const Cell &c) const {
    return grid.obstacles.at(c.first).contains(c.second);
  }

  virtual Status analizeNextPosition(const Cell &position_next, int dir) const {
    if (position_next.first < 0 || position_next.first == grid.rows ||
        position_next.second < 0 || position_next.second == grid.cols) {
      return Status::EXIT;
    }
    if (isObstacle(position_next)) {
      return Status::OBSTACLE;
    }
    return Status::VALID;
  }

  Status next() {
    auto [position_next, dir] = makeNextPosition();
    auto next_st = analizeNextPosition(position_next, dir);
    switch (next_st) {
    case Status::VALID: {
      position = position_next;
      visited[position].at(dir) = true;
    }
      return Status::VALID;
    case Status::OBSTACLE:
      ++direction;
      visited[position].at(static_cast<int>(direction % 4)) = true;
      return Status::VALID;
    default:
      break;
    }
    return next_st;
  }

  std::size_t direction{0};
  Grid &grid;
  Cell position;
  Cells visited;
};

std::size_t phase1(Grid &&grid) {
  Path path{grid, grid.start};
  Status s = Status::VALID;
  for (; s == Status::VALID; s = path.next()) {
  }
  return path.visited.size();
}

struct PathWithAddedObstacle : Path {
  PathWithAddedObstacle(Grid &g, Cell start, const Cell &ob)
      : Path{g, start}, obstacleAdded_{ob} {}

  bool isObstacle(const Cell &c) const override {
    return c == obstacleAdded_ || this->Path::isObstacle(c);
  }

  Status analizeNextPosition(const Cell &position_next,
                             int dir) const override {
    auto res = this->Path::analizeNextPosition(position_next, dir);
    if (res == Status::VALID && contains(visited, position_next, dir)) {
      res = Status::LOOP_DETECTED;
    }
    return res;
  }

  Cell obstacleAdded_;
};

std::size_t phase2(Grid &&grid) {
  Path path{grid, grid.start};
  Status s = Status::VALID;
  for (; s == Status::VALID; s = path.next()) {
  }
  path.visited.erase(grid.start);

  return std::count_if(path.visited.begin(), path.visited.end(),
                       [&grid = grid](const auto &ob) {
                         PathWithAddedObstacle path{grid, grid.start, ob.first};
                         Status s = Status::VALID;
                         for (; s == Status::VALID; s = path.next()) {
                         }
                         return s == Status::LOOP_DETECTED;
                       });
}

int main() {
  // std::cout << phase1(Grid{"sample.txt"}) << std::endl;
  std::cout << phase2(Grid{"input.txt"}) << std::endl;

  return 0;
}
