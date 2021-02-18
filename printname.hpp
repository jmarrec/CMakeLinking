#ifndef EMPTY_HPP
#define EMPTY_HPP

#include <string>

namespace Test {
  class Person;
}

  // A free-standing function taking an object as argument
std::string personName(const Test::Person& person);

#endif // EMPTY_HPP
