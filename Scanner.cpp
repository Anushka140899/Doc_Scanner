#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;


Mat imgOrg, imgGray, imgcanny,imgThre,imgBlur,imgdil,imgWarp,imgcrop;
vector<Point> InitialPoints,docpoints;
float w = 420, h = 596;

Mat preprocessing(Mat img)
{
	cvtColor(img, imgGray, COLOR_BGR2GRAY);      //Change colour to gray
	GaussianBlur(img, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgcanny, 50, 50);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(2, 2));
	dilate(imgcanny, imgdil, kernel);
	return imgdil;
}

vector<Point> getContour(Mat imgDil)
{
	vector<vector<Point>>contours;
	vector<Vec4i>heirarchy;
	findContours(imgDil, contours, heirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	//drawContours(img, contours, -1, Scalar(255, 0, 255),2);

	vector<vector<Point>>conpoly(contours.size());
	vector<Rect>bound(contours.size());
	vector<Point> Biggest;
	string object;
	int maxarea=0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		cout << area << endl;
		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conpoly[i], 0.02 * peri, true);

			if (area > maxarea && conpoly[i].size() == 4)
			{
				//drawContours(imgOrg, conpoly, i, Scalar(255, 0, 255), 5);
				maxarea = area;
				Biggest = { conpoly[i][0],conpoly[i][1] ,conpoly[i][2] ,conpoly[i][3] };
			}
		
			//drawContours(imgOrg, conpoly, i, Scalar(255, 0, 255), 2);
			//rectangle(imgOrg, bound[i].tl(), bound[i].br(), Scalar(0, 0, 0), 2);

		}
	}
	return Biggest;
}

void drawpoints(vector<Point> points, Scalar color)
{
	for (int i = 0; i < points.size(); i++)
	{
		circle(imgOrg, points[i], 10 , color, FILLED);
		putText(imgOrg, to_string(i), points[i], FONT_HERSHEY_PLAIN, 4, color, 4);
	}
}

vector<Point> reorder(vector<Point> points)
{
	vector<Point> newpoints;
	vector<int>sumpoint, diffpoint;

	for (int i = 0; i < points.size(); i++)
	{
		sumpoint.push_back(points[i].x + points[i].y);
		diffpoint.push_back(points[i].x - points[i].y);
	}
	newpoints.push_back(points[min_element(sumpoint.begin(), sumpoint.end())-sumpoint.begin()]);
	newpoints.push_back(points[max_element(diffpoint.begin(), diffpoint.end()) - diffpoint.begin()]);
	newpoints.push_back(points[min_element(diffpoint.begin(), diffpoint.end()) - diffpoint.begin()]);
	newpoints.push_back(points[max_element(sumpoint.begin(), sumpoint.end()) - sumpoint.begin()]);
	
	return newpoints;
}

Mat getWarp(Mat img, vector<Point> points, float w, float h)
{
	Point2f src[4] = {points[0],points[1],points[2],points[3] };
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };
	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));

	return imgWarp;
}

int main() {

    VideoCapture cap(0);

	while (true)
	{
		cap.read(imgOrg);
		//resize(imgOrg, imgOrg, Size(), 0.5, 0.5);

		//Preprocessing//
		imgThre = preprocessing(imgOrg);

		//Get contours//
		InitialPoints = getContour(imgThre);
		//drawpoints(InitialPoints, Scalar(0, 0, 255));
		docpoints = reorder(InitialPoints);
		//drawpoints(docpoints, Scalar(0, 255, 0));

		//Warping//

		imgWarp = getWarp(imgOrg, docpoints, w, h);

		//Crop
		int cropval = 10;
		Rect roi(cropval, cropval, w - (2 * cropval), h - (2 * cropval));
		imgcrop = imgWarp(roi);

		imshow("Image", imgOrg);
		imshow("Image Threshold", imgThre);
		imshow("Image Crop", imgcrop);

		waitKey(1);
	}
	
	return 0;

}