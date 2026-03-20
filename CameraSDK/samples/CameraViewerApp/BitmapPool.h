#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include <functional>

namespace CameraLibrary { class Bitmap; }

class BitmapPool {
public:
    using Bitmap = CameraLibrary::Bitmap;
    using BitmapFactory = std::function<Bitmap*(int w, int h, int bpp, int stride)>;

    explicit BitmapPool(BitmapFactory factory);
    Bitmap* acquire(int w, int h, int bpp, int stride);

    void release(Bitmap* b);

    BitmapPool(const BitmapPool&) = delete;
    BitmapPool& operator=(const BitmapPool&) = delete;
    BitmapPool(BitmapPool&&) = delete;
    BitmapPool& operator=(BitmapPool&&) = delete;

private:
    struct BmpKey {
        int w, h, bpp, stride;
        bool operator==(const BmpKey& o) const {
            return w == o.w && h == o.h && bpp == o.bpp && stride == o.stride;
        }
    };

    struct PooledBmp {
        BmpKey k;
        std::unique_ptr<Bitmap> bmp;
    };

    std::mutex bitmap_mutex;
    std::vector<PooledBmp> bitmap_pool;
    BitmapFactory bitmap_factory;
};
