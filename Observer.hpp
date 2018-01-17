// Observer.hpp
#ifndef BLXCPP_OBSERVER_HPP
#define BLXCPP_OBSERVER_HPP

#include <functional>
#include <set>
#include <vector>
#include "Lazy.hpp"
#include "function_traits.hpp"
#include <cassert>

namespace blxcpp {

class ObserverBase {
protected:
    mutable std::set<ObserverBase*> m_refs; // 所有引用
    const std::vector<ObserverBase*> m_deps; // 所有依赖
    mutable bool m_is_activated = true;

    explicit ObserverBase(const std::vector<ObserverBase*>& deps);

    explicit ObserverBase(const std::vector<ObserverBase*>&& deps);

public:
    bool isActivated() const;
    virtual void activate() = 0;
    virtual ~ObserverBase();
};

template<typename T>
class ObserverValue: public ObserverBase {
private:
    T m_data;

public:

    using Type = T;

    ObserverValue(const T& data)
        : ObserverBase({})
        , m_data(data) {  }

    ObserverValue(const T&& data)
        : ObserverBase({})
        , m_data(std::move(data)) {  }

    const T& operator()() const { return m_data; }

    const T& operator=(const T& data) { m_data = data; activate(); }
    const T& operator=(const T&& data) { m_data = std::move(data); activate(); }

    void activate() override {
        for (ObserverBase* ref : m_refs)
            if (!ref->isActivated()) ref->activate();
    }
};

template <typename T>
class ObserverFunc : public Lazy<T>, public ObserverBase {
private:

    template <typename OB>
    using GetType = typename OB::Type;

public:

    using Type = T;

    template<typename ...Args>
    ObserverFunc(const std::function<T(GetType<Args>...)>& func, Args&... args)
        : Lazy<T>([&func, args...]{ return func(*args()...); })
        , ObserverBase({args...}) { }

    void activate() override {
        this->reset();
        for (ObserverBase* ref : m_refs)
            if (!ref->isActivated()) ref->activate();
    }
};

template<typename T>
auto obval(const T&& t) -> ObserverValue<T> {
    return ObserverValue<T>(std::forward<T>(t));
}

template<typename Func, typename ...Args>
auto obfunc(const Func&& func, const Args&& ...args) -> ObserverFunc<typename function_traits<Func>::return_type> {
    using T = ObserverFunc<typename function_traits<Func>::return_type>;
    return ObserverFunc<T>(std::forward<Func>(func), std::forward<Args>(args)...);
}

}

#endif // BLXCPP_OBSERVER_HPP
