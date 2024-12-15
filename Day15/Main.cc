#include <utils.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <algorithm>

using Cell = std::pair<int, int>;

struct Grid {
  Grid(std::ifstream &stream) {
    std::string buffer;
    bool pos_found = false;
    while (true) {
      std::getline(stream, buffer);
      if (buffer.empty()) {
        return;
      }
      auto at_pos = buffer.find('@');
      if (at_pos != std::string::npos) {
        buffer[at_pos] = '.';
        position.first = static_cast<int>(data_.size());
        position.second = static_cast<int>(at_pos);
        pos_found = true;
      }
      data_.emplace_back(std::move(buffer));
    }
    if(!pos_found) {
      throw std::runtime_error{"unable to find start position"};
    }
  }

  const auto &data() const { return data_; }

  void move(char dir) {
    Cell delta = std::make_pair(0, 0);
    switch (dir) {
    case '>':
      ++delta.second;
      break;
    case '^':
      --delta.first;
      break;
    case '<':
      --delta.second;
      break;
    default: // 'v'
      ++delta.first;
      break;
    }

    Cell firstInterestingPoint = position;
    do {
      firstInterestingPoint =
          std::make_pair(firstInterestingPoint.first + delta.first,
                         firstInterestingPoint.second + delta.second);
    } while (at(firstInterestingPoint) == 'O');

    if (at(firstInterestingPoint) == '#') {
      return;
    }

    data_.at(firstInterestingPoint.first).at(firstInterestingPoint.second) =
        'O';
    position =
        std::make_pair(position.first + delta.first, position.second + delta.second);
    data_.at(position.first).at(position.second) = '.';
  }

  char at(const Cell &pos) const { return at(pos.first, pos.second); }

  char at(int r, int c) const { return data_.at(r).at(c); }

  void print(std::ostream &stream) const {
    for (int r = 0; r < data_.size(); ++r) {
      const auto &row = data_[r];
      if (r == position.first) {
        stream << std::string_view{row.data(),
                                    static_cast<std::size_t>(position.second)};
        stream << '@';
        stream << std::string_view{row.data() + position.second + 1};
      } else {
        stream << row;
      }
      stream << std::endl;
    }
  }

  char obstacles = 'O';
  std::string_view logName = "phase1";

  template <typename Pred>
  void forEachBox(Pred pred) const {
    for (int r = 0; r < data_.size(); ++r) {
      for (int c = 0; c < data_.front().size(); ++c) {
        auto cell = std::make_pair(r, c);
        if (obstacles == at(cell)) {
          pred(cell);
        }
      }
    }
  }

protected:
  Cell position;
  std::vector<std::string> data_;
};

struct GridPhase2 : Grid {
  GridPhase2(std::ifstream &stream, bool enlarge = true) : Grid{stream} {
    obstacles = '[';
    logName = "phase2";

    if(enlarge) {
      position.second *= 2;
      std::vector<std::string> buffer;
      buffer.reserve(data_.size());
      for (auto &row : data_) {
        auto &added = buffer.emplace_back();
        added.reserve(row.size() * 2);
        for (auto c : row) {
          switch (c) {
          case '#':
            added += "##";
            break;
          case 'O':
            added += "[]";
            break;
          default:
            added += "..";
            break;
          }
        }
      }
      data_ = std::move(buffer);
    }
  }

  void move(char dir) {
    switch (dir) {
    case '>':
      moveHorz<false>();
      break;
    case '^':
      moveVert<true>();
      break;
    case '<':
      moveHorz<true>();
      break;
    default: // 'v'
      moveVert<false>();
      break;
    }
  }

private:
  template <bool UpDown> void moveVert() {
    const Cell start = position;
    int delta = UpDown ? -1 : 1;
    std::vector<std::unordered_set<int>> levels;
    levels.emplace_back().emplace(start.second);
    Cell buffer = start;
    buffer.first += delta;
    for (; true; buffer.first += delta) {
      std::unordered_set<int> update;
      for (auto prev : levels.back()) {
        buffer.second = prev;
        switch (at(buffer)) {
        case '#':
          return;
        case '[':
          update.emplace(prev);
          update.emplace(prev + 1);
          break;
        case ']':
          update.emplace(prev);
          update.emplace(prev - 1);
          break;
        default:
          break;
        }
      }
      if (update.empty()) {
        break;
      }
      levels.emplace_back(std::move(update));
    }
    position = start;
    position.first += delta;
    if (1 < levels.size()) {
      int r_to = start.first + levels.size() * delta;
      int r_from = r_to - delta;
      std::for_each(levels.rbegin(), levels.rend() - 1, [&](const auto &level) {
        for (auto col : level) {
          data_.at(r_to).at(col) = data_.at(r_from).at(col);
          data_.at(r_from).at(col) = '.';
        }
        r_from -= delta;
        r_to -= delta;
      });
    }
  }

