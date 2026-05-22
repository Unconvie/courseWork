#pragma once

#include <filesystem>
#include <string>

#include <opencv2/core.hpp>

namespace visionCore::core {

/**
 * @brief Доменная обёртка над пиксельными данными изображения.
 *
 * Несёт две вещи:
 *  1. Матрицу пикселей (cv::Mat) - то, над чем работают алгоритмы.
 *  2. Метаданные о происхождении - путь к исходному файлу.
 *
 * Зачем нужны метаданные. По pipeline дальше должны идти не только пиксели,
 * но и знание, откуда они пришли: иначе аннотатор не сможет сформировать
 * имя выходного файла вида "<stem>-shapeOutline.png" в той же папке.
 * Хранить путь рядом с матрицей удобнее, чем пробрасывать его параметром
 * через все шаги.
 *
 * Этот класс намеренно не занимается ни чтением, ни записью файлов - этим
 * ведает visionCore::io::ImageIO. Цель Image - быть «существительным»,
 * над которым работают «глаголы»-шаги pipeline.
 */
class Image {
public:
    /**
     * @brief Создать пустое изображение без данных и метаданных.
     *
     * Полезно для возврата «ничего не получилось» там, где исключения
     * нежелательны, либо как промежуточное состояние при поэтапной сборке.
     */
    Image() = default;

    /**
     * @brief Создать изображение с пиксельными данными и метаданными.
     * @param data Матрица пикселей OpenCV.
     * @param sourcePath Исходный путь к файлу (пустой - если изображение
     *        создано в памяти и нет осмысленного источника).
     */
    Image(cv::Mat data, std::filesystem::path sourcePath);

    /**
     * @brief Признак пустого изображения (нет пиксельных данных).
     */
    [[nodiscard]] bool empty() const noexcept { return m_data.empty(); }

    /**
     * @brief Ширина изображения в пикселях.
     */
    [[nodiscard]] int width() const noexcept { return m_data.cols; }

    /**
     * @brief Высота изображения в пикселях.
     */
    [[nodiscard]] int height() const noexcept { return m_data.rows; }

    /**
     * @brief Число каналов: 1 = grayscale, 3 = BGR, 4 = BGRA.
     *
     * Важно: OpenCV хранит цвет в порядке B-G-R, а не R-G-B.
     */
    [[nodiscard]] int channels() const noexcept { return m_data.channels(); }

    /**
     * @brief Доступ к матрице пикселей (только чтение).
     *
     * Возвращает константную ссылку, чтобы случайно не модифицировать
     * данные через геттер. Для модификации используется неконстантная
     * перегрузка ниже.
     */
    [[nodiscard]] const cv::Mat& data() const noexcept { return m_data; }

    /**
     * @brief Доступ к матрице пикселей (с возможностью модификации).
     *
     * Использовать осознанно: cv::Mat внутри reference-counted, и обычно
     * это единственный владелец, но cv::Mat::clone() при необходимости
     * глубокого копирования - ответственность вызывающего кода.
     */
    [[nodiscard]] cv::Mat& data() noexcept { return m_data; }

    /**
     * @brief Исходный путь к файлу.
     * @return Путь, переданный в конструктор. Может быть пустым.
     */
    [[nodiscard]] const std::filesystem::path& sourcePath() const noexcept {
        return m_sourcePath;
    }

    /**
     * @brief Имя файла без расширения (stem).
     *
     * Например, для "C:/data/photo.jpg" вернёт "photo". Используется для
     * формирования имён производных файлов: "<stem>-noiseFree<ext>",
     * "<stem>-shapeOutline<ext>" и т.п.
     */
    [[nodiscard]] std::string stem() const;

    /**
     * @brief Расширение исходного файла, включая точку (например ".jpg").
     *
     * При сохранении производных файлов мы по умолчанию сохраняем то же
     * расширение, что и у источника, чтобы не менять формат хранения без
     * явного указания.
     */
    [[nodiscard]] std::string extension() const;

private:
    /// Пиксельные данные. Пустая матрица означает «нет данных».
    cv::Mat m_data;

    /// Исходный путь к файлу. Пустой путь - изображение создано в памяти.
    std::filesystem::path m_sourcePath;
};

} // namespace visionCore::core
