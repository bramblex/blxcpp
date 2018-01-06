// Compose.hpp
#ifndef BLXCPP_COMPOSE_HPP
#define BLXCPP_COMPOSE_HPP

#include <functional>
#include <type_traits>

#include "function_traits.hpp"

namespace blxcpp {

template <typename F, typename G>
struct ComposeHelper {
    using FRet = typename function_traits<F>::return_type;
    // 如果需要在 typename 里面使用模板里面的模板， 则需要再加上 template 关键字
    using GArg = typename function_traits<G>::template args<0>::type;
    using Func = std::function<FRet(GArg)>;
};

/*
 * f(t2) -> t3
 * g(t1) -> t2
 * compose(f, g)(x) -> f(g(x))
 *
 * compose 默认是右结合的，所以要做一个左结合版本
 *
 * @TODO:
 * 	还需要一个任意长参数版本的，
 *  任意长参数版本有点难弄就是了
 *
 */

template<typename F, typename G>
auto compose(F const& f, G const& g)
    -> typename ComposeHelper<F, G>::Func {

    using GArg = typename ComposeHelper<F, G>::GArg;
    using FRet = typename ComposeHelper<F, G>::FRet;
    return [&](GArg arg) -> FRet{
        return f(g(std::forward<GArg>(arg)));
    };
}

// 左结合版本 compose Rgith
template<typename G, typename F>
auto composeR(G const& g, F const& f)
    -> typename ComposeHelper<F, G>::Func {

    using GArg = typename ComposeHelper<F, G>::GArg;
    using FRet = typename ComposeHelper<F, G>::FRet;
    return [&](GArg arg) -> FRet{
        return f(g(std::forward<GArg>(arg)));
    };
}

}

#endif // BLXCPP_COMPOSE_HPP
