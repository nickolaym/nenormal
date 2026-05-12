#include <string>
#include <vector>
#include <tuple>
#include <functional>
#include <iostream>

using RuleDef = std::tuple<std::string, std::string, bool>; // search, replace, continue
using RuleSet = std::vector<RuleDef>;
using Callback = std::function<void(const std::string&, const RuleDef&, const std::string&)>;

std::string run_normal(RuleSet rules, std::string src, Callback callback = {}) {
    bool running = true;
    std::string prev;
    while (running) {
        for (const auto& p : rules) {
            const auto& [s, r, c] = p;
            size_t pos = src.find(s);
            if (pos != std::string::npos) {
                if (callback) prev = src;
                src.replace(pos, s.size(), r);
                if (callback) callback(prev, p, src);
                running = c;
                break;
            }
        }
    }
    return src;
}

template<int N> struct sss { char data[N]; };

constexpr auto s = sss{"hello"};
static_assert(sizeof(s) == 6);

int main() {
    RuleSet program = {
        {"a", "b", true},
        {"c", "d", false},
        {"e", "f", true},
    };
    std::string const src = "eeecccaaa";
    std::cout << src << " -> " << run_normal(program, src) << std::endl;
    run_normal(program, src,
        [](const std::string& src, const RuleDef& p, const std::string& dst) {
            const auto& [s, r, c] = p;
            std::cout
                << "- " << src
                << "(" << s << " -> " << r << (c ? " cont" : " stop") << ")"
                << " -> " << dst
                << std::endl;
        }
    );
}