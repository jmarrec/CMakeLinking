#include "Person.hpp"
#include "printname.hpp"
#include <iostream>
#include <utility>

namespace Test {

Person::Person(std::string t_name) noexcept : m_name(std::move(t_name)) {}

std::string Person::getName() const {
  return m_name;
}

bool Person::setName(const std::string& t_newName) {
  m_name = t_newName;
  return true;
}

std::ostream& operator<<(std::ostream& os, const Test::Person& p) {
  os << "Person named '" << personName(p) << "'";
  return os;
}

}  // namespace Test
