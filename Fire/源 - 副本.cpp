#include "core/core.hpp"      
#include "highgui/highgui.hpp"      
#include "imgproc/imgproc.hpp"  
#include "video/tracking.hpp"  
#include<iostream>      
//思路 火焰分布的特征焰心 ，，，，， 烟雾特征  上大下小   火焰 上小夏大  
// 局限  只能分析常规火焰且在白天
using namespace cv;
using namespace std;
Mat CheckColor(Mat &inImg);
void DrawFire(Mat &inputImg, Mat fireImg, int B, int G, int R);
double dist(int r, int g, int b);
bool countsmoke( Mat fireImg);
void delete_jut(Mat& src, Mat& dst, int uthreshold, int vthreshold, int type);
void imageblur(Mat& src, Mat& dst, Size size, int threshold);
int main()
{
	string filepath = "fire9.jpg";
	Mat inputImg = imread(filepath,1);
	Mat m_ResImg;
	Mat gray;
	//Canny边缘检测
	//cvCvtColor(inputImg, gray, CV_BGR2GRAY);
	//cvtColor(inputImg, gray, CV_RGB2GRAY);//把图片转化为灰度图
	//imshow("灰度图 ", gray);
	Canny(gray, m_ResImg, 50, 200);
	
	//namedWindow("Canny边缘检测", 1);
	//imshow("Canny边缘检测", m_ResImg);
	//waitKey(0);
  	CheckColor(inputImg);
	return 0;
}
//////////////////////////////////
//The Color Check is According to "An Early Fire-Detection Method Based on Image Processing"
//The Author is:Thou-Ho (Chao-Ho) Chen, Ping-Hsueh Wu, and Yung-Chuen Chiou
//////////////////////////////////////
Mat CheckColor(Mat &inImg)
{
	Mat fireImg;
	fireImg.create(inImg.size(),CV_8UC1);
	Mat smokeImg;
	smokeImg.create(inImg.size(), CV_8UC1);
	int redThre = 135; // 115~135
	int saturationTh = 45; //55~65
	Mat multiRGB[3];
	int a = inImg.channels();
	split(inImg,multiRGB); //将图片拆分成R,G,B,三通道的颜色

	for (int i = 0; i < inImg.rows; i ++)
	{
		for (int j = 0; j < inImg.cols; j ++)
		{
			float B,G,R;
			B = multiRGB[0].at<uchar>(i,j); //每个像素的R,G,B值
			G = multiRGB[1].at<uchar>(i,j);
			R = multiRGB[2].at<uchar>(i,j);	

			/*B = inImg.at<uchar>(i,inImg.channels()*j + 0); //另一种调用图片中像素RGB值的方法
			G = inImg.at<uchar>(i,inImg.channels()*j + 1);
			R = inImg.at<uchar>(i,inImg.channels()*j + 2);*/

			int maxValue = max(max(B,G),R);
			int minValue = min(min(B,G),R);

			double S = (1-3.0*minValue/(R+G+B));

			//R > RT  R>=G>=B  S>=((255-R)*ST/RT)
			if(R > redThre && R >= G && G >= B && S >0.20 && S >((255 - R) * saturationTh/redThre))
			{
				fireImg.at<uchar>(i,j) = 255;

			}
			else
			{
				fireImg.at<uchar>(i,j) = 0;
			}
		 	if (R >= 200 && G >= 200 && G >= B&&B <= 180 ) fireImg.at<uchar>(i, j) = 255;
			if (R <= 80 && G <=80 && G <= B&&B <= 100  )// && dist(R, G, B)<10
		//  if (dist(R,G,B)<10) 
				smokeImg.at<uchar>(i, j) = 255;
			else
				smokeImg.at<uchar>(i, j) = 0;

		}
	}

	//dilate(fireImg,fireImg,Mat(5,5,CV_8UC1));
	Mat  tmp;
	tmp.create(inImg.size(), CV_8UC1);
	dilate(smokeImg, smokeImg, Mat(10, 10, CV_8UC1));
	dilate(inImg, inImg, Mat(10, 10, CV_8UC1));
	delete_jut(inImg, tmp, 10, 10, 1);
	imageblur(tmp, inImg, Size(20, 20), 125);
	imshow("原图", inImg);
	//delete_jut(smokeImg, tmp, 10,10, 1);
	//Size size = (10, 10);
	//imageblur(tmp,smokeImg,  Size(20,20),125);
	//imshow("fire",fireImg);
	//if (countsmoke(smokeImg))
	  imshow("smoke", smokeImg);
	  Mat m_ResImg;
	  Canny(smokeImg,m_ResImg, 50, 200);
	  imshow("Canny边缘检测", m_ResImg);
	//if (countfire(smokeImg))
	  //DrawFire(inImg,fireImg,0,255,0);
	DrawFire(inImg, smokeImg, 255, 0,  0);
	imshow("FinalShow", inImg);
	//waitKey(0);
	waitKey(0);
	return fireImg;
}

