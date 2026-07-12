#pragma once

namespace Mupfel
{
/**
 * Assigns each component type `T` a stable, process-lifetime index (0, 1, 2, ...) on first use,
 * used as the bit position in `Entity::Signature` and the slot in `Registry::component_buffer`.
 */
class ComponentIndex
{
public:
	/** Returns `T`'s index, assigning the next unused one the first time it's called for `T`. */
	template <typename T> static size_t Index() noexcept
	{
		static const size_t id = comp_counter++;
		return id;
	}

private:
	/** Next index to hand out to a not-yet-seen component type. */
	static inline size_t comp_counter = 0;
};
} // namespace Mupfel
