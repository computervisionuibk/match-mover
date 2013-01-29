#include <cv.h>

using namespace cv;

namespace op
{
  Matx14d triangulate
    ( const Matx13d& point1
    , const Matx13d& point2
    , const Matx33d& fundamentalMat
    , const Matx34d& camera1
    , const Matx34d& camera2);
}
