 # 1. What is WordHunt?
 In WordHunt, players are presented with a 4x4 letter board. In 90
 seconds, they match as many words as they can.
 A "match" is accomplished by starting at one letter and moving to
 adjacent squares (including diagonals), building a word along the way.
 Letter cells cannot be reused in a single match.
 
 Example:
 O A T R
 I H P S
 H T N R
 E N E I
 TENS is a valid match.
 RST is not a valid match (it is not a word)
 TENT is not a valid match (reuses a letter)
 THAT is a valid match (T is used twice, but the Ts are different)

 Players receive points depending on the length of their matches.
 Each 3-letter match: 100 points
 4-letter match: 400 points
 5-letter match: 800 points
 6-letter match: 1400 points
 7-letter match: 1800 points
 ...

# 2. What is a WordHunt solver?
A WordHunt solver calculates all possible words on the grid.
It runs DFS from each letter in the grid to search for possible words.
It uses trie data structure to store possible words, and prune the DFS
when there are no more potential words along a path.

After finding all possible words, it displays the words to the user,
with the longest words first.

# 3. What does this WordHunt solver do differently?
Other WordHunt solvers display words from longest to shortest. While
this is intuitive, it is not optimal in practice. Players have to scan
the board for each input word. This wastes significant time.

For example, they may suggest words in the following order. This is
very time consuming – it would be better for the player to do
Stamp, Stampede, and Stampedes right after each other.
STAMPEDES
GREATNESS
...
...
STAMPEDE
...
...
STAMP

This word hunt solver has two sigificant optimizations:
1. It chooses a subset of the very best words to display.
This is based on a reward-to-complexity ratio, which factors in:
- Reward: the number of points a word is worth
- Complexity: considers multiple factoring, such as
- diagonals
- ambiguious word path
- (most importantly) similarity to other words.
2. It displays the words in DFS order, meaning that similar words
are near each other. This is only possible because we chose a subset
of the very best words – something that other solves do not do.


# 4. Implementation
To find all possible words, the solver runs DFS on the inputted word board.
It constructs a trie using the 2021 Scrabble Dictionary, and uses it to prune
DFS branches. During DFS, information about word complexity is stored.

To find the subset of the best words to display, the solver chooses
the word with the best reward-to-complexity ratio. It then relaxes the cost
of neighboring words based on similarity. It repeats this process until
a threshold complexity is reached. The algorithm is similar to Dijkstra's.

Note:
This is not necessarily optimal, as we are solving a constraint optimization problem.
However, since the "complexity threshold" is very high and an estimate of player speed,
the algorithm performs just as DP would in practice. It has the benefit of being
much quicker also.

# 5. Results:
This WordHunt solver is significantly better. Data coming soon!