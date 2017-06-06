#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;
#define  _ShowImage 1
////////////////////////////////////////////////////////
/**
* @name  getAllImgsFilePath
* @brief 获取指定路径下的所有文件全路径(还是这个方便啊.)
* @param[in] szDir:指定路径(比如C:\\_Sunyard\\Project\\浙江农信\\data\\照片\\Merge表格\\*.*";)
* @param[in/out]  testImgFiles,所有文件全路径存在这里
* @return 0 成功 / -1 失败
* @author [lintao 2015-10-09]
**/
////////////////////////////////////////////////////////
int getAllImgsFilePath(char *szDir, vector<string>& testImgFiles)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile((szDir), &ffd);
	//testImgFiles.push_back(ffd.cFileName);//do 是先做..会push进去的
	// List all the files in the directory with some info about them.
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
			//cout<<ffd.cFileName<<endl;
		}
		else
		{
			//_tprintf(TEXT("  %s  \n"), ffd.cFileName);
			//cout<<ffd.cFileName<<endl;
			string searchImgDirWithType = szDir;                         //搜索目录带有搜索文件类型的
			int npos = searchImgDirWithType.find_last_of("\\");           //
			string searchImgDir = searchImgDirWithType.substr(0, npos + 1); //纯目录    //substr 返回pos开始的n个字符组成的字符串
			string imgWholePath = searchImgDir + ffd.cFileName;
			testImgFiles.push_back(imgWholePath);
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//判断轮廓是不是一个圆，通过轮廓的面积pi*r*r的差异，判断是不是圆。
//contour：输入轮廓
//minRadius: 半径长度至少大于minRadius
//////////////////////////////////////////////////////////////////////////
bool isCircleContour(vector<Point>& contour, double minRadius)
{
	bool isCircle = false;
	//double perimeter = arcLength(contour,TRUE);
	double perimeter = arcLength(contour, false);
	double perimeter2 = arcLength(contour, true);
	Rect rect = boundingRect(contour);
	double r = (rect.width + rect.height) / 4.0;	//半径约等于轮廓外接矩的边长/2
	double area = contourArea(contour);
	double pi = 3.1415926;
	//cout<<"x/y/width/height\t"<<rect.x<<"  "<<rect.y<<"  "<<rect.width<<"  "<<rect.height<<endl;
	//cout<<"perimeter:FALSE\t"<<perimeter<<endl;
	//cout<<"perimeter2:TRUE\t"<<perimeter2<<endl;
	//cout<<"area:\t"<<area<<endl;
	//cout<<"r: "<<r<<"  计算出周长: "<<2*pi*r<<"  计算出面积: "<<pi*r*r<<endl;
	double perimeterDiffRatio = fabs(2 * pi*r - perimeter)*2.0 / (2 * pi*r + perimeter) * 100;
	double areaDiffRatio = fabs(pi*r*r - area)*2.0 / (pi*r*r + area) * 100;
	double sideDiffRatio = abs(rect.width - rect.height)*2.0 / (rect.width + rect.height) * 100;
	//CString CstrInfo3;
	//CstrInfo3.Format(_T(" perimeterDiffRatio: %f\n areaDiffRatio: %f \n  sideDiffRatio:%f"),perimeterDiffRatio,areaDiffRatio,sideDiffRatio);
	//AfxMessageBox(CstrInfo3);
	if (r>minRadius)
	{
		cout << "周长差异百分比:  " << perimeterDiffRatio << "%" << endl;;
		cout << "面积差异百分比:  " << areaDiffRatio << "%" << endl;
		cout << "边长差异百分比:  " << sideDiffRatio << "%" << endl;;
		cout << endl << endl;
	}
	//原始的
	//const double areaDiffThresh = 18.0;       //目前统计的，最大的面积差2.58%
	//const double perimeterDiffThresh = 15;    //目前统计的，最大的周长差5.65%
	//const double sideDiffThresh = 15;     //目前统计的，最大的边长差7.3%
	//const double avgDiffThresh = 15.0;        //三个差异比值的加权平均要求小于5.0;
	const double areaDiffThresh = 5;        //目前统计的，最大的面积差2.58%
	const double perimeterDiffThresh = 7.25; //目前统计的，最大的周长差5.65%
	const double sideDiffThresh = 10;       //目前统计的，最大的边长差7.3%
	const double avgDiffThresh = 5.0;       //三个差异比值的加权平均要求小于5.0;
	double avdDiffRatio = (areaDiffRatio + perimeterDiffRatio + sideDiffRatio) / 3.0;
	//计算出的轮廓周长是半径的2*pi倍数
	if ((areaDiffRatio <areaDiffThresh)
		&& (perimeterDiffRatio <perimeterDiffThresh)
		&& (sideDiffRatio <sideDiffThresh)
		&& (avdDiffRatio<avgDiffThresh)
		&& (r>minRadius))
	{
		isCircle = true;
	}
	return isCircle;
}
int countShootingRingsNum(Mat& orgImg)
{
	if (orgImg.empty())
		return -1;
	//转换到灰度图
	Mat grayImage;
	if (orgImg.channels() == 3)
		cvtColor(orgImg, grayImage, CV_BGR2GRAY);
	if (orgImg.channels() == 1)
		grayImage = orgImg.clone();
	imshow("grayImage", grayImage);
	//模块：二值化
	int isBlackCharacter = false;
	Mat binImg;
	double otsuThresh = 0.0;
	if (isBlackCharacter)
		otsuThresh = threshold(grayImage, binImg, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
	else
		otsuThresh = threshold(grayImage, binImg, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	//CString CstrInfo3;
	//CstrInfo3.Format(_T(" otsuThresh: %f"),otsuThresh);
	//AfxMessageBox(CstrInfo3);
	imshow("binImg", binImg);
	//腐蚀膨胀
	int erodeSize = 3;
	int dilateSize = 3;
	Mat elementForErode = getStructuringElement(MORPH_RECT, Size(erodeSize, erodeSize), Point(-1, -1));
	Mat elementForDilate = getStructuringElement(MORPH_RECT, Size(dilateSize, dilateSize), Point(-1, -1));
	dilate(binImg, binImg, elementForDilate);
	//erode(binImg ,binImg , elementForErode );
	imshow("膨胀腐蚀后的Binary Image", binImg);
	//waitKey();
	//模块：提取轮廓(保留轮廓面积大于_tooLessPixels)
	vector< vector<Point> > contours;   // 轮廓
	vector< vector<Point> > filterContours; // 筛选后的轮廓
	vector< Vec4i > hierarchy;    // 轮廓的结构信息
	contours.clear();
	hierarchy.clear();
	filterContours.clear();
	Mat binaryImageCopy1 = binImg.clone();
	//findContours(binaryImageCopy1, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	findContours(binaryImageCopy1, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	// 去除伪轮廓: 轮廓上像素点数太少的
	const int _tooLessPixels = 10 * 10;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (fabs(contourArea(contours[i])) > _tooLessPixels)
		{
			//先都push进去
			filterContours.push_back(contours[i]);
		}
	}
	Mat rgbImg = binImg.clone();
	cvtColor(rgbImg, rgbImg, CV_GRAY2BGR);
	//binaryImage.setTo(0);
	drawContours(rgbImg, filterContours, -1, Scalar(0, 255, 0), 5); //8, hierarchy);
																	//cvNamedWindow("filterContours",0);
	imshow("filterContours", rgbImg);
	//模块：外接矩的宽高比比例失调的过滤掉
	const double w2hThresh2 = 2.0;      //外接矩的宽高比比例大于3过滤掉
	const double h2wThresh2 = 2.0;      //外接矩的宽高比比例大于3过滤掉
	for (int i = 0; i<filterContours.size(); i++)
	{
		Rect tmpRect = boundingRect(filterContours[i]);
		if ( //(tmpRect.width>maxRectWidth)
			 // (tmpRect.height>maxRectHeight)
			((double)tmpRect.width / (double)tmpRect.height > w2hThresh2)
			|| ((double)tmpRect.height / (double)tmpRect.width > h2wThresh2)
			)
		{
			filterContours.erase(filterContours.begin() + i);
			i--;//删除掉一个后，索引回退一个
		}
	}
	//不是圆的过滤掉
	double minRadius = 20 / 2;            //目前最小的圆直径30
	for (int i = 0; i<filterContours.size(); i++)
	{
		if (!isCircleContour(filterContours[i], minRadius))
		{
			//selectContours.push_back(filterContours[i]);
			filterContours.erase(filterContours.begin() + i);
			i--;//删除掉一个后，索引回退一个
		}
	}
	Mat rgbImg2 = binImg.clone();
	cvtColor(rgbImg2, rgbImg2, CV_GRAY2BGR);
	//binaryImage.setTo(0);
	drawContours(rgbImg2, filterContours, -1, Scalar(0, 255, 0), 5); //8, hierarchy);
																	 //cvNamedWindow("filterContours2",0);
	imshow("filterContours2", rgbImg2);
	Mat grayImg;
	cvtColor(orgImg, grayImg, CV_BGR2GRAY);
	Mat diffImg;
	Mat greenImg;
	absdiff(grayImg, greenImg, diffImg);
	normalize(diffImg, diffImg, 0, 255, NORM_MINMAX);
	imshow("绿色通道和灰度图差异图图", diffImg);
	return 0;
}
int testCountShootingRingsNum()
{
	//char *szDir = "C:\\Projects\\ComputerVisionProject\\网上看到的项目\\数打靶的环数\\data\\webData\\*.*";
	char *szDir = "C:\\Projects\\ComputerVisionProject\\网上看到的项目\\数打靶的环数\\data\\*.*";
	vector<string> testImgFiles;
	getAllImgsFilePath(szDir, testImgFiles);
	//system("time");
	for (int i = 0; i<testImgFiles.size(); i++)
	{
		//输出文件名
		int npos = testImgFiles[i].find_last_of("\\");
		string fileName = testImgFiles[i].substr(npos + 1, testImgFiles[i].length() - (npos + 1));  //获取目录
		cout << fileName << endl;
		Mat orgImg = imread(testImgFiles[i]);
		if (orgImg.empty())
			continue;
		imshow("orgImg", orgImg);
		countShootingRingsNum(orgImg);
		waitKey();
	}
	system("time");
	return 0;
}
void main1()
{
	testCountShootingRingsNum();
}

