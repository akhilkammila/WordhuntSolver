// These includes are alerady in main file - used for debugging
// #include <iostream>
// #include <vector>
// #include <map>

using namespace std;


// Letter
struct letter {
    char l;
    int row;
    int col;
    bool isDiag;
    int repeat;
    int repeatNum;
};
map<string,string> colors = {
    {"default", "\033[1;30m"}, //black
    {"black", "\033[1;30m"},
    {"purple", "\033[1;35m"},
    {"yellow", "\033[1;33m"},
    {"blue","\033[1;34m" },
    {"green","\033[1;32m"},
    {"red","\033[1;31m"}
};
ostream& operator<<(ostream &os, const letter&l) {
    string color = "default";
    if (l.repeat) {
        color = (l.repeatNum == 0) ? "blue" : "red";
    }
    else if (l.isDiag) color = "purple";
    return os << colors[color] << l.l << colors["default"];
}
bool operator==(const letter& la, const letter& lb)
{
    return (la.row == lb.row && la.col == lb.col);
}

// Word
struct word {
    vector<letter> path;
    int complexity;
    int complexityUpdate;
    bool chosen;
};
ostream& operator<<(ostream &os, const word& w) {
    for(letter l : w.path) {
        os << l;
    }
    return os;
}
bool operator==(word &a, word &b) {
    return a.path == b.path;
}
bool operator<(word &a, word &b) { // bs ordering
    return a.path.size() < b.path.size();
}

// TrieNode
struct TrieNode {
    map<char, TrieNode*> children;
    bool isWord;
};