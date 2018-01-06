// Optional.hpp
#ifndef BLXCPP_OPTIONAL_HPP
#define BLXCPP_OPTIONAL_HPP

#include <type_traits>
#include <utility>
#include <cstdlib>
#include <stdexcept>

namespace blxcpp {

template<typename T>
class Optional {

public:
    // std::alignement_of<T>::value 和 alignof 等同
    // 用来创建一个字节对齐的内存, 用来存储 T 对象
    // 千万要看好了后面那个 ::type , 很容易就忘掉的
    using DataT = typename std::aligned_storage<sizeof (T), alignof (T)>::type;

    // 强制转换指针用帮助模板
    template<typename Target, typename Origin>
    static Target* pointer_cast(Origin pointer) {
        return (reinterpret_cast<Target*>(reinterpret_cast<size_t>(pointer)));
    }

private:
    bool m_has_init = false;
    DataT m_data;

    template<class... Args>
    void create(Args&&... args) {
        // 在 m_data 这片内存上初始化一个 T 对象
        new (&m_data) T(std::forward<Args>(args)...);
        m_has_init = true;
    }

    void destroy() {
        if (m_has_init) {
            m_has_init = false;
            // 将指针强制转换成 T 指针并用 T 的析构函数析构
            (pointer_cast<T>(&m_data))->~T();
        }
    }

    // 用于左值构造函数, 高效创建对象
    void assign(const Optional& other) {
        if (other.isInit()) {
            // 复制对象
            copy(other.m_data);
            m_has_init = true;
        } else { destroy(); }
    }

    void copy(const DataT& data) {
        destroy();
        // 这里依旧是利用 T 的左值构造, 在 m_data 这片内存上构造
        new (&m_data) T(*pointer_cast<T>(&data));
    }

public:
    Optional() { }
    Optional(const T& v)  { create(v); }
    Optional(const Optional& other) { if (other.isInit()) assign(other); }
    ~Optional() { destroy(); }

    template<class... Args>
    void emplace(Args&&... args) {
        destroy();
        create(std::forward<Args>(args)...);
    }

    bool isInit() const { return m_has_init; }

    explicit operator bool() const { return isInit(); }
    T const& operator *() const {
        if (isInit()) return *(pointer_cast<T>(&m_data));
        else throw std::logic_error("Optional object does not init.");
    }
};

template <typename T>
Optional<T> optional() {
    return Optional<T>();
};

template <typename T>
Optional<T> optional(const T& value) {
    return Optional<T>(value);
}

};

#endif // BLXCPP_OPTIONAL_HPP
