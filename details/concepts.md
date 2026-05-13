# Концепты

В проекте есть множество шаблонных структур, которые удобно идентифицировать
с помощью концептов, вместо паттерн-матчинга.

```cpp
template<Parameter Definitions Of Foo> struct foo {...};
template<Parameter Definitions Of Bar> struct bar {...};

// C++17 : многословно
template<Parameter Definitions Of Foo And Bar>
void f(foo<Parameters Of Foo> x, bar<Parameters Of Bar> y);

// без концептов: слишком общо и ошибкоопасно
void f(/*foo<???>*/ auto x, /*bar<???>*/ auto y);

// с концептами
void f(Foo auto x, Bar auto y);
```

Для удобства и избавления от рутины сделаны макросы
```cpp
CONCEPT(Foo) // вводит концепт Foo<T> - "это некая структура с меткой Foo"

template<.....>
struct foo {
    REPRESENTS(Foo) // делает Foo<foo> истинным

    REPRESENTS_COND(Bar, bool_condition) // делает Bar<foo> истинным по условию
};
```

## Способы реализации

- внешняя булева метафункция
- статический член
  - булев флажок
  - дискриминирующая функция

### Специализация метафункции

Классический способ реализации подобных концептов - через специализацию метафункций.
```cpp
template<class T> constexpr bool is_Foo_v = false;
template<class T> concept Foo = is_Foo_v<T>;

...

template<.....> struct foo;
template<.....> constexpr bool is_Foo_v<foo<.....>> = true;
```
Плюсы:
- универсальность (можно применять и к посторонним типам)
- достаточно предобъявления структуры

Минусы:
- метафункция специализируется строго после объявления класса
- для использования внутри определения класса нужно делать предобъявление и его, и метафункции
- если не сделать предобъявление, то получим неопределённое поведение
  (вместо специализации возьмём основной шаблон)
- условная принадлежность должна быть выражена снаружи
- плохо запихивается в макросы:
  в определении специализации нужно указать и имя концепта, и имя типа, и в двух местах параметры шаблона
- неприменимо для локальных классов

### Проверочный член - флажок

Альтернативный способ - внести в класс статический проверочный член.
```cpp
template<class T> concept Foo = (T::is_Foo_v);

template<.....> struct foo {
    static constexpr bool is_Foo_v = true; // или условное значение
};
```

Плюсы:
- нет специализаций метафункции, разорванных областей видимости и связанного с этим риска неопределённого поведения
- можно пользоваться концептом внутри определения
- компактные определения, в которых фигурирует только имя концепта

Минусы:
- не поддерживает предобъявление класса
- не поддерживает квалифицированные имена!
- концепт и класс должны быть в общем пространстве имён,
  и если есть одноимённые концепты в разных пространствах имён,
  то возникнут ложные совпадения.

Опасность опечаток: для объявления флажка не требуется существование концепта.

Эту проблему можно устранить, если с помощью макроса рядом с флажком писать
`static_assert(!Foo<void>)` или что-нибудь в таком роде.

### Проверочный член - дискриминирующая функция

Дискриминирующая функция - это функция, чьё значение (и-или тип) зависит от типа аргумента.

Объявлять внутри класса шаблоны с явными специализациями нельзя,
но можно объявлять перегрузки функций.

Введём семейство статических членов `represent_concept(auto)`.

Поскольку сам концепт не является первоклассной сущностью,
то определим рядом с концептом вспомогательный тип-значение.

Для проверки - будем передавать это значение и смотреть
- удалось ли вообще найти подходящее имя функции, а затем и подходящую перегрузку
- что при этом получилось

```cpp
struct Foo_probe_t {};
template<class T> concept Foo = (... T::represents_concept(Foo_probe_t{}) ...);

template<.....> struct foo {
    static constexpr represents_concept(Foo_probe_t) .....
    static constexpr represents_concept(Bar_probe_t) .....
};

Плюсы:
- теперь поддерживает квалифицированные имена
  (можно даже в макросах, если имя проверочного типа сделать суффиксом)
- нет путаницы между одноимёнными концептами из разных пространств имён
```cpp
struct foo {
    static constexpr represents_concept(::some::ns::Foo_probe_t) .....
};
```

Минусы:
- всё ещё требует объявления класса

#### Функция, вычисляющая булево значение

```cpp
template<class T> concept Foo = (T::represents_concept(Foo_probe_t{}) == true);

template<.....> struct foo {
    static constexpr represents_concept(Foo_probe_t) { return true; }
    static constexpr represents_concept(Bar_probe_t) { return something; }
};
```

Внезапно, минус!
- нельзя использовать внутри определения класса в отношении самого себя!

```cpp
struct Foo_probe_t {};

