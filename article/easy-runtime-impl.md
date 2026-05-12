# Реализация Нормальных Алгорифмов Маркова в рантайме

Сразу добавим возможность трассировки - колбек, который будет вызываться при каждой подстановке.

Вот пример на питоне
```py
RuleDef = Tuple [str, str, bool] # search, replace, continue
RuleSet = List [RuleDef]
Callback = Callable [[str, RuleDef, str], None]

def run_normal(ps: RuleSet, src: str, callback: Optional [Callback] = None) -> str:
    running = True
    while running:
        for p in ps:
            search, replace, cont = p
            if s in src:
                dst = src.replace(search, replace, 1)
                if callback:
                    callback(src, p, dst)
                src, running = dst, cont
                break
    return src
```
или на C++
```cpp
using RuleDef = std::tuple<std::string, std::string, bool>;
using RuleSet = std::vector<RuleDef>;
using Callback = std::function<void(const std::string&, const RuleDef&, const std::string&)>;

std::string run_normal(RuleSet rules, std::string src, Callback callback = {}) {
    bool running = true;
    std::string prev;
    while (running) {
        for (const auto& p : rules) {
            const auto& [search, replace, cont] = p;
            size_t pos = src.find(search);
            if (pos != std::string::npos) {
                if (callback) prev = src;
                src.replace(pos, search.size(), replace);
                if (callback) callback(prev, p, src);
                running = cont;
                break;
            }
        }
    }
    return src;
}
```
