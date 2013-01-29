#include <stdio.h>
#include <stdlib.h>

#include "Triangulation.h"


void test()
{
  const int N = 10; 
  vector<Point2d> points1(N);
  vector<Point2d> points2(N);
  Vector<Matx13d> points1M(N);
  Vector<Matx13d> points2M(N);
  srand(time(NULL)); 
  double X,Y,Z; 
  for (int j = 0; j < N; j++) { 
      for (int i = 0; i < N; ++i) { 
          X = (i+1-(N+1)/2.); 
          Y = (j+1-(N+1)/2.); 
          Z = (i+1-(N+1)/2.+10); 
          Point2d point1, point2;
          point1.x = X/Z + ((double)rand())/RAND_MAX - 0.5;
          point1.y = Y/Z + ((double)rand())/RAND_MAX - 0.5;;
          points1[j] = point1;
          point2.x = (Z-10)/(10-X) + ((double)rand())/RAND_MAX - 0.5;;
          point2.y = Y/(10-X) + ((double)rand())/RAND_MAX - 0.5;;
          points2[j] = point2;

          Matx13d point1M, point2M;
          point1M(0,0) = X/Z + ((double)rand())/RAND_MAX - 0.5;
          point1M(0,1) = Y/Z + ((double)rand())/RAND_MAX - 0.5;;
          point1M(0,2) = Z;
          points1M[j] = point1M;
          point2M(0,0) = X/Z + ((double)rand())/RAND_MAX - 0.5;
          point2M(0,1) = Y/Z + ((double)rand())/RAND_MAX - 0.5;;
          point2M(0,2) = Z;
          points2M[j] = point2M;
      } 
  } 

  // Find a fundamental matrix based on the correspondences 
  //cout << "Points1" << endl << points1 << endl;
  //cout << "Points2" << endl << points2 << endl;
  cout << "Calculating fundamental matrix..."; 
  Matx33d f;
  f = cv::findFundamentalMat(points1, points2, CV_FM_LMEDS); 
  cout << "Fundamental matrix:" << endl << f << endl;
  cout << "Before optimization:" << endl << points1 << endl;

  Vector<Matx14d> npoints1M = triangulateFromFundamentalMat(points1M,points2M,f);
  cout << "After optimization:" << endl;
  cout << npoints1M[0] << endl;


}

