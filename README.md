# ByLabel: A Boundary Based Semi-Automatic Image Annotation Tool
This is a tool for image and video annotation. Currently, it can only run on 64bit Ubuntu OS.</br>

USED LIBRARIES
====

This implementation is based on Opencv 2.4.9(3.1.0), and ubuntu 14.04 64 bit.</br>

The edge detector used here is EdgeDrawing(EDLib.h) which is proposed in 
"C. Topal, C. Akinlar, Edge Drawing: A Combined Real-Time Edge and Segment Detector,” Journal of Visual Communication and Image Representation, 23(6), 862-872, 2012." 
and can be downloaded from http://ceng.anadolu.edu.tr/CV/downloads/downloads.aspx. We include the lib in the root directory. It is worthynote that we are using the 64 bit ubuntu version of Edge Drawing. We sugggest to test our algorithm on a 64bit Ubuntu OS.

INSTALLATION
====

Just Download the code: *git clone https://github.com/NathanUA/ground-truth-labeling.git*</br>
The code has already been compiled.

RUNNING
====

Go to the root folder <ground-truth-labeling> and run: ./GDTL
