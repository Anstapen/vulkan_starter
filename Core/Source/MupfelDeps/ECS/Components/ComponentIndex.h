#pragma once

namespace Mupfel {
	class ComponentIndex
	{
	public:

		template<typename T>
		static size_t Index() noexcept {
			static const size_t id = comp_counter++;
			return id;
		}

	private:
		static inline size_t comp_counter = 0;
	};
}


