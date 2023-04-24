# Steps
1. Compute global average
2. Assign new pixel values depending on global average

# Solution
1. P0 reads in image
2. P0 scatters parts of the image accross all processes
3. Each process computes average on their part of image.
4. Barriere
5. Do allreduce with MPI_SUM
6. Each process updates their part of the image based on allreduce result
7. Barriere
8. Gather results on P0
9. Assemble and save image from P0