********************
File:	 readme.txt
Author:	 Igor Olonetsky
Date:	 01.06.2012
Version: 1.0
********************

==============================================================================================

This code implements the algorithm described in the paper:
  "TreeCANN - kd-tree Coherence Approximate Nearest Neighbor algorithm"
   See the paper for details of the matching algorithm.

The algorithm matches small (e.g. 8x8) fixed size rectangular patches between two images A and B (which
may also be the same image). The patches are densely overlapping: one patch is defined around every pixel
of the image. For each patch in image A, it finds an approximate nearest neighbor patch in image B, such
that the L2 distance between corresponding RGB pixels in the A patch and the B patch is minimized. 

==============================================================================================

Minimum requirements:
1. Matlab 2008 or later.
2. C/CPP compiler installed properly for mex compiling.

==============================================================================================

To install and run the TreeCANN algorithm, please use the following instructions:

1. run compile_mex.

2. Recompile the kd-tree libraty (ANN) if necessary. You can use the matlab wrapper written by Shai Bagon.

3. Run the following function to calculate NNF: [nnf_dist nnf_X nnf_Y] = run_TreeCANN(uint8(A), uint8(B), patch_w, grid); 
   The "grid" parameter defines the accuracy of the results (grid=1 for the highest accuracy)

4. Run the following function to calculate EXACT NNF: [nnf_dist nnf_X nnf_Y] = ENN_matching(uint8(A), uint8(B), patch_w); 

5. You can use test_TreeCANN.m script to run several examples of the "run_TreeCANN" and "ENN_matching" functions.


==============================================================================================
==============================================================================================
   
   License

   This software is provided under the provisions of the Lesser GNU Public License (LGPL). 
   see: http://www.gnu.org/copyleft/lesser.html.

   This software can be used only for research purposes, you should cite
   the aforementioned papers in any resulting publication.

   The Software is provided "as is", without warranty of any kind.

==============================================================================================

   CITATION
   
   If you use this code in a scientific project, you should cite the following works in any resulting publication:
   "Igor Olonetsky and Shai Avidan : TreeCANN - kd-tree Coherence Approximate Nearest Neighbor algorithm, ECCV, 2012"

==============================================================================================

   REFERENCES

C. Barnes, E. Shechtman, A. Finkelstein, and D. B. Goldman. PatchMatch source code:
http://www.cs.princeton.edu/gfx/pubs/Barnes_2010_TGP/index.php, 2010.


@ELECTRONIC{Mount2006,
  author = {David M. Mount and Sunil Arya},
  month = {August},
  year = {2006},
  title = {ANN: A Library for Approximate Nearest Neighbor Searching},
  note = {version 1.1.1},
  url = {http://www.cs.umd.edu/~mount/ANN/},
  owner = {bagon},
  timestamp = {2009.02.04}
}

@ELECTRONIC{Bagon2009,
  author = {Shai Bagon},
  month = {February},
  year = {2009},
  title = {Matlab class for ANN},
  url = {http://www.wisdom.weizmann.ac.il/~bagon/matlab.html},
  owner = {bagon},
  timestamp = {2009.02.04}
}




