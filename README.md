# SkeLibEd #
SkeLibEd is a parallel skeleton library developed for The University of Edinburgh. It includes several algorithmic skeletons and some tests on real problems.

## Skeletons:
  * Map (The workload is statically allocated before threads are run)
  * Reduce
  * MapReduce
  * Scan
  * Dynamic Map (Workload is allocated and balanced at run-time. Note there are few different variants as they were part of the development stages.
  * Stencil (with 4 boundary-handling strategies NORMAL, WRAP, MIRROR, BORDER. Note there are also four different experiments for optimisation over the Normal boundary handling)


## Current available tests:

  * Collatz (for Map and Dynamic Map)
  * Mandelbrot set (for Map and Dynamic Map)
  * Nbody (for Map and Dynamic Map)
  * Heat Distribution (for Stencil)
  * Game of Life / celullar automata test (for Stencil)
  * Image post-processing (for Stencil)

## Running tests (NOTE: The testing is currently at a horrible state and needs to be cleaned and fixed.)
TODO: fix testing
