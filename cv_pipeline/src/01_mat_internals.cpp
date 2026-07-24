#include <iostream>
#include <opencv2/opencv.hpp>

static bool detect_quad_opencv(const cv::Mat &gray, float outCorners[8])
{
    // first is to apply guassian blur
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);

    // second use canny for edge detection
    cv::Mat edges;
    cv::Canny(blurred, edges, 50, 150);

    // next is to find contours on edges
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    double maxArea = 0.0;
    std::vector<cv::Point> bestQuad;

    for (const auto &contour : contours)
    {
        double area = cv::contourArea(contour);
        if (area < 1000)
        {
            continue;
        }

        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, 0.04 * cv::arcLength(contour, true), true);

        if (approx.size() == 4 && cv::isContourConvex(approx))
        {
            if (area > maxArea)
            {
                bestQuad = approx;
                maxArea = area;
            }
        }
    }

    if (bestQuad.size() != 4)
    {
        return false;
    }

    std::vector<cv::Point2f> pts;
    for (int i = 0; i < 4; i++)
    {
        pts.push_back(cv::Point2f(bestQuad[i].x, bestQuad[i].y));
    }
    std::sort(pts.begin(), pts.end(), [](const cv::Point2f &a, const cv::Point2f &b)
              { return a.y < b.y; });

    cv::Point2f tl = pts[0].x < pts[1].x ? pts[0] : pts[1];
    cv::Point2f tr = pts[0].x > pts[1].x ? pts[0] : pts[1];
    cv::Point2f bl = pts[2].x < pts[3].x ? pts[2] : pts[3];
    cv::Point2f br = pts[2].x > pts[3].x ? pts[2] : pts[3];

    std::vector<cv::Point2f> orderedPts = {tl, tr, br, bl};

    for (int i = 0; i < 4; i++)
    {
        outCorners[i * 2 + 0] = orderedPts[i].x;
        outCorners[i * 2 + 1] = orderedPts[i].y;
    }

    return true;
}

void printMat(const cv::Mat &m, const std::string &label)
{
    std::cout << label << std::endl;
    for (int r = 0; r < m.rows; r++)
    {
        for (int c = 0; c < m.cols; c++)
        {
            std::cout << (int)m.at<uchar>(r, c) << "\t";
        }
        std::cout << std::endl;
    }
}

cv::Mat letterbox(const cv::Mat &src, int targetSize, uint8_t padValue = 114)
{
    int srcH = src.rows;
    int srcW = src.cols;

    float scale = std::min((float)targetSize / srcH, (float)targetSize / srcW);

    int scaledH = (int)std::round(srcH * scale);
    int scaledW = (int)std::round(srcW * scale);

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(scaledW, scaledH), 0, 0, cv::INTER_LINEAR);

    int padTop = (targetSize - scaledH) / 2;
    int padBottom = targetSize - scaledH - padTop;
    int padLeft = (targetSize - scaledW) / 2;
    int padRight = targetSize - scaledW - padLeft;

    cv::Mat out;
    cv::copyMakeBorder(resized, out, padTop, padBottom, padLeft, padRight, cv::BORDER_CONSTANT, cv::Scalar(padValue, padValue, padValue));

    return out;
}

std::vector<float> toTensor(const cv::Mat &bgrImg)
{
    cv::Mat rgb;
    cv::cvtColor(bgrImg, rgb, cv::COLOR_BGR2RGB);

    cv::Mat floatImg;
    rgb.convertTo(floatImg, CV_32FC3, 1.0 / 255.0);

    int H = floatImg.rows;
    int W = floatImg.cols;
    int C = 3;
    std::vector<float> tensor(C * H * W);

    for (int c = 0; c < C; c++)
    {
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
            {
                tensor[c * H * W + h * W + w] = floatImg.at<cv::Vec3f>(h, w)[c];
            }
        }
    }

    return tensor;
}

cv::Mat letterbox2(cv::Mat &src, int targetSize, uint8_t padValue)
{
    int srcH = src.rows;
    int srcW = src.cols;

    float scale = std::min((float)targetSize / srcH, (float)targetSize / srcW);

    int scaledH = (int)std::round(srcH * scale);
    int scaledW = (int)std::round(srcW * scale);

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(scaledW, scaledH), 0, 0, cv::INTER_LINEAR);

    int paddingTop = (targetSize - scaledH) / 2;
    int paddingBottom = targetSize - scaledH - paddingTop;
    int paddingLeft = (targetSize - scaledW) / 2;
    int paddingRight = targetSize - scaledW - paddingLeft;

    cv::Mat output;
    cv::copyMakeBorder(resized, output, paddingTop, paddingBottom, paddingLeft, paddingRight, cv::BORDER_CONSTANT, cv::Scalar(padValue, padValue, padValue));

    return output;
}

std::vector<float> preprocess(cv::Mat &bgraImage)
{
    // STEP 1: Resize
    cv::Mat lb = letterbox2(bgraImage, 6, 114);

    // STEP 2: Convert to rgb
    cv::Mat rgb;
    cv::cvtColor(lb, rgb, cv::COLOR_BGR2RGB);

    // STEP 3: Convert to float and normalize
    cv::Mat floatImg;
    rgb.convertTo(floatImg, CV_32FC3, 1.0 / 255.0);

    int H = floatImg.rows;
    int W = floatImg.cols;
    int C = 3;

    std::vector<float> tensor(H * W * C);

    for (int c = 0; c < C; c++)
    {
        for (int h = 0; h < H; h++)
            for (int w = 0; w < W; w++)
            {
                tensor[c * H * W + h * W + w] = floatImg.at<cv::Vec3f>(h, w)[c];
            }
    }

    return tensor;
}

int main()
{
    cv::Mat frame(10, 6, CV_8UC3, cv::Scalar(200, 100, 50));
    std::cout << "Input: " << frame.rows << "x" << frame.cols << "\n";

    std::vector<float> tensor = preprocess(frame);

    int H = 6, W = 6, C = 3;
    std::string chNames[] = {"R", "G", "B"};
    for (int c = 0; c < C; c++)
    {
        std::cout << "Channel " << chNames[c] << ":\n";
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
                std::cout << tensor[c * H * W + h * W + w] << "\t";
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    return 0;
}
