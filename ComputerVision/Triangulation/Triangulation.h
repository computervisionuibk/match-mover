#include <cv.h>

using namespace std;
using namespace cv;

vector<Matx14d> linearLSTriangulation
  ( const vector<Point2d>& points1
  , const vector<Point2d>& points2
  , const Matx34d& camera1 
  , const Matx34d& camera2);

vector<Matx14d> optimalTriangulation
  ( const vector<Point2d>& points1
  , const vector<Point2d>& points2
  , const Matx33d& fundamentalMat
  , const Matx34d& camera1 
  , const Matx34d& camera2);



