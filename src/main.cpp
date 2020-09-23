
#include "interface/EngineIO.hpp"
#include <iostream>

using namespace siegbert;

int main(int argc, char **argv) {
  EngineIO engineIO;
  engineIO.run(std::cin, std::cout);
  return EXIT_SUCCESS;
}