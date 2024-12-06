#include <utils.h>
#include <iostream>
#include <algorithm>
 
int nextNumber(std::string_view &view) {
  int res = std::atoi(view.data());
  auto sep = view.find(' ');
  if (sep == std::string::npos) {
    view = {};
  } else {
    view = std::string_view{view.data() + sep + 1, view.size() - sep - 1};
  }
  return res;
}
 
template<typename Pred>
std::uint64_t countValidLines(const std::filesystem::path& src, Pred pred) {
    std::uint64_t res{0};
    utils::forEachFileLine(src, [&](std::string_view line) {
        if(pred(line)) {
            ++res;
        }
    });
    return res;
}

bool checkLinePhase1(std::string_view line) {
    auto prev = nextNumber(line);
    enum class Trend { UNKNOWN, UP, DOWN };
    Trend t = Trend::UNKNOWN;
    while (!line.empty()) {
      auto current = nextNumber(line);
      if (current == prev) {
        return false;
      }
      switch (t) {
      case Trend::UNKNOWN:
        if (3 < std::abs(current - prev)) {
          return false;
        }
        t = prev < current ? Trend::UP : Trend::DOWN;
        break;
      case Trend::UP:
        if (prev > current || prev + 3 < current) {
          return false;
        }
        break;
      case Trend::DOWN:
        if (prev < current || prev - 3 > current) {
          return false;
        }
        break;
      }
      prev = current;
    }
    return true;

}
 
#include <list>
#include <vector>
 
bool checkLinePhase2_bruteForce(std::string_view line) {
    if (checkLinePhase1(line)) {
        return true;
    }

    auto compose = [](const std::vector<int> &vals) {
        std::stringstream buffer;
        buffer << vals.front();
        std::for_each(vals.begin() + 1, vals.end(), [&buffer](int a) {
        buffer << ' ';
        buffer << a;
        });
        return buffer.str();
    };

    std::vector<int> vals;
    while (!line.empty()) {
        vals.push_back(nextNumber(line));
    }

    for (int pos = 0; pos < vals.size(); ++pos) {
        auto clone = vals;
        clone.erase(clone.begin() + pos);
        auto tmp = compose(clone);
        if (checkLinePhase1(tmp.data())) {
            return true;
        }
    }
    return false;
}

bool checkLinePhase2_(std::string_view line, bool removalDone = false,
                    std::list<int> cache = {}) {
    while (!line.empty()) {
        auto current = nextNumber(line);
        enum class Problem { NONE, EQUAL, BAD_TREND, TOO_DISTANT };
        auto analize = [&]() {
        switch (cache.size()) {
        case 0:
            return Problem::NONE;

        case 1: {
            if (cache.back() == current) {
            return Problem::EQUAL;
            }
            return std::abs(cache.back() - current) <= 3 ? Problem::NONE
                                                        : Problem::TOO_DISTANT;
        }

        default: {
            if (cache.back() == current) {
            return Problem::EQUAL;
            }
            auto it = cache.rbegin();
            auto it_prev = it;
            ++it_prev;
            bool upDown = *it_prev < *it;
            if (upDown) {
            if (cache.back() > current) {
                return Problem::BAD_TREND;
            }
            if (cache.back() + 3 < current) {
                return Problem::TOO_DISTANT;
            }
            } else {
            if (cache.back() < current) {
                return Problem::BAD_TREND;
            }
            if (cache.back() - 3 > current) {
                return Problem::TOO_DISTANT;
            }
            }
        }
        }
        return Problem::NONE;
        };

        auto maybe_problem = analize();
        if (maybe_problem == Problem::NONE) {
        cache.push_back(current);
        continue;
        }

        if (removalDone) {
        return false;
        }

        if (cache.size() == 2 && maybe_problem == Problem::BAD_TREND) {
        // try change trend by removing first element
        int front_snap = cache.front();
        cache.pop_front();
        if (analize() == Problem::NONE) {
            cache.push_back(current);
            if (checkLinePhase2_(line, true, cache)) {
            return true;
            }
            cache.pop_back();
        }
        cache.push_front(front_snap);
        }

        // try to not add current
        if (checkLinePhase2_(line, true, cache)) {
        return true;
        }

        // try to remove previous val in cache
        removalDone = true;
        cache.pop_back();
        if (analize() != Problem::NONE) {
        return false;
        }

        cache.push_back(current);
    }
    return true;
}

bool checkLinePhase2(std::string_view line) {
    return checkLinePhase2_(line);
}

int main() {
  std::cout << countValidLines("input.txt", &checkLinePhase1) << std::endl;

  std::cout << countValidLines("input.txt", &checkLinePhase2_bruteForce) << std::endl;
  std::cout << countValidLines("input.txt", &checkLinePhase2) << std::endl;
 
  return 0;
}