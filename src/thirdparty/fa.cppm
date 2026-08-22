module;
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <type_traits>

export module fa;

export namespace fa {
#include "FA-Binary-Patches/magic_classes.h"
#include "FA-Binary-Patches/moho.h"
} // namespace fa
