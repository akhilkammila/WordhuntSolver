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
#include <set>
#include "FinalWordHuntSolver.h"
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
const int complexityBound = 1000;
const int baseComplexity = 50;
const int diagComplexity = 10;
const int repeatComplexity = 10;

map<int,int> points = {{3,100}, {4,400}, {5, 800}, {6, 1400},
    {7, 1800}, {8, 2200}, {9, 2600}, {10, 3000}, {11, 3400}, {12, 3800}};
map<int,int> similarityWorth = {{0,0},{1,0},{2,0},{3,5},{4,10},{5,50},
    {6,1000},{7,1000},{8,1000},{9,1000},{10,1000}};

vector<vector<char>> board;
vector<word> words; // all possible words
vector<word> chosenWords; // the subset of words we choose
vector<word> filteredWords; //chosenWords but in optimal order
TrieNode root;

ifstream fin("InputOutput/dictionary.txt");
ofstream fout("InputOutput/solved.txt");

/*
Step 1:
Parses words from dictionary.txt and
creates a trie to store all the words
*/
void construct_trie() {
    string w;
    while(fin >> w) {
        cout << w << endl;
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

    // while(bString.size() != N*N) {
    cout << "Input Board:" << endl;
    cin >> bString;
    // }

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

// Check if a letter's position + an offset stays in bounds
bool inBounds(const letter &l, const pair<int,int> &offset) {
    int x = l.row + offset.first; int y = l.col + offset.second;
    return 0 <= x && x < N && 0 <= y && y < N;
}

// Checks if a letter l can be added to word w
// without causing a repeat
bool notRepeated(const letter &l, const word &w) {
    for(letter used : w.path) {
        if (l == used) return false;
    }
    return true;
}

// Checks if we should prune the branch or not if we are at a letter l
// and a TrieNode curr
bool existsWord(const letter &l, const TrieNode* curr) {
    return curr->children.find(l.l) != curr->children.end();
}

// Given a word and a current node, adds the word to words if it is valid
// returns whether a word was added
bool addWord(const word &word, const TrieNode* curr) {
    int n = word.path.size();
    char c = word.path[n-1].l;

    if (n >= 3 && curr->isWord) {
        words.push_back(word); // WORDS MODIFIED HERE
        return true;
    }
    return false;
}

vector<pair<int,int>> directions = {{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0}};

bool diagonal(const letter &l, const word &w) {
    letter prev = w.path[w.path.size() - 1];
    return (l.row - prev.row) != 0 && (l.col - prev.col) != 0;
}

pair<int,int> repeats(const letter &l, const word &w) {
    letter lastLetter = w.path[w.path.size() - 1];
    int repeats = 0;
    int lRepeatNum = 0; //if there are repeats, check if l is 1st or 2nd
    for(pair<int,int> change : directions) {
        if (!inBounds(lastLetter, change)) continue;

        letter n;
        n.row = lastLetter.row + change.first;
        n.col = lastLetter.col + change.second;

        if (l.row == n.row && l.col == n.col) lRepeatNum = repeats;
        else if (board[n.row][n.col] == l.l && notRepeated(n, w)) repeats++;
    }

    return make_pair(repeats, lRepeatNum);
}

// Recurses through every possible word from a certain letter
void dfs(word w, TrieNode* curr) {
    if (addWord(w, curr)) curr->isWord = false; //prevents repeats

    letter last = w.path[w.path.size() - 1];

    for (pair<int,int> offset : directions) {
        if (!inBounds(last, offset)) continue;

        int r = last.row + offset.first; int c = last.col + offset.second;
        letter l = {board[r][c], r, c, false, 0};

        if (notRepeated(l, w) && existsWord(l, curr)) {
            l.isDiag = diagonal(l, w);
            tie(l.repeat, l.repeatNum) = repeats(l, w);
            int cChange = l.isDiag * diagComplexity + l.repeat * repeatComplexity;

            w.path.push_back(l);
            w.complexity += cChange;
            dfs(w, curr->children[l.l]);
            w.path.pop_back();
            w.complexity -= cChange;
        }
    }
}

// Starts a dfs at each of the 16 cells in the 4x4 board
void searchWords() {
    for(int r = 0; r < N; r++) {
        for(int c = 0; c < N; c++) {
            cout << "searching " << r << " " << c << endl;
            word w = {
                vector<letter>(), baseComplexity, 0, false
            };
            letter l = {
                board[r][c], r, c, false, 0
            };

            w.path.push_back(l);
            dfs(w, root.children[l.l]);
            w.path.pop_back();
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
int calculateReward(word &w) {
    return points[w.path.size()];
}

// Chooses the best word to add based on the reward-to-complexity ratio
word* findBestRatio() {
    word* best;
    double currRatio = 0;

    for(int i = 0; i < words.size(); i++) {
        word *w = &words[i];
        if ((*w).chosen) continue;
        double ratio = (double) calculateReward(*w) / ((*w).complexity - (*w).complexityUpdate);

        if (ratio > currRatio) {
            currRatio = ratio;
            best = w;
        }
    }
    return best;
}

// Finds the similarity between two words (used to update complexity)
int findSimilarity(word &a, word &b) {
    int l = min(a.path.size(), b.path.size());
    int i = 0;
    while(i < l && a.path[i] == b.path[i]) i++;
    return i;
}

// Given that a word was added, set its complexity to INF
// and update all other words (complexity of similar words
// are reduced)
void updateComplexities(word &w) {
    for(int i = 0; i < words.size(); i++) {
        word *w2 = &words[i];
        int similarity = findSimilarity(*w2, w);
        int update = similarityWorth[similarity];

        update = max(update, (*w2).complexityUpdate);
        if ((*w2).complexity - update <= 0) update = (*w2).complexity - 1;
        (*w2).complexityUpdate = update;
    }
}

void orderOptimally() {
    for(int i = 0; i < words.size(); i++) {
        bool isChosen = false;
        for(word w : chosenWords) {
            if (words[i] == w) isChosen = true;
        }
        if (isChosen) filteredWords.push_back(words[i]);
    }
}

// Function to filter words. Repeatedly chooses the word with the
// best reward-to-complexity ratio, and updates other words' complexities
void chooseWords() {
    int complexityLeft = complexityBound;

    while(complexityLeft > 0) {
        word* w = findBestRatio();

        (*w).chosen = true;
        complexityLeft -= ((*w).complexity - (*w).complexityUpdate);
        chosenWords.push_back(*w);
        updateComplexities(*w);
    }
}

/*
Step 5:
Prints the words
Records stats of a board
*/
void printFilteredWords() {
    for(int i = 0; i < filteredWords.size(); i++) {
        if (i > 0 && !(filteredWords[i].path[0] == filteredWords[i-1].path[0])) cout << '\n';
        cout << filteredWords[i] << '\n';
    }
    cout << endl;
}

bool compareByLength(const word &a, const word &b) {
    return a.path.size() > b.path.size();
}
void printAdditionalWords() {
    vector<word> sortedWords(words.begin(), words.end());
    sort(sortedWords.begin(), sortedWords.end(), compareByLength);

    for(word w: sortedWords) {
        if (!w.chosen) cout << w << '\n';
    }
}

WordListStats allWordsStats() {
    WordListStats s = {0,0,0};

    for(word w : words) {
        s.numWords += 1;
        s.totalReward += calculateReward(w);
        s.totalComplexity += w.complexity;
    }
    return s;
}
WordListStats chosenWordsStats() {
    WordListStats s = {0,0,0};

    for(word w : filteredWords) {
        s.numWords += 1;
        s.totalReward += calculateReward(w);
        s.totalComplexity += w.complexity;
    }
    return s;
}
void trackStats() {
    WordListStats all = allWordsStats();
    WordListStats chosen = chosenWordsStats();

    cout << "total reward: " << all.totalReward << '\n';
    cout << "total words: " << all.numWords << '\n';
    cout << "chosen reward: " << chosen.totalReward << '\n';
    cout << "chosen words: " << chosen.numWords << '\n';
}

int main() {
    construct_trie(); //Part 1
    cout << "Part 1 complete" << endl;
    inputBoard(); //Part 2
    cout << "Part 2 complete" << endl;
    searchWords(); //Part 3
    cout << "Part 3 complete" << endl;
    chooseWords(); // Part 4
    cout << "Part 4 complete" << endl;
    orderOptimally();
    printFilteredWords(); // Part 5
    printAdditionalWords();
    trackStats();
    cout << "Part 5 complete" << endl;
}