#include "visionCore/tools/SampleImageGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace visionCore::tools {

namespace {

const int kBg = 245;
const cv::Scalar kBgColor(kBg, kBg, kBg);
const cv::Scalar kInk(25, 25, 25);
const cv::Scalar kFaintInk(175, 175, 175);

struct Scene {
    const char* fileName;
    void (*build)(cv::Mat& canvas, std::mt19937& rng);
};

cv::Mat blankCanvas(int width, int height) {
    return cv::Mat(height, width, CV_8UC3, kBgColor);
}

std::vector<cv::Point> regularPolygon(cv::Point center, int radius, int sides,
                                      double rotationRad = -CV_PI / 2.0) {
    std::vector<cv::Point> pts;
    pts.reserve(static_cast<size_t>(sides));
    for (int i = 0; i < sides; ++i) {
        const double a = rotationRad + (2.0 * CV_PI * i) / sides;
        pts.emplace_back(
            center.x + static_cast<int>(std::lround(radius * std::cos(a))),
            center.y + static_cast<int>(std::lround(radius * std::sin(a))));
    }
    return pts;
}

void drawAllShapesFilled(cv::Mat& img) {
    cv::circle(img, {140, 150}, 55, kInk, cv::FILLED, cv::LINE_AA);

    const auto tri = regularPolygon({320, 150}, 62, 3);
    cv::fillPoly(img, tri, kInk, cv::LINE_AA);

    cv::rectangle(img, cv::Rect(430, 95, 110, 110), kInk, cv::FILLED, cv::LINE_AA);

    cv::rectangle(img, cv::Rect(600, 110, 150, 85), kInk, cv::FILLED, cv::LINE_AA);

    const auto hex = regularPolygon({140, 400}, 58, 6);
    cv::fillPoly(img, hex, kInk, cv::LINE_AA);

    const auto tri2 = regularPolygon({340, 400}, 50, 3, CV_PI / 6.0);
    cv::polylines(img, tri2, true, kInk, 3, cv::LINE_AA);

    cv::circle(img, {560, 400}, 48, kInk, 3, cv::LINE_AA);
}

void drawFaintOutlines(cv::Mat& img) {
    const auto tri = regularPolygon({200, 180}, 70, 3);
    cv::polylines(img, tri, true, kFaintInk, 2, cv::LINE_AA);

    cv::rectangle(img, cv::Rect(360, 120, 120, 120), kFaintInk, 2, cv::LINE_AA);

    cv::circle(img, {620, 180}, 60, kFaintInk, 2, cv::LINE_AA);

    const auto hex = regularPolygon({200, 430}, 65, 6);
    cv::polylines(img, hex, true, kFaintInk, 2, cv::LINE_AA);

    cv::rectangle(img, cv::Rect(400, 360, 140, 90), kFaintInk, 2, cv::LINE_AA);

    cv::circle(img, {640, 420}, 55, kFaintInk, 2, cv::LINE_AA);
}

void drawNestedOutline(cv::Mat& img) {
    cv::circle(img, {400, 300}, 120, kInk, 3, cv::LINE_AA);
    cv::rectangle(img, cv::Rect(355, 255, 90, 90), kInk, 3, cv::LINE_AA);
}

void drawOverlappingCircles(cv::Mat& img) {
    cv::circle(img, {330, 300}, 85, kInk, cv::FILLED, cv::LINE_AA);
    cv::circle(img, {470, 300}, 85, kInk, cv::FILLED, cv::LINE_AA);
}

void drawMixedFillOutline(cv::Mat& img) {
    cv::circle(img, {150, 150}, 50, kInk, cv::FILLED, cv::LINE_AA);
    cv::circle(img, {350, 150}, 55, kInk, 3, cv::LINE_AA);

    const auto tri = regularPolygon({550, 150}, 60, 3);
    cv::fillPoly(img, tri, kInk, cv::LINE_AA);

    cv::rectangle(img, cv::Rect(120, 330, 100, 100), kInk, 3, cv::LINE_AA);
    cv::rectangle(img, cv::Rect(300, 340, 130, 80), kInk, cv::FILLED, cv::LINE_AA);

    const auto hex = regularPolygon({560, 390}, 55, 6);
    cv::polylines(img, hex, true, kInk, 3, cv::LINE_AA);
}

void addGaussianNoise(cv::Mat& img, std::mt19937& rng, double sigma) {
    cv::Mat noise(img.size(), CV_8UC3);
    std::normal_distribution<> dist(0.0, sigma);
    for (int y = 0; y < img.rows; ++y) {
        auto* row = img.ptr<cv::Vec3b>(y);
        for (int x = 0; x < img.cols; ++x) {
            for (int c = 0; c < 3; ++c) {
                const int v = static_cast<int>(std::lround(row[x][c] + dist(rng)));
                row[x][c] = static_cast<uchar>(std::clamp(v, 0, 255));
            }
        }
    }
}

void addSaltAndPepper(cv::Mat& img, std::mt19937& rng, double ratio) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    const int count = static_cast<int>(img.total() * ratio);
    for (int i = 0; i < count; ++i) {
        const int y = static_cast<int>(rng() % img.rows);
        const int x = static_cast<int>(rng() % img.cols);
        const uchar v = (dist(rng) < 0.5) ? 0 : 255;
        img.at<cv::Vec3b>(y, x) = cv::Vec3b(v, v, v);
    }
}

