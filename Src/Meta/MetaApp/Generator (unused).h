#pragma once

/*
template <typename range_ty>
struct generator {
    using value_type = range_ty;

    struct promise_type {
       value_type current_value;

       std::suspend_always yield_value(value_type value) {
          current_value = value;
          return {};
          }

       std::suspend_never initial_suspend() { return {}; }
       void return_void() {}
       void unhandled_exception() { std::terminate();  }

       generator get_return_object() {
          return generator { std::coroutine_handle<promise_type>::from_promise(*this) };
          }
       };

    using handle_type = std::coroutine_handle<promise_type>;

    handle_type coro;

    generator(handle_type h) : coro(h) { }
    ~generator() { if (coro) coro.destroy();  }

    generator(generator const&) = delete;
    generator(generator&& other) noexcept : coro(other.coro) { other.coro = nullptr;  }

    bool next() {
       if (!coro || coro.done()) return false;
       else {
          coro.resume();
          return !coro.done();
          }
       }

    value_type value() const {
       return coro.promise().current_value;
        }

    struct iterator {
       generator* gen;

       iterator(generator* g) : gen(g) {
          if (gen) gen->next();
          }

       iterator& operator ++ () {
          if (gen) gen->next();
          return *this;
          }

       value_type operator *() const {
          /// \todo check for existence and generate a exception
          return gen->value();
          }

       bool operator == (std::default_sentinel_t) const {
          /// \todo check for existence and generate a exception or return true
          return gen->coro.done();
          }

       };

    iterator begin() {
       return iterator{ this };
       }

    std::default_sentinel_t end() {
       return { };
       }

    };
*/
