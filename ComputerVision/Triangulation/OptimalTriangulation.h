#include <cv.h>

using namespace cv;

Matx14d optimalTriangulation_
  ( const Point2d& point1
  , const Point2d& point2
  , const Matx33d& fundamentalMat
  , const Matx34d& camera1
  , const Matx34d& camera2
  );
