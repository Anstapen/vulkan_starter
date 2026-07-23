#pragma once
#include <expected>

namespace Mupfel
{

enum class Error
{
	NO_MEMORY,
	FILE_NOT_FOUND
};

template <typename T>
using Expected = std::expected<T, Error>;

}