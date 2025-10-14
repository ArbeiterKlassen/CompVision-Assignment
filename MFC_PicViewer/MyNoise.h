#pragma once
#include "pch.h"
#include "framework.h"
#include "afxdialogex.h"
#include <opencv2/opencv.hpp>
#include <stack>
#include <random>
#include <vector>
#include <cmath>
#include <limits>

using namespace std;
using namespace cv;

class PoissonGenerator {
public:
    explicit PoissonGenerator(uint64_t seed = std::mt19937_64::default_seed)
        : rng(seed) {}

    // 主入口：返回 Poisson(λ) 样本
    int operator()(double lambda) {
        if (lambda <= 0.0) return 0;
        if (lambda < 30.0) return knuthSmall(lambda);
        return gaussianLarge(lambda);
    }

private:
    std::mt19937_64 rng;
    std::uniform_real_distribution<double> U{ 0.0, 1.0 };

    // Knuth 算法：O(λ)
    int knuthSmall(double lambda) {
        int k = 0;
        double p = 1.0;
        const double L = std::exp(-lambda);
        do {
            ++k;
            p *= U(rng);
        } while (p > L);
        return k - 1;
    }

    // 高斯近似 + rejection，λ≥30
    int gaussianLarge(double lambda) {
        const double sq = std::sqrt(lambda);
        const double lg = std::log(lambda);
        for (;;) {
            double u = U(rng);
            double v = U(rng);
            // 高斯候选
            double x = sq * (1.0 - 0.5 / sq + 0.5 * std::log(v) / sq);
            if (x < 0.0) continue;
            int k = static_cast<int>(std::floor(x + lambda + 0.5));
            if (k < 0) continue;
            // rejection 校正
            double rho = 0.5 * (x * x / lambda - 1.0) + k * lg - std::lgamma(k + 1);
            if (std::log(u) < rho) return k;
        }
    }
};

class Rng {
public:
    explicit Rng(uint64_t s = 123456789) : state(s) {}
    // [0,1) 均匀浮点
    double uniform() {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return (state * 0x2545F4914F6CDD1DULL) * (1.0 / 18446744073709551616.0);
    }
    // N(0,1) 使用 Box-Muller
    double gaussian() {
        if (hasSpare) { hasSpare = false; return spare; }
        double u, v, mag;
        do {
            u = 2.0 * uniform() - 1.0;
            v = 2.0 * uniform() - 1.0;
            mag = u * u + v * v;
        } while (mag >= 1.0 || mag == 0.0);
        double fac = std::sqrt(-2.0 * std::log(mag) / mag);
        spare = v * fac;
        hasSpare = true;
        return u * fac;
    }
    // Tukey(0,1) 重尾采样
    double tukey() {
        const double c = 5.0;          // 尾部参数，可调
        double u = uniform();
        if (u <= 0.5) {
            return std::pow(2.0 * u, 1.0 / c) - 1.0;
        }
        else {
            return 1.0 - std::pow(2.0 * (1.0 - u), 1.0 / c);
        }
    }
private:
    uint64_t state;
    bool hasSpare = false;
    double spare = 0.0;
};

class PoissonGen {
public:
    explicit PoissonGen(Rng& r) : rng(r) {}
    int operator()(double lambda) {
        if (lambda <= 0.0) return 0;
        if (lambda < 30.0) return knuth(lambda);
        return gaussianApprox(lambda);
    }
private:
    Rng& rng;
    int knuth(double lambda) {
        int k = 0;
        double p = 1.0;
        double L = std::exp(-lambda);
        do {
            ++k;
            p *= rng.uniform();
        } while (p > L);
        return k - 1;
    }
    int gaussianApprox(double lambda) {
        double sq = std::sqrt(lambda);
        double g = rng.gaussian() * sq + lambda;
        return static_cast<int>(std::round(g));
    }
};

namespace Noises {
	namespace Modules {
		namespace CLASSIC {
			enum {
				AWGN = 1,POISSON_GAUSSIAN,ELD_SFRN
			};
		}
	}
    inline int cvPoisson(double lambda);
	Mat noise(Mat input, float sigma, UINT MODULE);
	Mat noise(Mat input, double gain, double sigma, UINT MODULE);
	Mat noise(Mat input, double alpha, double sigmaT, double sigmaG, double q, UINT MODULE);
}