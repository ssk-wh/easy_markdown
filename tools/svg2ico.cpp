#include <QApplication>
#include <QSvgRenderer>
#include <QImage>
#include <QPainter>
#include <QFile>
#include <QDataStream>
#include <QBuffer>
#include <vector>
#include <cstdio>

bool writeIco(const QString& path, const std::vector<QImage>& images) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;
    QDataStream ds(&file);
    ds.setByteOrder(QDataStream::LittleEndian);

    ds << quint16(0) << quint16(1) << quint16(images.size());

    std::vector<QByteArray> pngDatas;
    for (auto& img : images) {
        QByteArray ba;
        QBuffer buf(&ba);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        pngDatas.push_back(ba);
    }

    int offset = 6 + (int)images.size() * 16;
    for (size_t i = 0; i < images.size(); i++) {
        int w = images[i].width();
        int h = images[i].height();
        ds << quint8(w >= 256 ? 0 : w);
        ds << quint8(h >= 256 ? 0 : h);
        ds << quint8(0) << quint8(0);
        ds << quint16(1) << quint16(32);
        ds << quint32(pngDatas[i].size());
        ds << quint32(offset);
        offset += pngDatas[i].size();
    }

    for (auto& data : pngDatas) {
        file.write(data);
    }
    return true;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    if (argc < 4) {
        fprintf(stderr, "Usage: svg2ico <input.svg> <output.ico> <output.png>\n");
        return 1;
    }

    QSvgRenderer renderer(QString::fromLocal8Bit(argv[1]));
    if (!renderer.isValid()) {
        fprintf(stderr, "Failed to load SVG: %s\n", argv[1]);
        return 1;
    }

    int sizes[] = {16, 24, 32, 48, 64, 128, 256};
    std::vector<QImage> images;

    for (int s : sizes) {
        QImage img(s, s, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter painter(&img);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        renderer.render(&painter);
        painter.end();
        images.push_back(img);
    }

    // Save 256px PNG
    images.back().save(QString::fromLocal8Bit(argv[3]));
    printf("Saved PNG: %s\n", argv[3]);

    // Save ICO
    if (writeIco(QString::fromLocal8Bit(argv[2]), images)) {
        printf("Saved ICO: %s (%zu sizes)\n", argv[2], images.size());
    }
    return 0;
}
