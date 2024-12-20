#include <utils.h>

#include <iostream>
#include <unordered_map>
#include <vector>

using Dict = std::unordered_map<char, std::vector<std::string>>;

struct Input {
  Input(const std::filesystem::path &src) {
    bool dictDone = false;
    utils::forEachFileLine(src, [&](const std::string_view &line) {
      if (dictDone) {
        words.emplace_back(line.data(), line.size());
      } else {
        if (line.empty()) {
          dictDone = true;
          return;
        }
        parseDict(line);
      }
    });
    std::reverse(words.begin(), words.end());
  }

  bool next(std::string &word) {
    if (words.empty()) {
      return false;
    }
    word = std::move(words.back());
    words.pop_back();
    return true;
  }

  Dict dictionary;

private:
  void parseDict(std::string_view rest) {
    auto add = [&](std::string_view toAdd) {
      dictionary[toAdd.front()].emplace_back(toAdd.data(), toAdd.size());
    };

    while (!rest.empty()) {
      auto sep = rest.find(", ");
      if (sep == std::string::npos) {
        add(rest);
        break;
      }
      add(std::string_view{rest.data(), sep});
      rest = std::string_view{rest.data() + sep + 2};
    }

    for (auto &[_, elemenets] : dictionary) {
      std::sort(
          elemenets.begin(), elemenets.end(),
          [](const auto &a, const auto &b) { return a.size() < b.size(); });
    }
  }

  std::vector<std::string> words;
};

bool canBeMade(std::string_view rest, const Dict &dict) {
  auto match = [](std::string_view small, std::string_view big) {
    return small.size() <= big.size() &&
           small == std::string_view{big.data(), small.size()};
  };

  while (!rest.empty()) {
    auto it = dict.find(rest.front());
    if (it == dict.end()) {
      return false;
    }
    if (it->second.size() == 1) {
      if (match(it->second.front(), rest)) {
        rest = std::string_view{rest.data() + it->second.size()};
        continue;
      } else {
        return false;
      }
    }

    return std::any_of(
        it->second.begin(), it->second.end(), [&](const std::string &m) {
          if (match(m.data(), rest)) {
            return canBeMade(std::string_view{rest.data() + m.size()}, dict);
          }
          return false;
        });
  }
  return true;
}

std::uint64_t phase1(const std::filesystem::path &src) {
  Input input{src};
  std::string word;
  std::uint64_t res = 0;
  while (input.next(word)) {
    bool ok = canBeMade(word.data(), input.dictionary);
    // if (ok) {
    //   std::cout << "  possible: " << word << std::endl;
    // } else {
    //   std::cout << "impossible: " << word << std::endl;
    // }
    if (ok) {
      ++res;
    }
  }
  return res;
}

std::uint64_t numberOfWaysCanBeMade(const std::string &str, const Dict &dict) {
  struct Interval {
    int begin;
    int end;
  };
  std::vector<Interval> elements;
  for (int k = 0; k < str.size(); ++k) {
    auto it = dict.find(str[k]);
    if (it == dict.end()) {
      continue;
    }
    for (const auto &w : it->second) {
      if (w.size() <= str.size() - k &&
          w == std::string_view{str.data() + k, w.size()}) {
        elements.emplace_back(Interval{k, k + static_cast<int>(w.size())});
      }
    }
  }

  std::vector<std::uint64_t> ways;
  std::unordered_map<int, std::vector<int>> end_to_ways_indices;
  for (int k = 0; k < elements.size(); ++k) {
    std::uint64_t val = 1;
    if (elements[k].begin != 0) {
      val = 0;
      for (auto k_prev : end_to_ways_indices[elements[k].begin]) {
        val += ways[k_prev];
      }
    }
    ways.push_back(val);
    end_to_ways_indices[elements[k].end].push_back(k);
  }
  std::uint64_t res = 0;
  for (auto k : end_to_ways_indices[str.size()]) {
    res += ways[k];
  }
  return res;
}

std::uint64_t phase2(const std::filesystem::path &src) {
  Input input{src};
  std::string word;
  std::uint64_t res = 0;
  while (input.next(word)) {
    res += numberOfWaysCanBeMade(word, input.dictionary);
  }
  return res;
}

int main() {
  // std::cout << phase1("input.txt") << std::endl;
  std::cout << phase2("input.txt") << std::endl;

  return 0;
}
