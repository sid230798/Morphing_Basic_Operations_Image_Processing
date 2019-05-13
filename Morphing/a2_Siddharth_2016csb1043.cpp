#include<bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
//#include <windows.h>
using namespace std;
using namespace cv;

#define PI 3.14159265

/*Compile using :  g++ -ggdb a2_2016csb1043_Siddharth.cpp -o facedetect `pkg-config --cflags --libs opencv`*/
vector<Point2f> getTransfeatureList(vector<Point2f> srcList,vector<Point2f> dstList,double t){

       int i;
       vector<Point2f> featureList;
       for(i=0;i<srcList.size();i++){
       
          Point2f cord;
          cord.x = (1-t)*srcList.at(i).x + t*dstList.at(i).x;
          cord.y = (1-t)*srcList.at(i).y + t*dstList.at(i).y;
          featureList.push_back(cord);       
       
       }
       
       return featureList;


}


vector<Point2f> getPointsList(char* file_path){

     fstream obj;
     obj.open(file_path);
     vector<Point2f> list;
     
     while(!obj.eof()){
     
         float a,b;
         obj>>a>>b;
         if(obj.eof())
           break;
         Point2f pt(b,a);
         list.push_back(pt);
     
     }
     
     obj.close();
     return list;


}

bool isValid(Point2f pt,Mat img){

     if(pt.x<img.rows && pt.x>=0 && pt.y>=0 && pt.y<img.cols)
       return true;
       
     return false;

}

int searchPoint(Point2f pt,vector<Point2f> Points){

    //vector<Point2f>::iterator it;
    int j;
    for(j=0;j<Points.size();j++){
    
       Point2f match = Points.at(j);
       if(match.x == pt.x && match.y == pt.y)
         return j;
    
    }

    return -1;
}
vector<Vec3f> getIndices(vector<Vec6f> TriangleList,vector<Point2f> Points,Mat img){

        
    int i,j;
    vector<Vec3f> IndexList;
    for(i=0;i<TriangleList.size();i++){
    
       Point2f p1(TriangleList.at(i)[0],TriangleList.at(i)[1]);
       Point2f p2(TriangleList.at(i)[2],TriangleList.at(i)[3]);
       Point2f p3(TriangleList.at(i)[4],TriangleList.at(i)[5]);
       if(isValid(p1,img) && isValid(p2,img) && isValid(p3,img)){
       
          Vec3f pt;
          pt[0] = searchPoint(p1,Points);
          pt[1] = searchPoint(p2,Points);
          pt[2] = searchPoint(p3,Points);
          
          IndexList.push_back(pt);
       
       }
    
    }
    
    return IndexList; 



} 

Mat getMatTie(vector< pair<Point2f,Point2f> > TiePoints){

   Point2f p1,p2,p3,p4,p5,p6;
   p1 = TiePoints.at(0).first;
   p2 = TiePoints.at(0).second;
   p3 = TiePoints.at(1).first;
   p4 = TiePoints.at(1).second;
   p5 = TiePoints.at(2).first;
   p6 = TiePoints.at(2).second;

   float a[3][3] = {{p2.x,p2.y,1},{p4.x,p4.y,1},{p6.x,p6.y,1}};
   float x[3] = {p1.x,p3.x,p5.x};
   float y[3] = {p1.y,p3.y,p5.y};
   Mat A = Mat(3,3,CV_32FC1,a);
   Mat X = Mat(3,1,CV_32FC1,x);
   Mat Y = Mat(3,1,CV_32FC1,y);
   
   Mat inv = A.inv();
   Mat invX = inv*X;
   Mat invY = inv*Y;
   
   float aff[2][3] = {{invX.at<float>(0,0),invX.at<float>(1,0),invX.at<float>(2,0)},{invY.at<float>(0,0),invY.at<float>(1,0),invY.at<float>(2,0)}};
   Mat affTrans = Mat(2,3,CV_32FC1,aff);
   //cout<<"";
  // cout<<"From function : "<<affTrans<<endl;
   return affTrans;

}
/* A utility function to calculate area of triangle formed by (x1, y1), 
   (x2, y2) and (x3, y3) */
float area(int x1, int y1, int x2, int y2, int x3, int y3)
{
   return abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2.0);
}
 
/* A function to check whether point P(x, y) lies inside the triangle formed 
   by A(x1, y1), B(x2, y2) and C(x3, y3) */
bool isInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
{   
   /* Calculate area of triangle ABC */
   float A = area (x1, y1, x2, y2, x3, y3);
 
   /* Calculate area of triangle PBC */  
   float A1 = area (x, y, x2, y2, x3, y3);
 
   /* Calculate area of triangle PAC */  
   float A2 = area (x1, y1, x, y, x3, y3);
 
   /* Calculate area of triangle PAB */   
   float A3 = area (x1, y1, x2, y2, x, y);
   
   /* Check if sum of A1, A2 and A3 is same as A */
   return (A == A1 + A2 + A3);
}
 

