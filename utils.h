#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

namespace utils {
template<typename Pred>
void forEachFileLine(const std::filesystem::path& src, Pred pred) {
    std::ifstream stream{std::filesystem::path{FOLDER} / src};
    std::string buffer;
    while (!stream.eof()) {
        std::getline(stream, buffer);
        pred(std::string_view{buffer.data()});
    }
}

std::string readFile(const std::filesystem::path& src) {
    std::ifstream stream{std::filesystem::path{FOLDER} / src};
    std::stringstream buff;
    buff << stream.rdbuf();
    return buff.str();
}
}
