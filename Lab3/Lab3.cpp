#include "stdafx.h"
#include <opencv2\core\core.hpp>
#include <opencv2\opencv.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

struct SUserdata {
	cv::Mat image;
};



double avg(cv::Mat img){
	unsigned char* ptr;
	int x, y, b = 0, g = 0, r = 0, col = 0;


	for (x = 0; x <= img.cols; x++){
		for (y = 0; y < img.rows; y++){
			ptr = (uchar*)(img.data + y*img.step);   //подсчитаем среднее значение для каждого компонента цветов
			b = ptr[3 * x];
			g = ptr[3 * x + 1]; // вот так мы как раз обращаемся к различным компонентам цветов
			r = ptr[3 * x + 2]; // b - blue, r - red, g - green
			col += b + g + r;
		}
	}
	return col / ((img.rows)*(img.cols));
}

void LocalAdaptiveFilter(cv::Mat& src)
{
	int rad = 2;
	cv::Mat dst = src.clone();
	int _numS = (2 * rad + 1); //количество элементов в окрестности
	_numS = pow(_numS, 2);
	int disp = 1000, dispL = 0, g = 0, Sum = 0, mL = 0;
	vector<int> S(_numS);
	copyMakeBorder(src, dst, rad, rad, rad, rad, cv::BORDER_REFLECT);


	for (int x = rad; x < dst.cols - rad; x++)
	{
		for (int y = rad; y < dst.rows - rad; y++)
		{
			g = dst.at<uchar>(y, x);
			Sum = 0;
			for (int l = -rad, i = 0; l <= rad; l++)
				for (int k = -rad; k <= rad; k++)
				{
					S[i] = dst.at<uchar>(y + k, x + l);
					Sum += S[i];
					i++;
				}

				mL = Sum / _numS;
			dispL = 0;
			for (int i = 0; i < _numS; i++)
			{
				dispL += (pow(S[i],2) - pow(mL,2));
			}
			dispL /= (_numS - 1);
			if (disp <= dispL)
			{
				dst.at<uchar>(y, x) = g - disp / dispL * (g - mL);
			}
			else
				dst.at<uchar>(y, x) = mL;
		}
	}

	cv::Mat result = dst(cv::Rect(rad, rad, src.cols, src.rows));
	cv::imshow("Original", src);
	cv::imshow("Адаптивный локальный фильтр", result);
	cv::imwrite("1.jpg", result);
	cv::waitKey();
	cv::destroyAllWindows();
	cout << endl;
}

void EffectImplementationOfMedianFilter(cv::Mat& src)
{
	int rad = 2; 

	cv::Mat dst = src.clone();
	int _numS = (2 * rad + 1); 
	_numS = pow(_numS, 2);
	int result = 0;
	vector<int> S(_numS);
	copyMakeBorder(src, dst, rad, rad, rad, rad, cv::BORDER_REFLECT);

	for (int x = rad; x < dst.cols - rad; x++)
		for (int y = rad; y < dst.rows - rad; y++)
		{
			for (int l = -rad, i = 0; l <= rad; l++)
				for (int k = -rad; k <= rad; k++)
				{
					S[i] = dst.at<uchar>(y + k, x + l);
					i++;
				}
			std::sort(S.begin(), S.end());
			result = S[(_numS - 1) / 2];
			dst.at<uchar>(y, x) = result;
		}

	cv::Mat img = dst(cv::Rect(rad, rad, src.cols, src.rows));
	cv::imshow("Original", src);
	cv::imshow("Эффективная реализация медианного фильтра", img);
	cv::imwrite("2.jpg", img);
	cv::waitKey();
	cv::destroyAllWindows();
	cout << endl;
}

