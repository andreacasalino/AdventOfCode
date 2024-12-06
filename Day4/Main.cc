#include <utils.h>
#include <iostream>
#include <algorithm>

#include <vector>

std::vector<std::string> parse_input(const std::filesystem::path& p) {
    std::vector<std::string> res;
    utils::forEachFileLine(p, [&res](std::string_view line){
        res.emplace_back(std::string{line.data(), line.size()});
    });
    return res;
}

template<typename Pred>
bool match(const std::vector<std::string>& mat, std::pair<int, int> pos, std::string_view remaining, Pred advance) {
    while (!remaining.empty()) {
        pos = advance(pos);
        if(pos.first < 0 || pos.second < 0 || pos.first == mat.size() || pos.second == mat.front().size() || 
           mat[pos.first][pos.second] != remaining.front()) {
            return false;
        }
        remaining = std::string_view{remaining.data() + 1};
    }
    return true;
}

std::uint64_t phase1(const std::filesystem::path& p) {
    std::uint64_t res{0};
    auto mat = parse_input(p);

    for(int r =0 ; r < mat.size(); ++r) {
    for(int c =0 ; c < mat.front().size(); ++c) {
        if(mat[r][c] == 'X') {
            if(match(mat, std::make_pair(r, c), "MAS", [](const std::pair<int, int>& from) {
                return std::make_pair(from.first + 1, from.second);
            })) {
                ++res;
            }
            if(match(mat, std::make_pair(r, c), "MAS", [](const std::pair<int, int>& from) {
                return std::make_pair(from.first - 1, from.second);
            })) {
                ++res;
            }
            if(match(mat, std::make_pair(r, c), "MAS", [](const std::pair<int, int>& from) {
                return std::make_pair(from.first, from.second + 1);
            })) {
                ++res;
            }
            if(match(mat, std::make_pair(r, c), "MAS", [](const std::pair<int, int>& from) {
                return std::make_pair(from.first, from.second - 1);
            })) {
                ++res;
            }


            if(match(mat, std::make_pair(r, c), "MAS", [](const std::pair<int, int>& from) {
                return std::make_pair(from.first + 1, from.second + 1);
            })) {
                ++res;
            }
            if(match(mat, std::make_pair(r, c), "MAS", [](const std::pair<int, int>& from) {
                return std::make_pair(from.first + 1, from.second -1);
            })) {
                ++res;
            }
            if(match(mat, std::make_pair(r, c), "MAS", [](const std::pair<int, int>& from) {
                return std::make_pair(from.first - 1, from.second + 1);
            })) {
                ++res;
            }
            if(match(mat, std::make_pair(r, c), "MAS", [](const std::pair<int, int>& from) {
                return std::make_pair(from.first - 1, from.second - 1);
            })) {
                ++res;
            }
        }
    }
    }
    return res;    
}

#include <array>

struct ROI {
    std::array<char, 2> diag1;
    std::array<char, 2> diag2;
};

template<typename Pred>
void forEachROI(const std::vector<std::string>& mat, Pred pred) {
    ROI roi;
    for(int r = 2; r < mat.size(); ++r) {
    for(int c = 2; c < mat.size(); ++c) {
        if(mat[r-1][c-1] != 'A') {
            continue;
        }
        roi.diag1[0] = mat[r-2][c-2];
        roi.diag1[1] = mat[r][c];

        roi.diag2[0] = mat[r-2][c];
        roi.diag2[1] = mat[r][c-2];

        pred(roi);
    }
    }
}

std::uint64_t phase2(const std::filesystem::path& p) {
    std::uint64_t res{0};
    auto mat = parse_input(p);

    forEachROI(mat, [&res](const ROI& roi) {
        auto isDiagOk = [](const auto& diag) {
            if(diag[0] == 'M' && diag[1] == 'S') {
                return true;
            }
            if(diag[0] == 'S' && diag[1] == 'M') {
                return true;
            }
            return false;
        };

        if(isDiagOk(roi.diag1) && isDiagOk(roi.diag2)) {
            ++res;
        }
    });

    return res;
}

int main() {
  std::cout << phase1("input.txt") << std::endl;
  std::cout << phase2("input.txt") << std::endl;

  return 0;
}