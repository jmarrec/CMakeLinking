#ifndef EXAMPLE_HPP
#define EXAMPLE_HPP

#include <string>

namespace Test {

class Person
{
 public:
  Person(std::string name) noexcept;

  std::string getName() const;
  bool setName(const std::string& t_newName);

 private:
  std::string m_name;
};

std::ostream& operator<<(std::ostream&, const Test::Person&);

}  // Namespace Test

#endif  // ifndef PERSON_HPP
