/*
 * File:   OptimizedTriangulation.cpp
 * Author: Michael Schaper
 *
 * This file provides an implementation of "The optimal triangulation method"
 * (see Hartley and Zisserman, Multiple View Geometry, Algorithm 12.1).
 *
 * Construct World Points:
 * Given image points correspondences and the fundamental matrix (or the
 * cameras, respectively), the corresponding world points are generated. An
 * optimization of the correspondences is performed in advance.
 *
 * Optimize Correspondences:
 * Given a measured point correspondence x <-> x' and a fundamental matrix F,
 * compute the corrected correspondences ~x <-> ~x' that minimize the
 * geometric error subject to the epipolar constraint ~x'^TF~x = 0.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "Triangulation.h"

void mkCamerasFromFundamentalMat
  ( const Matx33d& fundamentalMat
  , Matx34d& camera1
  , Matx34d& camera2);

Matx14d mkWorldPoint 
  ( const Matx13d& point1
  , const Matx13d& point2 
  , const Matx34d& camera1
  , const Matx34d& camera2);

void optimizeCorrespondences
  ( const Point2d& point1
  , const Point2d& point2
  , const Matx33d& fundamentalMat
  , Matx13d& newPoint1
  , Matx13d& newPoint2);

Matx33d mkTransformationMat(const Point2d& v);

Matx13d mkEpipole(const Matx33d& fundamentalMat);

Matx13d normalize2(const Matx13d& v);

Matx33d mkSkewSymmetricMat(const Matx13d& v);

Matx33d mkRotationMat(const Matx13d& v);

typedef Matx<double, 1, 7> Matx17d;
Matx17d mkPolynomial
  ( double f, double g
  , double a, double b
  , double c, double d);

double squaredDistance
  ( double t
  , double f, double g
  , double a, double b
  , double c, double d);

double squaredDistanceDt
  ( double f, double g
  , double a, double c);

Matx13d closestPoint(Matx13d line);


// -- IMPL
Matx14d optimalTriangulation_ ( const Point2d& u , const Point2d& v
                              , const Matx33d& f
                              , const Matx34d& p , const Matx34d& q)
{
  Matx13d nu, nv;
  optimizeCorrespondences(u,v,f,nu,nv);
  return mkWorldPoint(nu,nv,p,q);
}



void optimizeCorrespondences( const Point2d& u, const Point2d& v
                            , const Matx33d& fm
                            , Matx13d& nu, Matx13d& nv)
{
  //cout << "---" << endl;
  // init
  Matx33d f = fm;
  //cout << "u" << u << endl;
  //cout << "v" << v << endl;
  //cout << "f" << f << endl;
  // (i)
  Matx33d tu = mkTransformationMat(u);
  Matx33d tv = mkTransformationMat(v);
  //cout << "tu" << tu << endl;
  //cout << "tv" << tv << endl;
  // (ii)
  f = tv.inv().t() *f* tu.inv();
  //cout << "f" << f << endl;
  //(iii)
  // right epipole F*eu=0; left epipole ev'*F=0 -> F'*ev=0
  Matx13d eu = normalize2(mkEpipole(f));
  Matx13d ev = normalize2(mkEpipole(f.t()));
  //cout << "eu" << eu << endl;
  //cout << "ev" << ev << endl;
  // (vi)
  Matx33d ru = mkRotationMat(eu);
  Matx33d rv = mkRotationMat(ev);
  //cout << "ru" << ru << endl;
  //cout << "rv" << rv << endl;
  // (v)
  f = rv *f* ru.t();
  //cout << "f" << f << endl;
  // (vi)
  double fu,fv,a,b,c,d;
  fu = eu(2);
  fv = ev(2);
  a  = f(1,1);
  b  = f(1,2);
  c  = f(2,1);
  d  = f(2,2);
  // (vii)
  //cout << "f" << f << endl;
  Mat roots(1,6,CV_64FC2);
  Matx17d polynomial = mkPolynomial(fu,fv,a,b,c,d);
  //cout << "poly" << polynomial << endl;
  solvePoly(polynomial,roots,100);
  //cout << "roots" << roots << endl;
  Matx16d realRoots (roots.at<Vec2d>(0,0)[0],roots.at<Vec2d>(0,1)[0],roots.at<Vec2d>(0,2)[0]
                    ,roots.at<Vec2d>(0,3)[0],roots.at<Vec2d>(0,4)[0],roots.at<Vec2d>(0,5)[0]);
  //cout << "realroots" << realRoots << endl;
   
  // (viii) minimize cost function (squared root)
  // asymptotic s for t=infinity
  double t_min, s_val;
  t_min = FLT_MAX;
  s_val = squaredDistanceDt(fu,fv,a,c);
  // s(t) for real roots
  double t,s;
  for (int i = 0; i < 6; i++) {
    t = realRoots(0,i);
    s = squaredDistance(t,fu,fv,a,b,c,d);
    if (s < s_val) {
      s_val = s;
      t_min = t;
    }
  }
  // (ix) compute lines and closest point ~x,~x'
  Matx13d lu(t_min*fu,1,-t_min);
  Matx13d lv(-fv*(c*t_min+d),a*t_min+b,c*t_min+d);
  Matx13d tmpu = closestPoint(lu); 
  Matx13d tmpv = closestPoint(lv); 
  // (x) transfer back
  nu = ( tu.inv()*ru.t()*tmpu.t() ).t();
  nv = ( tv.inv()*rv.t()*tmpv.t() ).t();

}

Matx14d mkWorldPoint( const Matx13d& u
                    , const Matx13d& v
                    , const Matx34d& p
                    , const Matx34d& q)
{
  double ux,uy,vx,vy;
  ux = u(0); uy = u(1);
  vx = v(0); vy = v(1);

  Matx14d p1,p2,p3, q1,q2,q3;
  p1 = p.row(0); q1 = q.row(0);
  p2 = p.row(1); q2 = q.row(1);
  p3 = p.row(2); q3 = q.row(2);

  Matx44d a;
  a.row(0) = ux*p3-p1;
  a.row(1) = uy*p3-p2;
  a.row(2) = vx*q3-q1;
  a.row(3) = vy*q3-q2;

  SVD svd = SVD(a);
  Matx41d x = svd.vt.col(2);
  
  return x.t(); 
}

Matx33d mkTransformationMat(const Point2d& v)
{
  double x,y;
  x = v.x; y = v.y;
  Matx33d t (1,0,-x
            ,0,1,-y 
            ,0,0,1);
  return t;
}

Matx33d mkRotationMat(const Matx13d& v)
{
  double x,y;
  x = v(0); y = v(1);
  Matx33d r ( x,y,0
            ,-y,x,0 
            , 0,0,1);
  return r;
}

Matx13d mkEpipole(const Matx33d& f)
{
  SVD svd = SVD(f);
  Matx31d t = svd.vt.col(2);
  return t.t();
}

Matx33d mkSkewSymmetricMat(const Matx13d& v)
{
  double x,y,z;
  x = v(0); y = v(1); z = (2);
  Matx33d m ( 0,-z, y
            , z, 0,-x
            ,-y, 1, 0
            );
  return m;
}

Matx13d normalize2(const Matx13d& v)
{
  double scale = sqrt(pow(v(0),2) + pow(v(1),2));
  return v*scale;
} 

Matx17d mkPolynomial( double f, double g
                    , double a, double b
                    , double c, double d)
{
  //double a2,b2,c2,d2,f2,g2;
  //a2 = pow(a,2); b2 = pow(b,2);
  //c2 = pow(c,2); d2 = pow(d,2);
  //f2 = pow(f,2); g2 = pow(g,2);
  //double a3,b3,c3,d3,f3,g3;
  //a3 = pow(a,3); b3 = pow(b,3);
  //c3 = pow(c,3); d3 = pow(d,3);
  //f3 = pow(f,3); g3 = pow(g,3);
  //double a4,b4,c4,d4,f4,g4;
  //a4 = pow(a,4); b4 = pow(b,4);
  //c4 = pow(c,4); d4 = pow(d,4);
  //f4 = pow(f,4); g4 = pow(g,4);


  //// hurray for WolframAlpha
  double k0,k1,k2,k3,k4,k5,k6;
  //k0 = a*b*d2 + b2*c*d;
  //k1 = -a2*d2 + b*4 + b2*c2 + 2*b2*d2*g2 + d4*g4;
  //k2 = -a2*c*d + 4*a*b3 + a*b*c2 - 2*a*b*d2*f2 + 4*a*b*g2 + 2*b2+c+d+f2 + 4*b2*c*d*g2 + 4*c*d3*g4;
  //k3 = 6*a2*b2 -  2*a2*d2*f2 + 2*a2*d2*g2 + 8*a*b*c*d*g2 + 2*b2*c2*f2 + 2*b2*c2*g2 + 6*c2*d2*g4;
  //k4 = 4*a3*b - 2*a2*c*d*f2 + 4*a2*c*d*g2 + 2*a*b*c2*f2 + 4*a*b*c2*f2 - a*b*d2*f4 + b2*c*d*f4 + 4*c3*d*g4;
  //k5 = a4 + 2*a2*c2*g2 - a2*d2*f4 + b2*c2*f4 + c4*g4;
  //k6 = a*b*c2*f4 - a2*c*d*f4;

  double f1 = f;
  double f2 = g;

  k6 = +b*c*c*f1*f1*f1*f1*a-a*a*d*f1*f1*f1*f1*c;
  k5 = +f2*f2*f2*f2*c*c*c*c+2*a*a*f2*f2*c*c-a*a*d*d*f1*f1*f1*f1+b*b*c*c*f1*f1*f1*f1+a*a*a*a;
  k4 = +4*a*a*a*b+2*b*c*c*f1*f1*a+4*f2*f2*f2*f2*c*c*c*d+4*a*b*f2*f2*c*c+4*a*a*f2*f2*c*d-2*a*a*d*f1*f1*c-a*d*d*f1*f1*f1*f1*b+b*b*c*f1*f1*f1*f1*d;
  k3 = +6*a*a*b*b+6*f2*f2*f2*f2*c*c*d*d+2*b*b*f2*f2*c*c+2*a*a*f2*f2*d*d-2*a*a*d*d*f1*f1+2*b*b*c*c*f1*f1+8*a*b*f2*f2*c*d;
  k2 = +4*a*b*b*b+4*b*b*f2*f2*c*d+4*f2*f2*f2*f2*c*d*d*d-a*a*d*c+b*c*c*a+4*a*b*f2*f2*d*d-2*a*d*d*f1*f1*b+2*b*b*c*f1*f1*d;
  k1 = +f2*f2*f2*f2*d*d*d*d+b*b*b*b+2*b*b*f2*f2*d*d-a*a*d*d+b*b*c*c;
  k0 = -a*d*d*b+b*b*c*d;


  Matx17d m(k0,k1,k2,k3,k4,k5,k6);
  return m;
}

double squaredDistance( double t
                      , double f, double g
                      , double a, double b
                      , double c, double d)
{
  return   (t*t)/(1+f*f*t*t)
         + pow(c*t+d,2)/(pow(a*t+b,2) + g*g*pow(c*t+d,2));
}

double squaredDistanceDt( double f, double g
                        , double a, double c)
{
  return 1./(f*f) + (c*c)/(a*a+g*g*c*c); 
}

Matx13d closestPoint(Matx13d l)
{
  double x,y,z;
  x = l(0);
  y = l(1);
  z = l(2);
  return (-x*z,-y*z,x*x-y*y);
}

