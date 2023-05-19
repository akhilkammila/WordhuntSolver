# Usage

## Setup (only 2 commands)
 1. Install docker (https://www.docker.com/products/docker-desktop/)
 2. Pull the docker image: In terminal, run "docker pull ahilio/wordhunt_solver"
 3. Run a container: In terminal, run "docker run -it ahilio/wordhunt_solver"

<img width="600" alt="Commands" src="https://github.com/akhilkammila/WordhuntSolver/assets/68196076/2f1af823-ccd3-45f9-8a69-1d13f96ddd8b">


## Using the Solver
1. Input the Board

The program will prompt "Input Board:"
Type in the board as a 16-character string.
Ex: The below board is "oatrihpshtnrenei"

<img width="200" alt="Commands" src="https://github.com/akhilkammila/WordhuntSolver/assets/68196076/44f11fe9-42f5-47a0-af62-e81a203b50a8">

2. Use the Solver

After you input the board, the program will print a list of words. For example, here is the output for the board "oatrihpshtnrenei":

<img width="800" alt="Commands" src="https://github.com/akhilkammila/WordhuntSolver/assets/68196076/072608f3-e2c2-42d8-913e-e3e38724dda2">

This output is broken up into two sections.

1. The chosen words

Our program chooses a subset of words that you can realistically get in 90 seconds. These are the words from HAPTEN to INNERS.

The words are grouped based on their starting letter. They are ordered from top-left to bottom right. For example, HAPTEN HAPTENE and HAPTENS all start with the first H (on the second row). SPRINT, SPRENT, etc. start with the S on the same row. HENNIER HENNER and HENNERS start with the second H (on the third row).

2. Additional words

The rest of the words are not as valuable, but we still print them in case you have additional time. This section contains the words from PTERINS to the very end (HOA). These words are printed in longest-to-shortest order.

3. Point Summary
Lastly, we print a summary. The board has 64,900 points available (if you got all 119 possible words). The first section contained a subset of 18 words that you can realistically get, which totals to 26,400 points.

For a video demo, see here:

## Tips and Advanced Strategies
Once you get used to the solver, you can use these strategies to step up your game.

1. Read multiple words at once

Rather than looking up at the screen for every word, try reading multiple at a time. For example, read HAPTEN HAPTENE HAPTENS, then type in all three, then look up again.

2. Use the colors

The colors hint that the word is "ambiguous." What does this mean? Lets look at the word HAPTEN as an example. We start with the H on the second row, and then go to A and P. But now, there are two T's to choose from. Which one do we go to? A blue color signals that we should look counterclockwise starting from the top left. We choose the first T we find. A red color signals that we should look clockwise starting from the top. Again, we choose the first T we find.

<img width="200" alt="Commands" src="https://github.com/akhilkammila/WordhuntSolver/assets/68196076/44f11fe9-42f5-47a0-af62-e81a203b50a8">

The colors help significantly, as they help warn you and reduce errors. If you accidentally chose the wrong letter, you would have to restart the word!

# About

How is this solver better than every other solver? How does it give you consistently higher scores? Well heres how it works.

## 1. What do other WordHunt solvers do?
Search up WordHunt solver, and you will see dozens of websites. All of them do the exact same thing.

First, they calculate all possible words on the grid.
They runs DFS from each letter in the grid to search for possible words.
They uses trie data structure to store possible words, and prune the DFS
when there are no more potential words along a path.

After finding all possible words, they displays the words to the user,
with the longest words first. Some solvers show you how to draw the path on the board (but it doesn't help with speed).

<img width="329" alt="Screen Shot 2023-05-19 at 1 09 47 PM" src="https://github.com/akhilkammila/WordhuntSolver/assets/68196076/b0431588-05d9-4708-a2a4-f0160310933c">

## 3. What does this WordHunt solver do differently?
Every single online WordHunt solver display words from longest to shortest (seriously, every one, try searching online).
While this is intuitive, it is not optimal in practice.

For example, let's look at the output of the solver shown above. Many similar words are not displayed together. HAPTEN and HAPTENS are nearly identical, but they are listed at completely different points. Doing them one-after-the-other would be much faster than going back to the word at a later point. The same goes for TENNER and TENNERS, HENNIER and HENNER and HENNERS, and more.

How does our solver group these words together?

1. It chooses a subset of the very best words to display.
This is based on a reward-to-complexity ratio, which factors in:
- Reward: the number of points a word is worth
- Complexity: includes factors such as
- diagonals
- ambiguious word path
- (most importantly) similarity to other words.
2. It displays the words in DFS order, meaning that similar words
are near each other. This is only possible because we chose a subset
of the very best words – something that other solvers do not do.

## 4. Implementation
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

## 5. Results and Testing
I played ten games against myself, by sending WordHunts from my phone to iPad.
On one device, I used the best online solver I could find (https://www.dcode.fr/word-hunt-game-pigeon-solver). On the other device, I used this solver.
I alternated which solver I started with and which device I used.

My own solver beat the online solver 10/10 times. It averaged 24,280 versus 20,460. While this may not seem significant, it is a huge difference when playing against good players.

I made this project in an attempt to beat my roomate Abhinav. I could not beat him with online solvers, since he is a sweat and averages over 20k. Playing him with online solvers, I lost 4/5 times. With my latest solver, I beat him 4/5 times. So, it seems to work!

Try it out for yourself – try beating that one guy in the friend group that is cracked at Word Hunt. See if you can beat them with online solvers, and then try with this one!

Losing vs Abhinav (won 1/5 times with old solver)

<img width="200" alt="Screen Shot 2023-05-19 at 1 09 47 PM" src="https://github.com/akhilkammila/WordhuntSolver/assets/68196076/4e5d8be2-7511-4864-9fef-2b6d3b511f90">

Winning vs Abhinav (won 4/5 times with new solver)

<img width="200" alt="Screen Shot 2023-05-19 at 1 09 47 PM" src="https://github.com/akhilkammila/WordhuntSolver/assets/68196076/97f4747e-133e-4e15-8255-ba3293e48f87">