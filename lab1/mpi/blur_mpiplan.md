# Solution
1. P0 reads in image
2. P0 scatters a number of rows of the image accross all processes
3. Each process computes their assigned rows, doing row-wise average
4. Barrier
5. Gather results on P0
6. P0 scatters parts of the image accross all processes (rows + radius amount of rows on each direction)
7. Each process computes their assigned columns
8. Barrier
9. Gather results on P0
10. Assemble and save image from P0