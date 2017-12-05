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

RUN
====

Go to the root folder *ground-truth-labeling* and run: ./GDTL

ANNOTATION configurations
====

Go to the root folder ByLabel, open bylabel.cfg:<br>

1. multi_class     #indicate the number of to-be-annotated classes<br>

    "0"     binary classes annotation.<br>

    "1"     multiple classes annotation.<br>

2. source_type     #indicate the type of to-be-annotated files<br>

    "1"     Images (All formats that supported by OpenCV such as JPG, PNG, BMP, TIF and MPEG).<br>

    "2"     Video (All formats that supported by OpenCV such as MP4 and AVI).<br>

3. input_path     #set the path of source images/video<br>

     If "souce_type" is "1", it should be the folder name of images, such as "./Test/images".<br>

     If "souce_type" is "2", it should be the full path and name of the video, such as "./Test/demo.avi".<br>

4. output_path     #set the path of outputs<br>

     Such as "./Test/annotation". If the folder "annotation" does not exit, the tool will create a new folder named as "annotation".<br>

5. simple_shape     #indicate the shape complexity of to-be-annotated objects<br>

    "0"     Target objects with multiple boundaries.<br>

    "1"     Target objects with single boundary.<br>

6. start_idx     #indicate the starting index of to-be-annotated images<br>

     This is used to neglect the first "start_idx" images/frames.<br>

ANNOTATION Operations
====
Mouse and keyboard operations of ByLabel:<br>

1. MOUSE LEFT BUTTON CLICK<br>

    Select detected edge fragments or draw control points.<br>

2. MOUSE MIDDLE BUTTON (MOUSE WHEEL) CLICK<br>

    Close the selected boundary.<br>

3. MOUSE WHEEL SCROLL<br>

    Zoom in/out the image.<br>

4. "A"<br>

    Switch between "selecting" and "drawing" mode.<br>

5. "B"<br>

    Break edge fragments at the pixel where mouse cursor is located.<br>

6. "E"<br>

    Enable or disable showing of detected edge fragments.<br>

7. "F"<br>

    Unselect the last selected edge fragment or segment.<br>

8. "ESC"<br>

    Exit ByLabel.<br>

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

