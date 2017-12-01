#ifndef _EDGE_MAP_H_
#define _EDGE_MAP_H_

#include <memory.h>

enum GradientOperator {PREWITT_OPERATOR=101, SOBEL_OPERATOR=102, SCHARR_OPERATOR=103};

struct Pixel {int r, c;};

struct EdgeSegment {
  Pixel *pixels;       // Pointer to the pixels array
  int noPixels;        // # of pixels in the edge map
};

struct EdgeMap {
public:
  int width, height;        // Width & height of the image
  unsigned char *edgeImg;   // BW edge map


  Pixel *pixels;            // Edge map in edge segment form
  EdgeSegment *segments;     
  int noSegments;
      
public:
  // constructor
  EdgeMap(int w, int h){
    width = w;
    height = h;

    edgeImg = new unsigned char[width*height];

    pixels = new Pixel[width*height];
    segments = new EdgeSegment[width*height];
    noSegments = 0;
  } //end-EdgeMap

/*  EdgeMap operator=(EdgeMap& edgemapTmp){
      //width = edgemapTmp->width;
      //height = edgemapTmp->height;

      //edgeImg = new unsigned char[edgemapTmp->width*edgemapTmp->height];
      *(edgeImg) = *(edgemapTmp.edgeImg);

      //pixels = new Pixel[edgemapTmp->width*edgemapTmp->height];
      *(pixels) = *(edgemapTmp.pixels);

      //segments = new EdgeSegment[edgemapTmp->width*edgemapTmp->height];
      *(segments) = *(edgemapTmp.segments);

      noSegments = edgemapTmp.noSegments;
      return *this;
 }
*/
  // Destructor
  ~EdgeMap(){
    delete edgeImg;
    delete pixels;
    delete segments;
  } //end-~EdgeMap


  void ConvertEdgeSegments2EdgeImg(){
    memset(edgeImg, 0, width*height);

    for (int i=0; i<noSegments; i++){
      for (int j=0; j<segments[i].noPixels; j++){
        int r = segments[i].pixels[j].r;
        int c = segments[i].pixels[j].c;

        edgeImg[r*width+c] = 255;
      } //end-for
    } //end-for
  } //end-ConvertEdgeSegments2EdgeImg
};


#endif
