#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>

namespace epp {

    using EventQueue = std::queue<std::unique_ptr<void*>>;

    namespace details {

        using id_type = uint32_t;

        class type_id {
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
    }  // namespace details

    class dispatcher {
       public:
        using EventCallback = std::function<void(void*)>;

        template <class EventType, class ListenerCallback>
        void on(ListenerCallback listener) {
            if (!_listeners.contains(details::type_id::get<EventType>())) {
                _listeners.try_emplace(details::type_id::get<EventType>(),
                                       std::vector<EventCallback>());
            }
            _listeners[details::type_id::get<EventType>()].push_back([=](void* event) {
                listener(*static_cast<EventType*>(event));
            });
        }

        template <class EventType, typename... Args>
        void emit(Args... args) {
            EventType event {std::forward<Args>(args)...};
            emit_impl(event);
        }

        template <class EventType, typename... Args>
        void emit(EventType&& event) {
            emit_impl(event);
        }

        void clear() { _listeners.clear(); }

       private:
        template <class EventType>
        void emit_impl(EventType& event) {
            if (_listeners.contains(details::type_id::get<EventType>())) {
                for (auto it : _listeners[details::type_id::get<EventType>()]) {
                    it(&event);
                }
            }
        }

       private:
        std::unordered_map<details::id_type, std::vector<EventCallback>>
            _listeners;
    };
}  // namespace epp