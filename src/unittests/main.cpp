#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include "logging/Logging.hpp"

int main(int argc, char *argv[], char **env) {
  logging::basicConfig(argc, argv, logging::LogLevel::Debug).withUdp();
  int result = Catch::Session().run(argc, argv);
  return result;
}