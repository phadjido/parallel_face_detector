# Parallel face detector

A convolutional face detector in C. 

The uploaded code is sequential but has been derived by the OpenMP version by excluding all related directives and library calls. 
However, some code transformations the enable thread safety and loop parallelization have remained. 


# Related publication 

A high-performance face detection system using OpenMP.
P. E. Hadjidoukas, V.V. Dimakopoulos, M. Delakis, C. Garcia.
Concurrency and Computation: Practice & Experience, 21(15):1819-1837, October 2009.
https://doi.org/10.1002/cpe.1389

We present the development of a novel high-performance face detection system using a neural network-based classification algorithm and an efficient parallelization with OpenMP. 
We discuss the design of the system in detail along with experimental assessment. 
Our parallelization strategy starts with one level of threads and then exploits nested parallel regions to improve the image-processing capability by up to 19%. 
The presented system can process images in real-time (38 images/sec) by sustaining almost linear speedups on a system with a quad-core processor and a particular OpenMP runtime library.


# How to build and run the face detector

1. Edit src/Makefile, to set the compiler and its options
2. Run `make`, to build the executable (facedetect)
3. Edit `list.txt`, specify the list of input images
4. Run `facedetect`, get the location of the faces

4. Check `python/requirements.txt` and install the packages
5. Edit `python/show_faces.py`, to set the image and the location of the faces
6. Run `python/show_faces.py`, to see the bounding boxes on the image
 

# Todo

1. Upload OpenMP versions

