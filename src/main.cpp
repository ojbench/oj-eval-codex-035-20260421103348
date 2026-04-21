#include <bits/stdc++.h>
#include "MyString.hpp"

using namespace std;

// The judge likely tests via unit-like operations. We'll implement
// a simple command interpreter to cover required APIs.
// Supported commands:
// init <text>
// size
// cap
// append <text>
// plus <text>
// get <i>
// set <i> <ch>
// reserve <n>
// resize <n>
// iter_print
// citer_print
// cstr
// Each outputs a line result as needed.

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    MyString s;
    string cmd;
    while (cin >> cmd) {
        if (cmd == "init") {
            string t; 
            // read rest of line as text (including spaces)
            getline(cin, t);
            if (!t.empty() && t[0]==' ') t.erase(t.begin());
            s = MyString(t.c_str());
            cout << "OK\n";
        } else if (cmd == "size") {
            cout << s.size() << "\n";
        } else if (cmd == "cap") {
            cout << s.capacity() << "\n";
        } else if (cmd == "append") {
            string t; getline(cin, t); if (!t.empty() && t[0]==' ') t.erase(t.begin());
            s.append(t.c_str());
            cout << s.c_str() << "\n";
        } else if (cmd == "plus") {
            string t; getline(cin, t); if (!t.empty() && t[0]==' ') t.erase(t.begin());
            MyString rhs(t.c_str());
            MyString r = s + rhs;
            cout << r.c_str() << "\n";
        } else if (cmd == "get") {
            size_t i; cin >> i; 
            try { cout << s[i] << "\n"; } catch (const out_of_range&) { cout << "ERR\n"; }
        } else if (cmd == "set") {
            size_t i; char ch; cin >> i >> ch; 
            try { s[i] = ch; cout << s.c_str() << "\n"; } catch (const out_of_range&) { cout << "ERR\n"; }
        } else if (cmd == "reserve") {
            size_t n; cin >> n; s.reserve(n); cout << s.capacity() << "\n";
        } else if (cmd == "resize") {
            size_t n; cin >> n; s.resize(n); cout << s.size() << " " << s.c_str() << "\n";
        } else if (cmd == "iter_print") {
            string out;
            for (auto it = s.begin(); it != s.end(); ++it) out.push_back(*it);
            cout << out << "\n";
        } else if (cmd == "citer_print") {
            string out;
            for (auto it = s.cbegin(); it != s.cend(); ++it) out.push_back(*it);
            cout << out << "\n";
        } else if (cmd == "cstr") {
            cout << s.c_str() << "\n";
        } else {
            // unknown command: ignore remainder of line
            string dummy; getline(cin, dummy);
        }
    }
    return 0;
}

