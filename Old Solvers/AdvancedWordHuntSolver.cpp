/**
 * Author: Akhil Kammila (https://github.com/akhilkammila)
 * 
 * Implements an advanced solver for WordHunt – an iMessage word game
 * 
 * This solver finds all possible words on a WordHunt board. It then chooses
 * a subset of the words based on a reward-to-complexity ratio. Complexity is
 * based on number of diagonals, ambiguity in the word's path, and (most importantly)
 * similarity to other chosen words. It then displays words in an optimal ordering.
 * 
 * The words are chosen using a greedy approach similar to Dijstra's algorithm.
 * The word with the best reward-to-complexity ratio is chosen, and the complexity of
 * "neighboring" words with similar prefixes is reduced.
 * 
 * Usage:
 * Hit run, and input the board in the form of a lowercase string, with no spaces.
 * The solved board will be printed to "solved.txt" (takes less than a second).
 */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
#include <unordered_set>
using namespace std;

// Debug template (ignore)
#ifdef LOCAL
#define DEBUG(...) debug(#__VA_ARGS__, __VA_ARGS__)
#include </Users/akhilkammila/Competitive Programming/debug.h>
#else
#define DEBUG(...) 47;
#endif

/*
Global variables
*/
const int N = 4;
const int complexityBound = 1400;
const int baseComplexity = 50;
const int diagComplexity = 5;
const int repeatComplexity = 20;
map<int,int> points = {{3,100}, {4,400}, {5, 800}, {6, 1400},
    {7, 1800}, {8, 2200}, {9, 2600}, {10, 3000}, {11, 3400}, {12, 3800}};
vector<vector<char>> board;
vector<pair<string,pair<int,int>>> words; //<word, <complexity, complexityUpdate>>
vector<string> filteredWords;

struct TrieNode {
    map<char, TrieNode*> children;
    bool word;
};
TrieNode root;

ifstream fin("../dictionary.txt");
ofstream fout("../solved.txt");

/*
Step 1:
Parses words from dictionary.txt and
creates a trie to store all the words
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
Ensures the string is 16 letters long
*/
void inputBoard() {
    // Reads in 16 letter long string
    board.resize(N, vector<char>());
    string bString;
    while(bString.size() != N*N) {
        cout << "Input Board:" << endl;
        cin >> bString;
    }

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
Search (dfs) through all possible words
*/

/*
Helper functions to
1) ensure that the cell we are attempting to visit is within the 4x4 grid
2) ensure that the cell is not already on the current path
3) ensure that if we visit the cell, there is a word that exists along that path
4) adds the word if it is valid (if it is marked in the trie as a word)
5) find the complexity that a new letter adds (based on diagonals, repeat letters, etc.)
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

void addWord(TrieNode* curr, string word, int complexity) {
    if (word.size() >= 3 && curr->word) {
        words.push_back(make_pair(word, make_pair(complexity,0)));
        curr->word = false; //prevents repeats
    }
}

vector<pair<int,int>> directions = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1}, {-1,0},{-1,-1}};
int complexityChange(pair<int,int> &cell, pair<int,int> &offset) {
    bool diag = offset.first != 0 && offset.second != 0;
    char l = board[cell.first + offset.first][cell.second + offset.second];
    int repeats = 0;

    for(pair<int,int> change : directions) {
        pair<int,int> newCell = make_pair(cell.first + change.first, cell.second + change.second);
        if (inBounds(newCell) && board[newCell.first][newCell.second] == l) repeats++;
    }
    repeats--;

    return diag * diagComplexity + repeats*repeatComplexity;
}

// Recurses through every possible word from a certain start cell
void dfs(pair<int,int> cell, vector<vector<bool>> &visited, string word, TrieNode* curr, int complexity) {
    addWord(curr, word, complexity);
    for (pair<int,int> offset : directions) {
        pair<int,int> newCell = make_pair(cell.first + offset.first, cell.second + offset.second);

        if (inBounds(newCell) && notRepeated(newCell, visited) && existsWord(newCell, curr)) {
            char l = board[newCell.first][newCell.second];
            int cShift = complexityChange(cell, offset);

            visited[newCell.first][newCell.second] = true;
            dfs(newCell, visited, word + l, curr->children[l], complexity + cShift);
            visited[newCell.first][newCell.second] = false;
        }
    }
}


// Starts a dfs at each of the 16 cells in the 4x4 board
void searchWords() {
    vector<vector<bool>> visited(N, vector<bool>(N, false));
    for(int y = 0; y < N; y++) {
        for(int x = 0; x < N; x++) {
            pair<int,int> cell = make_pair(y, x);
            char l = board[y][x];
            string s(1, l);

            visited[cell.first][cell.second] = true;
            dfs(make_pair(y, x), visited, s, root.children[l], baseComplexity);
            visited[cell.first][cell.second] = false;
        }
    }
}

/*
Step 4:
Filters and sorts the words
We cannot get through all the words - which ones should we choose?
    We filter based on multiple factors that affect "complexity":
    a base complexity that every word has
    # of diagonals in the word
    # of repeat letters in the word
    similarity to other added words (this is a very important one!)
The order that the words are printed out also greatly affects speed
*/

// Returns the reward that a word gives
int calculateReward(string &word) {
    return points[word.size()];
}

// Chooses the best word to add based on the reward-to-complexity ratio
pair<string,pair<int,int>> findBestRatio() {
    pair<string,pair<int,int>> best;

    double currRatio = 0;
    for(pair<string,pair<int,int>> entry : words) {
        double ratio = (double) calculateReward(entry.first) / (entry.second.first - entry.second.second);
        if (ratio > currRatio) {
            currRatio = ratio;
            best = entry;
        }
    }
    return best;
}

// Finds the similarity between two words (used to update complexity)
int findSimilarity(string &a, string &b) {
    int l = min(a.size(), b.size());
    int i = 0;
    while(i < l && a[i] == b[i]) i++;

    if (i > 5) return 60;
    if (i > 3) return 20;
    return 0;
}

// Given that a word was added, set its complexity to INF
// and update all other words (complexity of similar words
// are reduced)
void updateComplexities(string word) {
    for(int i = 0; i < words.size(); i++) {
        pair<string,pair<int,int>> *entry = &words[i];
        int update = findSimilarity((*entry).first, word);
        int currentUpdate = (*entry).second.second;
        (*entry).second.second = min((*entry).second.first-1, max(update, currentUpdate));
        if ((*entry).first == word) (*entry).second.first = INT_MAX;
    }
}

// Function to filter words. Repeatedly chooses the word with the
// best reward-to-complexity ratio, and updates other words' complexities
void filterByComplexity() {
    unordered_set<string> chosenWords;
    int complexityLeft = complexityBound;

    while(complexityLeft > 0) {
        pair<string,pair<int,int>> entry = findBestRatio();
        complexityLeft -= (entry.second.first - entry.second.second);
        chosenWords.insert(entry.first);
        updateComplexities(entry.first);
    }

    // Puts the chosenWords into a list (with dfs order preserved)
    for(int i = 0; i < words.size(); i++) {
        if (chosenWords.count(words[i].first)) {
            filteredWords.push_back(words[i].first);
        }
    }
}

/*
Step 5:
Prints the words
*/
void printWords() {
    for(int i = 0; i < filteredWords.size(); i++) {
        string word = filteredWords[i];
        cout << word << '\n';
    }
    cout << endl;
}

int main() {
    construct_trie();
    inputBoard();
    searchWords();
    filterByComplexity();
    printWords();
}