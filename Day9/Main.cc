#include <utils.h>

#include <vector>
#include <deque>
#include <iostream>

struct Slot {
    int spaces;
    std::deque<std::pair<int, int>> vals; // < val, how_many >

    void add(int val, int count) {
        vals.emplace_back(std::make_pair(val, count));
        spaces -= count;
    }

    void removeFirstVal(int count) {
        vals.front().second -= count;
        if(vals.front().second == 0) {
            vals.pop_front();
        }
        spaces += count;
    }

    int size() const {
        int res = spaces;
        for(auto [_, len] : vals) {
            res += len;
        }
        return res;
    }

    static std::uint64_t checksum(const std::vector<Slot>& slots) {
        std::size_t index{0};
        std::uint64_t res{0};
        for(const auto& slot : slots) {
            for(auto [val, count] : slot.vals) {
                for(int k=0; k<count; ++k) {
                    res += (index++) * val;
                }
            }
        }        
        return res;        
    }

    static std::vector<Slot> parse(std::string line) {
        line += '0';
        std::vector<Slot> res;
        int val = 0;
        for(size_t k=0; k<line.size(); k += 2, ++val) {
            auto& added = res.emplace_back();
            int count = int{line[k]} - 48;
            added.spaces = int{line[k+1]} - 48;
            added.vals.emplace_back(std::make_pair(val, count));
        }
        return res;
    }
};

void print(const std::vector<Slot>& slots) {
    for(const auto& slot :slots) {
        for(auto [val, c] : slot.vals) {
            for(int k=0; k<c; ++k) {
                std::cout << val;
            }
        }
        for(int k=0; k<slot.spaces; ++k) {
            std::cout << '.';
        }
    }
    std::cout << std::endl;
}

template<bool Print = true>
std::uint64_t phase1(const std::filesystem::path& src) {
    auto slots = Slot::parse(utils::readFile(src));

    int front = 0;
    int back = slots.size() - 1;

    auto shiftFront = [&]() {
        for( ; front < back && 0 == slots[front].spaces; ++front) {
        }
        return front < back && 0 < slots[front].spaces;
    };

    auto shiftBack = [&]() {
        for( ; front < back && 0 == slots[back].vals.size(); --back) {
        }
        return front < back && 0 < slots[back].vals.size();
    };

    while(shiftFront() && shiftBack()) {
        auto& recipient = slots[front];
        auto& under_move = slots[back];
        while (!under_move.vals.empty() && 0 < recipient.spaces) {
            int toMove = std::min<int>(under_move.vals.front().second, recipient.spaces);
            recipient.add(under_move.vals.front().first , toMove);
            under_move.removeFirstVal(toMove);
        }
    }

    if constexpr (Print) {
        print(slots);
    }

    return Slot::checksum(slots);
}

#include <list>
#include <variant>
#include <algorithm>

struct SlotsMap {
    std::uint64_t checksum() {
        std::size_t index{0};
        std::uint64_t res{0};
        for(const auto& slot : slots) {
            std::visit([&](const auto& val) {
                if constexpr (std::is_same_v< decltype(val) , const SlotsMap::Space& >) {
                    index += val.len;
                }
                if constexpr (std::is_same_v< decltype(val) , const SlotsMap::Value& >) {
                    for(int k = 0; k< val.len; ++k) {
                        res += (index++) * val.val;
                    }
                }
            }, slot);
        }        
        return res;        
    }

    static SlotsMap parse(std::string line) {
        auto tmp = Slot::parse(std::move(line));
        SlotsMap res;
        res.table.resize(tmp.size());
        for(const auto& el : tmp) {
            auto it = res.slots.insert(res.slots.end(), SlotVariant{});
            auto& val = it->emplace<Value>();
            val.val = el.vals.front().first;
            val.len = el.vals.front().second;
            res.table[val.val] = it;

            if(0 < el.spaces) {
                auto& space = res.slots.emplace_back().emplace<Space>();
                space.len = el.spaces;
            }
        }
        return res;
    }

    struct Space {
        size_t len;
    };
    struct Value {
        size_t len;
        int val;
    };
    using SlotVariant = std::variant<Space, Value>; 
    using SlotsList = std::list<SlotVariant>;

    std::pair<SlotsList::iterator, bool> firstSuitableSpace(SlotsList::iterator end) {
        int required_len = std::get<Value>(*end).len;
        std::pair<SlotsList::iterator, bool> res;
        res.second = false;
        res.first = std::find_if(slots.begin(), end, [&required_len](const SlotVariant& s) {
            const Space* as_space = std::get_if<Space>(&s);
            return as_space && required_len <= as_space->len;
        });
        if(res.first != end) {
            res.second = true;
        }
        return res;
    }

    void transfer(SlotsList::iterator space, SlotsList::iterator subject) {
        auto val = Value{std::get<Value>(*subject)};
        auto& val_space = std::get<Space>(*space);

        slots.insert(subject, Space{val.len});
        slots.erase(subject);

        subject = slots.insert(space, SlotVariant{val});
        val_space.len -= val.len;
        if(val_space.len == 0) {
            slots.erase(space);
        }
        table[val.val] = subject;
    }

    SlotsList slots;
    std::vector<SlotsList::iterator> table;
};

void print(const SlotsMap& slots) {
    for(const auto& val : slots.slots) {
        std::visit([](const auto& val) {
            if constexpr (std::is_same_v< decltype(val) , const SlotsMap::Space& >) {
                for(int k = 0; k< val.len; ++k) {
                    std::cout << '.';
                }
            }
            if constexpr (std::is_same_v< decltype(val) , const SlotsMap::Value& >) {
                for(int k = 0; k< val.len; ++k) {
                    std::cout << val.val;
                }
            }
        }, val);
    }
    std::cout << std::endl;
}

template<bool Print = true>
std::uint64_t phase2(const std::filesystem::path& src) {
    auto slots = SlotsMap::parse(utils::readFile(src));

    if constexpr (Print) {
        print(slots);
    }

    for(int back = slots.table.size() - 1; 0 < back; --back) {
        auto it_val = slots.table[back];
        auto [it_space, ok] = slots.firstSuitableSpace(it_val);
        if(ok) {
            slots.transfer(it_space, it_val);
        }

        // if constexpr (Print) {
        //     print(slots);
        // }
    }

    if constexpr (Print) {
        print(slots);
    }

    return slots.checksum();
}

int main() {
    // std::cout << phase1("sample.A.txt") << std::endl << std::endl;
    // std::cout << phase1("sample.B.txt") << std::endl << std::endl;
    // std::cout << phase1<false>("input.txt") << std::endl << std::endl;
    
    std::cout << phase2("sample.B.txt") << std::endl << std::endl;
    std::cout << phase2<false>("input.txt") << std::endl << std::endl;

    return 0;
}
