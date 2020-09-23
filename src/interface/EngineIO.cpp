#include "interface/EngineIO.hpp"
#include "interface/UciInterface.hpp"
#include "interface/XBoardInterface.hpp"

namespace siegbert {

EngineIO::EngineIO() { interface = new UciInterface(this); }

EngineIO::~EngineIO() { delete interface; }

void EngineIO::run(std::istream &in, std::ostream &out) {
  out_ = &out;
  for (std::string line; std::getline(in, line);) {

    if (line.compare("uci") == 0) {
      delete interface;
      interface = new UciInterface(this);
    }

    else if (line.compare("xboard") == 0) {
      delete interface;
      interface = new XBoardInterface(this);
    }

    interface->receive(line);

    if (interface->is_exit_required()) {
      break;
    }
  }
}

void EngineIO::send(const std::string &line) {
  if (out_) {
    (*out_) << line << std::endl;
  }
}
} // namespace siegbert