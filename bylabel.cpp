#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <map>
#include <ctime>
#include <sstream>
#include <algorithm>

#include <sys/stat.h> //create directory
#include <errno.h>    // errno, ENOENT, EEXIST
#if defined(_WIN32)
#include <direct.h>   // _mkdir
#endif
#include <iterator>

#include "Timer.h"
#include "LS.h"
#include "EdgeMap.h"
#include "EDLib.h"

#define DivideErrThre_e 1.4//edge breaking threshold
#define DivideErrThre_m 500//5

//#define start_frame_id 0 //starting video frame number

using namespace cv;
using namespace std;

struct EdgeFragment{//divided edge fragments from detected edge segments
    int seg_id;//edge fragements belong to the seg_id th detected segment
    int start_idx;// its start index in the seg_id th detected segment
    int end_idx;// its end index in the seg_id th detected segment
};

//int label_flag = 1;//finish this whole frame labeling (press key 's')
int mbd_gt = 0;//mouse button down for finish current shape labeling
int lbd_gt = 0;//mouse middle button dow-L lib -Wl,-rpath,'$$ORIGIN/lib'n for selecting edge fragments
Point pt_lbd_gt, pt_mv_gt;//left mouse button down point and mouse move points
//vector<Point>gtLabelPts;//store left mouse button clicked points

void createColorPallet(vector<vector<int> > &color_pallet);

void CallBackFunc(int event, int x, int y, int flags, void* userdata);//mouse event response function
bool isDirExist(const std::string& path);//check if a directory exists
bool makePath(const std::string& path);//create a new directory

float distance(float x1, float y1, float x2, float y2);//compute distance between two points (x1,y1) and (x2,y2)
int EdgeBreakFit(float xyh[3][4]);//edge segment breaking measure computing
vector<vector<int> > randColor(int num_color);//generate different colors for each fragment
Mat drawESs(EdgeMap *map, Mat frame_label_tmp);//draw edge segments
vector<vector<Point> > EdgeFilterBreak(EdgeMap *map);//edge segment breaking algorithm
int findEF(Point pt, vector<vector<Point> > edgeFragments, int &EF_id, int &Pixel_id);//find the closest fragment to the mouse pointer
int drawEF(vector<Point> &edgeFragment, Mat &frame_label_tmp, int rc, int gc, int bc);//draw one edge fragment
int drawEFs( vector<vector<Point> > &edgeFragments, Mat &frame_label_tmp, vector<vector<int> > rand_color);//draw edge fragments
int linesOrder(Point pt1, Point pt2, Point pt3, Point pt4);//find the closed endpoints of two segments
int linesToPtOrder(Point pt0, Point pt1, Point pt2);//find the closed endpoints of a segments to a certain point
int pointsOnLine(Mat &frame, Point pt1, Point pt2, vector<Point> &tmp_points);//find points on a straight line
int addLabelFragments(Mat &frame, int EF_LS_flg, Point pt_lbd_gt,vector<vector<Point> > edgeFragments,
                      vector<vector<Point> > &labeledFragments, vector<int> &labeledEFID, vector<int> &labeledEFtype);//add new fragment or segmet to current shape
int finishCurrentLabel(Mat &frame,vector<vector<Point> > &labeledFragments, vector<int> &labeledEFID, vector<int> &labeledEFtype);//finish current shape labeling and results a closed boundary
int drawLFs(vector<vector<Point> > &labeledFragments, vector<int> &labeledEFtype, Mat &show_label_tmp);//draw label fragments and segments
//int drawLabelEdges(Mat &bw_im_edge, Mat &bw_im_region, vector<vector<Point> > &labeledFragments);//output binary edge and region image of labels
void drawLabelResults(int biMultiClass_flg, vector<vector<int> > &color_pallet, Mat &edge_map_classes, Mat &edge_map_instances, Mat &region_map_classes, Mat &region_map_instances,
                      vector<vector<vector<Point> > > &labeledShapes, vector<int> &labeledShapeIdx, vector<int> &labeledShapeClassName);

template<typename Out>
void split(const string &s, char delim, Out result);
vector<string> split(const string &s, char delim);
void outputLabels(vector<vector<int> > &color_pallet, Mat &edge_map_classes, Mat &edge_map_instances, Mat &region_map_classes, Mat &region_map_instances, Mat &show_label_tmp, vector<vector<vector<Point> > > &labeledShapes, vector<int> &labeledShapeIdx,
                  vector<vector<int> > &labeledShapeEFID, vector<vector<int> > &labeledShapeEFtypes, vector<string> &classNameList, vector<int> &labeledShapeClassName,
                  int video_proc, int frame_id, vector<string> &directories,vector<string> &namePatches, vector<string> &outputPathName);//output label results to corresponding files
void saveEdgePixelsToTxt(vector<vector<vector<Point> > > &labeledShapes, vector<int> &labeledShapeIdx, vector<vector<int> > &labeledShapeEFID, vector<vector<int> > &labeledShapeEFtypes,
                         vector<string> &classNameList,vector<int> &labeledShapeClassName, string &txtName);
void saveEFPixelsToTxt(vector<vector<Point> > &edgeFragments, string &EFPixelsPathName);//saving detected Edge Fragments
int inputClassName(string &tmp_class_name, int mbd_gt);//get input of class name
int inputNewShapeYN(int mbd_gt);//if start labeling a new shape on the current image

//===========================MAIN FUNCTION START===========================
//===========================MAIN FUNCTION START===========================
//===========================MAIN FUNCTION START===========================
//===========================MAIN FUNCTION START===========================

