from typing import Tuple, List, Optional, Callable

Regular, Final = True, False # continue / stop the loop

RuleDef = Tuple [str, str, bool] # search, replace, continue
RuleSet = List [RuleDef]
Callback = Callable [[str, RuleDef, str], None]

def run_normal(ps: RuleSet, src: str, callback: Optional [Callback] = None) -> str:
    running = True
    while running:
        for p in ps:
            search, replace, kind = p
            if s in src:
                dst = src.replace(search, replace, 1)
                if callback:
                    callback(src, p, dst)
                src, running = dst, kind
                break
    return src
