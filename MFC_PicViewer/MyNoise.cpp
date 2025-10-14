#include "pch.h"
#include "framework.h"
#include "afxdialogex.h"
#include "MyNoise.h"
#include <opencv2/opencv.hpp>
#include <stack>
#include <random>
#include <vector>
#include <cmath>
#include <limits>
using namespace std;
using namespace cv;

namespace Noises {
    inline int cvPoisson(double lambda) {
        static PoissonGenerator pg(cv::getTickCount());
        return pg(lambda);
    }
	Mat noise(Mat input, float sigma, UINT MODULE) {
        if (MODULE == Modules::CLASSIC::AWGN) {
            CV_Assert(input.type() == CV_8UC1 || input.type() == CV_8UC3);
            if (input.type() == CV_8UC3)cvtColor(input, input, COLOR_BGR2GRAY);
            Mat noise(input.size(), input.type());
            randn(noise, 0, sigma);                     // 生成 N(0,σ^2)
            Mat dst;
            add(input, noise, dst, noArray(), CV_8UC3);   // 自动 saturate_cast
            return dst;
        }
	}
    Mat noise(Mat input, double gain, double sigma, UINT MODULE) {
        if (MODULE == Modules::CLASSIC::POISSON_GAUSSIAN) {
            CV_Assert(input.type() == CV_8UC1 || input.type() == CV_8UC3);
            if (input.type() == CV_8UC3)cvtColor(input, input, COLOR_BGR2GRAY);
            Mat lambda;
            input.convertTo(lambda, CV_32FC1, 1.0 / gain);      // λ = x / gain

            Mat poissonNoise(lambda.size(), CV_32FC1);
            randu(poissonNoise, 0.f, 1.f);                  // 先填随机数
            for (int i = 0; i < lambda.rows; ++i)
                for (int j = 0; j < lambda.cols; ++j)
                    poissonNoise.at<float>(i, j) =
                    static_cast<float>(cvPoisson(lambda.at<float>(i, j))) - lambda.at<float>(i, j);
            // 上面减均值，让泊松项零均值

            Mat gaussianNoise(lambda.size(), CV_32FC1);
            randn(gaussianNoise, 0.0, sigma);

            Mat noisy = lambda + poissonNoise + gaussianNoise; // 仍然浮点
            Mat z;
            noisy *= gain;                                    // 缩回 0~255 量级
            noisy.convertTo(z, CV_8UC1);                      //  saturate_cast
            return z;
        }
    }
    Mat noise(Mat input, double alpha, double sigmaT, double sigmaG, double q, UINT MODULE)
    {
        int seed = -1;
        if (MODULE == Modules::CLASSIC::ELD_SFRN) {
            CV_Assert(input.type() == CV_8UC1 || input.type() == CV_8UC3);
            if (input.type() == CV_8UC3)cvtColor(input, input, COLOR_BGR2GRAY);
            CV_Assert(alpha > 0 && sigmaT >= 0 && sigmaG >= 0 && q >= 1);
            Rng rng(seed >= 0 ? static_cast<uint64_t>(seed) : cv::getTickCount());
            PoissonGen poisson(rng);
            cv::Mat dst(input.size(), CV_8UC1);
            const double qHalf = q * 0.5;
            for (int i = 0; i < input.rows; ++i) {
                for (int j = 0; j < input.cols; ++j) {
                    int x = input.at<uchar>(i, j);
                    int p = poisson(x / alpha);               // Poisson(x/α)
                    double sig = alpha * p;                   // 缩放回电子数
                    double t = rng.tukey() * sigmaT;
                    double g = rng.gaussian() * sigmaG;
                    double u = (rng.uniform() - 0.5) * q;     // Uniform(-q/2,q/2)
                    double y = sig + t + g + u;
                    dst.at<uchar>(i, j) = cv::saturate_cast<uchar>(std::round(y));
                }
            }
            return dst;
        }
    }
}