int main(/*int argc, char* argv[]*/){//Run Command: "./GDTL [0/1 biclass/multiclass] [1/2 read images from image folder or video] [images folder/video path_name] [results folder]"

    //cout<<"==============================================="<<endl;
    cout<<">>>>>>>>>>>>>>>>>>READ CONFIGURES<<<<<<<<<<<<<<<"<<endl;
    cout<<"==============================================="<<endl;
    ifstream infile("./bylabel.cfg");

    vector<vector<string> > para_lines;
    string paras;
    while(getline(infile,paras)){
        //cout<<paras<<endl;
        istringstream iss(paras);
        string parse;
        vector<string> para_line;
        while(iss >> parse){
            para_line.push_back(parse);

        }
        para_lines.push_back(para_line);
        para_line.clear();
        vector<string>().swap(para_line);
    }

    for(int i = 0; i < 6; i++){
        cout<<para_lines[i][0]<<" "<<para_lines[i][1]<<endl;
    }


    cout<<">>>>>>>>>>>>>>>>>>START LABELING<<<<<<<<<<<<<<<"<<endl;
    //cout<<"==============================================="<<endl;
    int key;
    int video_proc = atoi(para_lines[1][1].c_str());//0;//video_proc: 0 read from webcam, 1 read from video file
    int biMultiClass_flg = atoi(para_lines[0][1].c_str());//atoi(argv[1]);//0: bi-class, 1: multi-class
    int simple_shape = atoi(para_lines[4][1].c_str());//0: objects are defined by single boundary, 1: objects are defined by multiple boundaries
    int start_frame_id = atoi(para_lines[5][1].c_str());

    //video_proc = atoi(argv[2]);

    VideoCapture cap;
    //VideoWriter videoRecord;

    Mat frame;//read each frame from video sequences
    Mat grayscaleFrame;//get gray scale frame for feature detection
    int frame_id = 0;
    int frame_width;
    int frame_height;

    vector<String> imPathNames;//image path and name
    vector<string> directories;
    vector<string> namePatches;
    vector<string> im_name_ext;//

    //create directories for saving label results
    //string outputFolder(argv[4]);
    string outputFolder(para_lines[3][1].c_str());
    directories.push_back(outputFolder+"/edge_map_classes/");
    directories.push_back(outputFolder+"/edge_map_instances/");
    directories.push_back(outputFolder+"/region_map_classes/");
    directories.push_back(outputFolder+"/region_map_instances/");
    directories.push_back(outputFolder+"/color_im_overlap/");
    directories.push_back(outputFolder+"/text_shape_pixels/");
    directories.push_back(outputFolder+"/text_EF_pixels/");

    makePath(outputFolder);//mkdir ../yourfolder
    makePath(directories[0]);
    makePath(directories[1]);
    makePath(directories[2]);
    makePath(directories[3]);
    makePath(directories[4]);
    makePath(directories[5]);
    makePath(directories[6]);

    if(1 == video_proc){//read one image
        cout<<"Read Images From: "<<endl;
        cout<<para_lines[2][1]<<endl;
        //cout<<argv[3]<<endl;
        string imPath(para_lines[2][1].c_str());
        String imPathStr = imPath;
        glob(imPathStr, imPathNames);

        frame_id = frame_id + start_frame_id;
    }
    else if(2 == video_proc){//read frame from recorded video

        cout<<"Read Frames From Video: "<<endl;
        cout<<para_lines[2][1]<<endl;
        //cout<<argv[3]<<endl;
        const char* video_path = para_lines[2][1].c_str();//argv[3];
        cap = VideoCapture(video_path);

        if(!cap.isOpened())//check if reading works
            return -1;

        for(int i = 0; i<start_frame_id; i++){
            cout<<" ignore frame: "<<i<<endl;
            if(!cap.read(frame)){
                break;
            }
        }
        frame_id = frame_id + start_frame_id;

        //get video frame width and height
        frame_width =  cap.get(CV_CAP_PROP_FRAME_WIDTH);
        frame_height =  cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    }

    //generate color pallet
    vector<vector<int> > color_pallet;
    createColorPallet(color_pallet);

    vector<string> classNameList;//store class name of each classes
    //+++++++++++++++++++++++++++++++++++++READ IMAGES/VIDEO FRAMES+++++++++++++++++++++++++++++++++
    for(;;){//start read frames

        cout<<"==============================================="<<endl;
        string tmp_txt_EFP;//path for saving detected Edge Fragments
        if(1 == video_proc){//read one image
            if(frame_id < imPathNames.size()){
                frame = imread(imPathNames[frame_id],1);
                frame_width = frame.size().width;
                frame_height = frame.size().height;

                if(!namePatches.empty()){
                    namePatches.clear();
                    vector<string>().swap(namePatches);
                }

                namePatches = split(imPathNames[frame_id],'/');
                im_name_ext = split(namePatches[namePatches.size()-1],'.');

                //cout<<"size: Name ext "<<im_name_ext.size()<<endl;
                //cout<<"im_name_ext: "<<im_name_ext[0]<<" "<<im_name_ext[im_name_ext.size()-1]<<endl;

                tmp_txt_EFP = directories[6] + im_name_ext[0] + ".txt";
                //tmp_txt_EFP[tmp_txt_EFP.size()-3] = 't';
                //tmp_txt_EFP[tmp_txt_EFP.size()-2] = 'x';
                //tmp_txt_EFP[tmp_txt_EFP.size()-1] = 't';
                cout<<"--Current Labeling Image Index: "<<frame_id<<endl;
                cout<<"--Current Labeling Image is the "<<frame_id + 1<<"-th one of "<<imPathNames.size()<<": "<<endl;
                cout<<"    "<<imPathNames[frame_id]<<endl;
                cout<<"-----------------------------------------------"<<endl;
            }
            else{
                return 0;
            }

        }else if(2 == video_proc ){
            cout<<"--Current Labeling Frame Index: "<<frame_id<<endl;
            cout<<"--Current Labeling Frame ID: "<<frame_id<<endl;
            cout<<"-----------------------------------------------"<<endl;

            char tmp_EFPName[20];
            sprintf(tmp_EFPName,"%04d.txt",frame_id);
            tmp_txt_EFP = directories[6] + tmp_EFPName;

            if(!cap.read(frame)){//read one frame
                return 0;
            }
        }

        //===============Edge/Line segments detection================START
        cvtColor(frame,grayscaleFrame, CV_BGR2GRAY);//convert rgb image to gray image
        unsigned char *srcImg;
        srcImg = grayscaleFrame.data;
        EdgeMap *map = DetectEdgesByED(srcImg, frame.size().width, frame.size().height, SOBEL_OPERATOR, 25, 8, 1.0);//36 8 1.0//Edge Drawing for edge detection
        vector<vector<Point> > edgeFragments;
        edgeFragments = EdgeFilterBreak(map);//divide edge segments into fragments
        saveEFPixelsToTxt(edgeFragments, tmp_txt_EFP);//save edge fragments to txt file
        //---------------Edge/Line segments detection-----------------END


        //=========================Labeling==============================START
        //store edge and region map
        Mat edge_map_classes, edge_map_instances, region_map_classes, region_map_instances;

        edge_map_classes = Mat::zeros(frame_height,frame_width,CV_8UC3);
        region_map_classes = Mat::zeros(frame_height,frame_width,CV_8UC3);
        edge_map_instances = Mat::zeros(frame_height,frame_width,CV_8UC3);
        region_map_instances = Mat::zeros(frame_height,frame_width,CV_8UC3);

        //Mat bw_im_edge, bw_im_region;
        //bw_im_edge = Mat::zeros(frame_height,frame_width,CV_8UC1);
        //bw_im_region = Mat::zeros(frame_height,frame_width,CV_8UC1);

        namedWindow("Labeling", CV_WINDOW_NORMAL);
        //cvSetWindowProperty("Ground Truth Labeling", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
        //resizeWindow("Ground Truth Labeling", 1280,960);

        int label_key = 0;
        int EF_LS_flg = 0;//indicate labeling by selecting edge fragments "0" or drawing lines "1"
        int EF_show_flg = 1;//show edge fragments or not


        //vector<string> classNameList;//store class name of each classes
        vector<int> labeledShapeClassName;//number equals to the number of objects, each object may contain multiple boundaries

        vector<vector<vector<Point> > > labeledShapes;//number equals to the number of closed boundaries
        vector<int> labeledShapeIdx;//number equals to the number of closed boundaries

        //number equals to the number selected and drawn segments (EF corresponding to that of the detected EF's index, otherwise -1)
        vector<vector<int> > labeledShapeEFID;
        //number equals to the number selected or drawn segments(EF 1, LS 2, GAP 3)
        vector<vector<int> > labeledShapeEFtypes;


        vector<vector<Point> > labeledFragments;
        vector<int> labeledEFID;//indicates the ID of labeled edge fragment EF
        vector<int> labeledEFtype;//1 indicates EF, 2 means draw line segment, and 3 means gap

        vector<string> outputPathName;//label results saving path

        vector<vector<int> > rand_color_EFs = randColor(edgeFragments.size());//generate rand colors for edge fragments

        while(1){

            Mat show_label_tmp = frame.clone();

            if(EF_show_flg){//show edge fragments or not
                drawEFs(edgeFragments, show_label_tmp, rand_color_EFs);//draw edge fragments
            }

            if(labeledShapes.size()>0){//draw labeled shapes
                for(int i = 0; i < labeledShapes.size(); i++){
                    drawLFs(labeledShapes[i],labeledShapeEFtypes[i],show_label_tmp);
                }

            }

            //drawLFs(labeledFragments, labeledEFtype, show_label_tmp);
            setMouseCallback("Labeling", CallBackFunc, &pt_lbd_gt);//get mouse events

            int foundIdx = -1;
            int pixelIdx = -1;

            if(!mbd_gt){//--continue current shape labeling
                if(lbd_gt){//EVENT-mouse left button down
                    addLabelFragments(frame, EF_LS_flg, pt_lbd_gt, edgeFragments, labeledFragments, labeledEFID, labeledEFtype);
                    lbd_gt = 0;
                }else{//EVENT-mouse move
                    if(!EF_LS_flg){//show the closet EF when at EF selecting mode
                        //int foundIdx = -1;
                        //int pixelIdx = -1;
                        findEF(pt_mv_gt, edgeFragments,foundIdx,pixelIdx);
                        if(foundIdx>-1 && foundIdx<edgeFragments.size()){
                            drawEF(edgeFragments[foundIdx], show_label_tmp, 255,0,0);//r g b

                            if(pixelIdx > 0 && pixelIdx < edgeFragments[foundIdx].size()-2){
                                //draw current breaking two pixels
                                Vec3b & color1 = show_label_tmp.at<Vec3b>(edgeFragments[foundIdx][pixelIdx].y,edgeFragments[foundIdx][pixelIdx].x);
                                color1[0] = 0;
                                color1[1] = 255;
                                color1[2] = 0;

                                //draw current breaking two pixels
                                Vec3b & color2 = show_label_tmp.at<Vec3b>(edgeFragments[foundIdx][pixelIdx+1].y,edgeFragments[foundIdx][pixelIdx+1].x);
                                color2[0] = 255;
                                color2[1] = 0;
                                color2[2] = 0;
                            }
                        }
                    }else{//show the connectivity from mouse pointer to start and end points
                        if(labeledFragments.size() == 1){
                            line(show_label_tmp,pt_mv_gt,labeledFragments[0][0],Scalar(0,255,0),1,8,0);
                            line(show_label_tmp,pt_mv_gt,labeledFragments[labeledFragments.size()-1][labeledFragments[labeledFragments.size()-1].size()-1],Scalar(0,255,0),1,8,0);
                        }
                        else if(labeledFragments.size() > 1){
                            line(show_label_tmp,pt_mv_gt,labeledFragments[0][0],Scalar(0,255,0),1,8,0);
                            line(show_label_tmp,pt_mv_gt,labeledFragments[labeledFragments.size()-1][labeledFragments[labeledFragments.size()-1].size()-1],Scalar(0,0,255),1,8,0);
                        }
                    }
                }//if(lbd_gt)
            }
            else{//--finish current shape labeling(mouse middle button down )
                if(labeledFragments.size()>0 && labeledFragments[0].size()>1){
                    //get closed shape contour
                    finishCurrentLabel(frame,labeledFragments,labeledEFID, labeledEFtype);
                    labeledShapes.push_back(labeledFragments);
                    labeledShapeEFID.push_back(labeledEFID);
                    labeledShapeEFtypes.push_back(labeledEFtype);

                    //generate binary edge and region maps of labels
                    //drawLabelEdges(bw_im_edge, bw_im_region, labeledFragments);
                    //draw current labeling shape
                    Mat show_label_tmp_tmp = show_label_tmp.clone();
                    drawLFs(labeledFragments, labeledEFtype, show_label_tmp_tmp);
                    imshow("Labeling",show_label_tmp_tmp);
                    //imshow("labeling2",show_label_tmp);


                    //follow codes handle objects with holes
                    if(labeledShapeIdx.size() == 0){
                        labeledShapeIdx.push_back(0);
                    }
                    int YorN = 1;
                    if(0 == simple_shape){
                        YorN = inputNewShapeYN(mbd_gt);
                    }else if(1 == simple_shape){
                        YorN = 1;
                    }



                    if(1 == YorN){//this is the last boundary of the current object

                        //drawLFs(labeledFragments, labeledEFtype, show_label_tmp_tmp);
                        //imshow("Labeling",show_label_tmp_tmp);

                        labeledShapeIdx.push_back(labeledShapeIdx[labeledShapeIdx.size()-1]+1);
                        //generate binary edge and region maps of labels


                        //allow user to input multiple classes names

                        if(biMultiClass_flg){//multi-classes names
                            //inputClassName(labeledShapeClassName, mbd_gt);
                            string tmp_class_name;

                            inputClassName(tmp_class_name, mbd_gt);

                            int classNameExist = -1;
                            for(int i = 0; i < classNameList.size(); i++){
                                if(0 == classNameList[i].compare(tmp_class_name)){
                                    classNameExist = i;
                                    break;
                                }//if
                            }//for

                            if(-1 == classNameExist){
                                classNameList.push_back(tmp_class_name);
                                labeledShapeClassName.push_back(classNameList.size()-1);
                                //cout<<"class name: "<<labeledShapeClassName[labeledShapeClassName.size()-1]<<endl;//
                            }else if(classNameExist>-1){
                                labeledShapeClassName.push_back(classNameExist);
                            }

                        }else{//for bi-class labeling all labeled shape class name is "1"
                            //labeledShapeClassName.push_back("1");
                            //tmp_class_name = "1";
                            if(0 == classNameList.size()){
                                classNameList.push_back("1");
                            }
                            labeledShapeClassName.push_back(0);
                        }

                        drawLabelResults(biMultiClass_flg, color_pallet, edge_map_classes, edge_map_instances, region_map_classes, region_map_instances, labeledShapes, labeledShapeIdx, labeledShapeClassName);

                        //cout<<"YorN: "<<YorN<<endl;
                        //output all of the label results
                        drawLFs(labeledFragments, labeledEFtype, show_label_tmp);
                        //outputLabels(bw_im_edge, bw_im_region, show_label_tmp, labeledShapes, labeledShapeIdx, labeledShapeEFID, labeledShapeEFtypes, classNameList, labeledShapeClassName,
                                     //video_proc, frame_id, directories, namePatches, outputPathName);///////////////

                        //outputColorPallet(outputFolder,color_pallet, classNameList, labeledShapeClassName);

                        outputLabels(color_pallet, edge_map_classes, edge_map_instances, region_map_classes, region_map_instances, show_label_tmp, labeledShapes, labeledShapeIdx,
                                          labeledShapeEFID, labeledShapeEFtypes, classNameList, labeledShapeClassName,
                                          video_proc, frame_id, directories, namePatches, outputPathName);

                        cout<<"--Labeled Object ID: "<<labeledShapeClassName.size()-1<<endl;
                        //cout<<"--Object Class Name: "<<labeledShapeClassName[labeledShapeClassName.size()-1]<<endl;
                        cout<<"--Object Class Name: "<<classNameList[labeledShapeClassName[labeledShapeClassName.size()-1]]<<endl;
                        //cout<<"--Shape Segments NO.: "<<labeledShapes[labeledShapes.size()-1].size()<<endl;
                        cout<<"-----------------------------------------------"<<endl;

                    }else if(0 == YorN){//this is not the last boundary of the current object
                         labeledShapeIdx.push_back(labeledShapeIdx[labeledShapeIdx.size()-1]);
                    }else if(-1 == YorN){//undo the last selection

                        labeledShapes.pop_back();
                        labeledShapeEFID.pop_back();
                        labeledShapeEFtypes.pop_back();

                        if(labeledFragments.size()>0){
                           labeledFragments.pop_back();//undo the last selection
                           labeledEFID.pop_back();
                           labeledEFtype.pop_back();

                           labeledShapeIdx.pop_back();
                        }
                         mbd_gt = 0;
                         continue;
                    }

                    labeledFragments.clear();
                    vector<vector<Point> >().swap(labeledFragments);
                    labeledEFID.clear();
                    vector<int>().swap(labeledEFID);
                    labeledEFtype.clear();
                    vector<int>().swap(labeledEFtype);
                }

                mbd_gt = 0;
            }//if(!mbd_gt)

            //draw current labeling shape
            drawLFs(labeledFragments, labeledEFtype, show_label_tmp);

            //------------------------------------LABEL CONTROL---------------------------------------
            imshow("Labeling",show_label_tmp);
            label_key = cvWaitKey(1) & 255;

           if(97 == label_key){//"a"
                EF_LS_flg = !EF_LS_flg;
            }
            else if(98 == label_key){//"b" split the current closet edge fragment into two edge fragments
               if(foundIdx>-1 && foundIdx<edgeFragments.size() && pixelIdx > 0 && pixelIdx < edgeFragments[foundIdx].size()-2){
                    vector<Point> temp;

                    int szEFi = edgeFragments[foundIdx].size();

                    for(int i = szEFi-1; i > pixelIdx; i--){

                        temp.push_back(edgeFragments[foundIdx][edgeFragments[foundIdx].size()-1]);
                        edgeFragments[foundIdx].pop_back();
                    }

                    edgeFragments.push_back(temp);
                    temp.clear();
                    vector<Point>().swap(temp);

                    vector<int> temp_clr;

                    temp_clr.push_back(100+int(rand()%156));
                    temp_clr.push_back(100+int(rand()%156));
                    temp_clr.push_back(100+int(rand()%156));

                    rand_color_EFs.push_back(temp_clr);

                    temp_clr.clear();
                    vector<int>().swap(temp_clr);


               }//
            }
            else if(101 == label_key){//"e"
                EF_show_flg = !EF_show_flg;
            }else if(102 == label_key){//"f" undo the last selection
                if(labeledFragments.size()>0){
                   labeledFragments.pop_back();//undo the last selection
                   labeledEFID.pop_back();
                   labeledEFtype.pop_back();
                }
            }else if((10 == label_key || 13 == label_key)&& 0 == labeledFragments.size()){//"Enter" finish current frame labeling

               saveEFPixelsToTxt(edgeFragments, tmp_txt_EFP);//save edge fragments to txt file

               cout<<"==Labeled Boundaries Number: "<<labeledShapes.size()<<endl;
               cout<<"==Labeled Objects Number: "<<labeledShapeClassName.size()<<endl;

               labeledShapes.clear();
               vector<vector<vector<Point> > >().swap(labeledShapes);

               labeledShapeIdx.clear();
               vector<int>().swap(labeledShapeIdx);

               labeledShapeEFID.clear();
               vector<vector<int> >().swap(labeledShapeEFID);

               labeledShapeEFtypes.clear();
               vector<vector<int> >().swap(labeledShapeEFtypes);

               labeledShapeClassName.clear();
               vector<int>().swap(labeledShapeClassName);

               outputPathName.clear();
               vector<string>().swap(outputPathName);
                break;
            }else if(27  == label_key){//"Esc"
               return 0;
           }
        }//while
        //-------------------------------------Labeling---------------------------------END
        frame_id++;

        //frame =  drawESs(map, frame);
        //namedWindow("Display",1);
        //imshow("Display",frame);
        //cvWaitKey(0);

        key = cvWaitKey(1) & 255;
        if(key == 27){
            break;
        }
    }//for(;;)start read frames

    return 0;
}
//___________________________MAIN FUNCTION END___________________________
//___________________________MAIN FUNCTION END___________________________
//___________________________MAIN FUNCTION END___________________________

