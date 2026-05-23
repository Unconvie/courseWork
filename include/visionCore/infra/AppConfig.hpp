#pragma once

namespace visionCore::infra
{

	/**
	 * @brief Параметры алгоритмов pipeline (значения по умолчанию).
	 *
	 * ═══════════════════════════════════════════════════════════════════
	 *  ГЛАВНЫЙ ФАЙЛ НАСТРОЕК «ЧУВСТВИТЕЛЬНОСТИ»
	 *  Меняешь числа здесь → пересобираешь проект → снова запускаешь exe.
	 *  В Фазе 4 те же поля планируется читать из JSON (--config).
	 * ═══════════════════════════════════════════════════════════════════
	 *
	 * Объект создаётся в main.cpp и передаётся в PipelineFactory, оттуда —
	 * в GaussianMedianFilter, ThresholdContourDetector, ContourFeatureExtractor,
	 * ShapeClassifier.
	 */
	struct AppConfig
	{

		// ─── Файл *-noiseFree (шаг INoiseFilter) ─────────────────────────────
		/// Диаметр bilateralFilter на цветном изображении (0 = не использовать).
		int bilateralDiameter = 5;

		/// Доп. размытие grayscale перед noiseFree (0 = выкл.).
		double gaussianSigma = 0.0;

		/// Median-фильтр grayscale (0 = выкл.).
		int medianKernel = 0;

		// ─── Детекция (шаг IDetector) ────────────────────────────────────────
		/// Сглаживание перед поиском контуров (0 = резкие края, лучше для обводок).
		double detectGaussianSigma = 0.0;

		/// false + thresholdValue = фиксированный порог вместо adaptiveThreshold.
		bool adaptiveThreshold = true;
		int thresholdValue = 127;

		/// CLAHE: усиливает слабый контраст (бледный квадрат на белом фоне).
		bool useClahe = true;
		double claheClipLimit = 3.0;

		/// Canny: основной и «слабый» проход для тонких обводок.
		bool useCannyEdges = true;
		int cannyLow = 35;
		int cannyHigh = 110;
		int faintCannyLow = 12;
		int faintCannyHigh = 55;
		double minFaintContourArea = 160.0;

		/// HoughCircles: залитые/пересекающиеся круги (выше param2 → меньше ложных).
		bool useHoughCircles = true;
		double houghDp = 1.2;
		double houghMinDist = 70.0;
		double houghParam1 = 100.0;
		double houghParam2 = 34.0;
		int houghMinRadius = 28;
		int houghMaxRadius = 130;

		/// Мин. площадь контура (px²): меньше значение → больше мелких объектов.
		double minContourArea = 80.0;
		double minCannyContourArea = 320.0;

		/// Вложенная обводка (квадрат в круге): сохранять, если площадь ≥ доли внешней.
		double nestedOutlineKeepMinRatio = 0.05;

		/// Удалять только «пятна» внутри круга (доля площади + высокая circularity).
		double nestedBlobSuppressMaxRatio = 0.28;

		/// Мин. сторона bbox; отсекает линии толщиной 1–2 px.
		int minBboxSide = 12;

		/// Не брать контур на весь кадр (доля площади кадра).
		double maxContourAreaRatio = 0.82;

		/// IoU bbox для слияния дубликатов (выше → агрессивнее объединяет).
		double contourDedupeIou = 0.62;

		// ─── Классификация (шаг IClassifier / ShapeClassifier) ───────────────
		/// Ниже этого порога top-класс → Unknown (см. ShapeClassifier.cpp).
		float minClassificationConfidence = 0.18f;

		/// Доля периметра для approxPolyDP → vertexCount → Triangle/Hexagon.
		double contourApproxEpsilon = 0.015;

		// ─── Сохранение на диск (main.cpp) ───────────────────────────────────
		/// Качество JPEG при записи *-noiseFree / *-shapeOutline [0..100].
		int jpegQuality = 90;

		/// Писать *-report.txt с результатами (можно отключить флагом --no-report).
		bool saveReportByDefault = true;
	};

} // namespace visionCore::infra
