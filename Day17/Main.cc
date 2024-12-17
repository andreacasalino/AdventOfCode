#include <utils.h>

#include <array>
#include <iostream>
#include <math.h>
#include <vector>

std::vector<int> parseInstructions(std::string_view line) {
  line = std::string_view{line.data() + std::string_view{"Program: "}.size()};
  std::vector<int> res;
  while (!line.empty()) {
    res.push_back(std::atoi(line.data()));
    auto sep = line.find(',');
    if (sep == std::string::npos) {
      break;
    }
    line = std::string_view{line.data() + sep + 1};
  }
  return res;
}

std::uint64_t parseRegister(const std::string &buffer) {
  auto sep = buffer.find(':');
  return static_cast<std::uint64_t>(std::atoll(buffer.data() + sep + 2));
}

class Program {
public:
  Program(const std::filesystem::path &src) {
    std::vector<std::string> buffer;
    utils::forEachFileLine(src, [&buffer](const std::string_view &line) {
      buffer.emplace_back(line.data(), line.size());
    });
    for (int k = 0; k < 3; ++k) {
      registers[k] = parseRegister(buffer[k]);
    }
    instructions = parseInstructions(buffer.back().data());
  }

  bool next() {
    if ((instructions.size() - 2) < instruction_pointer) {
      return false;
    }
    int opcode = instructions[instruction_pointer];
    int operand = instructions[instruction_pointer + 1];
    switch (opcode) {
    case 0: {
      std::uint64_t res = registers[0] / pow(2, getAsCombo(operand));
      registers[0] = static_cast<int>(res);
    } break;

    case 1: {
      int res = static_cast<int>(registers[1]) ^ operand;
      registers[1] = res;
    } break;

    case 2: {
      registers[1] = getAsCombo(operand) % 8;
    } break;

    case 3: {
      if (registers[0] != 0) {
        instruction_pointer = static_cast<std::size_t>(operand);
        return true;
      }
    } break;

    case 4: {
      registers[1] = registers[1] ^ registers[2];
    } break;

    case 5: {
      addToOut(getAsCombo(operand) % 8);
    } break;

    case 6: {
      std::uint64_t res = registers[0] / pow(2, getAsCombo(operand));
      registers[1] = static_cast<int>(res);
    } break;

    case 7: {
      std::uint64_t res = registers[0] / pow(2, getAsCombo(operand));
      registers[2] = static_cast<int>(res);
    } break;

    default:
      throw std::runtime_error{"Not recognized instruction!"};
      break;
    }

    instruction_pointer += 2;
    return true;
  }

  const auto &get() const { return output; }

private:
  int getAsCombo(int val) const {
    switch (val) {
    case 4:
      return registers[0];
      break;
    case 5:
      return registers[1];
      break;
    case 6:
      return registers[2];
      break;
    default:
      break;
    }
    return val;
  }

  std::size_t instruction_pointer = 0;
  std::vector<int> instructions;

  std::array<std::uint64_t, 3> registers;

  void addToOut(std::uint64_t val) {
    if (!output.empty()) {
      output += ',';
    }
    output += std::to_string(val);
  }
  std::string output;
};

std::string phase1(Program prog) {
  while (prog.next()) {
  }
  return prog.get();
}

int main() {
  std::cout << phase1(Program{"input.txt"}) << std::endl;

  return 0;
}
