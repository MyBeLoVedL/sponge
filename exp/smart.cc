#include "common.hh"

#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

void alloc_name() {
    string name("Alle");
    auto buf = std::vector<int>(100);
    // auto bufp = new vector<int>(200);
    auto sp = make_unique<int[]>(100000);
    sp[2] = 23;
    // *sp = 23;
    // auto s = new int(10);
    // *s = 10;
    // *sp = 12;
    // bufp->push_back(20);
    cout << name << "\n";
}

int main() {
    std::deque<int> Q;
    Q.push_back(1);
    Q.push_back(2);
    Q.push_back(3);
    for (auto i : Q) {
        cout << i << "\n";
        Q.pop_front();
    }
    cout << Q.size() << "\n";

    return 0;
}