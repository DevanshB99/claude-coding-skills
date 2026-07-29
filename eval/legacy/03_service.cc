#include <fstream>
#include <string>

class Service {
 public:
  bool Process() {
    std::ifstream in("/etc/service/input.txt");
    if (!in) { throw std::runtime_error("no input"); }
    std::string line;
    int total = 0;
    while (std::getline(in, line)) {
      total += atoi(line.c_str());
    }
    std::ofstream out("/var/log/service.log");
    out << "total=" << total << "\n";
    auto report = [&] { out << "done " << total << "\n"; };
    report();
    return true;
  }
};
