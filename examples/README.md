# Примеры нормальных алгорифмов

Смотрите в коде юниттестов.

## Пример: hello world

[hello_world.cpp](hello_world.cpp)

Самый простой случай. Заменяем независимые подстроки, пока заменяется.

```cpp
constexpr auto program = rules<
    rule<"hello"_ss, "privet"_ss>{},
    rule<"world"_ss, "mir"_ss>{}
>{};

constexpr auto src = "hello, world!"_cts;
constexpr auto dst = program(src);

std::cout << src.value.value << std::endl; // hello world
std::cout << dst.value.value << std::endl; // privet mir
```

## Пример: правильная скобочная последовательность

[correct_bracket_sequence.cpp](correct_bracket_sequence.cpp)

Это более сложный пример, здесь важен порядок выполнения правил.
1) сперва удаляем все парные скобки
  - "()" -> ""
  - "[()]" -> "[]" -> "" - по индукции
2) затем у нас остаётся ворох непарных скобок, мы их замазываем одним символом
  - "(" / "[" / ... -> "_"
3) затем сокращаем всю цепочку замазок до одиночного символа замазки
  - "__" -> "_"
  - "___" -> "__" -> "_" - по индукции
4) наконец, у нас либо пустая строка, либо символ замазки.
  - "_" -> "FAILURE"

```cpp
constexpr auto reduce_pairs = rules<  // сокращаем парные скобки
    rule<"()"_ss, ""_ss>{},
    rule<"[]"_ss, ""_ss>{},
    rule<"{}"_ss, ""_ss>{}
>{};

constexpr auto unify_unpaired = rules<  // замазываем непарные
    rule<"("_ss, "_"_ss>{},
    rule<")"_ss, "_"_ss>{},
    rule<"["_ss, "_"_ss>{},
    rule<"]"_ss, "_"_ss>{},
    rule<"{"_ss, "_"_ss>{},
    rule<"}"_ss, "_"_ss>{}
>{};

constexpr auto shrink_unpaired = rule<"__"_ss, "_"_ss>{};

constexpr auto failure_message = rule<"_"_ss, "FAILURE"_ss>{};

constexpr program = rules<
    reduce_pairs,
    // если дошли до этого места, то в тексте не осталось парных скобок
    unify_unpaired,
    // если дошли до этого места, то текст - это серия подчёркиваний, либо пустая строка
    shrink_unpaired,
    // если дошли до этого места, то текст - это одиночное подчёркивание, либо пустая строка
    failure_message
    // одиночное подчёркивание заменяем на слово FAILURE
>{};

static_assert(program(""_cts) == ""_cts);
static_assert(program("()({})[(()[])]"_cts) == ""_cts);
static_assert(program("()({})[(()[])"_cts) == "FAILURE"_cts);
```

## Пример: сложение десятичных чисел

Самый навороченный, на данный момент, пример.

Он показывает, что программирование на НАМ - не линейно.

Подробности вынес в отдельное место [arithmetics/](arithmetics/README.md)

## Пример: последовательность Коллаца (3N+1)

[collatz/](collatz/README.md)

Не могу пройти мимо, Коллац - это обязательное упражнение.
