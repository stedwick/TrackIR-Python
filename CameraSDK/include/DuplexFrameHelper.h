#pragma once

#include <memory>

#include "FrameCache.h"
#include "frame.h"

namespace CoreLogging
{
class cLogger;
}

namespace CameraLibrary {

class DuplexFrameHelper {

public:

   DuplexFrameHelper() = default;

   std::shared_ptr< CameraLibrary::Frame> GetCachedObjectFrame(int frameId) const;
   std::shared_ptr< CameraLibrary::Frame> PopCachedObjectFrame(int frameId);
   bool AddObjectFrame(const std::shared_ptr<Frame> &frame);
   void ClearObjectFrames();

   std::shared_ptr<CameraLibrary::Frame> GetCachedVideoFrame(int frameId) const;
   std::shared_ptr<CameraLibrary::Frame> PopCachedVideoFrame(int frameId);
   bool AddVideoFrame(const std::shared_ptr<Frame> &frame);
   void ClearVideoFrames();

   void ClearAll();
   void ResizeFrameCaches(size_t newSize);
   
private:

   FrameCache mCachedObjectFrames;
   FrameCache mCachedVideoFrames;
   std::shared_ptr<CoreLogging::cLogger> mLogger;
};

/*
 * Inline Implementations
 */
inline void DuplexFrameHelper::ResizeFrameCaches(size_t newSize)
{
    mCachedObjectFrames.Resize(newSize);
    mCachedVideoFrames.Resize(newSize);
}

inline
bool DuplexFrameHelper::AddObjectFrame(const std::shared_ptr<Frame>& frame)
{
   if(frame->FrameType() != Core::eVideoMode::ObjectMode) {
      return false;
   }

   mCachedObjectFrames.AddFrame(frame);
   return true;
}

inline
bool DuplexFrameHelper::AddVideoFrame(const std::shared_ptr<Frame>& frame)
{
   if(frame->FrameType() != Core::eVideoMode::MJPEGMode) {
      return false;
   }

   mCachedVideoFrames.AddFrame(frame);
   return true;
}

inline
std::shared_ptr<CameraLibrary::Frame> DuplexFrameHelper::GetCachedObjectFrame(int frameId) const
{
   return mCachedObjectFrames.GetFrame(frameId);
}

inline
std::shared_ptr<CameraLibrary::Frame> DuplexFrameHelper::PopCachedObjectFrame(int frameId)
{
   return mCachedObjectFrames.PopFrame(frameId);
}

inline
std::shared_ptr<CameraLibrary::Frame> DuplexFrameHelper::PopCachedVideoFrame(int frameId)
{
   return mCachedVideoFrames.PopFrame(frameId);
}

inline
std::shared_ptr<CameraLibrary::Frame> DuplexFrameHelper::GetCachedVideoFrame(int frameId) const
{
   return mCachedVideoFrames.GetFrame(frameId);
}

inline
void DuplexFrameHelper::ClearObjectFrames()
{
   mCachedObjectFrames.ClearFrames();
}

inline
void DuplexFrameHelper::ClearVideoFrames()
{
   mCachedVideoFrames.ClearFrames();
}

inline
void DuplexFrameHelper::ClearAll()
{
   ClearObjectFrames();
   ClearVideoFrames();
}

} // namespace CameraLibrary
