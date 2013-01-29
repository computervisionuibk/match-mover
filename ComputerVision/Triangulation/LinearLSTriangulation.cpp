/*
 * File:   LinearLSTriangulation.cpp
 * Author: Michael Schaper
 *
 * This file provides an implementation of the "Linear Least Square Triangulation" method.
 * (see Hartley and Sturm, Triangulation).
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "LinearLSTriangulation.h"


Matx14d linearLSTriangulation_( const Point2d& u, const Point2d& v
                              , const Matx34d& p, const Matx34d& q)
{
  double ux,uy,vx,vy;
  ux = u.x; uy = u.y;
  vx = v.x; vy = v.y;

  Matx14d p1,p2,p3, q1,q2,q3;
  p1 = p.row(0); q1 = q.row(0);
  p2 = p.row(1); q2 = q.row(1);
  p3 = p.row(2); q3 = q.row(2);

  Matx44d a;
  a.row(0) = ux*p3-p1;
  a.row(1) = uy*p3-p2;
  a.row(2) = vx*q3-q1;
  a.row(3) = vy*q3-q2;

  //Matx14d b ( -(ux*p(2,3)-p(0,3))
            //, -(uy*p(2,3)-p(1,3))
            //, -(vx*q(2,3)-q(0,3))
            //, -(vy*q(2,3)-q(1,3))
            //);

  // assume vec(x)=(x,y,z,1)
  SVD svd = SVD(a);
  Matx41d t = svd.vt.col(2);
  double z = t(4);
  Matx14d x ( t(0)/z
            , t(1)/z
            , t(2)/z
            , 1.0
            );

  return x; 
}

