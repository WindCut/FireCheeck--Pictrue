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
void edgedeal(Mat &inImg);
bool IsSmoke(Mat &inImg);
void RemoveFSmoke(Mat &Smoke, Mat fire);
void ChangeFlags(Mat &src);
int main()
{
//	string filepath = "Smoke//Smoke7.jpg";
	string filepath = "firepic//2.jpg";
	Mat inputImg = imread(filepath,1);
  	CheckColor(inputImg);
	return 0;
}
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
	//用颜色处理识别烟雾以及火焰区域
	for (int i = 0; i < inImg.rows; i ++)
	{
		for (int j = 0; j < inImg.cols; j ++)
		{
			float B,G,R;
			B = multiRGB[0].at<uchar>(i,j); //每个像素的R,G,B值
			G = multiRGB[1].at<uchar>(i,j);
			R = multiRGB[2].at<uchar>(i,j);	

		
			int maxValue = max(max(B,G),R);
			int minValue = min(min(B,G),R);

			double S = (1-3.0*minValue/(R+G+B));

			//R > RT  R>=G>=B  S>=((255-R)*ST/RT)
		if (R > redThre && R >= G && G >= B && S > 0.20 && S > ((255 - R) * saturationTh / redThre))
{
	fireImg.at<uchar>(i, j) = 255;
}
		else
		{
			fireImg.at<uchar>(i, j) = 0;
		}
		//	 	if (R >= 200 && G >= 200 && G >= B&&B <= 180 ) fireImg.at<uchar>(i, j) = 255;
		if (R <= 80 && G <= 80 && G <= B&&B <= 80)// && dist(R, G, B)<1)
		//  if (dist(R,G,B)<10) 
		smokeImg.at<uchar>(i, j) = 255;
		else
		smokeImg.at<uchar>(i, j) = 0;
	
		}
	}
	dilate(fireImg, fireImg, Mat(5, 5, CV_8UC1));//像的膨胀操作，将可能分散的点连接起来
	imshow("烟雾识别", smokeImg);
	edgedeal(smokeImg);//所有的边缘处理过程
	imshow("火焰识别", fireImg);
	RemoveFSmoke(smokeImg, fireImg);//去除非烟雾部分
	imshow("去除假烟雾", smokeImg);
	DrawFire(inImg, smokeImg, 255, 0, 0);
	DrawFire(inImg, fireImg, 0, 255, 0);
	imshow("FinalShow", inImg);
	waitKey(0);
	return fireImg;

}
void RemoveFSmoke(Mat &Smoke, Mat fire)
{
	int n = Smoke.rows;
	int m = Smoke.cols;
	int low = 0;

	//printf("\n%d %d\n", n, m);
	printf("!!!!");
	for (int i = 0; i < m; i++)
		for (int j = n - 1; j > 0; j--)
		{
			if (fire.at<uchar>(j, i) > 0)
			{
				//printf("low=%d\n",low);
				for (int k = j; k < n; k++)
					Smoke.at<uchar>(k, i) = 0;
				if (low == 0)
				{
					//printf("????");
					printf("%d %d",j,i);
					for (int k = 0; k <= i; k++)
					  for (int t=j;t<n;t++)
						Smoke.at<uchar>(t, k) = 0;
				}
				low = j;
				break;
			}
			if (j==0 && low>0)
				for (int k = low; k < n; k++)
					Smoke.at<uchar>(k, i) = 0;
		}
	if (low != 0)
		for (int i = m - 1; i--; i > 0)
			if (fire.at<uchar>(low, i) > 0) break;
			else
			{
				for (int k = low; k < n; k++)
					Smoke.at<uchar>(k, i) = 0;
			}
	//Smoke.at<uchar>(low,i) = 0;
	//imshow("1",Smoke);
	IsSmoke(Smoke);
}
void ChangeFlags(Mat &src)
{
	Mat dst = src.clone();
	//Mat gray_src=src.clone();;// = src.clone();
	threshold(dst, src, 99, 255, 0);
}
void edgedeal(Mat &inImg)
{
	Mat smokeImg;
	smokeImg = inImg.clone();
	//imshow("Firstsmoke", smokeImg);
	Mat  tmp;
	tmp.create(inImg.size(), CV_8UC1);
	dilate(smokeImg, smokeImg, Mat(10, 10, CV_8UC1));//腐蚀
	delete_jut(smokeImg, tmp, 10, 10, 1);//去除突起
	imageblur(tmp, smokeImg, Size(20, 20), 125);//平滑边缘
	Mat m_ResImg;
	Canny(smokeImg, m_ResImg, 50, 200);
	//if (IsSmoke(smokeImg)) printf("This is TRUE!\n");
	//else printf("this is FAKE!\n");
}
bool IsSmoke(Mat &inImg)
{
	int S = 0, L = 0;
	int q[4000][2];
	Mat gray_src;
	cvtColor(inImg, gray_src, CV_BGR2GRAY);//转化为灰度图
	inImg = gray_src.clone();
	int l = 0, r = 0;
	int n = inImg.rows;
	int m = inImg.cols;
	for (int i = 0; i < inImg.rows; i++)
	{
		for (int j = 0; j < inImg.cols; j++)
			if (inImg.at<uchar>(i, j) == 255)
			{
				l = 0; r = 1;
				S = 1; L = 0;
				q[l][0] = i; q[l][1] = j; inImg.at<uchar>(i, j) = 100;
				while (l != r)
				{
					int t = 0;
					int x = q[l][0] + 1; int y = q[l][1];// +1;
					if (x >= 0 && x < n && y >= 0 && y < m)
					{
						if (inImg.at<uchar>(x, y) == 255)
						{
							S++;   q[r][0] = x; q[r][1] = y; inImg.at<uchar>(x, y) = 100; r = (r++) % 3777;
						}
						else if (inImg.at<uchar>(x, y) == 0) t++;

					}
				   else t++;
					x = q[l][0] - 1; y = q[l][1];// +1;
					if (x >= 0 && x < n && y >= 0 && y < m)
				{
					if (inImg.at<uchar>(x, y) == 255)
					{
						S++;   q[r][0] = x; q[r][1] = y; inImg.at<uchar>(x, y) = 100; r = (r++) % 3777;
					}
					else if (inImg.at<uchar>(x, y) == 0) t++;

				}
					else t++;
					x = q[l][0]; y = q[l][1] - 1;
					if (x >= 0 && x < n && y >= 0 && y < m)
				{
					if (inImg.at<uchar>(x, y) == 255)
					{
						S++;   q[r][0] = x; q[r][1] = y; inImg.at<uchar>(x, y) = 100; r = (r++) % 3777;
					}
					else if (inImg.at<uchar>(x, y) == 0) t++;

				}
					else t++;
					x = q[l][0] ; y = q[l][1] +1;
					if (x >= 0 && x < n && y >= 0 && y < m)
				{
					if (inImg.at<uchar>(x, y) == 255)
					{
						S++;   q[r][0] = x; q[r][1] = y; inImg.at<uchar>(x, y) = 100; r = (r++) % 3777;
					}
					else if (inImg.at<uchar>(x, y) == 0) t++;

				}
					else t++;
					if (t > 0) L++;
					l = (l++) % 3777;
				}
			double tmp = (L*1.0*L) / S;//标准圆形此时应为4*pi  越大则说明越不规则
			
			int num = 250;
			if (tmp < 20)  num = 0;
			Mat tmpmat = inImg.clone();
			//cvThreshold(tmp, inImg, 100, 255, CV_THRESH_BINARY);
			//printf("S=%d L=%d 输出=%.2f %d \n", S, L, tmp,num);
			{
				l = 0; r = 1;
				inImg.at<uchar>(i, j) = num;
				q[l][0] = i; q[l][1] = j;
				while (l != r)
				{
					int x = q[l][0] + 1; int y = q[l][1];// +1;
					if ( x >= 0 && x<n && y >= 0 && y<m)
						if (inImg.at<uchar>(x, y) == 100)
					{    
                         q[r][0] = x; q[r][1] = y; inImg.at<uchar>(x, y) = num; r = (r++) % 3777;
					}
					
					x = q[l][0] - 1; y = q[l][1];// - 1;
					if (x >= 0 && x<n && y >= 0 && y<m)
						if (inImg.at<uchar>(x, y) == 100)
					{
						q[r][0] = x; q[r][1] = y; inImg.at<uchar>(x, y) = num; r = (r++) % 3777;
					}
					
					x = q[l][0]; y = q[l][1] + 1;
					if (x >= 0 && x<n && y >= 0 && y<m)
						if (inImg.at<uchar>(x, y) == 100)
					{
						 q[r][0] = x; q[r][1] = y; inImg.at<uchar>(x, y) = num; r = (r++) % 3777;
					}
					
					x = q[l][0] ; y = q[l][1] - 1;
					if (x >= 0 && x<n && y >= 0 && y<m)
						if (inImg.at<uchar>(x, y) == 100)
					{
						 q[r][0] = x; q[r][1] = y; inImg.at<uchar>(x, y) = num; r = (r++) % 3777;
					}

					l = (l++) % 3777;
					//printf("%d %d\n", q[l][0],q[l][1]);
				}
			}
			//printf("已处理\n");;
		}
		
	}
	
	ChangeFlags(inImg);
	/*for (int i = 0; i < n; i++)
		for (int j = 0; j <m; j++)
			if (inImg.at<uchar>(i, j) > 0)
				inImg.at<uchar>(i, j) == 255;*/

	return true;
	//for (int i = 0; i < inImg.rows; i++)
	//{

	//	for (int j = 0; j < inImg.cols; j++)
	//	if (inImg.at<uchar>(i, j) > 0)
	//	{
	//		
	//		S++;
	//		if (i > 0 && (inImg.at<uchar>(i - 1, j) == 0)) { L++; continue;	}
	//		if (j > 0 && (inImg.at<uchar>(i, j - 1)== 0)) { L++; continue; }
	//		if (i < n - 1 && (inImg.at<uchar>(i+1, j)== 0)) { L++; continue; }
	//		if (j < m - 1 && (inImg.at<uchar>(i, j+1)== 0)) { L++; continue; }

	//	}
	//}
	//double tmp = (L*1.0*L)/S;//标准圆形此时应为4*pi  越大则说明越不规则
	//printf("S=%d L=%d 输出=%.2f\n",S,L,tmp);
	//if (tmp > 20) return true;//大于12.56即为不是圆形
	//else return false;
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