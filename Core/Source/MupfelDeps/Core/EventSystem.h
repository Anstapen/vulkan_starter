#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <optional>
#include <ranges>
#include <functional>
#include "EventBuffer.h"

namespace Mupfel {

	/**
	 * @brief The EventSystem class.
	 * 
	 * The EventSystem class is responsible for storing all events that get issued by
	 * the Engine Subsystems and custom systems implemented by the Application that
	 * uses Mupfel.
	 * 
	 * Currently the Event System supports asynchronous events that are handled on a
	 * per-frame basis:
	 * Every Event that gets added is available in the next frame. So all events have
	 * a inherent delay of one frame.
	 * Every Event only exists for one frame, specifically the frame after the one in
	 * which it gets added.
	 * 
	 * Also, the Event System supports synchronous Events, that other systems can
	 * subscribe to. Whenever an Event gets issued using AddImmediateEvent, all 
	 * registered callbacks for that Event are executed.
	 */
	class EventSystem {
	public:
		/**
		 * @brief The Event System holds one EventBuffer for every Event type that gets
		 * added and stores them in an unordered map. To be safe regarding memory leaks,
		 * the Eventbuffers are stored as unique pointers.
		 */
		typedef std::vector<std::unique_ptr<IEventBuffer>> EventBufferArray;
	public:
		/**
		 * @brief The constructor of the EventSystem class.
		 */
		EventSystem();

		/**
		 * @brief Swaps the EventBufferMaps. Should be called at the beginning or the
		 * end of every frame, but to more than one time per frame.
		 */
		void Update();

		/**
		 * @brief Retrieve the number of events that were issued last frame.
		 * @return events last frame.
		 */
		uint64_t GetLastEventCount() const;

		/**
		 * @brief Add an Event to its Eventbuffer. If there is no EventBuffer for that
		 * Event type yet, it creates one.
		 * @tparam T The type of Event that should be added.
		 * @param event The event data.
		 */
		template<typename T>
			requires std::derived_from<T, Event>
		void AddEvent(T &&event);

		/**
		 * @brief Add an Immediate Event which synchronously calls all registered
		 * callbacks. Additionally, the Event is queued in the Event Queue.
		 * @tparam T The event type.
		 * @param event The event that will be added.
		 */
		template<typename T>
			requires std::derived_from<T, Event>
		void AddImmediateEvent(T&& event);

		/**
		 * @brief Get the amount of events currently pending for the specified
		 * Event type.
		 * @tparam T The Event type.
		 * @return The number of events pending for the given Event type.
		 */
		template<typename T>
			requires std::derived_from<T, Event>
		uint64_t GetPendingEvents();

		/**
		 * @brief Retieve an Event of type \p T at the given index.
		 * @tparam T The type of the wanted event.
		 * @param index The index of the wanted event.
		 * @return The event.
		 */
		template<typename T>
			requires std::derived_from<T, Event>
		std::optional<const T*> GetEvent(uint32_t index);

		/**
		 * @brief Retrieve the latest event of the given type.
		 * @tparam T The type of the event.
		 * @return The latest event.
		 */
		template<typename T>
			requires std::derived_from<T, Event>
		std::optional<const T*> GetLatestEvent();

		/**
		 * @brief Return a subrange of the Eventbuffer of the given type T.
		 * @tparam T The Event type for which the subrange should be returned.
		 * @return The range begin-end of the underlying Eventbuffer, or of an empty
		 * Eventbuffer.
		 */
		template<typename T>
			requires std::derived_from<T, Event>
		auto GetEvents() const noexcept
			-> std::ranges::subrange<typename EventBuffer<T>::const_iterator,
			typename EventBuffer<T>::const_iterator>;

		/**
		 * @brief Register an event callback for the given event type.
		 * @tparam T the event type for which the callback shall be registered.
		 * @param callback the callback.
		 */
		template<typename T>
		void RegisterListener(std::function<void(const T&)> callback);

		/**
		 * @brief Retrieve the ID for the given Event type.
		 * @tparam T the event type.
		 * @return The ID of the given Event Type.
		 */
		template<typename T>
			requires std::derived_from<T, Event>
		size_t EventTypeToID();

	private:

		/**
		 * @brief Returns the index for the given event type.
		 * @tparam T event type.
		 * @return index.
		 */
		template<typename T>
		static size_t EventIndex() noexcept {
			static const size_t id = evt_counter++;
			return id;
		}

		/**
		 * @brief Check whether the given buffer entry exists.
		 * @tparam T the buffer to check.
		 * @param buffer_index the index to check.
		 * @return true if the entry exists, false otherwise
		 */
		template<typename T>
			requires std::derived_from<T, Event>
		bool EventBufferEntryExists(uint32_t buffer_index) const;

