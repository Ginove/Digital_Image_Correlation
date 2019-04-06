#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;

static Mat calc_rects(const Mat xy,int halfwidth, const Mat img);
static void adjust_lo_edge(Mat &coordinates, int edge, Mat &breadth);
static void adjust_hi_edge(Mat coordinates, int edge, Mat &breadth);
static void adjust_width_height(Mat &coordinates, int edge);
static Mat Mat_Round(const Mat coordinates);
static void cpcorr(const Mat xymoving_in, const Mat xyfixed_in, const Mat moving, const Mat fixed, Mat &xymoving);

Mat Mat_Round(const Mat coordinates) {
	
	Mat coordinates_temp = coordinates.clone();
	int row = coordinates_temp.rows;
	int col = coordinates_temp.cols;
	Mat result(row, col, CV_64F);
	int i, j;
	for (i = 0; i < row; i++)
		for (j = 0; j < col; j++) {
			result.at<double>(i, j) = cvRound(coordinates_temp.at<double>(i, j));
		}
	return result;
}

void adjust_lo_edge(Mat &coordinates, int edge, Mat &breadth) {
	int row = coordinates.rows;
	int col = coordinates.cols;
	int i, j;
	for(i = 0 ; i < row ; i++)
		for (j = 0; j < col; j++) {
			if (coordinates.at<double>(i, j) < edge) {
				breadth.at<double>(i, j) = breadth.at<double>(i, j) -fabs(coordinates.at<double>(i, j) - edge);
				coordinates.at<double>(i, j) = edge;
			}
		}
}

void adjust_hi_edge(Mat coordinates, int edge, Mat &breadth) {

	Mat coordinates_t = coordinates.clone();
	int row = coordinates_t.rows;
	int col = coordinates_t.cols;
	int i, j;
	for (i = 0; i < row; i++)
		for (j = 0; j < col; j++) {
			if (coordinates_t.at<double>(i, j) > edge) {
				breadth.at<double>(i, j) = breadth.at<double>(i, j) -fabs(coordinates_t.at<double>(i, j) - edge);
			}
		}
}

void adjust_width_height(Mat &coordinates, int edge) {
	int row = coordinates.rows;
	int col = coordinates.cols;
	int i, j;
	for (i = 0; i < row; i++)
		for (j = 0; j < col; j++) {
			if (coordinates.at<double>(i, j) < edge) {
				
				coordinates.at<double>(i, j) = 0;
			}
		}
}

Mat calc_rects(const Mat xy,const int halfwidth, const Mat img) {
	int M = xy.rows;
	//∑µªÿ÷µ rect - M*4 æÿ’Û
	Mat rect(M, 4, CV_64F);
	Mat upperleft;

	//Calculate rectangles so imcrop will return image with xy coordinate inside center pixel
	int default_width = 2 * halfwidth;
	int default_height = default_width;

	//xy specifies center of rectangle, need upper left

	Mat xy_temp = Mat_Round(xy);
	upperleft = xy_temp - halfwidth;

	//need to modify for pixels near edge of images
	Mat upper = upperleft.col(1);
	Mat left  = upperleft.col(0);
	Mat lower = upper + default_height;
	Mat right = left + default_width;
	Mat width(upper.rows, upper.cols, CV_64F, Scalar(default_width));
	Mat height(upper.rows, upper.cols, CV_64F, Scalar(default_height));

	// check edges for coordinates outside image
	adjust_lo_edge(upper, 1, height);
	adjust_hi_edge(lower, img.rows , height);
	adjust_lo_edge(left, 1, width);
	adjust_hi_edge(right, img.cols, width);

	// set width and height to zero when less than default size
	adjust_width_height(width, default_width);
	adjust_width_height(height, default_height);
	left.copyTo(rect.col(0));
	upper.copyTo(rect.col(1));
	width.copyTo(rect.col(2));
	height.copyTo(rect.col(3));

	return rect;

}

void cpcorr(const Mat xymoving_in, const Mat xyfixed_in, const Mat moving, const Mat fixed, Mat &xymoving) {

	
	int CORRSIZE = 15;
	int M1 = xymoving_in.rows;
	int M2 = xyfixed_in.rows;

	Mat rects_moving(M1, 4, CV_64F);
	Mat rects_fixed(M2, 4, CV_64F);

	// get all rectangle coordinates
	rects_moving = calc_rects(xymoving_in, CORRSIZE, moving);
	rects_fixed = calc_rects(xyfixed_in, 2 * CORRSIZE, fixed);

	//cout << "rects_moving.row(1)" <<rects_moving.row(1) << endl;
	int ncp = xymoving_in.rows;
	xymoving = xymoving_in.clone();
	
	for (int i = 0; i < ncp; i++) {
		// near edge, unable to adjust
		if (countNonZero(rects_moving.colRange(3, 4)) == 0 || countNonZero(rects_fixed.colRange(3, 4)) == 0)
			continue;
		
		Mat sub_moving = moving(Rect(rects_moving.at<double>(i, 0), rects_moving.at<double>(i, 1), rects_moving.at<double>(i, 2), rects_moving.at<double>(i, 3)));
		Mat sub_fixed = fixed(Rect(rects_fixed.at<double>(i, 0), rects_fixed.at<double>(i, 1), rects_fixed.at<double>(i, 2), rects_fixed.at<double>(i, 3)));

		// check that template rectangle sub_moving has nonzero std
		Mat means,stddev;
		meanStdDev(sub_moving, means, stddev);
		if (stddev.at<double>(0, 0) == 0)
			continue;
		Mat result;
		int result_cols = sub_fixed.cols - sub_moving.cols + 1;
		int result_rows = sub_fixed.rows - sub_moving.rows + 1;
		result.create(result_cols, result_rows, CV_64F);

		matchTemplate(sub_moving, sub_fixed, result, TM_CCOEFF_NORMED);
		normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

		Point maxLoc;
		double maxVal;
		minMaxLoc(result, NULL, &maxVal, NULL, &maxLoc);

		double THRESHOLD = 0.5;
		if (maxVal < THRESHOLD)
			continue;

		Mat corr_offset = (Mat_<double>(1, 2) << (maxLoc.x - CORRSIZE), (maxLoc.y - CORRSIZE));
		
		// peak of norxcorr2 not well constrained, unable to adjust
		if (fabs(corr_offset.at<double>(0, 0)) > (CORRSIZE + 1) || fabs(corr_offset.at<double>(0, 1)) > (CORRSIZE + 1))
			continue;
	

		Mat xymoving_temp = Mat_Round(xymoving.row(i)*1000)/1000;
		Mat xyfixed_temp = Mat_Round(xyfixed_in.row(i)*1000)/1000;
		Mat moving_fractional_offset = xymoving.row(i) - xymoving_temp;
		Mat fixed_fractional_offset= xyfixed_in.row(i) - xyfixed_temp;

		xymoving.row(i) = xymoving.row(i) - moving_fractional_offset - corr_offset + fixed_fractional_offset;
	} 

}