void createColorPallet(vector<vector<int> > &color_pallet){
    int low = 115;
    int high = 255;

    vector<int> Ri, Gi, Bi;

    for(int i = low; i < high; i=i+10){
        Ri.push_back(i);
        Gi.push_back(i);
        Bi.push_back(i);
    }

    random_shuffle (Ri.begin(), Ri.end());
    random_shuffle (Gi.begin(), Gi.end());
    random_shuffle (Bi.begin(), Bi.end());

    for(int ri = 0; ri < Ri.size(); ri++){
        for(int gi = 0; gi < Gi.size(); gi++){
            for(int bi = 0; bi < Bi.size(); bi++){
                vector<int> tmp;
                tmp.push_back(Ri[ri]);
                tmp.push_back(Gi[gi]);
                tmp.push_back(Bi[bi]);

                color_pallet.push_back(tmp);

                tmp.clear();
                vector<int>().swap(tmp);

            }//for ri
        }//for gi
    }//for bi

}


void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        //Point* pt = (Point*) userdata;
        //if(mbd_gt==0){
        pt_lbd_gt.x = x;
        pt_lbd_gt.y = y;
            //gtLabelPts.push_back(pt_lbd_gt);
        lbd_gt=1;
        //}
        //cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if  ( event == EVENT_RBUTTONDOWN )
    {
        //cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if  ( event == EVENT_MBUTTONDOWN )
    {
        mbd_gt = 1;//finish one object lableing
        //cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if ( event == EVENT_MOUSEMOVE )
    {
        //if(mbd_gt==0){
            pt_mv_gt.x = x;
            pt_mv_gt.y = y;
        //}
    }

    return;
}


bool isDirExist(const std::string& path)
{
#if defined(_WIN32)
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & _S_IFDIR) != 0;
#else
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
#endif
}

