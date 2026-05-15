#pragma once

#include "../concepts.h"
#include "../ct.h"
#include "../str.h"
#include "../augmented.h"
#include "../tristate.h"
#include "../inplace/inplace_augmented.h"
#include "../inplace/inplace_tristate.h"

namespace nn {

// Note that these concepts do not distinguish value type itself and a reference to it.
// So, if using S = str<1>, these expressions are true: Str<S>, Str<const S>, Str<S&>.

// input
// - bare CtStr
// - augmented string

template<class T> concept MachineData = CtStr<T> || Augmented<T>;
CONCEPT_TYPECHECKER(MachineData)

// Tristate input is a subclass of Tristate of MachineData, namely NotMatchedYet

template<class T> concept RuleNotMatchedYetInput = NotMatchedYetOfTraits<T, is_MachineData>;

// output

template<class T> concept RuleOutput        = TristateOfTraits<T, is_MachineData>;
template<class T> concept RuleMatchedOutput = MatchedOfTraits<T, is_MachineData>;
template<class T> concept RuleFailedOutput  = NotMatchedYetOfTraits<T, is_MachineData>;
template<class T> concept RuleFinalOutput   = MatchedFinalOfTraits<T, is_MachineData>;
// Note that RuleNotMatchedYetInput == RuleFailedOutput

// inplace in-out arg

template<class T> concept RuleFixedInput = std::same_as<T, std::string> || InplaceAugmented<T>;
CONCEPT_TYPECHECKER(RuleFixedInput);
template<class T> concept RuleInplaceArg = InplaceOfTraits<T, is_RuleFixedInput>;

} // namespace nn
