#include <cv.h>

using namespace cv;

namespace ls
{
  Matx14d triangulate
    ( const Matx13d& u
    , const Matx13d& v
    , const Matx34d& p
    , const Matx34d& q);
}
