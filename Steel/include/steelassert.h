#ifndef STEEL_ASSERT_H
#define STEEL_ASSERT_H

#include "steeltypes.h"
#include "Debug.h"

#define STEEL_ASSERT(EXPR, ...) if(!(EXPR)){Steel::Debug::log("Assert in ", __FILE__,"@", __LINE__, " on ").quotes(#EXPR)("==", bool(EXPR), ": ")(__VA_ARGS__).endl().breakHere();}

#define STEEL_WRONG_CODE_PATH(...) Debug::error(STEEL_FUNC_INTRO,"wrong code path. ")(__VA_ARGS__).endl().breakHere()

#endif // STEEL_ASSERT_H