module;
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <xbyak/xbyak.h>

export module fa;

export namespace fa {
#define FA_MODULE_IMPORT 1
#include "FA-Binary-Patches/magic_classes.h"
#include "FA-Binary-Patches/moho.h"
#include "FA-Binary-Patches/runtime.h"
#undef FA_MODULE_IMPORT
} // namespace fa
