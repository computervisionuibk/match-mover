/*
 * File:   Triangulation.cpp
 * Author: Michael Schaper
 *
 * This file provides the interface to different triangulation methods:
 * + linear least suare triangulation
 * + optimal triangulation
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "Triangulation.h"
#include "LSTriangulation.h"
#include "OPTriangulation.h"



Vector<Matx14d> linearLSTriangulation
  ( const Vector<Matx13d>& us
  , const Vector<Matx13d>& vs
  , const Matx34d& p
  , const Matx34d& q)
{
  Vector<Matx14d> result(us.size());
  for(int i=0; i<us.size(); ++i){
    Matx13d u,v;
    u = us[i];
    v = vs[i];
    result[i]=ls::triangulate(u,v,p,q);
  }
  return result;
}

Vector<Matx14d> optimizedTriangulation
  ( const Vector<Matx13d>& us
  , const Vector<Matx13d>& vs
  , const Matx33d& f
  , const Matx34d& p
  , const Matx34d& q)
{
  Vector<Matx14d> result(us.size());
  for(int i=0; i<us.size(); ++i){
    Matx13d u,v;
    u = us[i];
    v = vs[i];
    result[i]=op::triangulate(u,v,f,p,q);
  }
  return result;
}

int main(int argc, const char *argv[])
{
  return 0;
}

// cf. 9.14
//void mkCamerasFromFundamentalMat( const Matx33d& f
                                //, Matx34d& p
                                //, Matx34d& q)
//{
  //Matx34d tp(1,0,0,0
            //,0,1,0,0
            //,0,0,1,0);
  //Matx13d e = mkEpipole(f);

  //Matx33d t = mkSkewSymmetricMat(e)*f;
  //Matx34d tq(t(0,0),t(0,1),t(0,2),e(0,0)
            //,t(1,0),t(1,1),t(1,2),e(0,1)
            //,t(2,0),t(2,1),t(2,2),e(0,2));

  //p = tp;
  //q = tq;
//}

//Matx14d mkWorldPoint( const Matx13d& u
                    //, const Matx13d& v
                    //, const Matx34d& p
                    //, const Matx34d& q)
//{
  //double ux,uy,vx,vy;
  //ux = u(0); uy = u(1);
  //vx = v(0); vy = v(1);

  //Matx14d p1,p2,p3, q1,q2,q3;
  //p1 = p.row(0); q1 = q.row(0);
  //p2 = p.row(1); q2 = q.row(1);
  //p3 = p.row(2); q3 = q.row(2);

  //Matx44d a;
  //a.row(0) = ux*p3-p1;
  //a.row(1) = uy*p3-p2;
  //a.row(2) = vx*q3-q1;
  //a.row(3) = vy*q3-q2;

  //SVD svd = SVD(a);
  //Matx31d x = svd.vt.col(2);
  
  //return x.t(); 
//}

