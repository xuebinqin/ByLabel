# ground-truth-labeling
This is a tool for image and video annotation. Currently, it can only run 64bit Ubuntu OS.

USED LIBRARIES
====

This implementation is based on Opencv 2.4.9(3.1.0), and ubuntu 14.04 64 bit.

The edge detector used here is EdgeDrawing(EDLib.h) which is proposed in 
"C. Topal, C. Akinlar, Edge Drawing: A Combined Real-Time Edge and Segment Detector,” Journal of Visual Communication and Image Representation, 23(6), 862-872, 2012." 
and can be downloaded from http://ceng.anadolu.edu.tr/CV/downloads/downloads.aspx. We include the lib in the root directory. It is worthynote that we are using the 64 bit ubuntu version of Edge Drawing. We sugggest to test our algorithm on a 64bit Ubuntu OS.

The RCC based tracker is adapted from the method developed in 
"J. S. Stahl and S. Wang, “Edge grouping combining boundary and region information,” IEEE Trans. Image Processing, vol. 16, no. 10, pp. 2590–2606, 2007." 
We download the code from https://cse.sc.edu/~songwang/software.html.

INSTALLATION
====

Just Download the code: *git clone https://github.com/NathanUA/ground-truth-labeling.git*
The code has already been compiled.

RUNNING
====

Go to the root folder <ground-truth-labeling> and run: ./GDTL
