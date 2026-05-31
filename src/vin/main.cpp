#include "vin/application.hpp"

int main(const int argc, char* argv[])
{
  const auto app{ vin::Application::create() };
  return app->run(argc, argv);
}