	private:
		/**
		 * @brief Two maps that hold the eventbuffers and get swapped every frame.
		 */
		EventBufferArray event_buffer_array[2];

		/**
		 * @brief The index of the current EventBufferMap. Events that are read 
		 * this frame are read from this map.
		 */
		uint32_t current;

		/**
		 * @brief The index of the next EventBufferMap. Events that are added
		 * this frame are put in this buffer and are available in the next frame.
		 */
		uint32_t next;

		/**
		 * @brief The amount of events that were issued last frame.
		 */
		uint64_t events_last_frame = 0;

		/**
		 * @brief The amount of events that were issued this frame.
		 */
		uint64_t events_this_frame = 0;

		/**
		 * @brief This counter serves as an ID for the different Event Types.
		 */
		static inline size_t evt_counter = 0;

		/**
		 * @brief The type of callback that can be registered for event.
		 */
		using EventCallback = std::function<void(const Event&)>;

		/**
		 * @brief A map of callbacks for all given events.
		 */
		std::unordered_map<uint64_t, std::vector<EventCallback>> listeners;
	};

	template<typename T>
		requires std::derived_from<T, Event>
	inline void EventSystem::AddEvent(T &&event)
	{
		size_t evt_index = EventIndex<T>();

		if (evt_index >= event_buffer_array[next].size())
		{
			event_buffer_array[next].resize(evt_index + 1);
		}

		if (!event_buffer_array[next][evt_index])
		{
			event_buffer_array[next][evt_index] = std::make_unique<EventBuffer<T>>(16);
		}
		
		EventBuffer<T>* current_evt_buffer = static_cast<EventBuffer<T> *>(event_buffer_array[next][evt_index].get());

		current_evt_buffer->Add(std::move(event));

		events_this_frame++;
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline void EventSystem::AddImmediateEvent(T&& event)
	{
		AddEvent<T>(T(event));
		size_t evt_index = EventIndex<T>();

		auto it = listeners.find(evt_index);

		if (it != listeners.end())
		{
			for (auto& callback : it->second)
			{
				callback(event);
			}
		}
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline std::optional<const T*> EventSystem::GetEvent(uint32_t index)
	{
		size_t evt_index = EventIndex<T>();

		if (!EventBufferEntryExists<T>(current))
		{
			return std::nullopt;
		}

		EventBuffer<T>* buf = static_cast<EventBuffer<T> *>(event_buffer_array[current][evt_index].get());
		return buf->Get(index);

	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline std::optional<const T*> EventSystem::GetLatestEvent()
	{
		size_t evt_index = EventIndex<T>();

		if (!EventBufferEntryExists<T>(current))
		{
			return std::nullopt;
		}

		EventBuffer<T>* buf = static_cast<EventBuffer<T> *>(event_buffer_array[current][evt_index].get());
		return buf->GetLatest();

	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline auto EventSystem::GetEvents() const noexcept -> std::ranges::subrange<typename EventBuffer<T>::const_iterator, typename EventBuffer<T>::const_iterator>
	{

		/*
			If the eventbuffer of the given type is not found (because no events were added yet),
			return the begin/end iterator of an empty buffer of that type.
		*/
		if (!EventBufferEntryExists<T>(current)) [[unlikely]]
		{
			static const EventBuffer<T> empty_buffer(0);
			return std::ranges::subrange(empty_buffer.begin(), empty_buffer.end());
		}

		const EventBuffer<T>* buf = static_cast<EventBuffer<T> *>(event_buffer_array[current][EventIndex<T>()].get());
		return std::ranges::subrange(buf->begin(), buf->end());
	}

	template<typename T>
	inline void EventSystem::RegisterListener(std::function<void(const T&)> callback)
	{
		size_t index = EventIndex<T>();
		listeners[index].push_back(
			[cb = std::move(callback)](const Event& evt) {
				cb(static_cast<const T&>(evt));
			}
		);
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline size_t EventSystem::EventTypeToID()
	{
		return EventIndex<T>();
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline bool EventSystem::EventBufferEntryExists(uint32_t buffer_index) const
	{
		size_t evt_index = EventIndex<T>();
		return (evt_index < event_buffer_array[buffer_index].size()) && event_buffer_array[buffer_index][evt_index];
	}

	template<typename T>
		requires std::derived_from<T, Event>
	inline uint64_t EventSystem::GetPendingEvents()
	{
		auto index = EventIndex<T>();
		if (!EventBufferEntryExists<T>(current))
		{
			return 0;
		}

		return event_buffer_array[current][index].get()->GetPendingEvents();

	}

}