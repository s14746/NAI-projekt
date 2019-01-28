/**
 *
 * g++ `pkg-config --cflags opencv` cv2.cpp -o cv `pkg-config --libs opencv`
 * */

#include <cv.hpp>
#include <highgui.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <list>
#include <cstdlib>

using namespace cv;
using namespace std;


int main() {
	const int imgwidth = 512, imgheight = 384;
	const int dilation_size = 2;
			
	int loRange[3] = {50,155,119};
	int hiRange[3] = {162,255,255};
	
	namedWindow("ustawienia", CV_WINDOW_AUTOSIZE);
	createTrackbar("loRange0", "ustawienia",&(loRange[0]), 255);
	createTrackbar("loRange1", "ustawienia",&(loRange[1]), 255);
	createTrackbar("loRange2", "ustawienia",&(loRange[2]), 255);
	createTrackbar("hiRange0", "ustawienia",&(hiRange[0]), 255);
	createTrackbar("hiRange1", "ustawienia",&(hiRange[1]), 255);
	createTrackbar("hiRange2", "ustawienia",&(hiRange[2]), 255);
	VideoCapture camera(0);

	auto structElem = getStructuringElement( MORPH_ELLIPSE,
				   Size( 2*dilation_size + 1, 2*dilation_size+1 ),
				   Point( dilation_size, dilation_size ) );
	list < Point2f > path;
	while ( waitKey(1) != 27  ) {
		vector < vector < Point > > contours;
		Mat frame, imgOrig;
		camera >> frame;
		flip(frame, frame, 1);
		resize(frame, frame,{imgwidth, imgheight});
		imgOrig = frame.clone();
		cvtColor(frame, frame, CV_RGB2HSV);
		inRange(frame, Scalar(loRange[0],loRange[1],loRange[2]),
						Scalar(hiRange[0],hiRange[1],hiRange[2]), frame);
		morphologyEx( frame, frame, MORPH_CLOSE, structElem );
		
		findContours(frame, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
		vector < pair < double, vector < Point > > > sortedContours;
		for (unsigned i = 0; i < contours.size(); i++) {
			drawContours(imgOrig, contours, i, Scalar(255,0,0),3);
			sortedContours.push_back({contourArea(contours[i], false),contours[i] });
		}
		if (sortedContours.size() > 0) {
			Point2f pos;
			float r;
			sort(sortedContours.begin(),sortedContours.end(),[](auto a, auto b){
				return a.first > b.first;
			});
			minEnclosingCircle(sortedContours[0].second, pos, r);
			if (r > 8) {
				if (path.size() < 70) {
					path.push_back(pos); // dopisujemy srodek okregu
				} else {
					path.pop_front();
					path.push_back(pos);
				}
				vector < Point > pathV;
				vector < Point2f > approximated;
				approxPolyDP(vector<Point2f>(path.begin(), path.end()),approximated,50, false);
				
				for (auto &p: approximated) pathV.push_back({(int)p.x,(int)p.y});
				polylines(imgOrig,{pathV},false,Scalar(0,255,0),2); // jesli chcemy pokazać ścieżkę
				int pathSize = 5;
				if (pathV.size() >= pathSize) {
					vector < Point > itr(pathV.end()-pathSize,pathV.end());
					int conditions = 0;
					double factor = (::abs(itr[0].x - itr[1].x) + ::abs(itr[0].y - itr[1].y))*2/3;

					// cout << "x0=" << itr[0].x << " y0=" << itr[0].y << endl;
					// cout << "x1=" << itr[1].x << " y1=" << itr[1].y << endl;
					// cout << "x2=" << itr[2].x << " y2=" << itr[2].y << endl;
					// cout << "x3=" << itr[3].x << " y3=" << itr[3].y << endl;
					// cout << "x4=" << itr[4].x << " y4=" << itr[4].y << endl;

					if ((::abs(itr[0].y - itr[1].y) > factor) && (::abs(itr[0].x - itr[1].x) < factor)) {
						// cout << "Linia pionowa 1 !!!" << endl;
						conditions++;
					}

					if ((::abs(itr[1].y - itr[2].y) > factor) && (::abs(itr[1].x - itr[2].x) > factor)) {
						// cout << "Linia skośna 1 !!!" << endl;
						conditions++;
					}

					if ((::abs(itr[2].y - itr[3].y) > factor) && (::abs(itr[2].x - itr[3].x) > factor)) {
						// cout << "Linia skośna 2 !!!" << endl;
						conditions++;
					}

					if ((::abs(itr[3].y - itr[4].y) > factor) && (::abs(itr[3].x - itr[4].x) < factor)) {
						// cout << "Linia pionowa 2 !!!" << endl;
						conditions++;
					}

					// cout << endl;

					if (conditions == 4) {
						cout << "Otwieram Mozilla FireFox !!" << endl;
						system("firefox www.google.com &");
						path.clear();
					}
				}
			}
		}
		imshow("ustawienia", frame);
		imshow("obraz", imgOrig);
	}
	return 0;
}
