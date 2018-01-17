// Observer.cpp
#include "Observer.hpp"

namespace blxcpp {

ObserverBase::ObserverBase(const std::vector<ObserverBase *>& deps)
    : m_deps(deps) { }

ObserverBase::ObserverBase(const std::vector<ObserverBase *>&& deps)
    : m_deps(std::move(deps)) { }

bool ObserverBase::isActivated() const { return m_is_activated; }

ObserverBase::~ObserverBase() {
    assert(m_refs.size() == 0
           && "Can not destroy an observer object which is dependented on.");
    for (auto& dep : m_deps) dep->m_refs.erase(this);
}

}
