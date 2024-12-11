#include <utils.h>
#include <vector>
#include <string_view>
#include <iostream>

std::vector<std::string> parse(const std::filesystem::path& src) {
    auto buffer = utils::readFile(src);
    std::string_view rest{buffer.data()};
    std::vector<std::string> res;
    while (!rest.empty()) {
        auto next = rest.find(' ');
        if(next == std::string::npos) {
            res.emplace_back(rest.data());
            break;
        }
        res.emplace_back(rest.data(), next);
        rest = std::string_view{rest.data() + next + 1};
    }
    return res;
}

std::string removeLeadingZeros(std::string_view buffer) {
    size_t leading_zeros = 0;
    for( ; buffer[leading_zeros] == '0'; ++leading_zeros) {
    }
    if(leading_zeros == 0) {
        return std::string{buffer.data(), buffer.size()};
    }
    if(leading_zeros == buffer.size()) {
        return "0";
    }
    return std::string{buffer.data() + leading_zeros, buffer.size() - leading_zeros};
}

std::pair<std::string, std::string> split(std::string val) {
    std::pair<std::string, std::string> res;
    res.second = removeLeadingZeros(std::string_view{val.data() + val.size() / 2});
    res.first = std::move(val);
    res.first.resize(res.first.size()/ 2);
    return res;
}

#include <unordered_map>

class Cache {
public:
    Cache(std::size_t steps) {
        cache_.resize(steps + 1);
    }

    std::size_t get(std::string val, std::size_t remaining_steps) {
        if(remaining_steps == 0) {
            return 1;
        }
        if(remaining_steps < cache_.size()) {
            auto& cache = cache_[remaining_steps];
            auto it = cache.find(val);
            if(it != cache.end()) {
                return it->second;
            }
        }
        return explore_(val, remaining_steps);
    }

private:
    std::size_t explore_(std::string val, std::size_t remaining_steps) {
        std::size_t res;
        if(val == "0") {
            res = get("1", remaining_steps - 1);
        }
        else if(val.size() % 2 == 0) {
            auto&& [left, right] = split(val);
            res = get(std::move(left), remaining_steps - 1) +
                  get(std::move(right), remaining_steps - 1);
        }
        else {
            std::uint64_t val_int = std::atoll(val.data());
            res = get( std::to_string(val_int * 2024) , remaining_steps - 1);
        }
        auto& cache = cache_[remaining_steps];
        cache.emplace(std::move(val), res);
        return res;
    }

    std::vector<std::unordered_map<std::string, std::size_t>> cache_; 
};

std::size_t predict(const std::filesystem::path& src, std::size_t steps) {
    Cache cache{steps};
    auto numbers = parse(src);
    std::size_t res = 0;
    for(auto&& val : numbers) {
        res += cache.get(val, steps);
    }
    return res;
}

std::size_t phase1(const std::filesystem::path& src) {
    return predict(src, 25);
}

std::size_t phase2(const std::filesystem::path& src) {
    return predict(src, 75);
}

int main() {
    std::cout << phase1("input.txt") << std::endl;
    std::cout << phase2("input.txt") << std::endl;

    return 0;
}
