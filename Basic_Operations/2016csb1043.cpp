#include<bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
using namespace cv;

#define PI 3.14159265



double getPSNR(const Mat& I1, const Mat& I2)
{
    Mat s1;
    absdiff(I1, I2, s1);       // |I1 - I2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |I1 - I2|^2

    Scalar s = sum(s1);        // sum elements per channel

    double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

    if( sse <= 1e-10) // for small values return zero
        return 0;
    else
    {
        double mse  = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }
}

/*
  Class definition for Distance Metric.
  
  Two Distance Metric have been specified:
   1.Nearest Neigbourhood
   2.Linear Interpolation
*/

class distMetric{

    /* Function definition for nearest neighbour and interpolation*/
     public:
         unsigned char biLinear(Mat src,float a,float b);
         unsigned char nearestNeighbour(Mat src,float a,float b);
                   
};
   /*Function declarations of both*/
   
   
   /*Linear Interpolation : 
   
    a. Finding four nearest neighbours and getting pixel value ans ratio of it
    b. For boundary conditions check wether it has neigbours or not
    c. Rounding the value obtained and returning the pixel value
    d. Time complexity is O(1) only 4 claculations are nedded
   */
unsigned char distMetric::biLinear(Mat src,float a,float b){

   /*Four nearest points pair of each 2*/
    float x1,x2,y1,y2;
   /*Ratio in which pixel is divivded*/
    float alp,bet;

   /*  Check for corner points and assigning it corner values*/
       if((round(a) == 0 && round(b) == 0) || (round(a)==src.rows && round(b) == 0) || (round(b) == src.cols && round(a) == 0) ||(round(a)==src.rows &&round(b) == src.cols)){
              x1 = round(a);y1=round(b);
              if(round(a) == src.rows)
                 x1 = round(a)-1;
              if(round(b) == src.cols)
                 y1 = round(b)-1;
              return  src.at<uchar>(x1,y1);
          
           }
           
     /*  Check for bottom and upper rows as they have only two neigbours */
           else if(round(a) == 0 || round(a) == src.rows){
           
            
               if(round(a) == src.rows)
                  x1 = round(a)-1;
               else
                 x1 = round(a);
                 
                 y1 = round(b) - 1;
                 y2 = round(b);
                 
                 bet = (y2+0.5-b);
                     
                 return round(bet*src.at<uchar>(x1,y1) + (1-bet)*src.at<uchar>(x1,y2)) ;   
              
           
           } 
        /* Check for two side columns as they also have only two neighbours  */
           else if(round(b) == 0 || round(b) == src.cols){
           
             
                if(round(b) == src.cols){
                y1= round(b)-1;
                   
                }
                else
                 y1 = round(b);
                 
                 x1 = round(a) - 1;
                 x2 = round(a);
                 
                 alp = (x2+0.5-a);
                     
                 return round(alp*src.at<uchar>(x1,y1) + (1-alp)*src.at<uchar>(x2,y1));
              
           
           }
           
           /* Check for all remaining points */
           else{
           
               x1 = round(a)-1;
               x2 = round(a);
               y1 = round(b)-1;
               y2 = round(b);
           
               alp = (x2+0.5-a);
               bet = (y2+0.5-b);
               return round(alp*bet*src.at<uchar>(x1,y1) + (1-alp)*bet*src.at<uchar>(x2,y1) +alp*(1-bet)*src.at<uchar>(x1,y2) + (1-bet)*(1-alp)*src.at<uchar>(x2,y2));
               
           } 

}


unsigned char distMetric::nearestNeighbour(Mat src,float a,float b){

      int x = round(a+0.5)-1;
      int y = round(b+0.5)-1;
      if(x<src.rows && x>=0 && y<src.cols && y>=0)
       return  src.at<uchar>(x,y);
      else
       return 0;

}


class Scale{

      public:
        Mat resize(Mat src,float scaleX,float scaleY,int option);
      private:
        distMetric obj;

};

