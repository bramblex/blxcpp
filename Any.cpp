// Any.cpp
#include "Any.hpp"

namespace blxcpp {

Any::Base::~Base() { }

Any::BasePtr Any::clone() const {
    if (m_ptr != nullptr)
        return m_ptr->clone();
    return nullptr;
}

Any::Any()
    : m_type(std::type_index(typeid(void))) { }

Any::Any(const Any &that)
    : m_ptr(that.clone())
    , m_type(that.m_type) { }

Any::Any(Any &&that)
    : m_ptr(std::move(that.m_ptr))
    , m_type(that.m_type) {
    that.m_ptr = nullptr;
}

bool Any::null() const { return m_ptr == nullptr; }

Any& Any::operator=(const Any &other) {
    if (m_ptr == other.m_ptr) return *this;
    m_ptr = other.clone();
    m_type = other.m_type;
    return *this;
}

}
