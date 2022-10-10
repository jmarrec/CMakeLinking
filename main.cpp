#include "Person.hpp"
#include "api.hpp"
#include <iostream>

int main() {
  Test::Person p("John Doe");
  std::cout << p << '\n';

  api::pythonTime();
}
