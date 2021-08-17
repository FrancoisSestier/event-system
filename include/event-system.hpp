#pragma once

#include <functional>
#include <queue>
#include <unordered_map>
#include <utility>
#include <memory>
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

        template <typename T, typename... Ts>
        struct are_distinct
            : std::conjunction<std::negation<std::is_same<T, Ts>>...,
                               are_distinct<Ts...>> {};

        template <typename T>
        struct are_distinct<T> : std::true_type {};

        template <typename... Ts>
        inline static constexpr bool are_distinct_v
            = are_distinct<Ts...>::value;

        template <typename T, typename... Ts>
        struct contains : std::disjunction<std::is_same<T, Ts>...> {};

        template <typename T, typename... Ts>
        inline static constexpr bool contains_v = contains<T, Ts...>::value;

        template <typename... Ts>
        concept distinct = are_distinct_v<Ts...>;

        template <class T, template <class...> class Template>
        struct is_specialization : std::false_type {};

        template <template <class...> class Template, class... Args>
        struct is_specialization<Template<Args...>, Template> : std::true_type {
        };

        template <class T, template <class...> class Template>
        inline static constexpr bool is_specialization_v
            = is_specialization<T, Template>::value;

    }  // namespace details


    template <typename event_type>
    class event {
       public:
        using undelying_type = event_type;

        event(event_type&& event) : event_(std::move(event)) {}

        template<typename ...Args>
        event(Args... args) : event_(event_type(std::forward<Args>(args)...)) {}


        void consume() { consumed_ = true; }

        event_type* operator->() { return &event_; }

        event_type& operator*() { return event_; }

        bool alive() const { return consumed_; }

       private:
        bool consumed_ = false;
        event_type event_;
    };

    template <typename T>
    concept dispatchable = details::is_specialization_v<T, event>;

    template <dispatchable... event_types>
    requires(details::are_distinct_v<event_types...>) class event_queue {
       public:
        using underlying_event_types = std::tuple<event_types...>;
        using internal_event_queues = std::tuple<std::vector<event_types>...>;

       public:
        event_queue() : internal_queue_(std::make_unique<internal_event_queues>()) {}
        ~event_queue() = default;
        event_queue(event_queue&& other) : internal_queue_(std::move(other.internal_queue_)) {}
        event_queue& operator=(event_queue&& other) {
            if(this != &other) {
                internal_queue_ = std::move(other.internal_queue_);
                other.internal_queue_.reset();
            }
        }

        event_queue(const event_queue& other) = delete;
        event_queue& operator=(const event_queue& other) = delete;

        template <dispatchable T, typename... Args>
        requires(details::contains_v<T, event_types...>) void push_back(
            Args... args) {
            std::get<std::vector<T>>(*internal_queue_)
                .emplace_back(T(std::forward<Args>(args)...));
        }

        template <dispatchable T>
        requires(details::contains_v<T, event_types...>) auto begin() {
            return std::get<std::vector<T>>(*internal_queue_).begin();
        }

        template <dispatchable T>
        requires(details::contains_v<T, event_types...>) auto end() {
            return std::get<std::vector<T>>(*internal_queue_).end();
        }

        bool empty() {
            bool empty = true;
            ((empty
              &= std::get<std::vector<event_types>>(*internal_queue_).empty()),
             ...);
            return empty;
        }

        void clear() {
            ((std::get<std::vector<event_types>>(*internal_queue_).clear()), ...);
        }

        size_t size() {
            size_t size(0);
            ((size += std::get<std::vector<event_types>>(*internal_queue_).size()),
             ...);
            return size;
        }

       private:
        std::unique_ptr<internal_event_queues> internal_queue_;
    };

    template <dispatchable... event_types>
    requires(details::are_distinct_v<event_types...>) class bus {
       public:
        template <dispatchable T>
        using event_callback = std::function<void(T)>;

        template <dispatchable T>
        using event_callback_list = std::vector<event_callback<T>>;

        using storage = std::tuple<event_callback_list<event_types>...>;

        template <dispatchable T>
        requires(details::contains_v<T, event_types...>) void attach_back(
            event_callback<T> callback) {
            std::get<event_callback_list<T>>(event_callback_list_)
                .push_back(callback);
        }

        template <dispatchable T>
        requires(details::contains_v<T, event_types...>) void attach_front(
            event_callback<T> callback) {
            auto& callbacks
                = std::get<event_callback_list<T>>(event_callback_list_);
            callbacks.insert(callbacks.begin(), 0);
        }

        template <dispatchable T>
        requires(details::contains_v<T, event_types...>) void detach(
            event_callback<T> callback) {
            auto& callbacks
                = std::get<event_callback_list<T>>(event_callback_list_);
            auto it = std::find(callbacks.begin(), callbacks.end(), callback);
            if (it != callbacks.end()) {
                callbacks.erase(it);
            }
        }

        template <dispatchable T, typename... Args>
        requires(details::contains_v<T, event_types...>) void dipatch(
            Args... args) {
            T e(T::undelying_type(std::forward<Args>(args)...));
            for (auto& callback : std::get<event_callback_list<T>>(event_callback_list_)) {
                if (e.alive()) {
                    callback(e);
                }
            }
        }

        template <dispatchable T>
        void dipatch(T& e) {
            if constexpr (details::contains_v<T, event_types...>) {
                for (auto& callback : std::get<event_callback_list<T>>(event_callback_list_)) {
                    if (e.alive()) {
                        callback(e);
                    }
                }
            }
        }

        template <dispatchable... Ts>
        void dipatch(event_queue<Ts...>& e_queue) {
            ((std::for_each(e_queue.begin<Ts>(), e_queue.end<Ts>(),
                            [=](Ts& e) { dipatch(e); })),
             ...);
        }

       private:
        storage event_callback_list_;
    };

    template<dispatchable ...Ts>
    struct event_type_list{
        using bus_type = bus<Ts...>;
        using queue_type = event_queue<Ts...>;
    };

    namespace details {

        template<dispatchable ...Ts>
        bus<Ts...> to_bus(event_type_list<Ts...>);

        template<dispatchable ...Ts>
        event_queue<Ts...> to_queue(event_type_list<Ts...>);

    }

    template<typename T> requires(details::is_specialization_v<T, event_type_list>)
    auto make_bus() {return decltype(details::to_bus(std::declval<T>()))();}

    template<typename T> requires(details::is_specialization_v<T, event_type_list>)
    auto make_event_queue() {return decltype(details::to_queue(std::declval<T>()))();}

    /*
        class dispatcher {
           public:
            using event_callback = std::function<void(void*)>;
            using listners
                = std::unordered_map<details::id_type,
       std::vector<event_callback>>;

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
                emit_impl(details::type_id<EventType>(), &event);
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
        */
}  // namespace epp