Mat Scale::resize(Mat src,float scaleX,float scaleY,int option=0){

      Mat dst = Mat(src.rows*scaleY,src.cols*scaleX,src.type(),Scalar::all(0));
      float e = 0;
      if(scaleX != (int)scaleX || scaleY !=(int)scaleY)
        e = 0.000005;
      int i,j;
      for(i=0;i<dst.rows;i++){
      
         for(j=0;j<dst.cols;j++){
         
            
            if(option==0){
             float x = (i+0.5-e)/scaleY;
             float y = (j+0.5-e)/scaleX;
              dst.at<uchar>(i,j) = obj.nearestNeighbour(src,x,y);
            }
            else{
             float x = (i+0.5)/scaleY;
             float y = (j+0.5)/scaleX;
              dst.at<uchar>(i,j) = obj.biLinear(src,x,y);
            }
         }
      
      }
      
      imwrite("Resize.jpg",dst);
      
      return dst;

}


class AffineTransform{

      public:
      
         Mat Translate(Mat src,int horizontalX,int verticalY);
         Mat Rotate(Mat src,double angle,int Screen_Option,int option);
         Mat Shear(Mat src,float horizontalX,float verticalY);
         Mat TiePoints(Mat src,vector< pair<Point2f,Point2f> > points,int option=0);
         
         unsigned char biLinear_(Mat src,float a,float b);
      private:
      
         distMetric obj;
};

unsigned char AffineTransform::biLinear_(Mat src,float a,float b){
       
         
         double x1,x2,y1,y2,alp,bet;
         if(a>=0 && a<=src.rows-1 && b>=0 && b<=src.cols-1){
                    
           x1 = floor(a);
           x2 = x1 + 1;
           y1 = floor(b);
           y2 = y1 + 1;
           alp = x2-a;
           bet = y2-b;
           
           return round(alp*bet*src.at<uchar>(x1,y1)+ (1-alp)*bet*src.at<uchar>(x2,y1) +alp*(1-bet)*src.at<uchar>(x1,y2) + (1-bet)*(1-alp)*src.at<uchar>(x2,y2));
           
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
           float val = bet*src.at<uchar>(x1,y1) + (1-bet)*src.at<uchar>(x1,y2);
           
           return round(alp*val);
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
           float val = alp*src.at<uchar>(x1,y1)+(1-alp)*src.at<uchar>(x2,y1);
           //alp = x2-a;
           return round(bet*val);
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
           return round(max*src.at<uchar>(x1,y1));
           
        
        }
        else
           return 0;


}

Mat AffineTransform::Shear(Mat src,float horizontalX,float verticalY){

     int rows = src.rows + verticalY*src.cols;
     int cols = src.cols + horizontalX*src.rows;
     float a = verticalY,b=horizontalX;
     Mat dst = Mat(rows,cols,src.type(),Scalar::all(0));
     int i,j;
     for(i=0;i<dst.rows-1;i++){
        for(j=0;j<dst.cols;j++){
        
            float x = (a*j-i)/(a*b-1);
            float y = (b*i-j)/(a*b-1);
            
            dst.at<uchar>(i+1,j) = biLinear_(src,x,y);
        }
     
     
     }
     imwrite("Shear.jpg",dst);
     
     return dst;


}

Mat AffineTransform::Translate(Mat src,int horizontalX,int verticalY){

 //Include Size option
         Mat dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
         int i,j;
         for(i=0;i<src.rows;i++){
         
             for(j=0;j<src.cols;j++){
              
                if(i+verticalY<src.rows && j+horizontalX<src.cols)
                  dst.at<uchar>(i+verticalY,j+horizontalX) = src.at<uchar>(i,j);
             
             }
         
         }
         imwrite("Translate.jpg",dst);
          return dst;

} 

Mat AffineTransform::Rotate(Mat src,double angle,int Screen_Option,int option){

         Mat dst;
         float c = cos(angle*PI/180)*src.rows + sin(angle*PI/180)*src.cols;
         float d = sin(angle*PI/180)*src.rows + cos(angle*PI/180)*src.cols;         
         //ceil(cos(angle*PI/180)*src.cols + sin(angle*PI/180)*src.rows);
         if(Screen_Option == 1)
           dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
         else
           dst = Mat(c,d,src.type(),Scalar::all(0));
           
         //cout<<dst.rows<<" "<<dst.cols<<endl;  
         double centerX = dst.rows/2;
         double centerY = dst.cols/2;
         //double e = 0.0005;
         double sr = src.rows;
         double sc= src.cols;
         int i,j;
         for(i=0;i<dst.rows;i++){
           for(j=0;j<dst.cols;j++){
       
              double x1 = ((i-centerX)*cos(angle*PI/180) +  (j-centerY)*sin(angle*PI/180) + sr/2);    //Yet to be updated
              double y1 =  ((j-centerY)*cos(angle*PI/180) - (i-centerX)*sin(angle*PI/180) + sc/2);
              //cout<<x1<<" "<<y1<<endl;
          
                if(option)
                 dst.at<uchar>(i,j) = biLinear_(src,x1,y1);
                else{
                  dst.at<uchar>(i,j) =  obj.nearestNeighbour(src,x1,y1);
                }
       
           }
       
        }
        imwrite("Rotate.jpg",dst);
       return dst;
      //cout<<dst<<endl;     


}

