#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <memory>

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

struct Folder {
    Folder(const std::string& prefix) {
        dir /= prefix;
        if(std::filesystem::exists(dir)) {
            std::filesystem::remove_all(dir);
        }
        std::filesystem::create_directories(dir);
    }

    std::unique_ptr<std::ofstream> make_stream(const std::string& label) const {
        return std::make_unique<std::ofstream>(dir / label);
    }

private:
    std::filesystem::path dir{FOLDER};
};
}
