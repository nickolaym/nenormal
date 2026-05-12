#include "nenormal/concepts.h"
#include <gtest/gtest.h>

#include <iostream>

namespace some {

CONCEPT(Foo1) // with single foo
CONCEPT(Foo2) // with single foo
CONCEPT(Bar) // with many bar1, bar2
CONCEPT(Buz) // empty

} // namespace some

static_assert(CONCEPT_IS_DEFINED(some::Foo1));
static_assert(CONCEPT_IS_DEFINED(some::Foo2));
static_assert(CONCEPT_IS_DEFINED(some::Bar));
static_assert(CONCEPT_IS_DEFINED(some::Buz));

namespace another {

struct foo {
    // a struct may represent multiple concepts
    REPRESENTS(some::Foo1);
    REPRESENTS(some::Foo2);

    // a concept is available inside the struct
    static_assert(some::Foo1<foo>);
};

// multiple structs may represent same concept
struct bar1 {
    REPRESENTS(some::Bar);
};
struct bar2 {
    REPRESENTS(some::Bar);
};

} // namespace another

static_assert(some::Foo1<another::foo>);
static_assert(some::Foo2<another::foo>);

static_assert(some::Bar<another::bar1>);
static_assert(some::Bar<another::bar2>);

// negative statements works, too
static_assert(!some::Bar<another::foo>);
static_assert(!some::Buz<another::foo>);

// conditional representation
template<bool C> struct maybe_bar {
    REPRESENTS_COND(::some::Bar, C);
};

static_assert(some::Bar<maybe_bar<true>>);
static_assert(!some::Bar<maybe_bar<false>>);
