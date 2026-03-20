#pragma once
#include <atomic>
#include <cstdint>


#include "cameralibrary.h"

using namespace CameraLibrary;

namespace CameraHelper {

    inline bool hasPrefix(const char* s, const char* prefix) {
        if (!s || !prefix) return false;
        const size_t n = std::strlen(prefix);
        return std::strncmp(s, prefix, n) == 0;
    }

    inline bool IsPrimeColor(const CameraLibrary::Camera& cam) {
        return hasPrefix(cam.Name(), "Prime Color");
    }

    inline bool IsPrimeX(const CameraLibrary::Camera& cam) {
        return hasPrefix(cam.Name(), "PrimeX ");
    }

    inline bool IsSlimX(const CameraLibrary::Camera& cam) {
        return hasPrefix(cam.Name(), "SlimX ");
    }

    inline bool IsVersaX(const CameraLibrary::Camera& cam) {
        return hasPrefix(cam.Name(), "VersaX ");
    }

    inline bool IsXCamera(const CameraLibrary::Camera& cam) {
        return (IsPrimeX(cam) || IsVersaX(cam) || IsSlimX(cam));
    }

    inline float MbpsToNormalized(float mbps) {
        float v = mbps / 100.0f;
        return std::clamp(v, 0.0f, 1.0f);
    }

    inline double frameSeconds(const Frame& f) {
        if (f.IsHardwareTimeStamp())
            return f.HardwareTimeStampValue();
        return f.TimeStamp();
    }

    class FrameRateCalculator {
        public:
            explicit FrameRateCalculator(double alpha = 0.5) : alpha_(alpha) {}

            void update(const Frame& f) {
                const int fid = f.FrameID();
                const double ts = f.IsHardwareTimeStamp()
                                ? f.HardwareTimeStampValue()
                                : f.TimeStamp();
                updateFromFrameInfo(fid, ts);
            }

            void reset() {
                std::lock_guard<std::mutex> lk(m_);
                last_id_ = -1;
                last_ts_ = 0.0;
                ewma_fps_ = 0.0;
                current_fps_.store(0.0, std::memory_order_relaxed);
            }

            double current(std::memory_order order = std::memory_order_relaxed) const {
                return current_fps_.load(order);
            }

            double current_locked() const {
                std::lock_guard<std::mutex> lk(m_);
                return ewma_fps_;
            }

        private:
            std::atomic<double> current_fps_{0.0};
            double alpha_{0.5};

            mutable std::mutex m_;
            int    last_id_  {-1};
            double last_ts_  {0.0};
            double ewma_fps_ {0.0};

            void updateFromFrameInfo(int frame_id, double ts_seconds) 
            {
                std::lock_guard<std::mutex> lk(m_);
                if (last_id_ >= 0) {
                    const int    dF = frame_id - last_id_;
                    const double dT = ts_seconds - last_ts_;
                    if (dF > 0 && dT > 0.0) {
                        const double inst = static_cast<double>(dF) / dT;
                        ewma_fps_ = (ewma_fps_ == 0.0) ? inst
                                                    : (1.0 - alpha_) * ewma_fps_ + alpha_ * inst;
                        current_fps_.store(ewma_fps_, std::memory_order_relaxed);
                    }
                }
                last_id_ = frame_id;
                last_ts_ = ts_seconds;
            }
    };
}