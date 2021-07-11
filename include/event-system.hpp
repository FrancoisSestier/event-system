#pragma once

#include <functional>
#include <queue>
#include <unordered_map>

namespace epp {

    namespace details {

        using id_type = uint32_t;

        using dtor = std::function<void(void*)>;

        class type_id_builder {
            inline static id_type identifier() noexcept {
                static id_type value = 0;
                return value++;
            }

            template <typename T>
            inline static id_type get_sanitized() {
                static const id_type value = identifier();
                return value;
            }

           public:
            template <typename T>
            inline static id_type get() noexcept {
                return get_sanitized<
                    std::remove_cv_t<std::remove_reference_t<T>>>();
            }
        };

        template <typename T>
        inline static id_type type_id() {
            return details::type_id_builder::get<T>();
        }
    }  // namespace details


    class event_queue {

       public:
        using internal_event_queue = std::queue<std::pair<details::id_type, void*>>;
        using event_dtors = std::unordered_map<details::id_type, details::dtor>;

       public:
        event_queue() = default; //ctor

        ~event_queue() { clear(); } //dtor

        event_queue(const event_queue& other)  // copy ctor
            : _event_queue(other._event_queue),
              _event_dtors(other._event_dtors) {}

        event_queue(event_queue&& other) noexcept //move ctor
        {
            std::swap(_event_queue,other._event_queue);
            std::swap(_event_dtors,other._event_dtors);
        }

        event_queue& operator=(const event_queue& other)  // copy assignment
        {
            return *this = event_queue(other);
        }

        event_queue& operator=(event_queue&& other) noexcept  // move assignment
        {
            return *this = event_queue(std::move(other));
        }

        template <typename Event, typename... Args>
        void push(Args... args) {
            ensure_dtor<Event>();
            _event_queue.push(std::make_pair<details::id_type,void*>(details::type_id<Event>(),
                              new Event(std::forward<Args>(args)...)));
        }

        auto front() { return _event_queue.front(); }

        void pop() {
            auto&& event = _event_queue.front();
            _event_dtors.at(event.first)(event.second); // calling appropriate dtor;
            _event_queue.pop();
        }

        bool empty() { return _event_queue.empty(); }

        void clear() {
            while (!empty()) {
                pop();
            }
        }

        size_t size() { return _event_queue.size(); }


       private:
        template <typename Event>
        void ensure_dtor() {
            if (!_event_dtors.contains(details::type_id<Event>())) {
                _event_dtors.emplace(details::type_id<Event>(), [](void* obj) {
                    delete static_cast<Event*>(obj);
                });
            }
        }

       private:
        internal_event_queue _event_queue;
        event_dtors _event_dtors;
    };

    class dispatcher {
       public:
        using event_callback = std::function<void(void*)>;
        using listners
            = std::unordered_map<details::id_type, std::vector<event_callback>>;

        template <class EventType, class ListenerCallback>
        void on(ListenerCallback listener) {
            if (!_listeners.contains(details::type_id<EventType>())) {
                _listeners.try_emplace(details::type_id<EventType>(),
                                       std::vector<event_callback>());
            }
            _listeners[details::type_id<EventType>()].push_back(
                [=](void* event) {
                listener(*static_cast<EventType*>(event));
            });
        }

        template <class EventType, typename... Args>
        void emit(Args... args) {
            EventType event{std::forward<Args>(args)...};
            emit_impl(event);
        }

        template <class EventType, typename... Args>
        void emit(EventType&& event) {
            emit_impl(event);
        }

        void emit(event_queue& queue) { 
            while (!queue.empty()) {
                emit_impl(queue.front().first, queue.front().second);
                queue.pop();
            }
        }

        void clear() { _listeners.clear(); }

       private:
        template <class EventType>
        void emit_impl(EventType& event) {
            emit_impl(details::type_id<EventType>(),&event);
        }

        void emit_impl(details::id_type id, void* event) {
            if (_listeners.contains(id)) {
                for (auto it : _listeners[id]) {
                    it(event);
                }
            }
        }

       private:
        listners _listeners;
    };
}  // namespace epp