bool makePath(const std::string& path)
{
#if defined(_WIN32)
    int ret = _mkdir(path.c_str());
#else
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);
#endif
    if (ret == 0)
        return true;

    switch (errno)
    {
    case ENOENT:
        // parent didn't exist, try to create it
        {
            int pos = path.find_last_of('/');
            if (pos == std::string::npos)
#if defined(_WIN32)
                pos = path.find_last_of('\\');
            if (pos == std::string::npos)
#endif
                return false;
            if (!makePath( path.substr(0, pos) ))
                return false;
        }
        // now, try to create again
#if defined(_WIN32)
        return 0 == _mkdir(path.c_str());
#else
        return 0 == mkdir(path.c_str(), mode);
#endif

    case EEXIST:
        // done!
        return isDirExist(path);

    default:
        return false;
    }
}

float distance(float x1, float y1, float x2, float y2){
    return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}


//split by distance threshold
/*int EdgeBreakFit(float xyh[3][4]){

    float abcXe = 0;//end point test
    float abcXm = 0;//middle point test

    float a=0, b=0, c=0;
    a = xyh[1][0]-xyh[1][2];
    b = xyh[0][2]-xyh[0][0];
    c = xyh[0][0]*xyh[1][2]-xyh[1][0]*xyh[0][2];

    abcXe = a*xyh[0][3]+b*xyh[1][3]+c;
    abcXm = a*xyh[0][1]+b*xyh[1][1]+c;

    float erre = abs(abcXe)/sqrt(pow(a,2)+pow(b,2));
    float errm = abs(abcXm)/sqrt(pow(a,2)+pow(b,2));

    if(erre<DivideErrThre_e && errm<DivideErrThre_m){
        return 1;
    }else{
        return 0;
    }

    //return err;
}*/

//split by angle threshold
int EdgeBreakFit(float xyh[3][4]){

    float x1 = xyh[0][2]-xyh[0][0];
    float y1 = xyh[1][2]-xyh[1][0];

    float x2 = xyh[0][3]-xyh[0][2];
    float y2 = xyh[1][3]-xyh[1][2];

    float cp = x1*x2 + y1*y2;
    float np = sqrt(pow(x1,2)+pow(y1,2)) * sqrt(pow(x2,2)+pow(y2,2));

    float rad = acos(cp/np);
    float ang = rad*180/3.1415926535898;

    //if(ang>60){
    //    cout<<ang<<endl;
    //}

    if(ang < 35){
        return 1;
    }else{
        return 0;
    }

}

vector<vector<int> > randColor(int num_color){
    vector<vector<int> > rand_color;

    int lowest=100, highest=255;
    int range=(highest-lowest)+1;
    vector<int> color_tmp;

    for(int i = 0; i < num_color; i++){
       color_tmp.push_back(lowest+int(rand()%range));
       color_tmp.push_back(lowest+int(rand()%range));
       color_tmp.push_back(lowest+int(rand()%range));

       rand_color.push_back(color_tmp);
       color_tmp.clear();
       vector<int>().swap(color_tmp);
    }

    return rand_color;
}

Mat drawESs(EdgeMap *map, Mat frame_label_tmp){//draw edge segments

    int lowest=100, highest=255;
    int range=(highest-lowest)+1;
    int rc, gc, bc;//the color of edge fragments

    for(int i = 0; i < map->noSegments; i++){

        rc = lowest+int(rand()%range);
        gc = lowest+int(rand()%range);
        bc = lowest+int(rand()%range);

        for(int j = 0; j < map->segments[i].noPixels; j++){
            int r = map->segments[i].pixels[j].r;
            int c = map->segments[i].pixels[j].c;

            Vec3b & color = frame_label_tmp.at<Vec3b>(r,c);
            color[0] = bc;
            color[1] = gc;
            color[2] = rc;
        }
    }

    return frame_label_tmp;
}

vector<vector<Point> > EdgeFilterBreak(EdgeMap *map){

    vector<EdgeFragment> EFs;//divided Edge Fragments

    for(int i = 0; i < map->noSegments; i++){

        EdgeFragment EF_tmp;

        if(map->segments[i].noPixels<5){
            EF_tmp.seg_id = i;
            EF_tmp.start_idx = 0;
            EF_tmp.end_idx = map->segments[i].noPixels-1;
            EFs.push_back(EF_tmp);
        }else{

            int newEdge = 1;//start a new fragment searching
            int iEgStrt = 0;

            float PT[3][4];

            for (int iEgEnd = iEgStrt+4; iEgEnd < map->segments[i].noPixels;){

                if(newEdge){//start a new small edge segment searching

                    EF_tmp.seg_id = i;
                    EF_tmp.start_idx = iEgStrt;

                    PT[0][0] = map->segments[i].pixels[iEgStrt].r;
                    PT[1][0] = map->segments[i].pixels[iEgStrt].c;
                    PT[2][0] = 1;
                }else{//////new breaking rule
                    PT[0][0] = map->segments[i].pixels[iEgEnd-4].r;////new breaking rule
                    PT[1][0] = map->segments[i].pixels[iEgEnd-4].c;///new breaking rule
                    PT[2][0] = 1;
                }//////new breaking rule

                PT[0][1] = map->segments[i].pixels[(iEgEnd+iEgStrt)/2].r;
                PT[1][1] = map->segments[i].pixels[(iEgEnd+iEgStrt)/2].c;
                PT[2][1] = 1;

                PT[0][2] = map->segments[i].pixels[iEgEnd-2].r;
                PT[1][2] = map->segments[i].pixels[iEgEnd-2].c;
                PT[2][2] = 1;

                PT[0][3] = map->segments[i].pixels[iEgEnd].r;
                PT[1][3] = map->segments[i].pixels[iEgEnd].c;
                PT[2][3] = 1;

                if(EdgeBreakFit(PT)==1){//<DivideErrThre_e

                    if(iEgEnd < map->segments[i].noPixels - 2 ){
                        newEdge = 0;
                        iEgEnd +=2;
                    }else{
                        newEdge = 1;
                        EF_tmp.end_idx = map->segments[i].noPixels-1;
                        //if((EF_tmp.end_idx-EF_tmp.start_idx+1)>edgeLenThre){//EF_tmp.end_idx!=EF_tmp.start_idx
                            EFs.push_back(EF_tmp);// find the last small edge segment
                        //}
                        break;
                    }

                }else{
                    newEdge = 1;//start a new small edge
                    EF_tmp.end_idx = iEgEnd - 2;
                    //if((EF_tmp.end_idx-EF_tmp.start_idx+1)>edgeLenThre){//
                        EFs.push_back(EF_tmp);// find a new small edge segment
                    //}//

                    iEgStrt = iEgEnd-1;
                    EF_tmp.start_idx =  iEgStrt;
                    if(map->segments[i].noPixels-iEgStrt < 5){// there are less than 5 remaining pixels in this edge segment
                        EF_tmp.end_idx = map->segments[i].noPixels-1;
                        //if((EF_tmp.end_idx-EF_tmp.start_idx+1)>edgeLenThre){//EF_tmp.end_idx!=EF_tmp.start_idx
                            EFs.push_back(EF_tmp);
                        //}
                        break;
                    }else{// there are less than 3 remaining pixels in this edge segment and start a new small edge segment searching
                        iEgEnd = iEgStrt + 4;
                    }
                }//end if(LeastSquaresLineFit < DividedErrThre){} else{}

            }//for(int j)
        }//if(map...
    }//for(int i)

    vector<vector<Point> > points;
    for(int i = 0; i < EFs.size(); i++){
        int tmp_seg_id = EFs[i].seg_id;
        vector<Point> tmp_pts;

        for(int j = EFs[i].start_idx; j < EFs[i].end_idx + 1; j++){
            Point pt;
            pt.y = map->segments[tmp_seg_id].pixels[j].r;
            pt.x = map->segments[tmp_seg_id].pixels[j].c;
            tmp_pts.push_back(pt);

        }
        points.push_back(tmp_pts);
        tmp_pts.clear();
        vector<Point>().swap(tmp_pts);
    }

    return points;
}

