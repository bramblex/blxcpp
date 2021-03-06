// Any.hpp
#ifndef BLXCPP_ANY_HPP
#define BLXCPP_ANY_HPP

#include <memory>
#include <typeindex>
#include <iostream>

namespace blxcpp {

class Any {
private:

    struct Base;
    typedef std::unique_ptr<Base> BasePtr;

    BasePtr m_ptr;
    std::type_index m_type;

    struct Base {
        virtual ~Base();
        virtual BasePtr clone() const = 0;
    };

    template<typename T>
    struct Derived : Base {

        T m_value;

        template<typename U>
        Derived(U&& value)
            : m_value(std::forward<U>(value)) { }

        BasePtr clone() const {
            return BasePtr(new Derived<T>(m_value));
        }
    };

    BasePtr clone() const;

public:

    Any(void);

    Any(const Any& that);

    Any(Any&& that);

    // 用来擦除类型，然后储存基类 Base 指针
    // 通过 std::decay 来一出引用和cv用于获取原始类型
    template<typename U, class = typename std::enable_if<!std::is_same<typename  std::decay<U>::type, Any>::value, U>::type>
    Any(U && value)
        : m_ptr(new  Derived <typename std::decay<U>::type>(std::forward<U>(value)))
        , m_type(std::type_index(typeid(typename std::decay<U>::type))) { }

    bool null() const;

    template<class U>
    inline bool is() const { return m_type == std::type_index(typeid (U)); }

    // 转换为实际类型
    template<class U>
    U& cast() {
        if (!is<U>()) {
            std::cout << "Can not cast "
                      << typeid(U).name()
                      << " to "
                      << m_type.name()
                      << "." << std::endl;
            throw std::bad_cast();
        }
        auto derived = dynamic_cast<Derived<U>*>(m_ptr.get());
        return derived->m_value;
    }

    // 赋值，同时擦除类型
    Any& operator=(const Any& other);

};

}

#endif // BLXCPP_ANY_HPP
