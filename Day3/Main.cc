#include <utils.h>
#include <iostream>
#include <algorithm>

#include <regex> 

bool isNumber(const std::string_view& str) {
    return std::all_of(str.begin(), str.end(), [](char c) {
        return '0' <= c && c <= '9';
    });
};

template<typename Pred>
void forEachMul(std::string_view remaining, Pred pred) {
    std::match_results<std::string_view::const_iterator> match;
    static std::regex expr{"mul\\((.*?)\\,(.*?)\\)"};
    while (std::regex_search(remaining.begin(), remaining.end(), match, expr)) {
        std::string_view first{match[1].first, match[1].second};
        std::string_view second{match[2].first, match[2].second};
        if(isNumber(first) && isNumber(second)) {
            int first_val = std::atoi(first.data());
            int second_val = std::atoi(second.data());
            pred(first_val, second_val);
            remaining = std::string_view{match[2].second, remaining.end()};
        }
        else {
            remaining = std::string_view{remaining.data() + 1};
        }
    }
}

std::uint64_t phase1(const std::filesystem::path& src) {
    std::uint64_t res{0};
    utils::forEachFileLine(src, [&res](std::string_view remaining) {
        forEachMul(remaining, [&](int a, int b) {
            // std::cout << a << "  " << b << std::endl;
            res += a * b;
        });
    });
    return res;
}

#include <array>
#include <optional>

using MatchResult = std::match_results<std::string_view::const_iterator>;

struct Match {
    std::size_t begin;
    int alternativeIndex;
    MatchResult res;
    std::string_view remaining;
};

template<std::size_t N>
std::optional<Match> nextMatch(std::string_view remaining, const std::array<std::regex*, N>& alternatives) {
    std::optional<Match> res;
    for(int index = 0; index < alternatives.size(); ++index) {
        MatchResult maybe_match;
        if(std::regex_search(remaining.begin(), remaining.end(), maybe_match, *alternatives[index])) {
            std::size_t begin = maybe_match[0].first - remaining.begin();
            if(!res.has_value() || begin < res->begin) {
                auto& added = res.emplace();
                added.begin = begin;
                added.alternativeIndex = index;
                added.remaining = std::string_view{remaining.data() + begin + 1};
                added.res = std::move(maybe_match);
            }
        }
    }
    return res;
}

std::uint64_t phase2(const std::filesystem::path& src) {
    std::string buffer = utils::readFile(src);
    std::string_view remaining{buffer.data()};

    enum class Status { NONE, OFF, ON };
    Status s = Status::NONE;

    std::uint64_t res{0};
    auto processMul = [&](const MatchResult& match) {
        std::string_view first{match[1].first, match[1].second};
        std::string_view second{match[2].first, match[2].second};
        if(isNumber(first) && isNumber(second)) {
            int first_val = std::atoi(first.data());
            int second_val = std::atoi(second.data());
            // std::cout << first_val << "   " << second_val << std::endl;
            res += first_val * second_val;
            remaining = std::string_view{match[2].second, remaining.end()};
        }
        else {
            remaining = std::string_view{remaining.data() + 1};
        }
    };

    static std::vector<std::regex> all_alternatives{
        std::regex{"do\\(\\)"}, 
        std::regex{"don't\\(\\)"},
        std::regex{"mul\\((.*?)\\,(.*?)\\)"} 
    };

    while (!remaining.empty()) {
        switch (s) {
        case Status::NONE: {
            static std::array<std::regex*, 3> alternatives{ 
                &all_alternatives[0],
                &all_alternatives[1],
                &all_alternatives[2]
            };
            auto maybe_next = nextMatch(remaining, alternatives);
            if(!maybe_next.has_value()) {
                remaining = {};
                break;
            }
            switch (maybe_next->alternativeIndex) {
            case 0:
                s = Status::ON;
                remaining = maybe_next->remaining;
                break;
            case 1:
                s = Status::OFF;
                remaining = maybe_next->remaining;
                break;
            default: 
                processMul(maybe_next->res);
                break;
            }
        }
            break;

        case Status::ON: {
            static std::array<std::regex*, 2> alternatives{ 
                &all_alternatives[1],
                &all_alternatives[2]
            };
            auto maybe_next = nextMatch(remaining, alternatives);
            if(!maybe_next.has_value()) {
                remaining = {};
                break;
            }
            switch (maybe_next->alternativeIndex) {
            case 0:
                s = Status::OFF;
                remaining = maybe_next->remaining;
                break;
            default: 
                processMul(maybe_next->res);
                break;
            }
        }
            break;

        case Status::OFF: {
            static std::array<std::regex*, 1> alternatives{ 
                &all_alternatives[0]
            };
            auto maybe_next = nextMatch(remaining, alternatives);
            if(!maybe_next.has_value()) {
                remaining = {};
                break;
            }
            remaining = maybe_next->remaining;
            s = Status::ON;
        }
            break;
        }
    }
    return res;
}

int main() {
//   std::cout << phase1("input.txt") << std::endl;
  std::cout << phase2("input.txt") << std::endl;

  return 0;
}