int findEF(Point pt, vector<vector<Point> > edgeFragments,int &EF_id, int &Pixel_id){
    int indx_sel = 0;
    int pixel_sel = 0;
    float dist_sel = distance(float(pt.x),float(pt.y),
                              float(edgeFragments[0][0].x),
                              float(edgeFragments[0][0].y));
    for(int i = 0; i < edgeFragments.size(); i++){
        for(int j = 0; j < edgeFragments[i].size(); j++){
            float tmp_dist = distance(float(pt.x),float(pt.y),
                                      float(edgeFragments[i][j].x),
                                      float(edgeFragments[i][j].y));
            if(dist_sel > tmp_dist){
                indx_sel = i;
                pixel_sel = j;
                dist_sel = tmp_dist;
            }
        }//for j
    }//fori

    if(dist_sel<3){
        Pixel_id = pixel_sel;
        EF_id = indx_sel;
        return 1;
    }else{
        return 0;
    }
}

int drawEF(vector<Point> &edgeFragment, Mat &frame_label_tmp, int rc, int gc, int bc){//draw one edge fragment
    if(edgeFragment.size() > 0){

        for(int i = 0; i < edgeFragment.size(); i++){
            int r = edgeFragment[i].y;
            int c = edgeFragment[i].x;

            Vec3b & color = frame_label_tmp.at<Vec3b>(r,c);
            color[0] = bc;
            color[1] = gc;
            color[2] = rc;
        }
        return 1;
    }
    else{
        return 0;
    }
    //return frame_label_tmp;
}

int drawEFs(vector<vector<Point> > &edgeFragments, Mat &frame_label_tmp, vector<vector<int> > rand_color){
    if(edgeFragments.size()>0){

        for(int i = 0; i < edgeFragments.size(); i++){

            drawEF(edgeFragments[i], frame_label_tmp, rand_color[i][0], rand_color[i][1],rand_color[i][2]);

        }

        return 1;
    }
    else{
        return 0;
    }

}

int linesOrder(Point pt1, Point pt2, Point pt3, Point pt4){
//pt1---------------pt2  pt4---------------pt3
//to find the closest two endpoints which determines the storage order of each edge pixel
    float dists[4];

    dists[0] = distance(float(pt1.x),float(pt1.y),float(pt3.x),float(pt3.y));
    dists[1] = distance(float(pt1.x),float(pt1.y),float(pt4.x),float(pt4.y));
    dists[2] = distance(float(pt2.x),float(pt2.y),float(pt3.x),float(pt3.y));
    dists[3] = distance(float(pt2.x),float(pt2.y),float(pt4.x),float(pt4.y));

    int lorder = 0;
    float temp_dist = dists[0];
    for(int i = 1; i < 4; i++){
        if(dists[i] < temp_dist){
            lorder = i;
            temp_dist = dists[i];
        }
    }

    return lorder;//0, 1, 2, 3
}

int linesToPtOrder(Point pt0, Point pt1, Point pt2){

    float dists01 = distance(float(pt0.x),float(pt0.y),float(pt1.x),float(pt1.y));
    float dists02 = distance(float(pt0.x),float(pt0.y),float(pt2.x),float(pt2.y));

    if(dists01<dists02){
        return 1;
    }else{
        return 2;
    }

}

int pointsOnLine(Mat &frame, Point pt1, Point pt2, vector<Point> &tmp_points){

    LineIterator it(frame, pt1, pt2, 8);

    for(int i =0; i < it.count; i++,++it){
        tmp_points.push_back(it.pos());
    }

    return tmp_points.size();
}

int addLabelFragments(Mat &frame, int EF_LS_flg, Point pt_lbd_gt,vector<vector<Point> > edgeFragments,
                                        vector<vector<Point> > &labeledFragments, vector<int> &labeledEFID, vector<int> &labeledEFtype){
    vector<Point> tmp_add_pt;
    if(!EF_LS_flg){//add selected edgefragment
        int selEFIdx = -1;
        int selPixelIdx = -1;
        findEF(pt_lbd_gt,edgeFragments, selEFIdx, selPixelIdx);

        if(selEFIdx>-1 && selEFIdx < edgeFragments.size()){
            if(labeledFragments.size() == 0){//zero fragment
                labeledFragments.push_back(edgeFragments[selEFIdx]);//add new fragment
                labeledEFID.push_back(selEFIdx);
                labeledEFtype.push_back(1);//EF
            }
            else if(labeledFragments.size() == 1){//one fragment
                if(labeledFragments[0].size() == 1){//the first segments only have one pixel(it is added by clicking)
                    int ltopt = linesToPtOrder(labeledFragments[0][0],edgeFragments[selEFIdx][0], edgeFragments[selEFIdx][edgeFragments[selEFIdx].size()-1]);

                    if(ltopt==2){
                        reverse(edgeFragments[selEFIdx].begin(),edgeFragments[selEFIdx].end());
                    }


                    pointsOnLine(frame, labeledFragments[0][0], edgeFragments[selEFIdx][0], tmp_add_pt);

                    if(tmp_add_pt.size()>1){
                        labeledFragments.pop_back();
                        labeledEFID.pop_back();
                        labeledEFtype.pop_back();

                        tmp_add_pt.pop_back();
                        labeledFragments.push_back(tmp_add_pt);
                        tmp_add_pt.clear();
                        vector<Point>().swap(tmp_add_pt);
                        labeledEFID.push_back(-1);//LS no EF ID
                        labeledEFtype.push_back(2);//LS

                        labeledFragments.push_back(edgeFragments[selEFIdx]);
                        labeledEFID.push_back(selEFIdx);
                        labeledEFtype.push_back(1);//EF
                    }

                }
                else{

                    int lorder = linesOrder(labeledFragments[0][0], labeledFragments[0][labeledFragments[0].size()-1],
                            edgeFragments[selEFIdx][0], edgeFragments[selEFIdx][edgeFragments[selEFIdx].size()-1]);

                    switch(lorder){
                    case 0:{//pt1-pt3
                        reverse(labeledFragments[0].begin(),labeledFragments[0].end());
                        break;
                    }
                    case 1:{//pt1-pt4
                        reverse(labeledFragments[0].begin(),labeledFragments[0].end());
                        reverse(edgeFragments[selEFIdx].begin(),edgeFragments[selEFIdx].end());
                        break;
                    }
                    case 2:{//pt2-pt3
                        break;
                    }
                    case 3:{//pt2-pt4
                        reverse(edgeFragments[selEFIdx].begin(),edgeFragments[selEFIdx].end());
                        break;
                    }
                    }//switch

                    pointsOnLine(frame, labeledFragments[labeledFragments.size()-1][labeledFragments[labeledFragments.size()-1].size()-1],
                            edgeFragments[selEFIdx][0], tmp_add_pt);
                    if(tmp_add_pt.size()>2){
                        tmp_add_pt.pop_back();
                        tmp_add_pt.erase(tmp_add_pt.begin());
                        labeledFragments.push_back(tmp_add_pt);
                        tmp_add_pt.clear();
                        vector<Point>().swap(tmp_add_pt);
                        labeledEFID.push_back(-1);//GAP no EF ID
                        labeledEFtype.push_back(3);//GAP
                    }

                    labeledFragments.push_back(edgeFragments[selEFIdx]);//add new selected edge fragment into labeling shape
                    labeledEFID.push_back(selEFIdx);//EF ID
                    labeledEFtype.push_back(1);//EF
                }

            }
            else{//two and more fragments
                int ltopt = linesToPtOrder(labeledFragments[labeledFragments.size()-1][labeledFragments[labeledFragments.size()-1].size()-1],
                        edgeFragments[selEFIdx][0], edgeFragments[selEFIdx][edgeFragments[selEFIdx].size()-1]);
                if(ltopt==2){
                    reverse(edgeFragments[selEFIdx].begin(),edgeFragments[selEFIdx].end());
                }

                pointsOnLine(frame, labeledFragments[labeledFragments.size()-1][labeledFragments[labeledFragments.size()-1].size()-1],
                        edgeFragments[selEFIdx][0], tmp_add_pt);

                if(tmp_add_pt.size()>2){
                    tmp_add_pt.pop_back();
                    tmp_add_pt.erase(tmp_add_pt.begin());
                    labeledFragments.push_back(tmp_add_pt);
                    tmp_add_pt.clear();
                    vector<Point>().swap(tmp_add_pt);
                    labeledEFID.push_back(-1);//GAP no EF ID
                    labeledEFtype.push_back(3);//GAP
                }

                labeledFragments.push_back(edgeFragments[selEFIdx]);//
                labeledEFID.push_back(selEFIdx);//EF ID
                labeledEFtype.push_back(1);//EF

            }//if(labeledFragments.size()
        }//if(selEFIdx
    }//if(!EF_LS_flg)
    else{//add a new point
        //vector<Point> tmp_add_pt;
        if(labeledFragments.size() == 0){
            tmp_add_pt.push_back(pt_lbd_gt);

            labeledFragments.push_back(tmp_add_pt);
            tmp_add_pt.clear();
            vector<Point>().swap(tmp_add_pt);
            labeledEFID.push_back(-1);//LS no EF ID
            labeledEFtype.push_back(2);//LS
        }
        else if(labeledFragments.size() == 1){
            if(labeledFragments[0].size()==1){
                pointsOnLine(frame, labeledFragments[0][0], pt_lbd_gt, tmp_add_pt);
                labeledFragments.pop_back();
                labeledEFID.pop_back();
                labeledEFtype.pop_back();

                labeledFragments.push_back(tmp_add_pt);
                tmp_add_pt.clear();
                vector<Point>().swap(tmp_add_pt);
                labeledEFID.push_back(-1);//LS no EF ID
                labeledEFtype.push_back(2);//LS
            }
            else{
                int ltopt = linesToPtOrder(pt_lbd_gt,
                        labeledFragments[labeledFragments.size()-1][0],
                        labeledFragments[labeledFragments.size()-1][labeledFragments[labeledFragments.size()-1].size()-1]);
                if(1 == ltopt){
                    reverse(labeledFragments[labeledFragments.size()-1].begin(),labeledFragments[labeledFragments.size()-1].end());
                }

                pointsOnLine(frame, labeledFragments[0][labeledFragments[0].size()-1], pt_lbd_gt, tmp_add_pt);

                if(tmp_add_pt.size()>1){
                    tmp_add_pt.erase(tmp_add_pt.begin());
                    labeledFragments.push_back(tmp_add_pt);
                    tmp_add_pt.clear();
                    vector<Point>().swap(tmp_add_pt);
                    labeledEFID.push_back(-1);//LS no EF ID
                    labeledEFtype.push_back(2);//LS
                }

            }

        }
        else{
            pointsOnLine(frame, labeledFragments[labeledFragments.size()-1][labeledFragments[labeledFragments.size()-1].size()-1], pt_lbd_gt, tmp_add_pt);


            if(tmp_add_pt.size()>1){

                tmp_add_pt.erase(tmp_add_pt.begin());
                labeledFragments.push_back(tmp_add_pt);
                tmp_add_pt.clear();
                vector<Point>().swap(tmp_add_pt);
                labeledEFID.push_back(-1);//LS no EF ID
                labeledEFtype.push_back(2);//LS
            }

        }
    }

    return labeledFragments.size();
}

