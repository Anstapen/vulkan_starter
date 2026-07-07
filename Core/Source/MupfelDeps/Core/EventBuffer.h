#pragma once
#include <cstdint>
#include <concepts>
#include <optional>
#include <vector>
#include "Event.h"

namespace Mupfel {
	/**
	 * @brief This interface adds a behavior to that every EventBuffer
	 * should implement.
	 */
	class IEventBuffer {
	public:
		/**
		 * @brief This pure virtual function needs to be implemented to
		 * add a way to retrieve the currently pending events of the
		 * EventBuffer.
		 * @return The currently pending events of the buffer.
		 */
		virtual uint64_t GetPendingEvents() = 0;

		/**
		 * @brief This pure virtual function needs to be implemented to
		 * add a way to clear the EventBuffer.
		 */
		virtual void Clear() = 0;
	};

	/**
	 * @brief The EventBuffer class that actually be instantiated.
	 * It basically just wraps a vector of the given type T.
	 * @tparam T The Event type that the buffer should hold.
	 */
	template<typename T>
		requires std::derived_from<T, Event>
	class EventBuffer : public IEventBuffer {
	public:
		using const_iterator = typename std::vector<T>::const_iterator;
	public:
		/**
		 * @brief The constructor.
		 * @param initial_size The initial size of the underlying vector.
		 */
		EventBuffer(uint32_t initial_size);

		/**
		 * @brief Add an event at the end of the buffer.
		 * @param event The event to be added.
		 */
		void Add(T &&event);

		/**
		 * @brief Retrieve the begin iterator of the underlying vector.
		 * @return begin iterator.
		 */
		const_iterator begin() const { return event_buf.begin(); }

		/**
		 * @brief Retrieve the end iterator of the underlying vector.
		 * @return end iterator.
		 */
		const_iterator end() const { return event_buf.end(); }

		/**
		 * @brief Get the event at the given index.
		 * @param index The index of the wanted event.
		 * @return If index is smaller than the current size of the
		 * buffer, returns the event at that location, otherwise it returns
		 * std::nullopt.
		 */
		std::optional<const T*> Get(uint32_t index);

		/**
		 * @brief Get the last event in the buffer.
		 * @return If the buffer currently holds events, retrieve the last one,
		 * std::nunllopt otherwise.
		 */
		std::optional<const T*> GetLatest();

		/**
		 * @brief Clear the buffer.
		 */
		void Clear() override;

		/**
		 * @brief Get amount of events currently in the buffer.
		 * @return Essentially the size of the underlying vector.
		 */
		uint64_t GetPendingEvents() override;
	private:
		/**
		 * @brief The vector that holds the events.
		 */
		std::vector<T> event_buf;
	};

	template<typename T>
		requires std::derived_from<T, Event>
	inline EventBuffer<T>::EventBuffer(uint32_t initial_size) : event_buf(initial_size)
	{
		event_buf.clear();
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline void EventBuffer<T>::Add(T &&event)
	{
		event_buf.emplace_back(event);
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline std::optional<const T*> EventBuffer<T>::Get(uint32_t index)
	{
		if (index < event_buf.size())
		{
			return &event_buf.at(index);
		}
		else {
			return std::nullopt;
		}
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline std::optional<const T*> EventBuffer<T>::GetLatest()
	{
		if (!event_buf.empty())
		{
			return &event_buf.back();
		}
		else {
			return std::nullopt;
		}
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline void EventBuffer<T>::Clear()
	{
		event_buf.clear();
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline uint64_t EventBuffer<T>::GetPendingEvents()
	{
		return event_buf.size();
	}
}