Mat AffineTransform::TiePoints(Mat src,vector< pair<Point2f,Point2f> > points,int option){

   int k = points.size(),i,j;
   float x_org[k],y_org[k],matNew[k][k];
         for(i=0;i<k;i++){
         
            x_org[i] = (points.at(i)).first.x;
            
            y_org[i] = (points.at(i)).first.y;
            
            matNew[i][0] = (points.at(i)).second.x;
            matNew[i][1] = (points.at(i)).second.y;
            matNew[i][2] = ((points.at(i)).second.x)*((points.at(i)).second.y);
            matNew[i][3] = 1;
            
            
         }
        
         Mat x = Mat(k,1,CV_32FC1,x_org);
         Mat y = Mat(k,1,CV_32FC1,y_org);
         Mat m = Mat(k,k,CV_32FC1,matNew);
       
         
         Mat inv = m.inv();
      
         Mat cx = inv*x;
         Mat cy = inv*y;
         
         
         Mat dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
         for(i=0;i<src.rows;i++){
            for(j=0;j<src.cols;j++){
            
               float arr[4] = {i,j,i*j,1};
               Mat obj_ = Mat(1,4,CV_32FC1,arr);
               Mat x_ = obj_*cx;
               Mat y_ = obj_*cy;
               double x1 = x_.at<float>(0,0);
               double y1 = y_.at<float>(0,0);
               
                         
                 if(option == 0)
                   dst.at<uchar>(i,j) = obj.nearestNeighbour(src,x1,y1);
                 else
                   dst.at<uchar>(i,j) = biLinear_(src,x1,y1);   
                
                       
            }
         
         }  

        imwrite("Reconstruct.jpg",dst);
         return dst;



}


class ContrastEnhancement{

      public:
         Mat ImageNeg(Mat src);
         Mat LogTransform(Mat src);
         Mat PowerTransform(Mat src,double params);
         Mat PiecewiseLinear(Mat src,int r1=-1,int r2=-1,int s1=0,int s2=255);
         Mat BitSlice(Mat src,vector<int> pl);
};

Mat ContrastEnhancement::ImageNeg(Mat src){

     Mat dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
     int i,j;
     for(i=0;i<src.rows;i++){
        for(j=0;j<src.cols;j++)
            dst.at<uchar>(i,j) = 255 - src.at<uchar>(i,j);
     
     }
     imwrite("ImgNeg.jpg",dst);
   //  cout<<dst<<endl;
    return dst;
}

Mat ContrastEnhancement::LogTransform(Mat src){

    Mat dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
     int i,j;
     for(i=0;i<src.rows;i++){
        for(j=0;j<src.cols;j++){
           double val = log10(src.at<uchar>(i,j)+1);
           val = val*255/log10(256);
           dst.at<uchar>(i,j) = round(val);
          
        }
    }
    imwrite("LogTrabsform.jpg",dst);
   return dst;

}

Mat ContrastEnhancement::PowerTransform(Mat src,double params){
     
     Mat dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
     int i,j;
     double max = pow(255,params);
     for(i=0;i<src.rows;i++){
        for(j=0;j<src.cols;j++){
        
           double r = src.at<uchar>(i,j);
           double val = pow(r,params)*255/max;
           dst.at<uchar>(i,j) = round(val);
        
        }
     
     
     }
imwrite("PowerTransform.jpg",dst);
    return dst;


}

