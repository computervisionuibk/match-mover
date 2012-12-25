/*
 * File:   Triangulation.cpp
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

Matx14d constructWorldPoint 
  ( const Matx13d& point1
  , const Matx13d& point2 
  , const Matx34d& camera1
  , const Matx34d& camera2);

void optimizeCorrespondences
  ( const Matx13d& point1
  , const Matx13d& point2
  , const Matx33d& fundamentalMatrix
  , Matx13d& newPoint1
  , Matx13d& newPoint2);

Matx33d mkTransformationMatrix(const Matx13d& v);

Matx13d mkEpipole(const Matx33d& fundamentalMatrix);

Matx33d mkRotationMatrix(const Matx13d& v);

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

Vector<Matx14d> triangulateFromCameras
  ( const Vector<Matx13d>& points1
  , const Vector<Matx13d>& points2
  , const Matx34d& camera1
  , const Matx34d& camera2)
{
  // TODO: Matx33d f = mkFundamentalMatrix(p,q);
  //return triangulate(us,vs,f,p,q);
}

Vector<Matx14d> triangulateFromFundamentalMatrix
  ( const Vector<Matx13d>& us
  , const Vector<Matx13d>& vs
  , const Matx33d& f)
{
  // TODO: Matx34d f = mkCam1(f);
  // TODO: Matx34d f = mkCam2(f);
  //return triangulate(us,vs,f,p,q);
}

Vector<Matx14d> triangulate
  ( const Vector<Matx13d>& us
  , const Vector<Matx13d>& vs
  , const Matx33d& f
  , const Matx34d& p
  , const Matx34d& q)
{
  Vector<Matx14d> result(us.size());
  for(int i=0; i<us.size(); ++i){
    Matx13d u,v, nu, nv;
    u = us[i];
    v = vs[i];
    optimizeCorrespondences(u,v,f,nu,nv);
    result[i]=constructWorldPoint(u,v,p,q);
  }
  return result;
}



Matx14d constructWorldPoint ( const Matx13d& u
                            , const Matx13d& v
                            , const Matx34d& p
                            , const Matx34d& q)
{
  float xu,yu,xv,yv;
  xu = u(0,0); yu = u(0,1);
  xv = v(0,0); yv = v(0,1);

  Matx14d p1,p2,p3, q1,q2,q3;
  p1 = p.row(0); q1 = q.row(0);
  p2 = p.row(1); q2 = q.row(1);
  p3 = p.row(2); q3 = q.row(2);
  // TODO: test correct?
  Matx44d a;
  a.row(0) = xu*p3-p1;
  a.row(1) = yu*p3-p2;
  a.row(2) = xv*q3-q1;
  a.row(3) = yv*q3-q2;
  // perform dlt
  Matx41d x;
  SVD::solveZ(a,x);
  
  return x.t(); 
}

void optimizeCorrespondences( const Matx13d& u, const Matx13d& v
                            , const Matx33d& fm
                            , Matx13d& nu, Matx13d& nv)
{
  // init
  Matx33d f = fm; //test should be copied?
  // (i)
  Matx33d tu = mkTransformationMatrix(u);
  Matx33d tv = mkTransformationMatrix(v);
  // (ii)
  f = tv.inv().t() *f* tu.inv();
  //(iii)
  // right epipole F*eu=0; left epipole ev*F=0 -> F'*ev=0
  Matx13d eu = mkEpipole(f);
  Matx13d ev = mkEpipole(f.t());
  // (vi)
  Matx33d ru = mkRotationMatrix(eu);
  Matx33d rv = mkRotationMatrix(ev);
  // (v)
  f = rv *f* ru.t();
  // (vi)
  double fu,fv,a,b,c,d;
  fu = eu(0,2);
  fv = ev(0,2);
  a  = f(1,1);
  b  = f(1,2);
  c  = f(2,1);
  d  = f(2,2);
  // (vii)
  Matx16d roots;
  Matx17d polynomial = mkPolynomial(fu,fv,a,b,c,d);
  solvePoly(polynomial,roots); // TODO: check if complex part is casted away
  // (viii) minimize cost function (squared root)
  // asymptotic s for t=infinity
  double t_min, s_val;
  t_min = FLT_MAX;
  s_val = squaredDistanceDt(fu,fv,a,c);
  // s(t) for real roots
  double t,s;
  for (int i = 0; i < 6; i++) {
    t = roots(0,i);
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

Matx33d mkTransformationMatrix(const Matx13d& v)
{
  double x,y;
  x = v(0,0); y = v(0,1);
  Matx33d t (1,0,-x
            ,0,1,-y 
            ,0,0,1);
  return t;
}

Matx33d mkRotationMatrix(const Matx13d& v)
{
  double x,y;
  x = v(0,0); y = v(0,1);
  Matx33d r ( x,y,0
            ,-y,x,0 
            ,0 ,0,1);
  return r;
}

Matx13d mkEpipole(const Matx33d& f)
{
  double scale; 
  Matx13d vZero3(0,0,0);
  Matx13d e; 
  solve(f, vZero3.t(), e.t(), DECOMP_SVD);
  scale = sqrt(pow(e(0,0),2) + pow(e(0,1),2));
  e = e*scale;
  return e;
}

Matx17d mkPolynomial( double f, double g
                    , double a, double b
                    , double c, double d)
{
  double a2,b2,c2,d2,f2,g2;
  a2 = pow(a,2); b2 = pow(b,2);
  c2 = pow(c,2); d2 = pow(d,2);
  f2 = pow(f,2); g2 = pow(g,2);
  double a3,b3,c3,d3,f3,g3;
  a3 = pow(a,3); b3 = pow(b,3);
  c3 = pow(c,3); d3 = pow(d,3);
  f3 = pow(f,3); g3 = pow(g,3);
  double a4,b4,c4,d4,f4,g4;
  a4 = pow(a,4); b4 = pow(b,4);
  c4 = pow(c,4); d4 = pow(d,4);
  f4 = pow(f,4); g4 = pow(g,4);


  // hurray for WolframAlpha
  double k0,k1,k2,k3,k4,k5,k6;
  k0 = a*b*d2 + b2*c*d;
  k1 = -a2*d2 + b*4 + b2*c2 + 2*b2*d2*g2 + d4*g4;
  k2 = -a2*c*d + 4*a*b3 + a*b*c2 - 2*a*b*d2*f2 + 4*a*b*g2 + 2*b2+c+d+f2 + 4*b2*c*d*g2 + 4*c*d3*g4;
  k3 = 6*a2*b2 -  2*a2*d2*f2 + 2*a2*d2*g2 + 8*a*b*c*d*g2 + 2*b2*c2*f2 + 2*b2*c2*g2 + 6*c2*d2*g4;
  k4 = 4*a3*b - 2*a2*c*d*f2 + 4*a2*c*d*g2 + 2*a*b*c2*f2 + 4*a*b*c2*f2 - a*b*d2*f4 + b2*c*d*f4 + 4*c3*d*g4;
  k5 = a4 + 2*a2*c2*g2 - a2*d2*f4 + b2*c2*f4 + c4*g4;
  k6 = a*b*c2*f4 - a2*c*d*f4;

  return Matx17d(k0,k1,k2,k3,k4,k5,k6);
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
  x = l(0,0);
  y = l(0,1);
  z = l(0,2);
  return (-x*z,-y*z,x*x-y*y);
}

int main ( int argc, char **argv )
{
  // try a bit
  Matx13d v (1,2,3);  
  cout << v(2);
  Matx33d tu (1,0,5,
              0,1,5, 
              0,0,1);
  Matx33d tv(tu);
  tv(1,1) = 2;
  cout << tu(0,2) << endl;
  cout << tu << endl;
  cout << pow(2,3) << endl;
  
  return 0;
}