int finishCurrentLabel(Mat &frame,vector<vector<Point> > &labeledFragments, vector<int> &labeledEFID, vector<int> &labeledEFtype){

    if(labeledFragments.size()>0 && labeledFragments[0].size()>1){
        vector<Point> tmp_gap_close;
        pointsOnLine(frame, labeledFragments[labeledFragments.size()-1][labeledFragments[labeledFragments.size()-1].size()-1], labeledFragments[0][0],tmp_gap_close);

        if(tmp_gap_close.size()>2){
            tmp_gap_close.pop_back();
            tmp_gap_close.erase(tmp_gap_close.begin());
            labeledFragments.push_back(tmp_gap_close);
            tmp_gap_close.clear();
            vector<Point>().swap(tmp_gap_close);
            labeledEFID.push_back(-1);//GAP no EF ID
            labeledEFtype.push_back(3);
        }

        return labeledFragments.size();
    }
    else{
        return 0;
    }

}


int drawLFs(vector<vector<Point> > &labeledFragments, vector<int> &labeledEFtype, Mat &show_label_tmp){

    if(labeledFragments.size()>0){

        for(int i = 0; i < labeledFragments.size(); i++){

            for(int j = 0; j < labeledFragments[i].size(); j++){
                int r = labeledFragments[i][j].y;
                int c = labeledFragments[i][j].x;

                Vec3b & color = show_label_tmp.at<Vec3b>(r,c);
                //int pixelColor = labeledEFtype[i];
                switch(labeledEFtype[i]){
                case 1:{
                    color[0] = 0;
                    color[1] = 0;
                    color[2] = 255;
                    break;
                }
                case 2:{
                    color[0] = 255;
                    color[1] = 0;
                    color[2] = 255;
                    break;
                }
                case 3:{
                    color[0] = 0;
                    color[1] = 255;
                    color[2] = 0;
                    break;
                }
                }//switch

            }
        }
        return 1;
    }
    else{
        return 0;
    }
}

/*
int drawLabelEdges(Mat &bw_im_edge, Mat &bw_im_region, vector<vector<Point> > &labeledFragments){

    if(labeledFragments.size() > 0 && labeledFragments[0].size() > 1){
        vector<Point> points;
        for(int i = 0; i < labeledFragments.size(); i++){
            for(int j = 0; j < labeledFragments[i].size(); j++){
                points.push_back(labeledFragments[i][j]);
            }//for j
        }//for i

        return 1;
        if(!points.empty()){
            const Point* ppt[1] = {&points[0]};
            int npt[] = {points.size()};
            polylines(bw_im_edge, ppt, npt, 1, 1, Scalar(255),1,8,0);
            fillPoly(bw_im_region, ppt, npt, 1, Scalar(255));

            points.clear();
            vector<Point>().swap(points);

            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
}*/

void drawLabelResults(int biMultiClass_flg, vector<vector<int> > &color_pallet, Mat &edge_map_classes, Mat &edge_map_instances, Mat &region_map_classes,
                      Mat &region_map_instances, vector<vector<vector<Point> > > &labeledShapes, vector<int> &labeledShapeIdx, vector<int> &labeledShapeClassName){

    if(labeledShapeIdx.size()>0){

        vector<vector<Point> > shapePoints;
        vector<Point> tmp_shp_points;
        int shapNum = 0;

        for(int j = 0; j < labeledShapes[labeledShapeIdx.size()-2].size(); j++){
            for(int t = 0; t < labeledShapes[labeledShapeIdx.size()-2][j].size(); t++){
                tmp_shp_points.push_back(labeledShapes[labeledShapeIdx.size()-2][j][t]);
            }//for(int t
        }//for(int j
        shapNum += 1;
        shapePoints.push_back(tmp_shp_points);
        tmp_shp_points.clear();
        vector<Point>().swap(tmp_shp_points);

        int tmp_idx = labeledShapeIdx[labeledShapeIdx.size()-2];
        for(int i = labeledShapeIdx.size()-3; i > -1; i--){
            if(tmp_idx == labeledShapeIdx[i]){

                for(int j = 0; j < labeledShapes[i].size(); j++){
                    for(int t = 0; t < labeledShapes[i][j].size(); t++){
                        tmp_shp_points.push_back(labeledShapes[i][j][t]);
                    }//for(int t

                }//for(int j

                shapNum += 1;
                shapePoints.push_back(tmp_shp_points);
                tmp_shp_points.clear();
                vector<Point>().swap(tmp_shp_points);
            }
            else{
                break;
            }

        }//for(int i

        if(!shapePoints.empty()){
            int shp_num = shapePoints.size();
            const Point** pts = new const Point*[shp_num]();
            for(int i = 0; i < shp_num; i++){
                pts[i] = new Point[shapePoints[i].size()];
                pts[i] = &shapePoints[i][0];
            }

            int *npt = new int[shp_num];
            for(int i = 0; i < shp_num; i++){
            //    *pt[i] = &vec_pts[i][0];
                npt[i] = shapePoints[i].size();
            }


            polylines(edge_map_instances,pts,npt,shp_num,1,Scalar(color_pallet[labeledShapes.size()-1][0],color_pallet[labeledShapes.size()-1][1],color_pallet[labeledShapes.size()-1][2]),1,8,0);
            fillPoly(region_map_instances, pts, npt, shp_num, Scalar(color_pallet[labeledShapes.size()-1][0],color_pallet[labeledShapes.size()-1][1],color_pallet[labeledShapes.size()-1][2]), 8);

            if(biMultiClass_flg){//multi-classes

                int tmp_r = color_pallet[labeledShapeClassName[labeledShapeClassName.size()-1]][0];
                int tmp_g = color_pallet[labeledShapeClassName[labeledShapeClassName.size()-1]][1];
                int tmp_b = color_pallet[labeledShapeClassName[labeledShapeClassName.size()-1]][2];

                polylines( edge_map_classes, pts, npt, shp_num, 1, Scalar(tmp_r,tmp_g,tmp_b), 1, 8, 0);
                fillPoly( region_map_classes, pts, npt, shp_num, Scalar(tmp_r,tmp_g,tmp_b), 8);

            }
            else{//bi-classes
                polylines( edge_map_classes, pts, npt, shp_num, 1, Scalar(255,255,255), 1, 8, 0);
                fillPoly( region_map_classes, pts, npt, shp_num, Scalar(255,255,255), 8);
            }

            //polylines( bw_im_edge, pts, npt, shp_num, 1, Scalar(250), 1, 8, 0) ;
            //fillPoly( bw_im_region, pts, npt, shp_num, Scalar(250), 8);
        }

    }//if(labeledShapeIdx.size()>0)
}

