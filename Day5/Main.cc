#include <utils.h>
#include <iostream>
#include <algorithm>

#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <algorithm>
#include <vector>

struct Input {
    Input(const std::filesystem::path& p)
    : stream_{std::filesystem::path{FOLDER} / p} {
        std::string buffer;
        auto parse = [&buffer]() {
            auto sep = buffer.find('|');
            return std::make_pair(
                std::atoi(buffer.data()),
                std::atoi(buffer.data() + sep + 1)
            );
        };

        while (!stream_.eof()) {
            std::getline(stream_, buffer);
            if(buffer.empty()) {
                break;
            }
            auto [prev, follower] = parse();
            dependencies_[follower].emplace(prev);
        }
    }

    template<typename Pred>
    void forEachSequence(Pred pred) {
        std::string buffer;
        while (!stream_.eof()) {
            std::getline(stream_, buffer);
            pred(std::string_view{buffer.data()}, dependencies_);
        }
    }

private:
    std::unordered_map<int, std::unordered_set<int>> dependencies_;
    std::ifstream stream_;
};

std::vector<int> split(std::string_view remaining) {
    std::vector<int> res;
    while (!remaining.empty()) {
        res.push_back(std::atoi(remaining.data()));
        auto sep = remaining.find(',');
        if(sep == std::string::npos) {
            break;
        }
        remaining = std::string_view{remaining.data() + sep + 1};
    }
    return res;
}

std::pair<bool, std::vector<int>> isRightOrder(std::string_view remaining, std::unordered_map<int, std::unordered_set<int>>& dependencies) {
    std::pair<bool, std::vector<int>> res;
    auto& seq = res.second; 
    seq = split(remaining);
    std::unordered_map<int, int> occ;
    for(int pos = 0; pos < seq.size(); ++pos) {
        occ.emplace(seq[pos], pos);
    }
    res.first = std::all_of(occ.begin(), occ.end(), [&occ, &dependencies](const auto& el) {
        auto [sym, pos] = el;
        for(auto dep : dependencies[sym]) {
            auto it_prev = occ.find(dep);
            if(it_prev != occ.end() && pos < it_prev->second) {
                return false;
            }
        }
        return true;
    });
    return res;
}

#include <ranges>

std::vector<int> makeRightOrder(const std::unordered_set<int>& involved, std::unordered_map<int, std::unordered_set<int>>& dependencies) {
    std::vector<int> res;
    std::unordered_set<int> remaining{involved};
    while (!remaining.empty()) {
        std::vector<int> additions;
        for(auto rem : remaining) {
            auto it_dep = dependencies.find(rem);
            if(it_dep == dependencies.end()) {
                additions.push_back(rem);
                continue;
            }

            auto involvedPrev = it_dep->second | std::views::filter([&involved](int val) {
                return involved.contains(val);
            });
            if(std::all_of(involvedPrev.begin(), involvedPrev.end(), [&remaining](int prev) {
                return !remaining.contains(prev);
            })) {
                additions.push_back(rem);
            }
        }
        if(additions.empty()) {
            throw std::runtime_error{"No progress!!!"};
        }
        res.insert(res.end(), additions.begin(), additions.end());
        for(auto toRm : additions) {
            remaining.erase(toRm);
        }
    }
    return res;
}

template<bool Phase1or2>
std::uint64_t process(const std::filesystem::path& p) {
    Input input{p};
    std::uint64_t res{0};
    input.forEachSequence([&res](std::string_view line, std::unordered_map<int, std::unordered_set<int>>& dependencies) {
        auto [ok, seq] = isRightOrder(line, dependencies);
        if constexpr (Phase1or2) {
            if(ok) {
                int middle = seq[seq.size() / 2];
                std::cout << line << " --> " << middle << std::endl;
                res += middle;
            }
        }
        else {
            if(!ok) {
                auto seq2 = makeRightOrder(std::unordered_set<int>{seq.begin(), seq.end()}, dependencies);
                int middle = seq2[seq2.size() / 2];
                auto printSeq = [&seq2]() {
                    std::stringstream buffer;
                    buffer << seq2.front();
                    std::for_each(seq2.begin() + 1, seq2.end(), [&buffer](int val) {
                        buffer << ',' << val;
                    });
                    return buffer.str();
                };
                std::cout << line << " --> " << printSeq() << " --> " << middle << std::endl;
                res += middle;
            }
        }
    });
    return res;
}

int main() {
//   std::cout << process<true>("input.txt") << std::endl;
  std::cout << process<false>("input.txt") << std::endl;

  return 0;
}