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

struct letter {
    char l;
    int row;
    int col;
    bool isDiag;
    int repeat;
};
ostream& operator<<(ostream &os, const letter&l) {
    return os << "\033[1;35m" << l.l;
}
bool operator==(const letter& la, const letter& lb)
{
    return (la.row == lb.row && la.col == lb.col);
}
struct word {
    vector<letter> path;
    int complexity;
    int complexityUpdate;
};
ostream& operator<<(ostream &os, const word& w) {
    for(letter l : w.path) {
        os << l;
    }
    return os;
}

map<int,int> points = {{3,100}, {4,400}, {5, 800}, {6, 1400},
    {7, 1800}, {8, 2200}, {9, 2600}, {10, 3000}, {11, 3400}, {12, 3800}};

vector<vector<char>> board;
vector<word> words; // all possible words
vector<word> filteredWords; // the subset of words we choose

struct TrieNode {
    map<char, TrieNode*> children;
    bool isWord;
};
TrieNode root;

ifstream fin("dictionary.txt");
ofstream fout("solved.txt");

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
        curr->isWord = true;
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
bool inBounds(const letter &l) {
    return 0 <= l.row && l.row < N && 0 <= l.col && l.col < N;
}

bool notRepeated(const letter &l, const word &w) {
    for(letter used : w.path) {
        if (l == used) return false;
    }
    return true;
}

bool existsWord(const letter &l, const TrieNode* curr) {
    return curr->children.find(l.l) != curr->children.end();
}

void initializeLetter (letter &l) {
    l.l = board[l.row][l.col];
    l.isDiag = false;
    l.repeat = 0;
}

// Given a word and a current node, adds the word to words if it is valid
void addWord(const word &word, TrieNode* curr) {
    int n = word.path.size();
    char c = word.path[n-1].l;

    if (n >= 3 && curr->isWord) {
        words.push_back(word);
        curr->isWord = false; //prevents repeats
    }
}

vector<pair<int,int>> directions = {{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0}};
int complexityChange(letter &l, word &w) {
    // Check for diagonal
    letter lastLetter = w.path[w.path.size() - 1];
    bool diag = (l.row - lastLetter.row) != 0 && (l.col - lastLetter.col) != 0;
    l.isDiag = diag;

    // Check for repeats
    int repeats = 0;
    for(pair<int,int> change : directions) {
        letter neighborL;
        neighborL.row = l.row + change.first; neighborL.col = l.col + change.second;

        if (inBounds(neighborL) && board[l.row][l.col] == l.l && notRepeated(neighborL, w)) repeats++;
    }
    repeats--;
    l.repeat = repeats;

    return diag * diagComplexity + repeats*repeatComplexity;
}

// Recurses through every possible word from a certain letter
void dfs(word w, TrieNode* curr) {
    addWord(w, curr); // if it is a valid word, add to words

    for (pair<int,int> offset : directions) {
        letter l;
        l.row = w.path[w.path.size() - 1].row + offset.first;
        l.col = w.path[w.path.size() - 1].col + offset.second;
        if (!inBounds(l)) continue;
        initializeLetter(l);

        if (notRepeated(l, w) && existsWord(l, curr)) {
            initializeLetter(l);
            w.complexity += complexityChange(l, w); //marks l also

            w.path.push_back(l);
            dfs(w, curr->children[l.l]);
            w.path.pop_back();
        }
    }
}


// Starts a dfs at each of the 16 cells in the 4x4 board
void searchWords() {
    for(int r = 0; r < N; r++) {
        for(int c = 0; c < N; c++) {
            word w;
            letter l; l.row = r; l.col = c; l.l = board[r][c];
        
            w.path.push_back(l);
            dfs(w, root.children[l.l]);
            w.path.pop_back();
        }
    }
}
// oatrihpshtnrenei

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

// // Returns the reward that a word gives
// int calculateReward(string &word) {
//     return points[word.size()];
// }

// // Chooses the best word to add based on the reward-to-complexity ratio
// pair<string,pair<int,int>> findBestRatio() {
//     pair<string,pair<int,int>> best;

//     double currRatio = 0;
//     for(pair<string,pair<int,int>> entry : words) {
//         double ratio = (double) calculateReward(entry.first) / (entry.second.first - entry.second.second);
//         if (ratio > currRatio) {
//             currRatio = ratio;
//             best = entry;
//         }
//     }
//     return best;
// }

// // Finds the similarity between two words (used to update complexity)
// int findSimilarity(string &a, string &b) {
//     int l = min(a.size(), b.size());
//     int i = 0;
//     while(i < l && a[i] == b[i]) i++;

//     if (i > 5) return 60;
//     if (i > 3) return 20;
//     return 0;
// }

// // Given that a word was added, set its complexity to INF
// // and update all other words (complexity of similar words
// // are reduced)
// void updateComplexities(string word) {
//     for(int i = 0; i < words.size(); i++) {
//         pair<string,pair<int,int>> *entry = &words[i];
//         int update = findSimilarity((*entry).first, word);
//         int currentUpdate = (*entry).second.second;
//         (*entry).second.second = min((*entry).second.first-1, max(update, currentUpdate));
//         if ((*entry).first == word) (*entry).second.first = INT_MAX;
//     }
// }

// // Function to filter words. Repeatedly chooses the word with the
// // best reward-to-complexity ratio, and updates other words' complexities
// void filterByComplexity() {
//     unordered_set<string> chosenWords;
//     int complexityLeft = complexityBound;

//     while(complexityLeft > 0) {
//         pair<string,pair<int,int>> entry = findBestRatio();
//         DEBUG(entry);
//         complexityLeft -= (entry.second.first - entry.second.second);
//         chosenWords.insert(entry.first);
//         updateComplexities(entry.first);
//     }

//     // Puts the chosenWords into a list (with dfs order preserved)
//     for(int i = 0; i < words.size(); i++) {
//         if (chosenWords.count(words[i].first)) {
//             filteredWords.push_back(words[i].first);
//         }
//     }
// }

/*
Step 5:
Prints the words
*/
// void printWords() {
//     for(int i = 0; i < filteredWords.size(); i++) {
//         string word = filteredWords[i];
//         // add space between sections
//         if (i > 0) {
//             string prevWord = filteredWords[i-1];
//             if (word[0] != prevWord[0]) cout << '\n';
//         }
//         cout << word << '\n';
//     }
//     cout << endl;
// }

int main() {
    construct_trie();
    inputBoard();
    searchWords();
    DEBUG(words);
    cout << words.size();
    // filterByComplexity();
    // printWords();
}