  template <bool LeftRight> void moveHorz() {
    const Cell start = position;
    int delta = LeftRight ? -1 : 1;
    Cell firstInterestingPoint = start;
    do {
      firstInterestingPoint.second += delta;
    } while (at(firstInterestingPoint) == '[' ||
             at(firstInterestingPoint) == ']');

    if (at(firstInterestingPoint) == '#') {
      return;
    }

    position = start;
    position.second += delta;
    if (position.second == firstInterestingPoint.second) {
      return;
    }

    auto &row = data_[firstInterestingPoint.first];
    if constexpr (LeftRight) {
      size_t begin = firstInterestingPoint.second;
      ++begin;
      std::string cut{row.data() + begin, start.second - begin};
      std::memcpy(row.data() + begin - 1, cut.data(), cut.size());
      row.at(begin + cut.size() - 1) = '.';
    } else {
      size_t begin = start.second;
      ++begin;
      size_t end = firstInterestingPoint.second;
      std::string cut{row.data() + begin, end - begin};
      std::memcpy(row.data() + begin + 1, cut.data(), cut.size());
      row.at(begin) = '.';
    }
  }
};

struct Moves {
  Moves(std::unique_ptr<std::ifstream> stream) : stream_{std::move(stream)} {}

  bool next(char &res) {
    res = '\n';
    while (res == '\n') {
      if (stream_->eof()) {
        return false;
      }
      stream_->read(&res, 1);
    }
    return true;
  }

private:
  std::unique_ptr<std::ifstream> stream_;
};

template <typename GridT> struct Logger {
public:
  Logger(const GridT &grid) : src{&grid} {
    fldr /= "log";
    fldr /= grid.logName;
    if (std::filesystem::exists(fldr)) {
      std::filesystem::remove_all(fldr);
    }
    std::filesystem::create_directories(fldr);
  }

  void print(char move) {
    logname = "iter_";
    logname += std::to_string(iter);
    logname += '_';
    switch (move) {
    case '>':
      logname += "Right";
      break;
    case '^':
      logname += "Up";
      break;
    case '<':
      logname += "Left";
      break;
    case 'v':
      logname += "Down";
      break;
    default:
      break;
    }
    std::ofstream stream{fldr / logname};
    if(!stream.is_open()) {
      throw std::runtime_error{"Unable to open stream to log file"};
    }
    src->print(stream);
    ++iter;
  }

private:
  const GridT *src;
  std::size_t iter = 0;
  std::string logname;
  std::filesystem::path fldr = std::filesystem::path{FOLDER};
};

#include <optional>

template <typename GridT>
std::uint64_t process(std::pair<GridT, Moves> subject, bool print,
                      size_t max_iter) {
  auto& [grid, moves] = subject;

  std::optional<Logger<GridT>> logger;
  if (print) {
    logger.emplace(grid);
  }
  if (logger.has_value()) {
    logger->print('*');
  }

  std::uint64_t res = 0;
  char m;
  for (size_t i = 0; i < max_iter && moves.next(m); ++i) {
    grid.move(m);
    if (logger.has_value()) {
      logger->print(m);
    }
  }
  grid.forEachBox([&res](const Cell &cell) {
    res += cell.first * 100 + cell.second;
  });
  return res;
}

template <typename GridT, typename ... ARGS>
std::pair<GridT, Moves> parse(const std::filesystem::path &src, ARGS&& ... args) {
  auto stream =
      std::make_unique<std::ifstream>(std::filesystem::path{FOLDER} / src);
  GridT grid{*stream, std::forward<ARGS>(args)...};
  return std::make_pair(std::move(grid), Moves{std::move(stream)});
}

std::uint64_t
phase1(const std::filesystem::path &src, bool print = false,
       size_t max_iter = std::numeric_limits<std::size_t>::max()) {
  return process(parse<Grid>(src), print, max_iter);
}

std::uint64_t
phase2(const std::filesystem::path &src, bool print = false,
       size_t max_iter = std::numeric_limits<std::size_t>::max()) {
  return process(parse<GridPhase2>(src), print, max_iter);
}

std::uint64_t
phase2_debug(const std::filesystem::path &src, bool print = false,
       size_t max_iter = std::numeric_limits<std::size_t>::max()) {
  return process(parse<GridPhase2>(src, false), print, max_iter);
}

int main() {
  // std::cout << phase1("sample.txt", true) << std::endl;
  // std::cout << phase2("sample.txt", true) << std::endl;
  std::cout << phase2("input.txt", false) << std::endl;

  return 0;
}
