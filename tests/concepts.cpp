#include "nenormal/concepts.h"
#include <gtest/gtest.h>

CONCEPT(Foo1) // with single foo
CONCEPT(Foo2) // with single foo
CONCEPT(Bar) // with many bar1, bar2
CONCEPT(Buz) // empty

static_assert(CONCEPT_IS_DEFINED(Foo1));
static_assert(CONCEPT_IS_DEFINED(Foo2));
static_assert(CONCEPT_IS_DEFINED(Bar));
static_assert(CONCEPT_IS_DEFINED(Buz));

struct foo {
    // a struct may represent multiple concepts
    REPRESENTS(Foo1);
    REPRESENTS(Foo2);

    // a concept is available inside the struct
    static_assert(Foo1<foo>);
};

// multiple structs may represent same concept
struct bar1 {
    REPRESENTS(Bar);
};
struct bar2 {
    REPRESENTS(Bar);
};

static_assert(Foo1<foo>);
static_assert(Foo2<foo>);

static_assert(Bar<bar1>);
static_assert(Bar<bar2>);

// negative statements works, too
static_assert(!Bar<foo>);
static_assert(!Buz<foo>);
