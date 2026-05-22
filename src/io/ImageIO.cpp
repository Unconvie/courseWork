#include "visionCore/io/ImageIO.hpp"

#include <system_error>
#include <utility>

#include <opencv2/imgcodecs.hpp>

namespace visionCore::io {

core::Image ImageIO::read(const std::filesystem::path& path) {
    std::error_code ec;

    if (!std::filesystem::exists(path, ec) || ec) {
        throw ImageIoError("Файл не существует: " + path.string());
    }
    if (!std::filesystem::is_regular_file(path, ec) || ec) {
        throw ImageIoError("Путь не является обычным файлом: " + path.string());
    }

    cv::Mat data = cv::imread(path.string(), cv::IMREAD_UNCHANGED);
    if (data.empty()) {
        throw ImageIoError(
            "OpenCV не смог распознать файл как изображение: " + path.string());
    }

    return core::Image{std::move(data), path};
}

void ImageIO::write(const core::Image& image,
                    const std::filesystem::path& path,
                    int jpegQuality) {
    if (image.empty()) {
        throw ImageIoError("Попытка записать пустое изображение в "
                           + path.string());
    }

    // Если родительская папка не существует - создаём её. Это удобно при
    // первом запуске, когда пользователь не подготовил структуру вывода.
    std::error_code ec;
    const auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            throw ImageIoError("Не удалось создать каталог "
                               + parent.string() + ": " + ec.message());
        }
    }

    std::vector<int> params;
    const auto ext = path.extension().string();
    if (ext == ".jpg" || ext == ".jpeg" || ext == ".JPG" || ext == ".JPEG") {
        params = {cv::IMWRITE_JPEG_QUALITY, jpegQuality};
    }

    const bool ok = params.empty()
        ? cv::imwrite(path.string(), image.data())
        : cv::imwrite(path.string(), image.data(), params);
    if (!ok) {
        throw ImageIoError("OpenCV не смог записать файл: " + path.string());
    }
}

} // namespace visionCore::io
