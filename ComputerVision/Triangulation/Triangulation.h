#include <cv.h>

using namespace std;
using namespace cv;

Vector<Matx14d> triangulate
  ( const Vector<Matx13d>& points1
  , const Vector<Matx13d>& points2
  , const Matx33d& fundamentalMat
  , const Matx34d& camera1 
  , const Matx34d& camera2);

Vector<Matx14d> triangulateFromFundamentalMat
  ( const Vector<Matx13d>& points1
  , const Vector<Matx13d>& points2
  , const Matx33d& fundamentalMat);


// TODO: check if necessary
//Vector<Matx14d> triangulateFromCameras
  //( const Vector<Matx13d>& points1
  //, const Vector<Matx13d>& points2
  //, const Matx34d& camera1
  //, const Matx34d& camera2);