Mat ContrastEnhancement::PiecewiseLinear(Mat src,int r1,int r2,int s1,int s2){
 
 
     Mat dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
     int i,j,min=256,max=0;
     if(r1 == -1 || r2 == -1){
    
       for(i=0;i<src.rows;i++){
       
          for(j=0;j<src.cols;j++){
              
               if(src.at<uchar>(i,j)<min)
                 min = src.at<uchar>(i,j);
                 
               if(src.at<uchar>(i,j) > max)
                  max= src.at<uchar>(i,j);
          
          }
       
       }
     
     }
     
     if(r1 == -1)
       r1 = min;
     if(r2 == -1)
       r2 = max;
       
     for(i=0;i<src.rows;i++){
     
        for(j=0;j<src.cols;j++){
        
            int r = src.at<uchar>(i,j);
            if(r<=r1){
            
               if(r1 != 0)
                  dst.at<uchar>(i,j) = r*s1/r1;
            }
            else if(r<=r2){
            
                  if(r1 != r2)
                    dst.at<uchar>(i,j) = r*(s2-s1)/(r2-r1) + (s1*r2-s2*r1)/(r2-r1);
                  else
                    dst.at<uchar>(i,j) = r1;  
                  
            
            }
            else{
            
               if(r2 != 255)
                  dst.at<uchar>(i,j) = r*(255-s2)/(255-r2) + (255*s2-255*r2)/(255-r2);
               else
                   dst.at<uchar>(i,j) = 255;
            }
        }
     
     }
    imwrite("PiecewiseLinear.jpg",dst);
    return dst;
     //  cout<<dst<<endl;

}

Mat ContrastEnhancement::BitSlice(Mat src,vector<int> planes){

     int i,j,k;
     Mat dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
     for(k=0;k<planes.size();k++){
         int pl = planes.at(k);
         for(i=0;i<src.rows;i++){
            for(j=0;j<src.cols;j++){
                
                int r = src.at<uchar>(i,j);
                r = r & (1<<pl);
                dst.at<uchar>(i,j) += r;
            }
             
             
         }
     }
     
     imwrite("BitSlice.jpg",dst);
     return dst;

}


class Histogram{

      public:
          Mat HistogramEqui(Mat src);
          Mat MatchHistogramEqui(Mat src1,Mat src2);
          Mat AdjHistogramEqui(Mat src,int m);


};

Mat Histogram::HistogramEqui(Mat src){
     
     double *val = new double[256];
     int t;
     for(t=0;t<256;t++)
       val[t] = 0; 
     Mat dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
     int i,j,max=-1,min=256;
     for(i=0;i<src.rows;i++){
        for(j=0;j<src.cols;j++){
           int r = src.at<uchar>(i,j);
           val[r]++;
           if(r<min)
             min = r;
        }
     
     }
    
     for(i=0;i<256;i++){
       val[i] = val[i]/(src.rows*src.cols);
     }
   
     
      
      
     for(i=1;i<256;i++){
       val[i] = val[i-1]+val[i];
     }
     
     float m = val[min];
     
    // min = val[min];
     //cout<<min<<endl;
     for(i=0;i<src.rows;i++){
        for(j=0;j<src.cols;j++){
            unsigned char r = src.at<uchar>(i,j);
            dst.at<uchar>(i,j) = round(255*(val[r]-m)/(1.0-m));
        }
     
     }
     
     
  imwrite("EquiHisto.jpg",dst);   
  return dst;
    
   // cout<<dst<<endl;

}