int getIndexofTriangle(int p,int q,vector<Vec3f> Indices,vector<Point2f> warpList){

   int i;
   for(i=0;i<Indices.size();i++){
   
      int a = Indices.at(i)[0],b = Indices.at(i)[1],c = Indices.at(i)[2];   //Indeices of ttrangulation points
      if(isInside(warpList.at(a).x,warpList.at(a).y,warpList.at(b).x,warpList.at(b).y,warpList.at(c).x,warpList.at(c).y,p,q))
        return i;  
   
   
   }

   return -1;

}
Vec3b biLinear(Mat src,float a,float b){
       
         
         double x1,x2,y1,y2,alp,bet;
         Vec3b pixel;
         if(a>=0 && a<=src.rows-1 && b>=0 && b<=src.cols-1){
                    
           x1 = floor(a);
           if(x1 != a)
            x2 = x1 + 1;
           else
            x2 = x1;
           y1 = floor(b);
           if(y1 != b)
            y2 = y1 + 1;
           else
            y2 = b;
           alp = x2-a;
           bet = y2-b;
           
           
           return alp*bet*src.at<Vec3b>(x1,y1)+ (1-alp)*bet*src.at<Vec3b>(x2,y1) +alp*(1-bet)*src.at<Vec3b>(x1,y2) + (1-bet)*(1-alp)*src.at<Vec3b>(x2,y2);
           //cout<<k<<endl;
           //return k;
        }
        else if(a > -1 && a<src.rows && b>=0 && b<=src.cols-1){
        
           if(a<0){
              x1 = 0;
              alp = 1+a;
           }
            
           else{
            x1 = x2 = src.rows-1;
            alp = src.rows - a;
           }
        
           y1 = floor(b);
           y2 = y1+1;
           bet = y2-b;
           Vec3b val = bet*src.at<Vec3b>(x1,y1) + (1-bet)*src.at<Vec3b>(x1,y2);
           
           return alp*val;
        }
        else if(b >- 1 && b<src.cols && a>=0 && a<=src.rows-1){
        
           if(b<0){
              y1 =0;
              bet = 1+b;
           }
           else{
              y1= src.cols-1;
              bet = src.cols -b;
           }
           x1 = floor(a);
           x2 = x1+1;
           alp = x2-a;
           Vec3b val = alp*src.at<Vec3b>(x1,y1)+(1-alp)*src.at<Vec3b>(x2,y1);
           //alp = x2-a;
           return bet*val;
        }
        else if(a >-1 && a<src.rows && b>-1 && b<src.cols){
           float diffX,diffY;
           if(a<0){
             diffX = 1+a;
             x1 = 0;
           }
           else{
              diffX = src.rows-a;
              x1 = src.rows-1;  
           }
           if(b<0){
              diffY = 1+b;
              y1=0;
           }
           else{
               diffY = src.cols-b;
               y1 = src.cols-1;
           } 
           
           float max =(diffX>=diffY)?diffX:diffY;
           return max*src.at<Vec3b>(x1,y1);
           
        
        }
        else
           return 0;


}