int inputNewShapeYN(int mbd_gt){

    int input_YN = 1;
    int txt_key = 255;
    namedWindow("New_Shape(y/n)",CV_WINDOW_AUTOSIZE);//CV_WINDOW_NORMAL
    Mat inputBoxImg = imread("./icon/start_new_shape_yn.png",1);

    string tmp_str = "";
    while(mbd_gt){

        Mat txt_im = inputBoxImg.clone();

        putText(txt_im, tmp_str, Point(620,80), CV_FONT_HERSHEY_COMPLEX, 3,
                Scalar(255,0,0), 3, 8, false);
        imshow("New_Shape(y/n)",txt_im);
        txt_key = cvWaitKey(1) & 255;

        if(89==txt_key || 121==txt_key){
            input_YN = 1;
            tmp_str = "y";
            //putText(txt_im, tmp_str, Point(620,80), CV_FONT_HERSHEY_COMPLEX, 3,
            //        Scalar(255,0,0), 3, 8, false);

            //imshow("New_Shape(y/n)",txt_im);
            //cvWaitKey(500);
            //destroyWindow("New_Shape(y/n)");
            //return input_YN;
        }else if(78 == txt_key || 110 == txt_key){
            input_YN = 0;
            tmp_str = "n";
            //putText(txt_im, tmp_str, Point(620,80), CV_FONT_HERSHEY_COMPLEX, 3,
            //        Scalar(255,0,0), 3, 8, false);

            //imshow("New_Shape(y/n)",txt_im);
            //cvWaitKey(500);
            //destroyWindow("New_Shape(y/n)");
            //return input_YN;
        }else if(8 == txt_key){//backspace---------------------------
            input_YN = 1;
            tmp_str = "";
            //putText(txt_im, tmp_str, Point(620,80), CV_FONT_HERSHEY_COMPLEX, 3,
            //        Scalar(255,0,0), 3, 8, false);

            //imshow("New_Shape(y/n)",txt_im);
            //cvWaitKey(500);
            //destroyWindow("New_Shape(y/n)");
            //return input_YN;
        }else if(102 == txt_key){//"f" undo the last selection

            destroyWindow("New_Shape(y/n)");
            return -1;
        }

        if(10 == txt_key || 13 == txt_key){//enter--------------------------------

            //cout<<"txt_key: "<<txt_key<<endl;
            //cout<<"input_YN: "<<input_YN<<endl;
            destroyWindow("New_Shape(y/n)");
            return input_YN;
        }


    }
    return 1;
}


template<typename Out>
void split(const string &s, char delim, Out result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        *(result++) = item;
    }
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

void outputLabels(vector<vector<int> > &color_pallet, Mat &edge_map_classes, Mat &edge_map_instances, Mat &region_map_classes, Mat &region_map_instances, Mat &show_label_tmp, vector<vector<vector<Point> > > &labeledShapes, vector<int> &labeledShapeIdx,
                  vector<vector<int> > &labeledShapeEFID, vector<vector<int> > &labeledShapeEFtypes, vector<string> &classNameList, vector<int> &labeledShapeClassName,
                  int video_proc, int frame_id, vector<string> &directories,vector<string> &namePatches, vector<string> &outputPathName){

    if(outputPathName.size() == 0){//generate saving path
        if(1 == video_proc){
            //outputPathName.push_back(directories[0]+namePatches[namePatches.size()-1]);
            //outputPathName.push_back(directories[1]+namePatches[namePatches.size()-1]);
            //outputPathName.push_back(directories[2]+namePatches[namePatches.size()-1]);

            vector<string> tmp_name_ext;
            tmp_name_ext = split(namePatches[namePatches.size()-1],'.');
            outputPathName.push_back(directories[0]+tmp_name_ext[0] + ".png");//edge map classes
            outputPathName.push_back(directories[1]+tmp_name_ext[0] + ".png");//edge map instances
            outputPathName.push_back(directories[2]+tmp_name_ext[0] + ".png");//region map classes
            outputPathName.push_back(directories[3]+tmp_name_ext[0] + ".png");//region map instances
            outputPathName.push_back(directories[4]+tmp_name_ext[0] + ".png");//color overlap
            outputPathName.push_back(directories[5]+tmp_name_ext[0] + ".txt");//pixels of boundaries

            //color correspondences
            outputPathName.push_back(directories[0]+"classesColor.txt");//edge map classes
            outputPathName.push_back(directories[1]+tmp_name_ext[0] + ".txt");//edge map instances
            outputPathName.push_back(directories[2]+"classesColor.txt");//region map classes
            outputPathName.push_back(directories[3]+tmp_name_ext[0] + ".txt");//region map instances
            //cout<<"edge map classes: "<<outputPathName[5]<<endl;
        }
        else if(2 == video_proc){

            char tmp_name[20];
            sprintf(tmp_name,"%04d.png",frame_id);
            char tmp_txt[20];
            sprintf(tmp_txt,"%04d.txt",frame_id);

            outputPathName.push_back(directories[0]+tmp_name);//edge map classes
            outputPathName.push_back(directories[1]+tmp_name);//edge map instances
            outputPathName.push_back(directories[2]+tmp_name);//region map classes
            outputPathName.push_back(directories[3]+tmp_name);//region map instances
            outputPathName.push_back(directories[4]+tmp_name);//color overlap
            outputPathName.push_back(directories[5]+tmp_txt);//pixels of boundaries

            //color correspondences
            outputPathName.push_back(directories[0]+"classesColor.txt");//edge map classes
            outputPathName.push_back(directories[1]+tmp_txt);//edge map instances
            outputPathName.push_back(directories[2]+"classesColor.txt");//region map classes
            outputPathName.push_back(directories[3]+tmp_txt);//region map instances
        }

        remove(outputPathName[5].c_str());//remove existing txt file which has the same name with target txt file

    }//if(outputPathName

    //output labels to corresponding files
    if(outputPathName.size() == 10){

        imwrite(outputPathName[0], edge_map_classes);
        imwrite(outputPathName[1], edge_map_instances);
        imwrite(outputPathName[2], region_map_classes);
        imwrite(outputPathName[3], region_map_instances);
        imwrite(outputPathName[4], show_label_tmp);

        //cout<<"last "<<outputPathName[3]<<endl;
        saveEdgePixelsToTxt(labeledShapes, labeledShapeIdx, labeledShapeEFID, labeledShapeEFtypes, classNameList, labeledShapeClassName, outputPathName[5]);

        const char* filename6 = outputPathName[6].c_str();
        FILE *fpt6 = fopen(filename6, "w");
        fprintf(fpt6,"%d\n",classNameList.size());
        for(int i = 0; i < classNameList.size(); i++){
            fprintf(fpt6, "%s %d %d %d ", classNameList[i].c_str(),color_pallet[i][0],color_pallet[i][1],color_pallet[i][2]);
            fprintf(fpt6, "\n");
        }
        fclose(fpt6);

        const char* filename7 = outputPathName[7].c_str();
        FILE *fpt7 = fopen(filename7, "w");
        fprintf(fpt7,"%d\n",labeledShapeClassName.size());
        for(int i = 0; i < labeledShapeClassName.size(); i++){
            fprintf(fpt7, "%s %d %d %d ", classNameList[labeledShapeClassName[i]].c_str(),color_pallet[i][0],color_pallet[i][1],color_pallet[i][2]);
            fprintf(fpt7, "\n");
        }
        fclose(fpt7);

        const char* filename8 = outputPathName[8].c_str();
        FILE *fpt8 = fopen(filename8, "w");
        fprintf(fpt8,"%d\n",classNameList.size());
        for(int i = 0; i < classNameList.size(); i++){
            fprintf(fpt8, "%s %d %d %d ", classNameList[i].c_str(),color_pallet[i][0],color_pallet[i][1],color_pallet[i][2]);
            fprintf(fpt8, "\n");
        }
        fclose(fpt8);

        const char* filename9 = outputPathName[9].c_str();
        FILE *fpt9 = fopen(filename9, "w");
        fprintf(fpt9,"%d\n",labeledShapeClassName.size());
        for(int i = 0; i < labeledShapeClassName.size(); i++){
            fprintf(fpt9, "%s %d %d %d ", classNameList[labeledShapeClassName[i]].c_str(),color_pallet[i][0],color_pallet[i][1],color_pallet[i][2]);
            fprintf(fpt9, "\n");
        }
        fclose(fpt9);
    }

}

