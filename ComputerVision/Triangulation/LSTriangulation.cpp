/*
 * File:   LSTriangulation.cpp
 * Author: Michael Schaper
 *
 * This file provides an implementation of the "LS Linear Triangulation" method.
 * (see Hartley and Sturm, Triangulation).
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "LSTriangulation.h"


namespace ls
{
  Matx14d triangulate ( const Matx13d& u , const Matx13d& v
                      , const Matx34d& p , const Matx34d& q)
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

    // assume x=(x,y,z,1), s.t. Ax=B
    Matx14d b ( -(ux*p(2,3)-p(0,3))
              , -(uy*p(2,3)-p(1,3))
              , -(vx*q(2,3)-q(0,3))
              , -(vy*q(2,3)-q(1,3))
              );

    // FIXME:
    SVD svd = SVD(a);
    Matx41d x = svd.vt.col(2);
    
    return x.t(); 
  }
}

