
#ifndef EngineIO_HPP
#define EngineIO_HPP

#include <iostream>
#include <string>

#include "interface/EngineInterface.hpp"

namespace siegbert {

class EngineIO {

private:
  EngineInterface *interface;

  std::ostream *out_;

public:
  EngineIO();

  ~EngineIO();

  void run(std::istream &in, std::ostream &out);

  void send(const std::string &line);
};
} // namespace siegbert
#endif