#include "arguments.h"
#include <iostream>

using namespace args;

int main(int argc, char **argv) {
  Parser parser{"exmaple", "1.0.0", "example arugment parsing program"};
  int cout = 0;
  Argument arg1{"cout", 'c', "count prvided parameters",
    [&cout](span<const string> elements) -> bool {
      cout += elements.size();
      return true;
  }};

  parser += &arg1;

  if (parser.parse(argc, argv) != ParserError::OK) {
    return 1;
  }
  std::cout << cout << std::endl;
  return 0;
}
