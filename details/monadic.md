# Объяснения на языке монад

**DISCLAIMER**

Это черновик рассуждений.
Несмотря на то, что реализация изоморфна монаде Either, но детали отличаются.

Тем не менее, наивная Either-реализация тоже оказалась работоспособной.
(Но ужасно неэфеективной с точки зрения нагрузки на компилятор).

Набросок кода - в [monadic.cpp](/experimental/monadic.cpp)

И в ветке [dev/monadic](https://github.com/nickolaym/nenormal/tree/dev/monadic)

### Домен

Доменом (предметной областью) является либо просто строка `CtStr`, либо аугментированная строка.

Когда мы ниже дойдём до аугментации, раскроем эту тему, а пока будем говорить об абстрактном домене `Domain`.

Теоретически, можем говорить про то, что у нас есть два домена - исходных значений и финальных.
То есть, обычные и финальные правила НАМ-машины могут давать "чуть-чуть разные строки" (или разную аугментацию).

Поэтому, чтобы не склеивать их раньше времени, введём ещё и `FinalDomain`.

### Первый подход к снаряду: Maybe или Either?

Правило подстановки принимает на вход значение Domain и возвращает неуспех / успех (Domain с признаком остановки/продолжения)

Кажется, это ложится на монаду Maybe, но на самом деле - на монаду Either. И вот каким образом.

Помимо одиночного правила, у нас есть последовательность альтернатив и цикл программы.

Тело цикла принимает на вход текущее состояние, пропускает его через программу (содержащую альтернативы) и получает Domain с признаком остановки/продолжения.

Правило принимает стартовое значение и отдаёт... разберёмся, что именно
```haskell
type Rule = Domain -> RuleOutput
```
Программа принимает стартовое значение и отдаёт... разберёмся, что именно
```haskell
type Program = Domain -> ProgramOutput
```

### Цикл

Цикл принимает стартовое значение и отдаёт финальное значение (явный или неявный останов)
```haskell
type MachineLoop = Program -> Domain -> FinalDomain

machineLoop prg src = (inject src >>= loopedPrg) $ extract
    where
        inject tmp = Right tmp
            -- для запуска цепочки нам нужно правое значение
        loopedPrg tmp = tmp >>= prg >>= loopedPrg
            -- распакованное правое значение запихиваем в цепочку,
            -- которая или оборвётся после prg, или продолжится в рекурсии
        extract either_dst_tmp = takeLeft undefined either_dst_tmp
            -- мы уверены, что если цикл остановился,
            -- то там не может быть Right tmp, а только Left dst
        -- откуда понятно, что
        loopedPrg :: Domain -> Either FinalDomain Domain
        -- и, конечно,
        prg :: Domain -> Either FinalDomain Domain
```

### Программа

Логично, что
```haskell
type ProgramOutput = Either FinalDomain Domain
type Program = Domain -> Either FinalDomain Domain
```
И мы тоже можем её завернуть в монаду Either.

Но правила (одиночное или цепочка альтернатив) могут вернуть три варианта
- успешная подстановка, продолжаем
- успешная подстановка, финиш
- неуспех

А нам для цикла нужно только два:
- подстановка, продолжаем
- подстановка, финиш

Чтобы неуспеха в принципе не было, добавим после альтернатив ещё одну: `FINAL_RULE("", "")`.

Это правило ничего не делает, кроме как переводит в состояние "финиш".

```haskell
stopper = finalRule "" ""

program ruleset src = (inject src >>= ruleset >>= stopper) $ extract
    where
        inject :: Domain -> Either ProgramOutput MachineData
        inject src = Right src
            -- для запуска альтернатив нам нужно правое значение

        extract either_dst_or_src = takeLeft undefined either_dst_or_src
            -- мы уверены (благодаря stopper), что правого значения там нет
```
Таким образом, `RuleOutput`
- это либо `Left ProgramOutput` в случае успешной подстановки,
- либо `Right Domain` в случае неуспешной!

```haskell
type RuleOutput = Either ProgramOutput Domain
```

### Цепочка альтернатив

Цепочка альтернатив (список правил)
```haskell
rules (r:rs) src = src >>= r >>= rules rs
rules []     src = Right src
```
Мы или обрываем перебор альтернатив на успешном r, или продолжаем.

Одиночное правило должно следовать этой же логике.
```haskell

-- пусть у нас есть примитив работы со строками
trySubstitute :: Str -> Str -> Str -> Maybe Str

-- и распаковка-упаковка строки в домен
fromDomain :: Domain -> Str
toNewDomain :: Domain -> Str -> Domain

singleRule search replace willFinishOrContinue = fun
    where
        fun :: Domain -> RuleOutput
        fun src = case (trySubstitute search replace (fromDomain src)) of
            Just t -> Left (willFinishOrContinue (toNewDomain src t)
            Nothing  -> Right src

willFinish   dst = Left dst
willContinue dst = Right dst
```
