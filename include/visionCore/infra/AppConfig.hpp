#pragma once

namespace visionCore::infra {

/**
 * @brief Параметры алгоритмов pipeline (значения по умолчанию).
 *
 * В Фазе 4 будет загрузка из JSON; сейчас задаётся в коде и при
 * необходимости меняется перед созданием pipeline.
 */
struct AppConfig {
    /// Сглаживание для файла *-noiseFree (bilateral на цвете; 0 = только копия).
    int bilateralDiameter = 5;

    /// Сигма Gaussian на grayscale (legacy; 0 = не применять).
    double gaussianSigma = 0.0;

    /// Median на grayscale (0 = не применять).
    int medianKernel = 0;

    /// Лёгкое сглаживание перед детекцией (0 = резкие контуры).
    double detectGaussianSigma = 0.0;

    /// Порог бинаризации [0..255], если adaptiveThreshold = false.
    int thresholdValue = 127;

    /// Использовать cv::adaptiveThreshold вместо фиксированного порога.
    bool adaptiveThreshold = true;

    /// Повысить контраст слабых линий (бледный квадрат).
    bool useClahe = true;
    double claheClipLimit = 3.0;

    /// Дополнительно искать тонкие контуры через Canny.
    bool useCannyEdges = true;

    int cannyLow = 35;
    int cannyHigh = 110;

    /// Второй проход Canny для очень слабых обводок.
    int faintCannyLow = 12;
    int faintCannyHigh = 55;
    double minFaintContourArea = 120.0;

    /// HoughCircles для залитых/пересекающихся кругов.
    bool useHoughCircles = true;
    double houghDp = 1.2;
    double houghMinDist = 55.0;
    double houghParam1 = 100.0;
    double houghParam2 = 28.0;
    int houghMinRadius = 25;
    int houghMaxRadius = 140;

    /// Минимальная площадь контура (залитые области), px².
    double minContourArea = 60.0;

    /// Минимальная площадь для контуров с Canny (тонкие линии), px².
    double minCannyContourArea = 280.0;

    /// Удалять контур, если он сильно меньше и внутри другого bbox.
    double nestedMaxAreaRatio = 0.42;

    /// Минимальная сторона bbox (отсев «линий» толщиной 1–3 px).
    int minBboxSide = 12;

    /// Не брать контур, занимающий больше этой доли кадра (рамка/фон).
    double maxContourAreaRatio = 0.82;

    /// Слияние дубликатов: IoU bbox выше порога — оставить больший контур.
    double contourDedupeIou = 0.55;

    /// Минимальная уверенность top-класса для вывода (иначе Unknown).
    float minClassificationConfidence = 0.18f;

    /// Точность аппроксимации контура (Дуглас–Пёкер), доля периметра.
    double contourApproxEpsilon = 0.015;

    /// Качество JPEG при сохранении производных файлов [0..100].
    int jpegQuality = 90;
};

} // namespace visionCore::infra
