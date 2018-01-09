// Chain.hpp
#ifndef BLXCPP_CHAIN_HPP
#define BLXCPP_CHAIN_HPP

#include <vector>

#include "Task.hpp"
#include "function_traits.hpp"

#include <map>
#include <unordered_map>
#include <algorithm>

namespace blxcpp {

/*

   @TODO:
   实现

   // 第一批实现
   map √
   sort √
   filter √
   group √
   select √ // 虽然都是 map 但是需要把 map 拉平成 vector

   // 未完成
   reverse

   flod
   count

   concat
   concatMap
   zip

*/

template<typename C>
struct ChainContainer;

template <typename K, typename V>
struct ChainContainer<std::map<K, V>> {

    template <typename T>
    using Remap = std::map<
        typename std::tuple_element<0, T>::type,
        typename std::tuple_element<1, T>::type
    >;

    template<typename T>
    static void insert(Remap<T>& container, const T& item) {
        container.insert(item);
    }

};


template <typename K, typename V>
struct ChainContainer<std::unordered_map<K, V>> {

    template <typename T>
    using Remap = std::unordered_map<
        typename std::tuple_element<0, T>::type,
        typename std::tuple_element<1, T>::type
    >;

    template<typename T>
    static void insert(Remap<T>& container, const T& item) {
        container.insert(item);
    }

};

template <template<typename...> class Container, typename ...Args>
struct ChainContainer<Container<Args...>> {

    template<typename T>
    using Remap = Container<T>;

    template<typename T>
    static void insert(Remap<T>& container, const T& item) {
        container.push_back(item);
    }
};

template<typename From, typename To>
class Chain {
private:

    using FromC = ChainContainer<From>;
    using ToC = ChainContainer<To>;
    using FromI = typename From::value_type;
    using ToI = typename To::value_type;

    const std::function<To(const From&)> m_func;

    template<typename Func>
    using Remap = typename ToC::template Remap<typename function_traits<Func>::return_type>;

public:

    Chain(const std::function<To(const From&)>& func)
        : m_func(func) { }

    template<typename Func>
    Chain<From, Remap<Func>> map(const Func& func) {
        auto last = m_func;

        auto next = [func, last](const From& container){
            auto tmp = last(container);
            Remap<Func> result;
            for (auto& item : tmp) {
                ToC::insert(result, func(item));
            }
            return result;
        };

        return Chain<From, Remap<Func>>(next);
    }

    template<typename Func>
    Chain<From, From> sort(const Func& func){
        auto last = m_func;
        auto next = [func, last](const From& container){
            auto tmp = last(container);
            std::sort(tmp.begin(), tmp.end(), func) ;
            return tmp;
        };
        return Chain<From, From>(next);
    }

    // 无参数版本
    Chain<From, From> sort(){
        auto last = m_func;
        auto next = [last](const From& container){
            auto tmp = last(container);
            std::sort(tmp.begin(), tmp.end()) ;
            return tmp;
        };
        return Chain<From, From>(next);
    }


    template<typename Func>
    Chain<From, From> filter(const Func& func){
        auto last = m_func;
        auto next = [func, last](const From& container){
            auto tmp = last(container);
            From result;
            for (auto& item : tmp) {
                if (func(item)) ChainContainer<From>::insert(result, item);
            }
            return result;
        };
        return Chain<From, From>(next);
    }

    template<typename Func>
    Chain<From, std::map<typename function_traits<Func>::return_type, From>> group(const Func& func){
        using Key = typename function_traits<Func>::return_type;
        using Next = std::map<Key, From>;

        auto last = m_func;
        auto next = [func, last](const From& container){
            auto tmp = last(container);
            Next result;
            for (auto& item : tmp) {
                auto& group = result[func(item)];
                ChainContainer<From>::insert(group, item);
            }
            return result;
        };
        return Chain<From, Next>(next);
    }

    template<typename Func>
    Chain<From, std::vector<typename function_traits<Func>::return_type>> select(const Func& func){
        using Ret = typename function_traits<Func>::return_type;
        using Next = std::vector<Ret>;

        auto last = m_func;
        auto next = [func, last](const From& container){
            auto tmp = last(container);
            Next result;
            for (auto& item : tmp) {
                ChainContainer<Next>::insert(result, func(item));
            }
            return result;
        };
        return Chain<From, Next>(next);
    }

    To value(const From& from) {
        return m_func(from);
    }

    To operator()(const From& from) { return value(from); }

};

template<typename T>
Chain<T, T> chain() {
    return Chain<T, T>([](const T& t){ return t; });
};


//template<typename T>
//struct ChainContainerHelper;

//template<template<typename...> class MatchedContainerT, typename MatchedItem, typename ...MatchedArgs>
//struct ChainContainerHelper<MatchedContainerT<MatchedItem, MatchedArgs...>> {

//    template<typename Item>
//    using ContainerT = MatchedContainerT<Item, MatchedArgs...>;

//    using Container = MatchedContainerT<MatchedItem, MatchedArgs...>;
//    using Item = typename Container::value_type;
//};

//template <typename T>
//struct ChainMapHelper;

//template<template<typename...> class MatchedMapT, typename MatchedKey, typename MatchedValue, typename ...MatchedArgs>
//struct ChainMapHelper<MatchedMapT<MatchedKey, MatchedValue, MatchedArgs...>> {

//    template<typename Key, typename Value>
//    using ContainerT = MatchedMapT<Key, Value, MatchedArgs...>;

//    using Container = MatchedMapT<MatchedKey, MatchedValue, MatchedArgs...>;
//    using Item = typename Container::value_type;
//};




//template<typename From, typename To>
//class Chain {
//private:
//    using FromItem = typename From::value_type;
//    using ToItem = typename To::value_type;

//    const std::function<To(From&)> m_func;




//public:

//    Chain(const std::function<To(From&)>& func)
//        : m_func(func) { }

//    template<typename Func>
//    Chain<To, typename Apply<To, typename function_traits<Func>::return_type>::type> map(const Func& func) {
//        auto last = std::move(m_func);
//        using C = typename Apply<To, typename function_traits<Func>::return_type>::type;
//        return Chain<To, C>([last](To& to){
//            auto tmp = last(to);
//            C result;
//            for (auto item : tmp) {
//            }
//            return result;
//        });
//    }
//};

//};

//template <typename Container, typename Return>
//class Chain {
//private:
//    const std::function<Return(const Container&)> m_func;
//public:
//    using Item = typename Container::value_type;
//    Chain(const std::function<const Return(const Container&)>& func)
//        : m_func(func) { }
//    Chain(const std::function<const Return(const Container&)>&& func)
//        : m_func(std::move(func)) { }
//    template<typename Func>
//    Chain<Container, typename function_traits<Func>::return_type> map(const Func& func) {
//        return Chain<Container, typename function_traits<Func>::return_type>([](){});
//    }
//};

//template <typename Container, typename Func>
//class Chain {
//private:
//    using value_type = typename Container::value_type;
//    Func func;

//public:
//};

}

#endif // BLXCPP_CHAIN_HPP
