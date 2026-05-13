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
CONCEPT_TYPECHECKER(RuleInput)
// concept of universal reference: f(RuleInputRef auto&& a)
template<class R> concept RuleInputRef = RuleInput<std::remove_cvref_t<R>>;

// Tristate input is a subclass of Tristate of RuleInput, namely NotMatchedYet

template<class T> concept RuleNotMatchedYetInput = NotMatchedYetOfTraits<T, is_RuleInput>;
// concept of universal reference: f(RuleNotMatchedYetInputRef auto&& a)
template<class R> concept RuleNotMatchedYetInputRef = RuleNotMatchedYetInput<std::remove_cvref_t<R>>;

// output

template<class T> concept RuleOutput        = TristateOfTraits<T, is_RuleInput>;
template<class T> concept RuleMatchedOutput = MatchedOfTraits<T, is_RuleInput>;
template<class T> concept RuleFailedOutput  = NotMatchedYetOfTraits<T, is_RuleInput>;
// Note that RuleNotMatchedYetInput == RuleFailedOutput

// inplace in-out arg

template<class T> concept RuleFixedInput = std::same_as<T, std::string> || InplaceAugmented<T>;
CONCEPT_TYPECHECKER(RuleFixedInput);
template<class T> concept RuleInplaceArg = InplaceOfTraits<T, is_RuleFixedInput>;

} // namespace nn
