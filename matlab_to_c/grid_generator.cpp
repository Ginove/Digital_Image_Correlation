#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;

Rect box;//矩形对象
bool drawing_box = false;//记录是否在画矩形对象
bool bk = false;

void onmouse(int event, int x, int y, int flag, void *img)//鼠标事件回调函数，鼠标点击后执行的内容应在此
{
	Mat& image = *(cv::Mat*) img;
	char text[16];
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN://鼠标左键按下事件
		drawing_box = true;//标志在画框
		box = Rect(x, y, 0, 0);//记录矩形的开始的点
		sprintf_s(text, "(%d,%d)", x, y);
		putText(image, text, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255, 255));
		break;
	case CV_EVENT_MOUSEMOVE://鼠标移动事件
		if (drawing_box) {//如果左键一直按着，则表明在画矩形

			box.width = x - box.x;
			box.height = y - box.y;//更新长宽
		}
		break;
	case CV_EVENT_LBUTTONUP://鼠标左键松开事件
		drawing_box = false;//不在画矩形
		bk = true;
		
		if (box.width < 0) {//排除宽为负的情况，在这里判断是为了优化计算，不用再移动时每次更新都要计算长宽的绝对值
			box.x = box.x + box.width;//更新原点位置，使之始终符合左上角为原点
			box.width = -1 * box.width;//宽度取正
		}
		if (box.height < 0) {//同上
			box.y = box.y + box.height;
			box.height = -1 * box.width;
		}
		break;
	default:
		break;
	}
}

void grid_generator(string image_name, int x_pixels, int y_pixels)
{
	Mat img = imread(image_name);//读取图像
	Mat temp;
	char text[16];

	namedWindow("grid_generator");//窗口
	setMouseCallback("grid_generator", onmouse, &img);//注册鼠标事件到“鼠标画个框”窗口，即使在该窗口下出现鼠标事件就执行onmouse函数的内容,最后一个参数为传入的数据。这里其实没有用到
	imshow("grid_generator", img);
	//img.copyTo(temp);放在这里会出现拖影的现象，因为上一次画的矩形并没有被更新
	while (1)
	{
		
		if (drawing_box) {//不断更新正在画的矩形
			img.copyTo(temp);//这句放在这里是保证了每次更新矩形框都是在没有原图的基础上更新矩形框。
			rectangle(temp, Point(box.x, box.y), Point(box.x + box.width, box.y + box.height), Scalar(0, 0, 255));

			sprintf_s(text, "(%d,%d)", box.x + box.width, box.y + box.height);
			putText(temp, text, Point(box.x + box.width, box.y + box.height), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255, 255));
			imshow("grid_generator", temp);//显示


		}
		if (bk)
		{
			//FILE *fp;
		//	fp = fopen("grid_x.txt", "w");
			FileStorage fs_grid_x("grid_x.xml", FileStorage::WRITE); //从grid.xml中取出grid_x,grid_y的值
			FileStorage fs_grid_y("grid_y.xml", FileStorage::WRITE);
			int M = box.width/ x_pixels + 1;
			int N = box.height / y_pixels + 1;
			Mat grid_x(M*N, 1, CV_64F);
			Mat grid_y(M*N, 1, CV_64F);
			int cot = 0;
			for (int i = 0; i < M; i++)
				for (int j = 0; j < N;j++) {
				grid_x.at<double>(cot, 0) = box.x + j * x_pixels;
				grid_y.at<double>(cot, 0) = box.y + i * y_pixels;
				cot++;
				}
			fs_grid_x << "grid_x" << grid_x;
			fs_grid_y << "grid_y" << grid_y;
			fs_grid_x.release();
			fs_grid_y.release();
			break;
		}
		if (waitKey(30) == 27) {//检测是否有按下退出键
			break;//退出程序
		}
	}

	
}
