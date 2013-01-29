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
#include "LinearLSTriangulation.h"
#include "OptimalTriangulation.h"



vector<Matx14d> linearLSTriangulation
  ( const vector<Point2d>& us
  , const vector<Point2d>& vs
  , const Matx34d& p
  , const Matx34d& q)
{
  vector<Matx14d> result(us.size());
  for(int i=0; i<us.size(); ++i){
    Point2d u,v;
    u = us[i];
    v = vs[i];
    result[i]=linearLSTriangulation_(u,v,p,q);
  }
  return result;
}

vector<Matx14d> optimalTriangulation
  ( const vector<Point2d>& us
  , const vector<Point2d>& vs
  , const Matx33d& f
  , const Matx34d& p
  , const Matx34d& q)
{
  vector<Matx14d> result(us.size());
  for(int i=0; i<us.size(); ++i){
    Point2d u,v;
    u = us[i];
    v = vs[i];
    result[i]=optimalTriangulation_(u,v,f,p,q);
  }
  return result;
}

int main(int argc, const char *argv[])
{
  return 0;
}