vector<float> getAffTransMat(int j,vector<Vec3f> Indices,vector<Point2f> list,vector<Point2f> warpList,Mat Point,Mat& trans){

       int a = Indices.at(j)[0],b = Indices.at(j)[1],c = Indices.at(j)[2];   //Indeices of ttrangulation points
       
       vector<pair<Point2f,Point2f> > tiePoints;
       
       tiePoints.push_back(make_pair(list.at(a),warpList.at(a)));
       tiePoints.push_back(make_pair(list.at(b),warpList.at(b)));
       tiePoints.push_back(make_pair(list.at(c),warpList.at(c)));
      
       trans = getMatTie(tiePoints);
       //cout<<"Output by function call "<<trans<<endl;
       
       float arr[2];
       arr[0] = trans.at<float>(0,0)*Point.at<float>(0,0)+trans.at<float>(0,1)*Point.at<float>(1,0)+trans.at<float>(0,2)*Point.at<float>(2,0);
       arr[1] = trans.at<float>(1,0)*Point.at<float>(0,0)+trans.at<float>(1,1)*Point.at<float>(1,0)+trans.at<float>(1,2)*Point.at<float>(2,0);
       Mat MapPoint = Mat(2,1,CV_32FC1,arr);
       //cout<<"POint = "<<Point<<endl<<"Output for mapping "<<MapPoint<<endl;
       
       vector<float> s;
       s.push_back(arr[0]);
       s.push_back(arr[1]);
       return s; 
      
       

}
int main(){


/*1. Got image feature pts for both images

  2. Get mid transition image pts for morphing.
  
  3. Use Dnulae trangulation for each transition image/or only for mid-level transition
  
  4. Get affine trnasformation for each traingle for both images.
  
  5. Get position of points in triangle and perform inverse map to source and destination.
  
  6. CrossDissolve both Wraps by transition factor
*/

char s[100];



vector<Point2f> srcList,dstList;
//Mat src,dst;

cout<<"Please enter the input image file location : ";
cin>>s;
Mat src = imread(s);

cout<<"Please eneter the second image file location : ";
cin>>s;
Mat dst = imread(s);





cout<<"Please Input points feature Location for first image : ";
cin>>s;

srcList = getPointsList(s);

cout<<"Please Input points feature Location for second image : ";
cin>>s;

dstList = getPointsList(s);
cout<<"Please Enter numer of transition images : ";
int t;
cin>>t;
double i=0;
int j;
int srcRows = src.rows,srcCols = src.cols,dstRows = dst.rows,dstCols = dst.cols;
cout<<srcRows<<" "<<srcCols<<" "<<dstRows<<" "<<dstCols<<endl;
vector<Mat> Transitions;

for(i=0;i<=1;i+= 1.0/t){
   //cout<<dstRows<<" "<<dstCols;
   int warpRows = round(i*dstRows + (1-i)*srcRows);
   int warpCols = round(i*dstCols + (1-i)*srcCols);
   cout<<warpRows<<" "<<warpCols<<endl;
  
   Mat warp = Mat(warpRows,warpCols,src.type(),Scalar::all(0));
  
   vector<Point2f> warpList = getTransfeatureList(srcList,dstList,i);
   
   Size size = warp.size();
   //cout<<size.width<<" "<<size.height<<endl;
   Rect rect(0, 0, size.height, size.width);
   
   Subdiv2D subdiv(rect);
   for(j=0;j<warpList.size();j++)
      subdiv.insert(warpList.at(j));
      
   vector<Vec6f> TriangleList ;
   subdiv.getTriangleList(TriangleList);
   
   vector<Vec3f> Indices = getIndices(TriangleList,warpList,warp);
   /*
   for(j=0;j<Indices.size();j++)
     cout<<"TriangleList points = "<<Indices.at(j)[0]<<" "<<Indices.at(j)[1]<<" "<<Indices.at(j)[2]<<endl;
   */  
   
   
   
 
  int IndiceSize = Indices.size(); 
  
   
   Mat warpSrc = Mat(warpRows,warpCols,src.type(),Scalar::all(0));
   Mat warpDst = Mat(warpRows,warpCols,src.type(),Scalar::all(0));
   int p,q;
    
   int count=0;
   for(p=0;p<warpRows;p++){
      for(q=0;q<warpCols;q++){
      float matrix[3] = {p,q,1};
      Mat PointMat = Mat(3,1,CV_32FC1,matrix);
      int triangleNumber = getIndexofTriangle(p,q,Indices,warpList);
       // cout<<p<<" "<<q<<" "<<triangleNumber<<endl;
        
      if(triangleNumber != -1){
              
       // cout<<p<<" "<<q<<endl;
        Mat aTrans = Mat(2,3,CV_32FC1,Scalar::all(0));
        vector<float> Srcarr=  getAffTransMat(triangleNumber,Indices,srcList,warpList,PointMat,aTrans);
       
          Mat bTrans = Mat(2,3,CV_32FC1,Scalar::all(0));
          vector<float> Dstarr = getAffTransMat(triangleNumber,Indices,dstList,warpList,PointMat,bTrans);
          
         float a = Srcarr[0], b = Srcarr[1];  
         float a1 = Dstarr[0], b1 = Dstarr[1];
        
         
        warpSrc.at<Vec3b>(p,q) = biLinear(src,a,b);
         
        warpDst.at<Vec3b>(p,q) = biLinear(dst,a1,b1);
      }
      else{
       // cout<<p<<" "<<q<<endl;
       count++;
      }
      }
   
   
   }
   
   int wi1,wi2;
   Mat transWarp=Mat(warpRows,warpCols,src.type(),Scalar::all(0));
   for(wi1=0;wi1<warpRows;wi1++){
   
      for(wi2=0;wi2<warpCols;wi2++){
        
         transWarp.at<Vec3b>(wi1,wi2) = (1-i)*warpSrc.at<Vec3b>(wi1,wi2) + i*warpDst.at<Vec3b>(wi1,wi2);
         
      }
   }
   
   cout<<count<<endl;
   cout<<"Image completed"<<endl;
   Transitions.push_back(transWarp);
   
}
//cout<<Transitions.at(0)<<endl;
string nameFig = "fig";
int kp1;
char c='a';
for(kp1=0;kp1<Transitions.size();kp1++){
   c++;
   nameFig+=c;
   imwrite(nameFig+".jpg",Transitions.at(kp1));
   namedWindow("Transitions",WINDOW_AUTOSIZE);
   imshow("Transitions",Transitions.at(kp1));
   waitKey(3000);
}
   





return 0;


}
