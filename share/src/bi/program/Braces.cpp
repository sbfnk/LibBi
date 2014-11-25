/**
 * @file
 *
 * @author Lawrence Murray <lawrence.murray@csirgroup.au>
 * $Rev$
 * $Date$
 */
#include "Braces.hpp"

#include "Reference.hpp"
#include "../visitor/Visitor.hpp"

#include <typeinfo>

biprog::Braces* biprog::Braces::clone() {
  return new Braces(stmt->clone());
}

biprog::Braces* biprog::Braces::accept(Visitor& v) {
  type = type->accept(v);
  stmt = stmt->accept(v);
  return v.visit(this);
}

bool biprog::Braces::operator<=(const Expression& o) const {
  try {
    const Braces& o1 = dynamic_cast<const Braces&>(o);
    return *stmt <= *o1.stmt && *type <= *o1.type;
  } catch (std::bad_cast e) {
    //
  }
  try {
    const Reference& o1 = dynamic_cast<const Reference&>(o);
    return !*o1.brackets && !*o1.parens && !*o1.braces && *type <= *o1.type;
  } catch (std::bad_cast e) {
    //
  }
  return false;
}

bool biprog::Braces::operator==(const Expression& o) const {
  try {
    const Braces& o1 = dynamic_cast<const Braces&>(o);
    return *stmt == *o1.stmt && *type == *o1.type;
  } catch (std::bad_cast e) {
    //
  }
  return false;
}

bool biprog::Braces::operator<=(const Statement& o) const {
  try {
    const Braces& o1 = dynamic_cast<const Braces&>(o);
    return *stmt <= *o1.stmt && *type <= *o1.type;
  } catch (std::bad_cast e) {
    //
  }
  try {
    const Reference& o1 = dynamic_cast<const Reference&>(o);
    return !*o1.brackets && !*o1.parens && !*o1.braces && *type <= *o1.type;
  } catch (std::bad_cast e) {
    //
  }
  return false;
}

bool biprog::Braces::operator==(const Statement& o) const {
  try {
    const Braces& o1 = dynamic_cast<const Braces&>(o);
    return *stmt == *o1.stmt && *type == *o1.type;
  } catch (std::bad_cast e) {
    //
  }
  return false;
}

void biprog::Braces::output(std::ostream& out) const {
  out << '{' << *stmt << '}';
}
