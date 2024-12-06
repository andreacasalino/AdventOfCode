#include <iostream>
#include <unordered_map>
#include <utils.h>
#include <vector>

struct Vals {
  void add(int v) {
    switch (values.size()) {
    case 0:
    case 1:
      values.push_back(v);
      break;

    default:
      values.back() = v;
      break;
    }
  }

  int get() {
    if (values.size() == 1) {
      values.push_back(values.front());
    }
    return values.front() * 10 + values.back();
  }

private:
  std::vector<int> values;
};

std::uint64_t phase1(const std::filesystem::path &src) {
  std::uint64_t res{0};
  utils::forEachFileLine(src, [&](std::string_view line) {
    Vals vals;
    for (auto c : line) {
      if ('0' <= c && c <= '9') {
        vals.add(int{c} - 48);
      }
    }
    res += vals.get();
  });
  return res;
}

std::uint64_t phase2(const std::filesystem::path &src) {
  std::uint64_t res{0};
  std::unordered_map<std::string_view, int> numbers{
      {"zero", 0}, {"one", 1}, {"two", 2},   {"three", 3}, {"four", 4},
      {"five", 5}, {"six", 6}, {"seven", 7}, {"eight", 8}, {"nine", 9}};
  std::unordered_map<char, std::vector<std::string_view>> numbers_start;
  for (const auto &[str, _] : numbers) {
    numbers_start[str.front()].emplace_back(str);
  }

  utils::forEachFileLine(src, [&](std::string_view line) {
    Vals vals;
    struct Match {
      std::string_view full;
      std::string_view remaining;
    };
    std::vector<Match> matches;
    for (auto c : line) {
      if ('0' <= c && c <= '9') {
        vals.add(int{c} - 48);
        matches.clear();
        continue;
      }
      std::vector<Match> update;
      for (const auto &prev : matches) {
        if (prev.remaining.front() != c) {
          continue;
        }
        Match next = prev;
        next.remaining = std::string_view{next.remaining.data() + 1};
        if (next.remaining.empty()) {
          vals.add(numbers.at(next.full));
        } else {
          update.emplace_back(next);
        }
      }
      if (auto it = numbers_start.find(c); it != numbers_start.end()) {
        for (auto &str : it->second) {
          auto &added = update.emplace_back();
          added.full = str;
          added.remaining = std::string_view{str.data() + 1};
        }
      }
      matches = std::move(update);
    }
    res += vals.get();
  });
  return res;
}

int main() {
  //   std::cout << phase1("sample.txt") << std::endl;
  std::cout << phase2("input.txt") << std::endl;

  return 0;
}
