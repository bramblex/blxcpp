// Variant.hpp
#ifndef BLXCPP_VARIANT_HPP
#define BLXCPP_VARIANT_HPP

#include <cassert>
#include <type_traits>
#include <tuple>
#include <typeindex>
#include <iostream>
#include "function_traits.hpp"

namespace blxcpp {

template<typename ...Types>
class Variant {
private:
    // 至少要有一个参数
    static_assert(std::tuple_size<std::tuple<Types...>>::value > 0
                  , "Variant should require more then 1 arugment");

    // 关键步骤，获取最大值
    template<size_t first, size_t... other>
    struct MaxSize
        : std::integral_constant<size_t, (
            first > MaxSize<other...>::value
            ? first : MaxSize<other...>::value
          )> { };

    template<size_t first>
    struct MaxSize<first> : std::integral_constant<size_t, first> { };

    // 逻辑或运算，用来比较类型
    template<bool first, bool ...other>
    struct BoolOr {
        const static bool value = first ? first : BoolOr<other...>::value ;
    };

    template<bool first>
    struct BoolOr<first> {
        const static bool value = first ;
    };

    // 关键步骤，计算得出一个对齐的最大缓冲区大小
    using DataBuffer = typename std::aligned_storage<
        MaxSize<sizeof(Types)...>::value,
        MaxSize<alignof(Types)...>::value
    >::type;

    // 强制转换指针用帮助模板
    template<typename Target, typename Origin>
    static Target* pointer_cast(Origin pointer) {
        return (reinterpret_cast<Target*>(reinterpret_cast<size_t>(pointer)));
    }

    template<typename First>
    struct HelperBase {

        // 判断是否为当前类型
        static bool is(const std::type_index& type) {
            return type == std::type_index(typeid(First));
        }

        // 销毁
        static void destroy(const std::type_index&, DataBuffer* const data) {
            pointer_cast<First>(data)->~First();
        }

        static void copy(DataBuffer* const data, const Variant<Types...>& that) {
            new (data) First(*(pointer_cast<First>(&that.m_data)));
        }

        // 右值版本，用 move 减少复制构造
        static void copy(DataBuffer* const data, const Variant<Types...>&& that) {
            // move 减少构造复制
            new (data) First(std::move(*(pointer_cast<First>(&that.m_data))));
        }
    };

    // 辅助类，用于寻找对应的 type
    template <typename First, typename ...Other>
    struct Helper {

        // 用宏来简化函数
#define BLXCPP_VARIANT_MATCH(TYPE, BODY) \
    if (HelperBase<First>::is(TYPE)) { \
    HelperBase<First>::BODY; \
    } else { \
    Helper<Other...>::BODY; \
    }

        // 辅助析构
        static void destroy(const std::type_index& type, DataBuffer* const data) {
            BLXCPP_VARIANT_MATCH(type, destroy(type, data));
        }

        static void copy(DataBuffer* const data, const Variant<Types...>& that) {
            BLXCPP_VARIANT_MATCH(that.m_type, copy(data, that));
        }

        // 右值版本，用 move 减少复制构造
        static void copy(DataBuffer* const data, const Variant<Types...>&& that) {
            BLXCPP_VARIANT_MATCH(that.m_type, copy(data, that));
        }

#undef BLXCPP_VARIANT_MATCH

    };

    template <typename First>
    struct Helper <First> {

        // 同样用宏简化函数
#define BLXCPP_VARIANT_MATCH(TYPE, BODY, ASSERT) \
    if (HelperBase<First>::is(TYPE))  HelperBase<First>::BODY; else  assert(false && ASSERT)

        static void destroy(const std::type_index& type, DataBuffer* const data) {
            BLXCPP_VARIANT_MATCH(
                        type,
                        destroy(type, data),
                        "Type does not matched when destroyed.");

        }

        static void copy(DataBuffer* const data, const Variant<Types...>& that) {
            BLXCPP_VARIANT_MATCH(
                        that.m_type,
                        copy(data, that),
                        "Type does not matched when copy."
                        );
        }

        // 右值版本，用 move 减少复制构造
        static void copy(DataBuffer* const data, const Variant<Types...>&& that) {
            BLXCPP_VARIANT_MATCH(
                        that.m_type,
                        copy(data, that),
                        "Type does not matched when copy."
                        );
        }

#undef BLXCPP_VARIANT_MATCH
    };

    void destroy() { if (null()) Helper<Types...>::destroy(m_type, &m_data); }

    bool m_has_inited = false;
    std::type_index m_type = std::type_index(typeid(void)); // 记录类型信息
    DataBuffer m_data; // 老样子，申请一片 buffer

public:

    // 测试是否存在某个类型
    // 重点：一定不能忘记 conditional 后面的 ::type !!!
    // 被这个 ::type 坑了好多次了
    template<typename T>
    struct Contains
        : std::conditional<
            BoolOr<std::is_same<T, Types>::value...>::value,
            std::true_type,
            std::false_type
        >::type { };

    // 构造
    Variant() { }


    Variant(const Variant& that)
        :  m_has_inited(true)
        ,  m_type(that.m_type) {
        Helper<Types...>::copy(&m_data, that);
    }

    Variant(const Variant&& that)
        :  m_has_inited(true)
        ,  m_type(that.m_type) {
        Helper<Types...>::copy(&m_data, that);
    }

    template<typename T>
    Variant(const T& data)
        :  m_has_inited(true)
        ,  m_type(std::type_index(typeid(T))) {
        new (&m_data) T(data);
    }

    template<typename T>
    Variant(const T&& data)
        :  m_has_inited(true)
        ,  m_type(std::type_index(typeid(T))) {
        new (&m_data) T(std::move(data));
    }

    ~Variant() {
        destroy();
    }

    // 判断是否为 null
    bool null() const { return !m_has_inited; }

    // 判断是否为当前类型
    template<typename T>
    bool is() const { return !null() && m_type == std::type_index(typeid(T)); }

    // 转换
    template<typename T>
    T& cast() const {
        using U = typename std::decay<T>::type;
        if (!is<U>()) {
            std::cout << "Can not cast "
                      << m_type.name()
                      << " to "
                      << typeid(U).name()
                      << "." << std::endl;
            throw std::bad_cast();
        }
        return *(pointer_cast<U>(&m_data));
    }

    using This = Variant<Types...>;

    // 赋值，同时重写类型
    This& operator=(const This& that) {
        destroy();
        m_type = that.m_type;
        Helper<Types...>::copy(&m_data, that);
        return *this;
    }

    // match
    template<typename Func>
    const This& match(const Func& func) const {

        static_assert(function_traits<Func>::size == 1, "Function should has only one argument.");

        using ArgType = typename std::decay<
            typename function_traits<Func>::template args<0>::type
        >::type;
        static_assert(Contains<ArgType>::value, "Can not match value type");

        if (is<ArgType>()) func(cast<ArgType>());
        return *this;
    }

};

}

#endif // BLXCPP_VARIANT_HPP