void AdaptiveMedianFilter(cv::Mat& src)
{
	int rad = 2;
	cv::Mat dst = src.clone();
	int S_max = 4 + rad, Rate_s = rad;
	vector<int> S;
	copyMakeBorder(src, dst, S_max, S_max, S_max, S_max, cv::BORDER_REFLECT);

	for (int x = S_max; x < dst.cols - S_max; x++)
	for (int y = S_max; y < dst.rows - S_max; y++)
		{
			int z_xy = dst.at<uchar>(y, x);
			bool exit = true;
			Rate_s = rad;

			while (exit)
			{
				S.clear();
				for (int l = -Rate_s; l <= Rate_s; l++)
				for (int k = -Rate_s; k <= Rate_s; k++)
					{
						S.push_back(dst.at<uchar>(y + k, x + l));
					}
				std::sort(S.begin(), S.end());
				int z_min = S[0];
				int z_max = S[S.size() - 1];
				int z_med = S[(S.size() - 1) / 2];
				// ветвь А
				int A1 = z_med - z_min;
				int A2 = z_med - z_max;
				if (A1 > 0 && A2 < 0)
				{
					//ветвь Б
					int B1 = z_xy - z_min;
					int B2 = z_xy - z_max;
					if (B1 > 0 && B2 < 0)
					{
						dst.at<uchar>(y, x) = z_xy;
						exit = false;
					}
					else
					{
						dst.at<uchar>(y, x) = z_med;
						exit = false;
					}
				}
				else
				{
					Rate_s++;
					if (Rate_s <= S_max)
						continue;
					else
					{
						exit = false;
						dst.at<uchar>(y, x) = z_xy;
					}
				}
			}
		}

	cv::Mat result = dst(cv::Rect(S_max, S_max, src.cols, src.rows));
	cv::imshow("Original", src);
	cv::imshow("Адаптивный медианный фильтр", result);
//	cv::imwrite("3.jpg", result);
	cv::waitKey();
	cout << endl;
	cv::destroyAllWindows();
}

void CompareFilter(cv::Mat& src)
{
	cv::Mat blurred, dst, img;
	unsigned char* ptr;
	int x, y, b = 0, g = 0, r = 0;
	int col = 0;
	cv::imwrite("3.jpg", src);
	cv::Mat noise = cv::Mat(src.size(), CV_8U);
	cv::Scalar a(0);
	cv::Scalar c(10);
	randn(noise, a, c);
	src = src + noise;
	cv::Mat srcClone = src.clone();
	LocalAdaptiveFilter(src);
	EffectImplementationOfMedianFilter(srcClone);
	for (int i = 0; i <= 2; i++)
	{

		if (i == 0)
		{
			img = cv::imread("1.jpg");
		}
		if (i == 1)
		{
			img = cv::imread("2.jpg");
		}
		if (i == 2)
		{
			img = cv::imread("3.jpg");
		}

		/*cv::imwrite("ttt",img[i]);
		cv::waitKey();*/

		double mu = avg(img);
		for (x = 0; x <= img.cols; x++){
			for (y = 0; y < img.rows; y++)
			{

				ptr = (uchar*)(img.data + y*img.step);

				b = ptr[3 * x];
				col += (abs(1.0*b - mu))*(abs(1.0*b - mu));
				g = ptr[3 * x + 1];
				col += (abs(1.0*g - mu))*(abs(1.0*g - mu));
				r = ptr[3 * x + 2];
				col += (abs(1.0*r - mu))*(abs(1.0*r - mu));
			}
		}

		cout << "Среднеквадратическое отклонение " << i+1<< "-го изображения: " << sqrt(abs(col) / ((img.cols)*(img.rows))) << endl;
		col = 0;
	}

	cout << endl;
}

int main()
{
	setlocale(LC_ALL, "RUS");

	cv::Mat img = cv::imread("image.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	int str, rad = 0;
	bool _exitProgram = true;
	SUserdata userdata;
	userdata.image = img;

	while (_exitProgram)
	{
		cout << "1 - Адаптивный локальный фильтр" << endl;
		cout << "2 - Эффективная реализация медианного фильтра" << endl;
		cout << "3 - Адаптивный медианный фильтр" << endl;
		cout << "4 - Сравнение фильтров" << endl;
		cout << "0 - Выход" << endl;
		cout << "Выберите задание: ";
		cin >> str;

		switch (str)
		{
		case 1:
			LocalAdaptiveFilter(userdata.image);
			break;

		case 2:
			EffectImplementationOfMedianFilter(userdata.image);
			break;

		case 3:
			AdaptiveMedianFilter(userdata.image);
			break;
		case 4:
			CompareFilter(userdata.image);
			break;
		case 0:
			_exitProgram = false;
			break;

		default:
			system("cls");
			cout << "Введите правильно."<< endl<< endl;
			break;
		}
	}

	return 0;
}