Mat Histogram::MatchHistogramEqui(Mat src1,Mat src2){


      int *val1 = new int[256];
      int *val2 = new int[256];
      double *val3 = new double[256];
      double *val4 = new double[256]; 
      int i,j,min=256;
      for(i=0;i<256;i++){
        val1[i]=0;
     }
     for(i=0;i<256;i++){
        val2[i] =0;
     }
      double r1 = src1.rows,c1 = src1.cols,r2 = src2.rows,c2=src2.cols;
     
     for(i=0;i<r1;i++){
        for(j=0;j<c1;j++){
           int r = src1.at<uchar>(i,j);
           //cout<<r<<" ";
           val1[r] = val1[r] +1;
           //cout<<val1[r]<<" "<<r<<" ";
           if(r<min)
             min =r;
        }
     
     }
     //cout<<endl;
    
     for(i=1;i<256;i++){
       val1[i] = val1[i]+val1[i-1];
      //cout<<val1[i]<<" ";
     }
     
     float minVal1 = val1[min]/(r1*c1);
     //cout<<minVal1<<endl;
     for(i=0;i<256;i++)
     {     
           //val1[i]= val1[i]/(r1*c1);
           if(val1[i] > 0)
              val3[i] = (val1[i]/(r1*c1)-minVal1)/(1-minVal1);
           else
             val3[i] = 0;
             
     //        cout<<val3[i]<<" ";
     
     }
     
    min=256;
     cout<<endl;
     for(i=0;i<r2;i++){
        for(j=0;j<c2;j++){
           int r = src2.at<uchar>(i,j);
           //cout<<r<<endl;
           val2[r]++;
           if(r<min)
            min =r;
        
        }
     
     }
     
     for(i=1;i<256;i++){
       val2[i] = val2[i]+val2[i-1];
       //cout<<val2[i]<<" ";
     }
     
     float minVal2 = val2[min]/(r2*c2);
     for(i=0;i<256;i++)
     {     
           //val2[i]= val2[i]/(r2*c2);
           if(val2[i] > 0)
             val4[i] = (val2[i]/(r2*c2)-minVal2)/(1-minVal2);
           else
             val4[i] = 0;
     
     }
     cout<<endl;
     
     Mat dst = Mat(r1,c1,src1.type(),Scalar::all(0));
     int k;
     for(i=0;i<r1;i++){
     
        for(j=0;j<c1;j++){
        
            int r = src1.at<uchar>(i,j);
            double val = val3[r];
            for(k=0;k<256;k++){
            
                double check = val4[k];
               if(val <= check)
               {
                  if(k>0){
                     if(val == check)
                        dst.at<uchar>(i,j) = k;
                     else{
                     
                        if((val-val4[k-1]) <= check-val)
                           dst.at<uchar>(i,j) = k-1;
                        else
                            dst.at<uchar>(i,j) = k;
                     }
                        
                  
                  }
                  else
                    dst.at<uchar>(i,j) = k; 
                   
                 break;   
               
               } 
            
            } 
        
        }
     
     
     }
     imwrite("MatchHisto.jpg",dst);
      return dst;
      //cout<<dst<<endl;
     

}

Mat Histogram::AdjHistogramEqui(Mat src,int m){

     Mat dst = Mat(src.rows,src.cols,src.type(),Scalar::all(0));
     m = m-2;
     int i,j,r,s;
     for(i=0;i<src.rows;i++){
        for(j=0;j<src.cols;j++){
        
            int rank = 0,count=0;
            int max = src.at<uchar>(i,j);
            for(r=i-m;r<=i+m;r++){
            
               for(s=j-m;s<=j+m;s++){
                   count++;
                   
                   int k =r,p=s;
                   if(r<0)
                     k = abs(r);
                   if(s<0)
                     p = abs(s);
                   if(k>src.rows-1)
                     k = src.rows-k+src.rows-2;
                   if(p>src.cols-1)
                     p = src.cols-p+src.cols-2;
                   if(src.at<uchar>(i,j) >= src.at<uchar>(k,p))
                      rank++;
                      
                   if(src.at<uchar>(k,p)>max)
                      max = src.at<uchar>(k,p);
                      
                      
               }
            
            
            }
            //cout<<max<<" "<<count<<endl;
            
            dst.at<uchar>(i,j) = rank*max/count;
        
        }
     
     
     }
   imwrite("AdaptHisto.jpg",dst);
    return dst;
}