void DrawFire(Mat &inputImg,Mat fireImg, int B, int G,int R)
{
	vector<vector<Point>> contours_set;//保存轮廓提取后的点集及拓扑关系

	findContours(fireImg,contours_set,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);	

	Mat result0;
	Scalar holeColor;
	Scalar externalColor;

	vector<vector<Point> >::iterator iter = contours_set.begin() ;
	for(; iter!= contours_set.end(); )
	{
		Rect rect = boundingRect(*iter );
		float radius;  
		Point2f center;  
		minEnclosingCircle(*iter,center,radius);  
		
		if (rect.area()> 0)		
		{

			rectangle(inputImg,rect,Scalar(B,G,R));	
			++ iter;

		}
		else
		{
			iter = contours_set.erase(iter);
		}
	}

	
}
bool countsmoke(Mat fireImg)
{
	int all=0,mayfire=0;
	int precount = 0,count=0;
	int startrow, endrow;
	for (startrow = 0; startrow < fireImg.rows; startrow++)
	{
		bool flag = 0;
		for (int j = 0; j < fireImg.cols; j++)
		{
			int color = fireImg.at<uchar>(startrow, j);
			if (color == 255)
				flag = 1;

		}
		if (flag) break;
	}
	for (endrow = fireImg.rows-1; endrow >0; endrow--)
	{
		bool flag = 0;
		for (int j = 0; j < fireImg.cols; j++)
		{
			int color = fireImg.at<uchar>(endrow, j);
			if (color == 255)
				flag = 1;

		}
		if (flag) break;
	}
	if (endrow < startrow) return false;

	int e = (endrow - startrow) / 3;
	for (int i = startrow; i < startrow + e; i++)
	{
		for (int j = 0; j < fireImg.cols; j++)
		{
			int color = fireImg.at<uchar>(i, j);
			if (color == 255)
				count++;
		}

	}
	precount = count;
	for (int i = startrow + e; i <  startrow + e*2; i++)
	{
		for (int j = 0; j < fireImg.cols; j++)
		{
			int color = fireImg.at<uchar>(i, j);
		    if(color==255)
			 {
				 count++;
			 }
		}
	}
	if (count > precount) return false;
	precount = count;
	for (int i = startrow + e*2; i <=endrow; i++)
	{
		for (int j = 0; j < fireImg.cols; j++)
		{
			int color = fireImg.at<uchar>(i, j);
			if (color == 255)
			{
				count++;
			}
		}
	}
	if (count > precount) return false;
	return true;
}
double dist(int r,int g,int b)
{
	//设a=(X1,Y1,Z1),b=(X2,Y2,Z2),
	//a×b = （Y1Z2 - Y2Z1, Z1X2 - Z2X1, X1Y2 - X2Y1）
	//(1, 2, 3)×(4, 5, 6) = (12 - 15, 12 - 6, 5 - 8) = （ - 3, 6, -3）
	//先叉乘(1,1,1)再算距离
	int R = g-b;
	int G = b-r;
	int B = r-g;
	double result = sqrt((G*G+R*R+B*B)/3.0);
	return result;
}
//去除二值图像边缘的突出部  
//uthreshold、vthreshold分别表示突出部的宽度阈值和高度阈值  
//type代表突出部的颜色，0表示黑色，1代表白色  
void delete_jut(Mat& src, Mat& dst, int uthreshold, int vthreshold, int type)
{
	int threshold;
	src.copyTo(dst);
	int height = dst.rows;
	int width = dst.cols;
	int k;  //用于循环计数传递到外部
	for (int i = 0; i < height - 1; i++)
	{
		uchar* p = dst.ptr<uchar>(i);
		for (int j = 0; j < width - 1; j++)
		{
			if (type == 0)
			{
				//行消除
				if (p[j] == 255 && p[j + 1] == 0)
				{
					if (j + uthreshold >= width)
					{
						for (int k = j + 1; k < width; k++)
							p[k] = 255;
					}
					else
					{
						for (k = j + 2; k <= j + uthreshold; k++)
						{
							if (p[k] == 255) break;
						}
						if (p[k] == 255)
						{
							for (int h = j + 1; h < k; h++)
								p[h] = 255;
						}
					}
				}
				//列消除
				if (p[j] == 255 && p[j + width] == 0)
				{
					if (i + vthreshold >= height)
					{
						for (k = j + width; k < j + (height - i)*width; k += width)
							p[k] = 255;
					}
					else
					{
						for (k = j + 2 * width; k <= j + vthreshold*width; k += width)
						{
							if (p[k] == 255) break;
						}
						if (p[k] == 255)
						{
							for (int h = j + width; h < k; h += width)
								p[h] = 255;
						}
					}
				}
			}
			else  //type = 1
			{
				//行消除
				if (p[j] == 0 && p[j + 1] == 255)
				{
					if (j + uthreshold >= width)
					{
						for (int k = j + 1; k < width; k++)
							p[k] = 0;
					}
					else
					{
						for (k = j + 2; k <= j + uthreshold; k++)
						{
							if (p[k] == 0) break;
						}
						if (p[k] == 0)
						{
							for (int h = j + 1; h < k; h++)
								p[h] = 0;
						}
					}
				}
				//列消除
				if (p[j] == 0 && p[j + width] == 255)
				{
					if (i + vthreshold >= height)
					{
						for (k = j + width; k < j + (height - i)*width; k += width)
							p[k] = 0;
					}
					else
					{
						for (k = j + 2 * width; k <= j + vthreshold*width; k += width)
						{
							if (p[k] == 0) break;
						}
						if (p[k] == 0)
						{
							for (int h = j + width; h < k; h += width)
								p[h] = 0;
						}
					}
				}
			}
		}
	}
}
void imageblur(Mat& src, Mat& dst, Size size, int threshold)
{
	int height = src.rows;
	int width = src.cols;
	blur(src, dst, size);
	for (int i = 0; i < height; i++)
	{
		uchar* p = dst.ptr<uchar>(i);
		for (int j = 0; j < width; j++)
		{
			if (p[j] < threshold)
				p[j] = 0;
			else p[j] = 255;
		}
	}
	//imshow("Blur", dst);
}