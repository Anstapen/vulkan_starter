#pragma once
#include <expected>

namespace Ping
{

enum class Error
{
	NO_MEMORY,
	FILE_NOT_FOUND
};

template <typename T>
using Expected = std::expected<T, Error>;

}