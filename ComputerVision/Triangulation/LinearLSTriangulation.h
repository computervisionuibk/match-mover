#include <cv.h>

using namespace cv;

Matx14d linearLSTriangulation_
  ( const Point2d& u
  , const Point2d& v
  , const Matx34d& p
  , const Matx34d& q
  );
