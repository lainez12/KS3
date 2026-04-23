#pragma once

#include <QImage>
#include <QObject>
#include <QString>
#include <atomic>
#include <optional>
#include <string>

#include <MvCameraControl.h>

#include "../ICamera.h"
#include <Config/machine_config.h>

namespace Kub3::HAL::Vision
{
    // Struct to hold GenICam node limits to ensure valid states
    template <typename T>
    struct NodeBounds {
        T min;
        T max;
        T inc;
    };

    class HikrobotCamera final : public ICamera
    {
        Q_OBJECT
    public:
        HikrobotCamera(Config::camera_config_t config);
        ~HikrobotCamera() override;

        [[nodiscard]] std::string getId() const override
        {
            return m_config.id;
        }

        bool connectDevice(void) override;
        void disconnectDevice(void) override;

        void startAcquisition(void) override;
        void stopAcquisition(void) override;

        // --- Consolidated Configuration API ---
        bool setExposure(double exposureUs) override;
        bool setGain(double gainDB) override;
        bool setFrameRate(double fps) override;
        bool setCenteredZoom(double zoom) override;
        bool setROI(int x, int y, int width, int height) override;

        // Smart Reduction: Attempts Binning first, falls back to Decimation
        bool setSmartReduction(int horizontal, int vertical);
        bool setReverse(bool flipX, bool flipY);

        // --- Getters for UI/Logic mapping ---
        [[nodiscard]] std::optional<double> getCurrentFPS() const;
        [[nodiscard]] std::optional<NodeBounds<int64_t>> getROIBounds(const char *dimensionKey) const;

    signals:
        // Hardware Watchdog Signals
        void s_connectionLost(const QString &reason);

    private slots:
        void acquisitionLoop(void);

    private:
        // Generic GenICam Node Accessors (C++20 std::optional for safety)
        bool setFloatNode(const char *key, float value);
        bool setIntNode(const char *key, int64_t value);
        bool setEnumNode(const char *key, int32_t value);
        bool setBoolNode(const char *key, bool value);
        bool executeCommandNode(const char *key);

        [[nodiscard]] std::optional<float> getFloatNode(const char *key) const;
        [[nodiscard]] std::optional<int64_t> getIntNode(const char *key) const;

    private:
        const Config::camera_config_t m_config;

        void *m_cameraHandle            = nullptr;
        std::atomic<bool> m_isAcquiring = false;

        MVCC_INTVALUE m_offsetXLimits;
        MVCC_INTVALUE m_offsetYLimits;
        MVCC_INTVALUE m_widthLimits;
        MVCC_INTVALUE m_heightLimits;

        // Watchdog state
        static constexpr int MAX_FRAME_DROPS = 5;
        int m_consecutiveDropCount           = 0;
    };

} // namespace Kub3::HAL::Vision
