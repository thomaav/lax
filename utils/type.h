#pragma once

#include <memory>
#include <stdint.h>

typedef uint8_t u8;
typedef int8_t i8;

typedef uint8_t u8;
typedef int8_t i8;

typedef uint32_t u32;
typedef int32_t i32;

typedef uint64_t u64;
typedef int64_t i64;

template <typename T> using uref = std::unique_ptr<T>;
template <typename T, typename... Args> std::unique_ptr<T> make_uref(Args &&...args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T> using ref = std::shared_ptr<T>;
template <typename T, typename... Args> std::shared_ptr<T> make_ref(Args &&...args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}
