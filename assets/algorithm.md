# Algorithm
We want the minimal set consisting of edges that should be removed to from a directed graph in order for it to become acyclic.
Since this Problem is NP-complete, it cannot be calculated precisely, since it would take too much time computing it. For this issue, we will use a randomized algorithm: Monte-Carlo algorithm for a randomized minimal feedback arc set.

# Randomized algorithm

Let G := <V,E>
V = {0,1,2,3,4,5,6}
E = {(0, 1), (1, 2), (2, 4), (1, 4), (1, 3), (4, 3), (4, 5), (3, 6), (6, 0)}


1. Create a randomized permutation of V 
    random = (3, 6, 1, 5, 0, 2, 4)
2. Create a set of $(a,b) \in E$ by selecting (a,b) with a > b in order of the randomized V set. 
    fb-arc-set = {(0, 1), (1, 3), (4, 3), (4, 5)}
    ∣fb-arc-set∣ = 4

3. Compare the size of the generated set with all other generated solutions. (For the supervisor process)
    Select the solution with the smallest number of sets
    Then repeat this process

By removing edges (a,b) with a > b only edges where a < b  are left it creates a topological sort of the graph which is acyclic.


In addition to step 2, you check the position of it so it means pos(a) > pos(b)
On the edge (0,1) -> pos(0) > pos(1)  <=> 4  > 2 ? Yes write it to the solution! 