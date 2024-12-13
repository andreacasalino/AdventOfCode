#include <array>
#include <iostream>
#include <optional>
#include <regex>
#include <utils.h>

using Point = std::pair<std::int64_t, std::int64_t>;

std::int64_t increase(std::int64_t val) { return val + 10000000000000; }

template <bool Increase> struct ProblemCase {
  ProblemCase(std::array<std::string, 3> lines) {
    buttonA.first = parse(lines[0]);
    buttonA.second = 3;
    buttonB.first = parse(lines[1]);
    buttonB.second = 1;
    target = parse(lines[2]);
    if constexpr (Increase) {
      target.first = increase(target.first);
      target.second = increase(target.second);
    }
  }

  std::int64_t totCost(std::int64_t a, std::int64_t b) const {
    return a * buttonA.second + b * buttonB.second;
  }

  std::pair<Point, int> buttonA; // 3
  std::pair<Point, int> buttonB; // 1
  Point target;

private:
  static Point parse(const std::string &line) {
    NumbsParser parser{line.data()};
    Point res;
    res.first = parser.next();
    res.second = parser.next();
    return res;
  }

  struct NumbsParser {
    NumbsParser(std::string_view line) : rest{line} {}

    int next() {
      auto isNumber = [](char c) { return '0' <= c && c <= '9'; };
      auto it = std::find_if(rest.begin(), rest.end(), isNumber);
      int res = std::atoi(rest.data() + size_t(it - rest.begin()));
      it = std::find_if(it + 1, rest.end(),
                        [isNumber](char c) { return !isNumber(c); });
      rest = std::string_view{rest.data() + int(it - rest.begin())};
      return res;
    }

    std::string_view rest;
  };
};

template <bool Increase, typename Pred>
void forEachTestCase(const std::filesystem::path &src, Pred pred) {
  std::array<std::string, 3> lines;
  std::size_t count = 0;
  utils::forEachFileLine(src, [&](std::string_view line) {
    if (!line.empty()) {
      lines[count % 4] = std::string(line.data(), line.size());
    }
    if (count % 4 == 2) {
      ProblemCase<Increase> cas{lines};
      pred(std::move(cas));
    }
    ++count;
  });
}

template <bool Increase> std::int64_t solve(const std::filesystem::path &src) {
  std::int64_t res = 0;
  forEachTestCase<Increase>(src, [&res](ProblemCase<Increase> &&cas) {
    auto [Ax, Ay] = cas.buttonA.first;
    auto [Bx, By] = cas.buttonB.first;
    auto [X, Y] = cas.target;

    if ((Bx * Y - By * X) % (Bx * Ay - By * Ax) == 0) {
      std::int64_t a = (Bx * Y - By * X) / (Bx * Ay - By * Ax);
      if ((X - a * Ax) % Bx == 0) {
        std::int64_t b = (X - a * Ax) / Bx;
        auto delta = cas.totCost(a, b);
        res += delta;
      }
    }
  });
  return res;
}

std::int64_t phase1(const std::filesystem::path &src) {
  return solve<false>(src);
}

std::int64_t phase2(const std::filesystem::path &src) {
  return solve<true>(src);
}

int main() {
  //   std::cout << phase1("input.txt") << std::endl;
  std::cout << phase2("input.txt") << std::endl;

  return 0;
}
