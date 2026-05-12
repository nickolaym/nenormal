#pragma once

#include "../concepts.h"
#include "../ct.h"
#include "../str.h"
#include "../augmented.h"
#include "../tristate.h"
#include "../inplace/inplace_augmented.h"
#include "../inplace/inplace_tristate.h"

namespace nn {

// naming conventions
// SomeConceptName corresponds to a value type
// SomeConceptNameRef is an universal reference,
// for arg-matching like foo(SomeConceptNameRef auto&& arg)

// input
// - bare CtStr
// - augmented string

template<class T> concept RuleInput = CtStr<T> || Augmented<T>;
// concept of universal reference: f(RuleInputRef auto&& a)
template<class R> concept RuleInputRef = RuleInput<std::remove_cvref_t<R>>;

// Tristate input is a subclass of Tristate of RuleInput, namely NotMatchedYet

template<class T> concept RuleNotMatchedYetInput = NotMatchedYet<T> && RuleInput<typename T::type>;
// concept of universal reference: f(RuleNotMatchedYetInputRef auto&& a)
template<class R> concept RuleNotMatchedYetInputRef = RuleNotMatchedYetInput<std::remove_cvref_t<R>>;

// output

template<class T> concept RuleOutput = Tristate<T> && RuleInput<typename T::type>;
template<class T> concept RuleMatchedOutput = RuleOutput<T> && !NotMatchedYet<T>;
template<class T> concept RuleFailedOutput = RuleOutput<T> && NotMatchedYet<T>;
// Note that RuleNotMatchedYetInput == RuleFailedOutput

// inplace in-out arg

template<class T> concept RuleFixedInput = std::same_as<T, std::string> || InplaceAugmented<T>;
template<class T> concept RuleInplaceArg = Inplace<T> && RuleFixedInput<typename T::type>;

} // namespace nn
