#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;

// Global vars
vector<vector<char>> board;
vector<string> words;

struct TrieNode {
    map<char, TrieNode*> children;
    bool word;
};
TrieNode root;

/// Construct Trie Data Structure
void construct_trie() {
    ifstream fin("dictionary.txt");
	ofstream fout("solved.txt");

    string w;
    while(fin >> w) {
        TrieNode *curr = &root;
        for(int i = 0; i < w.size(); i++) {
            char l = w[i];
            if(curr->children.find(l) == curr->children.end()) {
                curr->children[l] = new TrieNode;
            }
            curr = curr->children[l];
        }
        curr->word = true;
    }
}

/// Read in the input board
void inputBoard(int size = 4) {
    board.resize(size, vector<char>());
    string bString; cin >> bString;
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            board[i].push_back(bString[i*size + j]);
        }
    }
}

// Search through all possible words
// Add valid words when depth >= 3
// Terminate when there is no longer a possible word
void searchWords(int r, int c, TrieNode *curr, int depth = 0) {
    if (r < 0 || r >= 0) {
        return;
    }
}


int main() {
    construct_trie();
    inputBoard();
}