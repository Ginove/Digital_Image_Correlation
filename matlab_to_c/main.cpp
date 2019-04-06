#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "cplot.h"
#include "cpcorr.cpp"
#include "grid_generator.h"

using namespace cv;
using namespace std;

static void automate_image();  //处理数据
static vector<string> filelist_generator(string Prefix, string suffix, int img_num);  //产生文件名字符串集合 参数一 文件名前缀 ，参数2 后缀， 参数3 文件数量
static void fitLine(); //绘图
static void starin_plot(Mat validxy, Mat displxy, const char *title); //应变系数曲线描绘
static void displxy_plot(Mat displxy, const char *title);//平均位移变化描绘
extern void grid_generator(string image_name, int x_pixels, int y_pixels); //选取图片，在图片上画网格，参数1 文件名，参数2 x_pixels -> X方向间隔 ,参数3 y_pixels -> Y方向间隔

static vector<string> filelist_generator(string Prefix, string suffix ,int img_num) {

	;
	vector<string> filenamelist(img_num);
	for (int i = 0; i < img_num; i++)
	{
		if (i < 10)
			filenamelist[i] = "image/" + Prefix + "0" + to_string(i) + suffix;
		else if (i >= 10 && i < 100)
			filenamelist[i] = "image/" + Prefix + to_string(i) + suffix;

	}
	return filenamelist;
}

void automate_image() {
	try
	{
		Mat grid_x, grid_y;
		FileStorage fs_grid_x("grid_x.xml", FileStorage::READ); //从grid.xml中取出grid_x,grid_y的值
		FileStorage fs_grid_y("grid_y.xml", FileStorage::READ);
		fs_grid_x["grid_x"] >> grid_x;
		fs_grid_y["grid_y"] >> grid_y;
		fs_grid_x.release();
		fs_grid_y.release();

		int M = grid_x.rows;

		Mat movingPoints(M, 2, CV_64F);
		Mat fixedPoints(M, 2, CV_64F);

		grid_x.copyTo(movingPoints.col(0));
		grid_y.copyTo(movingPoints.col(1));
		grid_x.copyTo(fixedPoints.col(0));
		grid_y.copyTo(fixedPoints.col(1));

		int img_num = 67;
		string Prefix = "ctest0";
		string suffix = ".tif";
		vector<string> filenamelist(img_num);
		filenamelist = filelist_generator(Prefix, suffix,img_num);


		FileStorage fs_validx("validx.xml", FileStorage::WRITE);
		FileStorage fs_validy("validy.xml", FileStorage::WRITE);

		Mat validx(M, img_num - 1, CV_64F);
		Mat validy(M, img_num - 1, CV_64F);

		Mat fixed = imread(filenamelist[0], IMREAD_GRAYSCALE);
		for (int idx = 0; idx < img_num - 1; idx++) {
			cout << "Process Image " << idx + 1 << endl;
			Mat moving = imread(filenamelist[idx + 1], IMREAD_GRAYSCALE);
			Mat xymoving(M, 2, CV_64F);;
			cpcorr(movingPoints, fixedPoints, moving, fixed, xymoving);

			xymoving.col(0).copyTo(validx.col(idx));
			xymoving.col(1).copyTo(validy.col(idx));
			
			movingPoints.col(0) = xymoving.col(0).clone();
			movingPoints.col(1) = xymoving.col(1).clone();

			//grid_x.copyTo(fixedPoints.col(0));
			//grid_y.copyTo(fixedPoints.col(1));

		}
		fs_validx << "validx" << validx;
		fs_validy << "validy" << validy;
		fs_validx.release();
		fs_validy.release();


	}
	catch (Exception e) {
		cout << "error " << endl;
	}
}