struct foo {
    static constexpr represents_concept(Foo_probe_t) { return true; }
    static_assert(represents_concept(Foo_probe_t) == true);
    // error: non-constant condition for static assertion
    // error: `static constexpr foo::represents_concept(Foo_probe_t)`
    // called in a constant expression before its definition is complete
};
```

#### Объявление (без определения) функции с зависимым типом

Этого недостатка лишены функции, отличающиеся типом возвращаемого значения.

Чтобы извлечь булево значение, достаточно вывести тип (с помощью decltype).
Определение теперь не нужно, только объявление.

Эту технику использовали ещё в C++98, кстати сказать.

```cpp
template<class T> concept Foo = decltype(foo::represents_concept(Foo_probe_t{}))::value;

struct foo {
    static constexpr auto represents_concept(Foo_probe_t) -> std::true_type;
    static constexpr auto represent_concept(Bar_probe_t) -> std::bool_constant<cond>;

    static_assert(decltype(foo::represents_concept(Foo_probe_t{}))::value); // ok
    static_assert(Foo<foo>);
};
```

#### Управляемое объявление функции

Другой способ проверки - это просто наличие подходящей сигнатуры.

Концепт проверяет её с помощью requires.

А представитель включает-выключает сигнатуру - любым доступным способом.
- безусловно
- через технику SFINAE
- через технику requires - наиболее просто

```cpp
template<class T> concept Foo = requires { T::represents_concept(Foo_probe_t{}); };

struct foo {
    static constexpr void represents_concept(Foo_probe_t);
    static constexpr void represents_concept(Bar_probe_t) requires true;
    static constexpr void represents_concept(Buz_probe_t) requires cond;
};
```

### Существенная особенность проверочных членов

Концепт перестаёт отличать исходный тип (структуру) от ссылок на него.

В каких-то случаях это важно, в каких-то - нет.

## Концепты и универсальные ссылки

Когда мы пишем
```cpp
void f(Some auto&& x);
// эквивалентно
template<class T> void f(T&& x) requires Some<T>;
```
то в концепт Some передаётся не обязательно значение, а `V` / `const V` / `V&` / `const V&`,
где V - тип значения переменной x.

Поэтому, например, бессмысленно писать
```cpp
void f(std::same_as<int> auto&& x);
// стандартный концепт same_as<T, Param> = std::is_same_v<T, Param>
// то есть, здесь std::is_same_v<T, int>

void g() {
    f(123); // auto = int, подошло

    int x = 123;
    f(x); // auto = int&, не подошло
    f(std::move(x)); // auto = int, подошло
    
    int const y = x;
    f(y); // auto = int const&, не подошло
    f(std::move(y)); // auto = int const, не подошло!!!
}
```

Если у нас есть концепт значения `SomeValue<T>`,
то полезно определить концепт аргумента
```cpp
template <class T> concept SomeRef = SomeValue<std::remove_cvref_t<T>>;

void f(SomeRef auto&& x); // принимает SomeValue во всех видах ссылок
```

## Концепты и параметризованные типы

Если концепт заселён шаблонными типами,
то проверка "это какой-то инстанс данного шаблона" может оказаться слишком общей.

Можно ввести уточнение с помощью параметризованного концепта:
```cpp
template<class X> struct foo;

template<class T> concept Foo = /* как показано выше */;
// подходят все T = foo<X>

template<class T, class X> concept FooOfType =
    Foo<T> &&
    std::is_same_v<X, /* извлечь X из шаблона foo */>;
```
Самый простой способ извлечь параметр из шаблона структуры - это договориться,
что внутри структуры объявлен алиас (если параметр - тип)
или константа (если параметр - значение).

Традиционно это `using type = X` и `constexpr auto value = Value`.

Когда параметров много, то имена можно придумывать любые.

## Концепты и концепты

В ряде случаев (а в случае с компайл-таймовыми строками и их производными - всегда)
оказывается, что `FooOfType<T, ???>` указать невозможно, потому что вложенный тип
заранее неизвестен. И хотелось бы сделать что-то наподобие `FooOfConcept<T, Bar>`...

Концепт можно параметризовать
- типами
- константами
- шаблонами типов (в том числе шаблонами алиасов)

но не концептами и шаблонами констант.

Поэтому нам нужна метафункция - шаблон класса, моделирующего std::bool_constant.
Шаблон алиаса вполне устроит.

```cpp
template<class T> concept Foo = .....;
template<class T> using is_Foo = std::bool_constant<Foo<T>>;
template<class T> constexpr bool is_Foo_v = Foo<T>;

template<class T> concept Bar = .....;
template<class T, class U>
concept BarOfType = Bar<T> && std::same_as<typename T::type, U>;
template<class T, template<class>class C>
concept BarOfTraits = Bar<T> && C<T>::value;

// пример использования
void f(BarOfTraits<is_Foo> auto x) {
    using X = decltype(x);
    static_assert(Bar<X>);
    using XT = typename X::type;
    static_assert(Foo<XT>);
}

// или для краткости
template<class T> concept BarOfFoo = BarOfTraits<T, is_Foo>;
```
