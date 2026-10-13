#include "pch.h"
#include "framework.h"
#include "MyMorphology.h"

cv::Mat Morphology::morphology(cv::Mat input, UINT kernel_size, UINT MODULE)
{
	if (kernel_size < 1) kernel_size = 3;

	cv::Mat gray;
	if (input.channels() == 3) cvtColor(input, gray, cv::COLOR_BGR2GRAY);
	else if (input.channels() == 1) gray = input;
	else return cv::Mat();

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(static_cast<int>(kernel_size), static_cast<int>(kernel_size)));
	cv::Mat dst;

	switch (MODULE)
	{
	case Modules::ERODE:   cv::erode(gray, dst, kernel); break;
	case Modules::DILATE:  cv::dilate(gray, dst, kernel); break;
	case Modules::OPEN:    cv::morphologyEx(gray, dst, cv::MORPH_OPEN, kernel); break;
	case Modules::CLOSE:   cv::morphologyEx(gray, dst, cv::MORPH_CLOSE, kernel); break;
	default:               return cv::Mat();
	}
	return dst;
}
