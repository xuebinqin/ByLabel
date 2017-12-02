# ByLabel: A Boundary Based Semi-Automatic Image Annotation Tool
This is a tool for image and video annotation. Currently, it can only run on 64bit Ubuntu OS.</br>

USED LIBRARIES
====
Users do not have to set these libriaries up, since this tool has been compiled with required static libriaries.
----
This implementation is based on Opencv 2.4.9(3.1.0), and ubuntu 14.04 64 bit.</br>

The edge detector used here is EdgeDrawing(EDLib.h) which is proposed in 
"C. Topal, C. Akinlar, Edge Drawing: A Combined Real-Time Edge and Segment Detector,” Journal of Visual Communication and Image Representation, 23(6), 862-872, 2012." 
and can be downloaded from http://ceng.anadolu.edu.tr/CV/downloads/downloads.aspx. We include the lib in the root directory. It is worth to note that we are using the 64 bit ubuntu version of Edge Drawing. We sugggest to test our algorithm on a 64bit Ubuntu OS.

INSTALLATION
====

Just Download the code: *git clone https://github.com/NathanUA/ground-truth-labeling.git*</br>
The code has already been compiled.

RUNNING
====

Go to the root folder *ground-truth-labeling* and run: ./GDTL

ANNOTATION SETTINGS
====

Go to the root floder and open the GDTL.cfg as follows:</br>
multi_class 0  #set the number of classes in the annotation task: 0 two classes, 1 multiple classes</br>
source_type 1  #set the input of annotation: 1 images, 2 videos</br>
input_path ./Test/images #set the folder name of to-be-annotated images. If souce_type is 2, the full path and name of the video has to be set here</br>
output_path ../Test/annotation #set the output path, if folder annotation does not exit, the tool will create a new folder named as annotation. You can assign any name you want.</br>
simple_shape 1 </br>
start_idx 0 </br>

ANNOTATION CONTROL KEY
====
The main idea of this annotation tool is to label boundaries by selecting detected edge fragments.</br>

(1) MOUSE LEFT BUTTON CLICK
-
Move your mouse cursor over the target edge fragment and click left button to select it.

(2) a
-
If some boundary parts are not detected successfully, users can press 'a' to change selecting mode to drawing mode. Press 'a' again to change it back.

(3) f
-
Press 'f' to undo the last selection or drawing.

(4) b
-
If an edge fragment was not split successfully, move your mouse cursor over the postion where you want to split and press 'b'.

(5) e
-
Press 'e' to enable or disable the edge fragments showing.

(6) MOUSE MIDDLE BUTTON CLICK
-
Click mouse middle button to finish the closed boundary labeling.

CONTACTS
====

Xuebin Qin
----
Department of Computing Science</br>
University of Alberta</br>
Edmonton, AB, Canada, T6G 2E8</br>

Email:</br>
xuebin@ualberta.ca</br>
xuebinua@gmail.com

