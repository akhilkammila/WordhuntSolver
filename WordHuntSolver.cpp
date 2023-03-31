#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

// Debug
// #define DEBUG(...) debug(#__VA_ARGS__, __VA_ARGS__)
// #include </Users/akhilkammila/Competitive Programming/debug.h>

/*
Global variables
*/
const int N = 4;
vector<vector<char>> board;
vector<string> words;

struct TrieNode {
    map<char, TrieNode*> children;
    bool word;
};
TrieNode root;

ifstream fin("dictionary.txt");
ofstream fout("solved.txt");

/*
Step 1:
Parses words from dictionary.txt
and creates a trie to store all the words
*/
void construct_trie() {
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

/*
Step 2:
Reads in the user's board in the form of a string
Ensures the string is 16 letters long and formats to uppercase
*/
void inputBoard() {
    // Reads in 16 letter long string
    board.resize(N, vector<char>());
    string bString;
    while(bString.size() != N*N) {
        cout << "Input Board:" << endl;
        cin >> bString;
    }
    
    // Converts to upper case
    for(int i = 0; i < bString.size(); i++) {
        bString[i] = toupper(bString[i]);
    }

    // Creates a 4x4 board
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            board[i].push_back(bString[i*N + j]);
        }
    }
}

/*
Step 3:
Search through all possible words
*/

/*
Helper functions to
1) ensure that the cell we are attempting to visit is within the 4x4 grid
2) ensure that the cell is not already on the current path
3) ensure that if we visit the cell, there is a word that exists along that path
4) adds the word if it is valid (if it is marked in the trie as a word)
*/
bool inBounds(pair<int,int> cell) {
    return 0 <= cell.first && cell.first < N && 0 <= cell.second && cell.second < N;
}

bool notRepeated(pair<int,int> cell, vector<vector<bool>> &visited) {
    return !visited[cell.first][cell.second];
}

bool existsWord(pair<int,int> cell, TrieNode* curr) {
    char l = board[cell.first][cell.second];
    return curr->children.find(l) != curr->children.end();
}

void addWord(TrieNode* curr, string word) {
    if (word.size() >= 3 && curr->word) {
        words.push_back(word);
        curr->word = false; //prevents repeats
    }
}

/*
Recurses through every possible word
*/
vector<pair<int,int>> offset = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1}, {-1,0},{-1,-1}};
void dfs(pair<int,int> cell, vector<vector<bool>> &visited, string word, TrieNode* curr) {
    addWord(curr, word);
    for (pair<int,int> change : offset) {
        pair<int,int> newCell = make_pair(cell.first + change.first, cell.second + change.second);

        if (inBounds(newCell) && notRepeated(newCell, visited) && existsWord(newCell, curr)) {
            char l = board[newCell.first][newCell.second];

            visited[newCell.first][newCell.second] = true;
            dfs(newCell, visited, word + l, curr->children[l]);
            visited[newCell.first][newCell.second] = false;
        }
    }
}

/*
Starts a dfs at each of the 16 cells in the 4x4 board
*/
void searchWords() {
    vector<vector<bool>> visited(N, vector<bool>(N, false));
    for(int y = 0; y < N; y++) {
        for(int x = 0; x < N; x++) {
            pair<int,int> cell = make_pair(y, x);
            char l = board[y][x];
            string s(1, l);

            visited[cell.first][cell.second] = true;
            dfs(make_pair(y, x), visited, s, root.children[l]);
            visited[cell.first][cell.second] = false;
        }
    }
}

/*
Step 4: Prints out all the words
The way that the words are printed out greatly affects performance
There are multiple choices for word order
*/

/*
Basic: Prints out all words in dfs order, with a min length of 
    Pros: inputting each word is fast, because each path is not too dif. from prev path
    Cons: don't get through many words, miss out on high value words later on
Average score: 13k
Usually only get through words starting at the top row
*/
void printWords(int minLength = 5) {
    minLength = max(minLength, 3);
    // Prints words in dfs order
    for(int i = 0; i < words.size(); i++) {
        if (words[i].size() >= minLength)
            fout << words[i] << '\n';
    }
}

/*
Organizes words by length
This is what most wordhunt solvers do
    Pros: does not miss out on high value words
    Cons: inputting each word is very slow
Average score: also 10k
Putting each word is slow
*/
bool compareLen(string &a, string &b) {
    return a.size() > b.size();
}
void printWordsByLength() {
    sort(words.begin(), words.end(), compareLen);
    for(int i = 0; i < words.size(); i++) {
        fout << words[i] << '\n';
    }
}

int main() {
    construct_trie();
    inputBoard();
    searchWords();
    printWords();
}