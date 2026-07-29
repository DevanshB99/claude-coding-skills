#include <vector>

struct Node { int id; Node* next; };

class Graph {
 public:
  Graph() { root = NULL; }
  void Add(int id) {
    Node* n = new Node();
    n->id = id;
    n->next = root;
    root = n;
    all.push_back(n);
  }
  Node* Find(int id) {
    for (int i = 0; i < all.size(); i++)
      if (all[i]->id == id) return all[i];
    return NULL;
  }
  Node* root;
  std::vector<Node*> all;
};
