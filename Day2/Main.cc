#include <array>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <utils.h>

int parseGamePrefix(std::string_view& str) {
  str = std::string_view{str.data() + std::string_view{"Game "}.size()};
  int id = std::atoi(str.data());
  auto sep = str.find(':');
  str  = std::string_view{str.data() + sep + 1}; 
  return id;
}

std::unordered_map<std::string_view, std::size_t> INDICES{
    {"red", 0}, {"green", 1}, {"blue", 2}};


using Cubes = std::array<int, 3>;

template<char Sep>
std::string_view cutTillSep(std::string_view& src) {
  auto sep = src.find(Sep);
  if(sep == std::string::npos) {
    auto res = src;
    src = std::string_view{};
    return res;
  }
  auto res = std::string_view{src.data(), sep};
  src = std::string_view{src.data() + sep + 1, src.size() - sep - 1};
  return res;
}

struct CubesRegex {
  CubesRegex(std::string_view& src)
  : cubes{0,0,0} {
    auto raw = cutTillSep<';'>(src);
    while (!raw.empty()) {
      auto next = cutTillSep<','>(raw);
      int val = std::atoi(next.data() + 1);
      auto sep = next.find(' ', 1);
      next = std::string_view{next.data() + sep + 1, next.size() - sep - 1};
      cubes.at( INDICES.at(next) ) = val;
    }
  }

  const auto& get() const {
    return cubes;
  }

private: 
  Cubes cubes;
};

struct Line {
  Line(std::string_view line) : remaining{line} {
    id = parseGamePrefix(remaining);
  }

  auto getId() const { return id; }

  bool next(Cubes &res) {
    if(remaining.empty()) {
      return false;
    }
    res = CubesRegex{remaining}.get();
    return true;
  }

private:
  int id;
  std::string_view remaining;
};

std::uint64_t phase1(const std::filesystem::path &src) {
  std::uint64_t res{0};
  utils::forEachFileLine(src, [&res](std::string_view line) {
    Line l{line};
    Cubes set;
    while (l.next(set)) {
      if (12 < set[0] || 13 < set[1] || 14 < set[2]) {
        return;
      }
    }
    res += l.getId();
  });
  return res;
}

std::uint64_t phase2(const std::filesystem::path &src) {
  std::uint64_t res{0};
  utils::forEachFileLine(src, [&res](std::string_view line) {
    Line l{line};
    Cubes set;
    std::array<int, 3> minCubes{0,0,0};
    while (l.next(set)) {
      minCubes[0] = std::max<int>(minCubes[0], set[0]);
      minCubes[1] = std::max<int>(minCubes[1], set[1]);
      minCubes[2] = std::max<int>(minCubes[2], set[2]);
    }
    res += minCubes[0] * minCubes[1] * minCubes[2];
  });
  return res;
}

int main() {
  // std::cout << phase1("input.txt") << std::endl;
  std::cout << phase2("input.txt") << std::endl;

  return 0;
}
