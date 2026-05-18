#include "vin/vin.hpp"

#include <cstddef>

int main(const int argc, const char* argv[])
{
  const auto vin{ vin::Vin::create() };
  return vin->run({ argv, static_cast<std::size_t>(argc) });
}