int main(int argc,char* argv[]){

   
    cout<<"Welocme to Image Processing app."<<endl;
    string s;
    while(1){
    
   
   
    //Mat A = Mat(4,4,CV_8UC1,a);
    cout<<"The Following functionalites are implemented : \n";
    cout<<" 1.Resize the Image \n";
    cout<<" 2.Translate the Image \n";
    cout<<" 3.Rotate the Image \n";
    cout<<" 4.Shear the Image \n";
    cout<<" 5.Reconstruct the Image with Tie points \n";
    cout<<" 6.Image Negative \n";
    cout<<" 7.Log Transformation \n";
    cout<<" 8.Power Transformation \n";
    cout<<" 9.PieceWise Linear Transformation \n";
    cout<<" 10.BitPlane Slicing \n";
    cout<<" 11.Histogram Equilization \n";
    cout<<" 12.Match Histogram Equilization \n";
    cout<<" 13.Adaptive Histogram Equilization \n";
    cout<<" 0.Enter 0 for close \n";
    cout<<"Enter the Key for corresponding function : ";
    int n;
    cin>>n;
    if(n == 0)
      break;
    cout<<endl;
    
     cout<<"Enter the Image location : ";
    cin>>s;
    Mat image = imread(s,0);
    switch(n){
    
         case 0:
             cout<<"Thanks!!!!"<<endl;
             
             break;
             
         case 1:
         {
             cout<<"For Resize of the images : \n";
            
             cout<<"Please Enter the Parameters : \n";
             float scaleX,scaleY;
             int nDist;
             cout<<"Enter Horizontal Scale factor : ";
             cin>>scaleX;
             cout<<"Enter Vertical Scale factor : ";
             cin>>scaleY;
             cout<<"Enter 0 for Nearest Neigbour method and 1 for Bilinear : ";
             cin>>nDist;
             Scale obj;
             Mat original = obj.resize(image,scaleX,scaleY,nDist);
             namedWindow("Original",WINDOW_AUTOSIZE);
             imshow("Original",original);
             Mat dst;
             if(nDist == 0)
               resize(image,dst,Size(),scaleX,scaleY,INTER_NEAREST);
             else
               resize(image,dst,Size(),scaleX,scaleY,INTER_CUBIC);
             namedWindow("Inbuilt",WINDOW_AUTOSIZE);
             imshow("Inbuilt",dst);
             waitKey(0);
             cout<<"PSNR Recieved from inbuilt and my implemenatation is : " <<getPSNR(original,dst)<<endl;
             //Write psnr function    
          }
             break;
          
         case 2:
         {
              cout<<"For Translate the image : \n";
              cout<<"Please Enter the Parameters : \n";
              float transX,transY;
              int n1;
              cout<<"Enter Horizontal trans factor : ";
              cin>>transX;
              cout<<"Enter Vertical trans factor : ";
              cin>>transY;
              AffineTransform trans;
              Mat original;
              original = trans.Translate(image,transX,transY);
              namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
              float tp[2][3] = {{1,0,transX},{0,1,transY}};
              Mat m = Mat(2,3,CV_32FC1,tp);
              Mat dst;
              warpAffine(image,dst,m,Size(image.cols,image.rows));
               namedWindow("Inbuilt",WINDOW_AUTOSIZE);
              imshow("Inbuilt",dst);
              waitKey(0);
              cout<<"PSNR Recieved from inbuilt and my implemenatation is : "<<getPSNR(original,dst)<<endl;
              //Write psnr function
          }
              break;          
          case 3:
          {
              cout<<"For Rotation the image : \n";
              cout<<"Please Enter the Parameters : \n";
              double angle;
              int n2,n3;
              
              cout<<"Enter angle of rotation : ";
              cin>>angle;
              cout<<"Enter Screen type 0 for Full size or 1 for Crop : ";
              cin>>n2;
              cout<<"Enter 0 Nearest Neighbourhood and 1 for BiLinear : ";
              cin>>n3;
              AffineTransform rot;
              Mat original = rot.Rotate(image,angle,n2,n3);
              namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
              
              Mat m = getRotationMatrix2D(Point2f(image.cols/2,image.rows/2),angle,1);
              Mat dst;
              warpAffine(image,dst,m,Size(image.rows,image.cols));
              namedWindow("Inbuilt",WINDOW_AUTOSIZE);
              imshow("Inbuilt",dst);
              imwrite("InbuiltRotate.jpg",dst);
              waitKey(0);
              cout<<"PSNR Recieved from inbuilt and my implemenatation is : "<<getPSNR(original,dst)<<endl;
              //Write psnr function
          }
              break;
              
           case 4:
           {
              cout<<"For Shearing the image : \n";
              cout<<"Please Enter the Parameters : \n";
              float a,b;
              cout<<"Enter horizontal Shear : ";
              cin>>a;
              cout<<"Enter Vertical Shear : ";
              cin>>b;
              AffineTransform sh;
              Mat original = sh.Shear(image,a,b);
               namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
              float tp[2][3] = {{1,a,0},{b,1,0}};
              Mat m = Mat(2,3,CV_32FC1,tp);
              Mat dst;
              warpAffine(image,dst,m,Size(image.cols+a*image.rows,image.rows+b*image.cols));
              namedWindow("Inbuilt",WINDOW_AUTOSIZE);
              imshow("Inbuilt",dst);
              imwrite("InbuiltShear.jpg",dst);
              waitKey(0);
              cout<<"PSNR Recieved from inbuilt and my implemenatation is : "<<getPSNR(original,dst)<<endl;
            }
              break;
              
          case 5:
          {
              cout<<"For Reconstruction of the image : \n";
              cout<<"Please Enter the Parameters : \n";
              
              vector<pair<Point2f,Point2f> > points;
              Point2f point1,point2;
              float x1,y1;
              int i;
              
              for(i=0;i<4;i++){
                      cout<<"Enter Known Image 2 coordinate : ";
                      cin>>x1>>y1;
                      point1.x = x1;
                      point1.y = y1;
                      cout<<"Enter Unknown Image 2 coordinate : ";
                      cin>>x1>>y1;
                      point2.x = x1;
                      point2.y = y1;
                      points.push_back(make_pair(point1,point2));
              }
              AffineTransform tie;
              Mat original = tie.TiePoints(image,points);
               namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
              waitKey(0);
              //cout<<"PSNR Recieved from inbuilt and my implemenatation is : ";
           }
              break;
              
           case 6:
           {
              cout<<"For Image negative: \n";
              //cout<<"Please Enter the Parameters : \n";
              ContrastEnhancement neg;
              Mat original = neg.ImageNeg(image);
               namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
              waitKey(0);
              //cout<<"PSNR Recieved from inbuilt and my implemenatation is : ";
            }
              break;
              
           case 7:
           {
              cout<<"For Log Transform : \n";
              float constant;
              cout<<"Enter the parameter c : \n";
              cin>>constant;
              ContrastEnhancement ln;
              Mat original = ln.LogTransform(image);
               namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
              waitKey(0);
            }
              break;
           case 8:
           {
              cout<<"For Power Transform : \n";
              double alp;
              cout<<"Enter the parameter gamma : \n";
              cin>>alp;
              ContrastEnhancement po;
              Mat original = po.PowerTransform(image,alp);
               namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
              waitKey(0);
           }
              break;
              
           case 9:
           
           {
              cout<<"For PieceWise Linear Transform : \n";
              int r1,r2,s1,s2;
              char c;
              cout<<"Do you want to enter the 4 parameters r1,r2,s1,s2 y/n : ";
              cin>>c;
              ContrastEnhancement piLine;
              Mat original;
              if(c == 'y'){
                cout<<"Enter the parameters in order r1,r2,s1,s2 : ";
                cin>>r1>>r2>>s1>>s2;
                original = piLine.PiecewiseLinear(image,r1,r2,s1,s2);
              }else
                original = piLine.PiecewiseLinear(image);
                 namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
                waitKey(0);
            }
                break;
                
            case 10:
            {
               cout<<"For BitPlane Slicing : \n";
               cout<<"How many planes you want to enter Combination slice will be output : ";
               int no_plane,j;
               vector<int> planes;
               cin>>no_plane;
               cout<<"Enter the planes : ";
               for(j=0;j<no_plane;j++){
                   int k;
                   cin>>k;
                   planes.push_back(k);
               }  
               ContrastEnhancement biPlane;
               Mat original = biPlane.BitSlice(image,planes);
                namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
               waitKey(0);
             }
               break;
               
               
             case 11:
             {
                cout<<"For Equilization of HistoGram : \n";
                Histogram equi;
               
               // equalizeHist(A,B);
                Mat original = equi.HistogramEqui(image);
                 namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
              Mat dst;
              equalizeHist(image,dst);
              namedWindow("Inbuilt",WINDOW_AUTOSIZE);
               imshow("Inbuilt",dst);
              imwrite("InbuiltHist.jpg",dst);
                waitKey(0);
              cout<<"PSNR Recieved from inbuilt and my implemenatation is : "<<getPSNR(original,dst)<<endl;
             }
                break;
                
             case 12:
             {
                cout<<"For Match Equilization : \n";
                string imgPath;
                cout<<"Enter Image for matching histo : \n";
                cin>>imgPath;
                Mat srcMatch = imread(imgPath,0);
                Histogram match;
                Mat original = match.MatchHistogramEqui(image,srcMatch);
                 namedWindow("Original",WINDOW_AUTOSIZE);
              imshow("Original",original);
                waitKey(0);
             }
                break;
                
              case 13:
              {
                cout<<"For Adaption of Histogram : \n";
                cout<<"Enter the congruence square size : ";
                int cong;
                cin>>cong;
                Histogram adapt;
                Mat original = adapt.AdjHistogramEqui(image,cong);
                 namedWindow("Original",WINDOW_AUTOSIZE);
                imshow("Original",original);
                waitKey(0);
              }
                break;
              
    
         default:
             cout<<"Invalid Number\n";
    }
   
    
 
    waitKey(0);
    }   
    
    return 0;

}


