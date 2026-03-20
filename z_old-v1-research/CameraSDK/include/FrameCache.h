#pragma once

#include <vector>
#include <memory>

namespace CameraLibrary {

class Frame;

class FrameCache {

public:
   static constexpr size_t CACHE_SIZE = 10U;
   static constexpr const char* LOGGER_TAG = "frame-cache";

   FrameCache(size_t cacheSize = CACHE_SIZE);

   std::shared_ptr< CameraLibrary::Frame> GetFrame(int frameId) const;
   std::shared_ptr< CameraLibrary::Frame> PopFrame(int frameId);
   void AddFrame(const std::shared_ptr<Frame> &frame);
   void ClearFrames();
   void Resize(size_t newSize);
   
private:

   std::vector<std::shared_ptr<CameraLibrary::Frame>> mCachedFrames;
   size_t mCurrentIndex = 0U;
};

} // namespace CameraLibrary