void starin_plot(Mat validxy, Mat displxy, const char *title) {
	int i,j;
	int length = validxy.cols;
	double *X = new double[validxy.cols];
	double *Y = new double[validxy.cols];

	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* point_seq = cvCreateSeq(CV_64FC2, sizeof(CvSeq), sizeof(CvPoint2D64f), storage);
	//double max = -10000, min = 10000;

	for (i = 0; i < validxy.cols; i++) {
		float *line = new float[4];
		for (j = 0; j < validxy.rows; j++)
			cvSeqPush(point_seq, &cvPoint2D64f(validxy.at<double>(j, i), displxy.at<double>(j, i)));

		cvFitLine(point_seq, CV_DIST_L2, 0, 0.01, 0.01, line);
		
		float k = line[1] / line[0];
		X[i] = (double)i;
		Y[i] = (double)k;
		//if (max < Y[i]) max = Y[i];
		//if (min > Y[i]) min = Y[i];

	}
	cvClearSeq(point_seq);
	cvReleaseMemStorage(&storage);

	cvNamedWindow(title, 1);
	CPlot plot;
	//plot.x_max = 70; //可以设定横纵坐标的最大，最小值
	//plot.x_min = 0;
	//plot.y_max = max;
	//plot.y_min = min;
	plot.axis_color = Scalar(0, 0, 0);
	plot.text_color = Scalar(0, 0, 0);
	plot.plot(X, Y, length, CV_RGB(0, 0, 255), '.'); //可以只传入Y值 X默认从0开始 
	plot.title(title); //可以设定标题 只能是英文 中文会乱码 有解决方案，但是很麻烦
	plot.xlabel("Image ", Scalar(255, 255, 0));
	plot.ylabel("True 1D Strain ", Scalar(255, 255, 0));
	cvShowImage(title, plot.Figure);

}

void displxy_plot(Mat displxy, const char *title) {
	int i;
	int length = displxy.cols;
	double *X = new double[displxy.cols];
	double *Y = new double[displxy.cols];

	//double max=-10000, min=10000;
	for (i = 0; i < displxy.cols; i++) {

		X[i] = (double)i;
		Y[i] = mean(displxy.col(i))[0] *1e2;
		//if (max < Y[i]) max = Y[i];
		//if (min > Y[i]) min = Y[i];
	}



	cvNamedWindow(title, 1);
	CPlot plot;

	plot.x_max = 70; //可以设定横纵坐标的最大，最小值
	plot.x_min = 0;
	//plot.y_max = max;
	//plot.y_min = min;
	plot.axis_color = Scalar(0, 0, 0);
	plot.text_color = Scalar(0, 0, 0);
	plot.plot(X, Y, length, CV_RGB(0, 0, 255), '.'); 
	plot.title(title);
	plot.xlabel("Image ", Scalar(255, 255, 0));
	plot.ylabel("Displacement/1e-3  ", Scalar(255, 255, 0));
	cvShowImage(title, plot.Figure);

}

void fitLine() {
	Mat validx, validy;
	FileStorage fs_validx("validx.xml", FileStorage::READ);
	FileStorage fs_validy("validy.xml", FileStorage::READ);
	fs_validx["validx"] >> validx;
	fs_validy["validy"] >> validy;
	fs_validx.release();
	fs_validy.release();

	int i;
	Mat displx;
	Mat validxfirst(validx.rows, validx.cols, CV_64F);
	for (i = 0; i < validx.cols; i++) {
		validx.col(0).copyTo(validxfirst.col(i));
	}
	displx = validx - validxfirst;

	Mat disply;
	Mat validyfirst(validy.rows, validy.cols, CV_64F);
	Mat temp = Mat::ones(1, validy.cols, CV_64F);
	validyfirst = validy.col(0) * temp;

	disply = validy - validyfirst;

	//displxy_plot(displx,"X Direction Average Displacement");
	displxy_plot(disply, "Y Direction Average Displacement");
	//starin_plot(validx, displx, "X Direction Strain ");
	starin_plot(validy, disply, "Y Direction Strain");

	cvWaitKey(0);
}
int main()
{
	//Mat xymoving_in = (Mat_<double>(2, 2) << 118 ,42, 99, 87);
	//Mat xyfixed_in = (Mat_<double>(2, 2) << 190 ,114,171 ,165);
	//Mat moving = imread("onion.png");
	//Mat fixed = imread("peppers.png");
	//Mat xymoving;
	//cpcorr(xymoving_in, xyfixed_in, moving, fixed, xymoving);
	//cout << xymoving << endl;

	/*********************************************************************************************/

	grid_generator("image/ctest000.tif",7,7);

	automate_image();

	fitLine();

	return 0;

 
}


   