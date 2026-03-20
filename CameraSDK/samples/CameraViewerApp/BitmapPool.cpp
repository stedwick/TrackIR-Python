#include "BitmapPool.h"
#include "bitmap.h"

using Bitmap = CameraLibrary::Bitmap;

BitmapPool::BitmapPool(BitmapFactory factory)
    : bitmap_factory(std::move(factory)) {}

Bitmap* BitmapPool::acquire(int w, int h, int bpp, int stride) {
    std::lock_guard<std::mutex> lk(bitmap_mutex);
    BmpKey want{w, h, bpp, stride};
    for (size_t i = 0; i < bitmap_pool.size(); ++i) {
        if (bitmap_pool[i].k == want) {
            auto* out = bitmap_pool[i].bmp.release();
            bitmap_pool.erase(bitmap_pool.begin() + static_cast<long>(i));
            return out;
        }
    }
    // Not found — create a new one via the provided factory.
    return bitmap_factory(w, h, bpp, stride);
}

void BitmapPool::release(Bitmap* b) {
    if (!b) return;
    const int w = b->PixelWidth();
    const int h = b->PixelHeight();
    const int bpp = b->GetBitsPerPixel();
    const int stride = b->ByteSpan();

    std::lock_guard<std::mutex> lk(bitmap_mutex);
    bitmap_pool.push_back(PooledBmp{BmpKey{w, h, bpp, stride}, std::unique_ptr<Bitmap>(b)});
}
