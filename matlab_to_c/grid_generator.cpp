#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;

Rect box;//���ζ���
bool drawing_box = false;//��¼�Ƿ��ڻ����ζ���
bool bk = false;

void onmouse(int event, int x, int y, int flag, void *img)//����¼��ص��������������ִ�е�����Ӧ�ڴ�
{
	Mat& image = *(cv::Mat*) img;
	char text[16];
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN://�����������¼�
		drawing_box = true;//��־�ڻ���
		box = Rect(x, y, 0, 0);//��¼���εĿ�ʼ�ĵ�
		sprintf_s(text, "(%d,%d)", x, y);
		putText(image, text, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255, 255));
		break;
	case CV_EVENT_MOUSEMOVE://����ƶ��¼�
		if (drawing_box) {//������һֱ���ţ�������ڻ�����

			box.width = x - box.x;
			box.height = y - box.y;//���³���
		}
		break;
	case CV_EVENT_LBUTTONUP://�������ɿ��¼�
		drawing_box = false;//���ڻ�����
		bk = true;
		
		if (box.width < 0) {//�ų���Ϊ����������������ж���Ϊ���Ż����㣬�������ƶ�ʱÿ�θ��¶�Ҫ���㳤��ľ���ֵ
			box.x = box.x + box.width;//����ԭ��λ�ã�ʹ֮ʼ�շ������Ͻ�Ϊԭ��
			box.width = -1 * box.width;//���ȡ��
		}
		if (box.height < 0) {//ͬ��
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
	Mat img = imread(image_name);//��ȡͼ��
	Mat temp;
	char text[16];

	namedWindow("grid_generator");//����
	setMouseCallback("grid_generator", onmouse, &img);//ע������¼�������껭���򡱴��ڣ���ʹ�ڸô����³�������¼���ִ��onmouse����������,���һ������Ϊ��������ݡ�������ʵû���õ�
	imshow("grid_generator", img);
	//img.copyTo(temp);��������������Ӱ��������Ϊ��һ�λ��ľ��β�û�б�����
	while (1)
	{
		
		if (drawing_box) {//���ϸ������ڻ��ľ���
			img.copyTo(temp);//�����������Ǳ�֤��ÿ�θ��¾��ο�����û��ԭͼ�Ļ����ϸ��¾��ο�
			rectangle(temp, Point(box.x, box.y), Point(box.x + box.width, box.y + box.height), Scalar(0, 0, 255));

			sprintf_s(text, "(%d,%d)", box.x + box.width, box.y + box.height);
			putText(temp, text, Point(box.x + box.width, box.y + box.height), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255, 255));
			imshow("grid_generator", temp);//��ʾ


		}
		if (bk)
		{
			//FILE *fp;
		//	fp = fopen("grid_x.txt", "w");
			FileStorage fs_grid_x("grid_x.xml", FileStorage::WRITE); //��grid.xml��ȡ��grid_x,grid_y��ֵ
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
		if (waitKey(30) == 27) {//����Ƿ��а����˳���
			break;//�˳�����
		}
	}

	
}
