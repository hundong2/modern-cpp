/**
 * @file coroutine3.cpp
 * @author hundong2
 * @brief 
 * @version 0.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @brief Promise 규칙 
 * 1. get_return_object()
 * 2. initial_suspend()
 * 3. unhandled_exception()
 * 4. return_voide()
 * 5. final_suspend()
 */
#include <iostream>
#include <coroutine>

class Generator
{
    public:
        struct Promise
        {
            Generator get_return_object()
            {
                return Generator{ std::coroutine_handle<Promise>::from_promise(*this)};
            }
            auto initial_suspend() { return std::suspend_always{}; }
            auto return_void() { return std::suspend_never{}; }
            auto final_suspend() noexcept { return std::suspend_always{}; }
            auto unhandled_exception() { std::exit(1); }
        };
        using promise_type = Promise;

        std::coroutine_handle<promise_type> coro;

        Generator( std::coroutine_handle<promise_type> c) : coro(c) {}
        ~Generator() { if(coro) coro.destroy(); }
};

Generator foo()
{
    std::cout << "Run1" << std::endl;
    co_await std::suspend_always {};
    std::cout << "Run2" << std::endl;
}