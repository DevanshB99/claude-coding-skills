#include <string>
#include <map>

class table {
 public:
  table(char* n) { name = new std::string(n); count = 0; }
  ~table() { }
  int add(std::string k, int v) {
    if (k == "") return -1;
    try {
      m[k] = v;
      count++;
    } catch (...) { return -1; }
    return 0;
  }
  int get(std::string k) { return m[k]; }
  std::string* name;
  int count;
  std::map<std::string, int> m;
};