void addSpeckleNoise(cv::Mat& img, std::mt19937& rng, double strength) {
    std::normal_distribution<> dist(0.0, strength);
    for (int y = 0; y < img.rows; ++y) {
        auto* row = img.ptr<cv::Vec3b>(y);
        for (int x = 0; x < img.cols; ++x) {
            const double factor = 1.0 + dist(rng);
            for (int c = 0; c < 3; ++c) {
                const int v =
                    static_cast<int>(std::lround(row[x][c] * factor));
                row[x][c] = static_cast<uchar>(std::clamp(v, 0, 255));
            }
        }
    }
}

void addUniformNoise(cv::Mat& img, std::mt19937& rng, int amplitude) {
    std::uniform_int_distribution<> dist(-amplitude, amplitude);
    for (int y = 0; y < img.rows; ++y) {
        auto* row = img.ptr<cv::Vec3b>(y);
        for (int x = 0; x < img.cols; ++x) {
            for (int c = 0; c < 3; ++c) {
                const int v = row[x][c] + dist(rng);
                row[x][c] = static_cast<uchar>(std::clamp(v, 0, 255));
            }
        }
    }
}

void buildClean(cv::Mat& img, std::mt19937&) {
    drawAllShapesFilled(img);
}

void buildGaussian(cv::Mat& img, std::mt19937& rng) {
    drawAllShapesFilled(img);
    addGaussianNoise(img, rng, 18.0);
}

void buildSaltPepper(cv::Mat& img, std::mt19937& rng) {
    drawAllShapesFilled(img);
    addSaltAndPepper(img, rng, 0.04);
}

void buildSpeckle(cv::Mat& img, std::mt19937& rng) {
    drawAllShapesFilled(img);
    addSpeckleNoise(img, rng, 0.12);
}

void buildUniform(cv::Mat& img, std::mt19937& rng) {
    drawAllShapesFilled(img);
    addUniformNoise(img, rng, 22);
}

void buildMixedNoise(cv::Mat& img, std::mt19937& rng) {
    drawAllShapesFilled(img);
    addGaussianNoise(img, rng, 12.0);
    addSaltAndPepper(img, rng, 0.02);
    addSpeckleNoise(img, rng, 0.06);
}

void buildFaintGaussian(cv::Mat& img, std::mt19937& rng) {
    drawFaintOutlines(img);
    addGaussianNoise(img, rng, 8.0);
}

void buildFaintSaltPepper(cv::Mat& img, std::mt19937& rng) {
    drawFaintOutlines(img);
    addSaltAndPepper(img, rng, 0.03);
}

void buildNested(cv::Mat& img, std::mt19937& rng) {
    drawNestedOutline(img);
    addGaussianNoise(img, rng, 6.0);
}

void buildOverlap(cv::Mat& img, std::mt19937& rng) {
    drawOverlappingCircles(img);
    addGaussianNoise(img, rng, 10.0);
    addSaltAndPepper(img, rng, 0.015);
}

void buildMixedShapes(cv::Mat& img, std::mt19937& rng) {
    drawMixedFillOutline(img);
    addGaussianNoise(img, rng, 14.0);
}

void writePng(const std::filesystem::path& path, const cv::Mat& img) {
    std::error_code ec;
    const auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            throw std::runtime_error("Не удалось создать каталог " + parent.string()
                                     + ": " + ec.message());
        }
    }
    if (!cv::imwrite(path.string(), img)) {
        throw std::runtime_error("Не удалось записать " + path.string());
    }
}

const Scene kScenes[] = {
    {"gen-01-clean.png", buildClean},
    {"gen-02-gaussian.png", buildGaussian},
    {"gen-03-salt-pepper.png", buildSaltPepper},
    {"gen-04-speckle.png", buildSpeckle},
    {"gen-05-uniform.png", buildUniform},
    {"gen-06-mixed-noise.png", buildMixedNoise},
    {"gen-07-faint-outlines-gauss.png", buildFaintGaussian},
    {"gen-08-faint-outlines-salt.png", buildFaintSaltPepper},
    {"gen-09-nested-outline.png", buildNested},
    {"gen-10-overlap-circles.png", buildOverlap},
    {"gen-11-mixed-fill-outline.png", buildMixedShapes},
};

}  // namespace

SampleGeneratorResult generateSampleImages(const SampleGeneratorOptions& options) {
    if (options.width < 64 || options.height < 64) {
        throw std::invalid_argument("Размер изображения должен быть не меньше 64×64");
    }

    SampleGeneratorResult result;
    std::mt19937 rng(options.seed);

    for (const Scene& scene : kScenes) {
        cv::Mat canvas = blankCanvas(options.width, options.height);
        scene.build(canvas, rng);

        const auto path = options.outputDir / scene.fileName;
        writePng(path, canvas);

        result.paths.push_back(path);
        ++result.filesWritten;
    }

    result.summary = "Создано файлов: " + std::to_string(result.filesWritten)
                       + " в " + options.outputDir.string();
    return result;
}

}  // namespace visionCore::tools