void saveEdgePixelsToTxt(vector<vector<vector<Point> > > &labeledShapes, vector<int> &labeledShapeIdx, vector<vector<int> > &labeledShapeEFID,
                         vector<vector<int> > &labeledShapeEFtypes, vector<string> &classNameList, vector<int> &labeledShapeClassName, string &txtName){

    //write boundaries of the current shape to txt file
    vector<int> tmp_pos;
    int tmp_idx = labeledShapeIdx[labeledShapeIdx.size()-2];
    tmp_pos.push_back(labeledShapeIdx.size()-2);

    for(int i = labeledShapeIdx.size()-3; i > -1; i--){
        if(tmp_idx == labeledShapeIdx[i]){
            tmp_pos.push_back(i);
        }else{
            break;
        }
    }//end for

    reverse(tmp_pos.begin(),tmp_pos.end());

    const char* filename = txtName.c_str();
    FILE *fpt = fopen(filename, "a");

    //fprintf(fpt,"%d %s %d %d ",tmp_idx, tmp_pos.size());//shape index, boundaries number
    for(int i = 0; i < tmp_pos.size(); i++){

        //fprintf(fpt,"%d %s %d %d ",tmp_idx, labeledShapeClassName[tmp_idx].c_str(), tmp_pos.size(), tmp_pos[i]);//shape index, class name, boundaries number

        //fprintf(fpt, "## %d %d %d",tmp_pos[i], tmp_idx, labeledShapes[tmp_pos[i]].size());//boundary index, shape index, EF number of current boundary
        //fprintf(fpt,"%s", labeledShapeClassName[tmp_pos[i]].c_str());
        //fprintf(fpt, "\n");

        for(int j = 0; j < labeledShapes[tmp_pos[i]].size(); j++){
            //cout<<"name ID: "<<classNameList[labeledShapeClassName[tmp_idx]]<<endl;
            fprintf(fpt,"# %d %s %d %d ",tmp_idx, classNameList[labeledShapeClassName[tmp_idx]].c_str(), tmp_pos.size(), tmp_pos[i]);//shape index, class name, boundaries number
            fprintf(fpt, "%d %d %d ", labeledShapes[tmp_pos[i]].size(), labeledShapeEFID[tmp_pos[i]][j], labeledShapeEFtypes[tmp_pos[i]][j]);
            for(int t = 0; t < labeledShapes[tmp_pos[i]][j].size(); t++){
                fprintf(fpt, "%d %d ", labeledShapes[tmp_pos[i]][j][t].x, labeledShapes[tmp_pos[i]][j][t].y);
            }//for(int t
            fprintf(fpt, "\n");
        }//for(int j

    }//for(int i
    fclose(fpt);


    /*fprintf(fpt, "# %d %d ", labeledShapes.size()-1,labeledShapes[labeledShapes.size()-1].size());
    fprintf(fpt, "%s", labeledShapeClassName[labeledShapeClassName.size()-1].c_str());
    fprintf(fpt, "\n");
    for(int i = 0; i < labeledShapes[labeledShapes.size()-1].size(); i++){
        fprintf(fpt, "%d %d %d ", i, labeledShapeEFID[labeledShapeEFID.size()-1][i], labeledShapeEFtypes[labeledShapeEFtypes.size()-1][i]);
        for(int j = 0; j < labeledShapes[labeledShapes.size()-1][i].size(); j++){
            fprintf(fpt, "%d %d ", labeledShapes[labeledShapes.size()-1][i][j].x,labeledShapes[labeledShapes.size()-1][i][j].y);
        }
        fprintf(fpt, "\n");
    }
    fclose(fpt);*/
}

void saveEFPixelsToTxt(vector<vector<Point> > &edgeFragments, string &EFPixelsPathName){//saving detected Edge Fragments

    remove(EFPixelsPathName.c_str());//remove the existing txt file which has the same name of the target file

    const char* filename = EFPixelsPathName.c_str();
    FILE *fpt = fopen(filename, "a");
    fprintf(fpt,"%d",edgeFragments.size());//total Edge Fragments number
    fprintf(fpt,"\n");
    for(int i = 0; i < edgeFragments.size(); i++){
        fprintf(fpt,"%d %d ",i,edgeFragments[i].size());//output EF's ID and pixels number
        for(int j = 0; j < edgeFragments[i].size(); j++){
            fprintf(fpt,"%d %d ",edgeFragments[i][j].x,edgeFragments[i][j].y);
        }
        fprintf(fpt, "\n");
    }
    fclose(fpt);
}

int inputClassName(string &tmp_class_name, int mbd_gt){

    int txt_label_key=255;
    vector<char> iptClasName;
    string iptClasNameStr;
    namedWindow("Class_Name_Input",CV_WINDOW_AUTOSIZE);//CV_WINDOW_NORMAL
    Mat inputBoxImg = imread("./icon/input_text_box.png",1);

    while(mbd_gt){

        //cout<<"inputting name..."<<endl;
        Mat txt_im = inputBoxImg.clone();

        if(iptClasNameStr.length()>0){
            putText(txt_im, iptClasNameStr, Point(265,38), CV_FONT_HERSHEY_COMPLEX, 1,
                    Scalar(255,0,0), 1, 8, false);
        }

        //imshow("Class_Name_Input",txt_im);
        //txt_label_key = cvWaitKey(1) & 255;

        //only 0~9, a~z, A~Z and _ are acceptable for naming classes
        if((47 < txt_label_key && txt_label_key < 58) ||(64 < txt_label_key && txt_label_key < 91)||(96 < txt_label_key && txt_label_key < 123)||(95 == txt_label_key)){

            iptClasName.push_back(char(txt_label_key));
            iptClasNameStr="";
            iptClasNameStr.insert(iptClasNameStr.begin(),iptClasName.begin(),iptClasName.end());
            //cout<<"input: "<<iptClasNameStr<<endl;
        }
        else if(8 == txt_label_key){//backspace
            if(iptClasName.size()>0){
                iptClasName.pop_back();
                iptClasNameStr="";
                iptClasNameStr.insert(iptClasNameStr.begin(),iptClasName.begin(),iptClasName.end());
                //cout<<"input: "<<iptClasNameStr<<endl;
            }
        }
        else if(10 == txt_label_key || 13 == txt_label_key){//enter
            if(iptClasName.size()>0){
                iptClasNameStr="";
                iptClasNameStr.insert(iptClasNameStr.begin(),iptClasName.begin(),iptClasName.end());
                //labeledShapeClassName.push_back(iptClasNameStr);
                //cout<<"Class Name/ID: "<<labeledShapeClassName[labeledShapeClassName.size()-1]<<endl;
                tmp_class_name = iptClasNameStr;
                cout<<"Class Name/ID: "<<tmp_class_name<<endl;
            }
            else{
                //labeledShapeClassName.push_back("1");
                //cout<<"Class Name/ID: "<<labeledShapeClassName[labeledShapeClassName.size()-1]<<endl;
                tmp_class_name = "1";
                cout<<"Class Name/ID: "<<tmp_class_name<<endl;
            }

            iptClasName.clear();
            vector<char>().swap(iptClasName);
            destroyWindow("Class_Name_Input");
            break;
        }
        else if(27 == txt_label_key){
            return 0;
        }

        imshow("Class_Name_Input",txt_im);
        txt_label_key = cvWaitKey(1) & 255;

    }//while
    return iptClasNameStr.size();
}
