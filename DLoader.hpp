// DLoader.hpp
#ifndef BLXCPP_DLOADER_HPP
#define BLXCPP_DLOADER_HPP

#include <string>

#ifdef _WIN32

// windows 下 include Windows.h
#include <Windows.h>

#else

// posix 下 include dlfcn.h
#include <dlfcn.h>

#endif

// @TODO: 这个先跳过吧，毕竟自己还没怎么用过 dll / so
// 不过应该是很好玩的东西 /w\ 以后写代码写成模块能动态热加载的一定就超好玩。

namespace blxcpp {

class DLoader {
private:

public:
    DLoader(const std::string& );
};

}


#endif // BLXCPP_DLOADER_HPP
