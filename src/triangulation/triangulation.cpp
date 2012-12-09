/*
 * Triangulation (S.12):
 * 
 * Compute world points X given x,P, x',P'. Due to errors x=PX, x'P'X or
 * equally x'^TFx=0 is not satisfied. We solve following esimation problem:
 * \hat{x}=P\hat{X} \hat{x}'=P\hat{X}'. i.e. maximum likelihood under Gaussian
 * noise by \hat{x} that minimizes the projection error.
 *
 * Method: The optimal triangulation method. (Alg.:12.1 MVG)
 *
 * Vectors are treated as row vectors.
 * 
 */

#include <iostream>

#include <cv.h>
#include <math.h>

using namespace cv;

// triangulation via single point
Mat triangulate_dlt(const Mat& point1, const Mat& camera1, 
                    const Mat& point2, const Mat& camera2);
// compose A for a single point(12.2)
Mat composeA(const Mat& point1, const Mat& camera1, 
             const Mat& point2, const Mat& camera2);


Mat triangulate_dlt(const Mat& u, const Mat& p, 
                    const Mat& v, const Mat& q)
{
  Mat m = composeA(u, v, p, q);
  Mat dst = Mat(4, 1, DataType<float>::type);
  SVD::solveZ(m.t(),dst);
  return dst.t();
}

Mat composeA(const Mat& u, const Mat& p, 
             const Mat& v, const Mat& q)
{
  const Mat p1=p.row(1), p2=p.row(2), p3=p.row(3);
  const Mat q1=q.row(1), q2=q.row(2), q3=q.row(3);

  const Mat ux=u.col(1), uy=u.col(2);
  const Mat vx=v.col(1), vy=v.col(2); 

  Mat bigA = Mat(1, 4, DataType<float>::type);
  bigA.col(0) = ux*p3-p1;
  bigA.col(1) = uy*p3-p2;
  bigA.col(2) = vx*q3-q1;
  bigA.col(3) = vy*q3-q2;

  // TODO: normalize A(i) = A(i)/norm(A(i));

  return bigA;
}

void triangulation(const Mat &u, const Mat& v, const Mat& f)
{
  const Mat ux=u.col(1), uy=u.col(2);
  const Mat vx=v.col(1), vy=v.col(2); 
  
  //(i) transformation matrices t,s
  Mat t=(Mat_<float>(3,3) << 1,0,-ux,0,1,-uy,0,0,1);
  Mat s=(Mat_<float>(3,3) << 1,0,-vx,0,1,-vy,0,0,1);
  //(ii) apply translations
  Mat tft = s.t()*f*t.inv(); 
  //(iii) compute (and normalize) epipoles
  Mat e1 = Mat(3, 1, DataType<float>::type);
  SVD::solveZ(tft,e1);
  Mat e2 = Mat(3, 1, DataType<float>::type);
  SVD::solveZ(tft.t(),e2);
  //(iv)
  //... 
}
