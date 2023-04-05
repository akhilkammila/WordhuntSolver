/**
 * Author: Akhil Kammila (https://github.com/akhilkammila)
 * 
 * Implements a naive solver for WordHunt – an iMessage word game
 * 
 * This solver finds all possible words on a WordHunt board using
 * DFS and a Trie for pruning. It then displays words in a naive
 * way, such as longest-to-shortest. Other online WordHunt solvers do
 * exactly this.
 * 
 * This solver has three settings:
 * 1. filterByLength(): orders words from longest to shortest (this is what
 * all other WordHunt solvers do)
 *     Pros: ensures that the highest value words are reached
 *     Cons: inputting words is very slow, there is no order
 * 
 * 2. filterBySize(): removes words under a certain length, and keeps DFS order
 *     Pros: preserves a fast order in which similar words are next to each other
 *     Cons: highest value words may not be reached
 * 
 * 3. filterByGoal(): chooses the longest words, but only until a certain point
 * threshold is reached. preserves dfs order. (essentially a better version of
 * filterByLength())
 *     Pros: ensures that high value words are reached, and keeps dfs order
 *     Cons: does not factor in word relatedness and other complexities
 *     This is better than filterByLength() and filterBySize() in practice
 * 
 * For an advanced (and better) solver, see AdvancedWordHuntSolver.cpp.
 * 
 * Usage:
 * Hit run, and input the board in the form of a lowercase string, with no spaces.
 * The solved board will be printed to "solved.txt" (takes less than a second).
 * To change settings, switch filterByLength() in the main function (at the bottom)
 * to the desired setting. (ex. replace filterByLength() with filterByGoal())
 * 
 * @copyright Copyright (c) 2023
 * 
 * Word Hunt Solver
 * Has filters by size, length, and goal
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

// Debug
#ifdef LOCAL
#define DEBUG(...) debug(#__VA_ARGS__, __VA_ARGS__)
#include </Users/akhilkammila/Competitive Programming/debug.h>
#else
#define DEBUG(...) 47;
#endif

// Declarations
int calculateReward(string &word);

/*
Global variables
*/
const int N = 4;
vector<vector<char>> board;
vector<string> words;
vector<int> complexities;

vector<string> filteredWords;
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
Step 4:
Sort and filter the words
The order that the words are printed out greatly affects speed
We cannot get through all the words - which ones should we choose?
*/

/*
Basic: Prints out all words in dfs order, with a min length of x
    Pros: inputting each word is fast, because each path is not too dif. from prev path
    Cons: don't get through many words, miss out on high value words later on
Average score: 13k
Usually only get through words starting at the top row
*/
void filterBySize(int minLength = 3) {
    minLength = max(minLength, 3);

    for(int i = 0; i < words.size(); i++) {
        if (words[i].size() >= minLength)
            filteredWords.push_back(words[i]);
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
void filterByLength() {
    filteredWords = words;
    sort(filteredWords.begin(), filteredWords.end(), compareLen);
}

/*
Places the words in finalWords into filteredWords
in sorted order
*/
unordered_set<string> finalWords;
void convertSetToList() {
    for(int i = 0; i < words.size(); i++) {
        if (finalWords.count(words[i])) {
            filteredWords.push_back(words[i]);
        }
    }
}
/*
Favors words with longer length, but only chooses
the words necessary to reach the goal
Words remain sorted in dfs order
    Pros: gets high value words and preserves sorted order
    Cons: don't account for diagonals, complexity, etc.
*/
void filterByGoal(int percentGoal = 25) {
    // Calculate the goal
    int total = 0;
    int length = 0;
    for(int i = 0; i < words.size(); i++) {
        total += calculateReward(words[i]);
        length = max(length, (int) words[i].size());
    }
    int goal = total * percentGoal/100;
    cout << total << ' ' << goal << endl;

    // Add the longest words while we are under our goal
    int points = 0;
    int i = 0;
    while (points < goal) {
        // Look for shorter words
        if (i == words.size()) {
            i = 0;
            length--;
        }

        // Check if word is of current size
        if (words[i].size() == length) {
            finalWords.insert(words[i]);
            points += calculateReward(words[i]);
        }
        i++;
    }
    convertSetToList();
}

/*
Step 5:
Print the words
For experimenting and optimization, this step stores results
Results stored:
    Score
    Word we got up until
    Similarity score
*/

// Calculates the similarity to the prevoius word
int calculateSimilarity(string &word, string &prevWord) {
    int similarity = 0;
    int minLength = min(word.size(), prevWord.size());

    int i = 0;
    while(i < minLength && word[i] == prevWord[i]) {
        similarity++;
        i++;
    }

    return similarity;
}

// Calculates the points that the word gives
map<int,int> points = {{3,100}, {4,400}, {5, 800}, {6, 1400},
{7, 1800}, {8, 2200}, {9, 2600}, {10, 3000}, {11, 3400}, {12, 3800}};
int calculateReward(string &word) {
    return points[word.size()];
}

// Prints the words
void printWords() {
    for(int i = 0; i < filteredWords.size(); i++) {
        fout << filteredWords[i] << '\n';
    }
    fout << endl;
}

void results() {
    string endWord;
    cout << "Input the last word you reached" << '\n';
    cin >> endWord;

    long long reward = 0;
    long long similarity = 0;
    long long numWords = 0;
    string lastWord = "";

    for(int i = 0; i < filteredWords.size() && lastWord != endWord; i++) {
        string currWord = filteredWords[i];
        reward += calculateReward(currWord);
        similarity += calculateSimilarity(currWord, lastWord);
        numWords++;
        lastWord = currWord;
    }
    cout << "score: " << reward << " similarity: " << similarity << " words: " << numWords << endl;
}

int main() {
    construct_trie();
    inputBoard();
    searchWords();
    filterByLength();
    printWords();
    // results();
}