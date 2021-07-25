#include <functional>
#include <map>

template<typename T, typename... Args>
std::function<T(Args...)> memoize(std::function<T(Args...)> f)
{
    std::map<std::tuple<Args...>, T> memo;
    return [f, memo](Args... args) mutable -> T {
        auto key = std::make_tuple(args...);
        auto memoized = memo.find(key);
        if (memoized != memo.end())
            return memoized->second;
        auto result = f(args...);
        memo[key] = result;
        return result;
    };
}
