#include <QDebug>
#include <QMetaObject>
#include <format>
#include <stdexcept>

#include <HAL/Vision/Hikrobot/HikrobotCamera.h>

namespace Kub3::HAL::Vision
{

    HikrobotCamera::HikrobotCamera(std::string id, std::string serialNumber) :
        m_id(std::move(id)),
        m_serialNumber(std::move(serialNumber))
    {}

    HikrobotCamera::~HikrobotCamera()
    {
        disconnectDevice();
    }

    bool HikrobotCamera::connectDevice(void)
    {
        if (m_cameraHandle)
            return true; // Already connected

        MV_CC_DEVICE_INFO_LIST stDeviceList;
        memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

        // Enum USB and GigE devices
        uint32_t nTLayerType = MV_GIGE_DEVICE | MV_USB_DEVICE;
        if (MV_CC_EnumDevices(nTLayerType, &stDeviceList) != MV_OK || stDeviceList.nDeviceNum == 0)
        {
            qWarning() << "Hikrobot: No devices found on the bus.";
            return false;
        }

        // Find specific camera by serial number
        int deviceIndex = -1;
        for (uint32_t i = 0; i < stDeviceList.nDeviceNum; ++i)
        {
            MV_CC_DEVICE_INFO *pDeviceInfo = stDeviceList.pDeviceInfo[i];
            std::string sn;

            if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
                sn = reinterpret_cast<char *>(pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
            else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
                sn = reinterpret_cast<char *>(pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);

            if (sn == m_serialNumber)
            {
                deviceIndex = static_cast<int>(i);
                break;
            }
        }

        if (deviceIndex == -1)
        {
            qWarning() << std::format("Hikrobot: Camera {} not found.", m_serialNumber).c_str();
            return false;
        }

        // Create Handle & Open
        if (MV_CC_CreateHandle(&m_cameraHandle, stDeviceList.pDeviceInfo[deviceIndex]) != MV_OK)
            return false;
        if (MV_CC_OpenDevice(m_cameraHandle, MV_ACCESS_Exclusive, 0) != MV_OK)
        {
            MV_CC_DestroyHandle(m_cameraHandle);
            m_cameraHandle = nullptr;
            return false;
        }

        // Optimize GigE Packet Size
        if (stDeviceList.pDeviceInfo[deviceIndex]->nTLayerType == MV_GIGE_DEVICE)
        {
            int nPacketSize = MV_CC_GetOptimalPacketSize(m_cameraHandle);
            if (nPacketSize > 0)
                setIntNode("GevSCPSPacketSize", nPacketSize);
        }

        qInfo() << std::format("Hikrobot: Camera {} successfully connected.", m_serialNumber).c_str();
        return true;
    }

    void HikrobotCamera::disconnectDevice(void)
    {
        stopAcquisition();
        if (m_cameraHandle)
        {
            MV_CC_CloseDevice(m_cameraHandle);
            MV_CC_DestroyHandle(m_cameraHandle);
            m_cameraHandle = nullptr;
        }
    }

    // =========================================================================
    // ACQUISITION LOOP
    // =========================================================================

    void HikrobotCamera::startAcquisition(void)
    {
        if (!m_cameraHandle || m_isAcquiring)
            return;

        if (MV_CC_StartGrabbing(m_cameraHandle) == MV_OK)
        {
            m_isAcquiring = true;
            // Start the non-blocking polling loop in the target thread
            QMetaObject::invokeMethod(this, &HikrobotCamera::acquisitionLoop, Qt::QueuedConnection);
        }
    }

    void HikrobotCamera::stopAcquisition(void)
    {
        if (!m_isAcquiring || !m_cameraHandle)
            return;
        m_isAcquiring = false;
        MV_CC_StopGrabbing(m_cameraHandle);
    }

    void HikrobotCamera::acquisitionLoop(void)
    {
        if (!m_isAcquiring)
            return;

        MV_FRAME_OUT stImageInfo = {0};
        int nRet                 = MV_CC_GetImageBuffer(m_cameraHandle, &stImageInfo, 1000);

        if (nRet == MV_OK)
        {
            m_consecutiveDropCount = 0; // Reset watchdog

            QImage frame(
                stImageInfo.pBufAddr,
                stImageInfo.stFrameInfo.nWidth,
                stImageInfo.stFrameInfo.nHeight,
                QImage::Format_Grayscale8);
            QImage frameCopy = frame.copy();

            MV_CC_FreeImageBuffer(m_cameraHandle, &stImageInfo);
            emit s_frameReady(std::move(frameCopy));
        }
        else if (nRet != MV_E_NODATA) // True error, not just an empty buffer
        {
            m_consecutiveDropCount++;
            qWarning() << std::format("Hikrobot: Frame drop [0x{:08X}] Count: {}", nRet, m_consecutiveDropCount).c_str();

            if (m_consecutiveDropCount >= MAX_FRAME_DROPS)
            {
                qCritical() << "Hikrobot: Max frame drops reached. Triggering hardware reset signal.";
                m_isAcquiring = false;
                emit s_connectionLost("Hardware Timeout / Max Frame Drops");
                return; // Break the loop
            }
        }

        // Re-queue
        if (m_isAcquiring)
        {
            QMetaObject::invokeMethod(this, &HikrobotCamera::acquisitionLoop, Qt::QueuedConnection);
        }
    }

    // =========================================================================
    // CONFIGURATION (GenICam Node Accessors)
    // =========================================================================

    bool HikrobotCamera::setExposure(double exposureUs)
    {
        setEnumNode("ExposureAuto", 0); // Off
        return setFloatNode("ExposureTime", static_cast<float>(exposureUs));
    }

    bool HikrobotCamera::setGain(double gainDB)
    {
        setEnumNode("GainAuto", 0); // Off
        return setFloatNode("Gain", static_cast<float>(gainDB));
    }

    bool HikrobotCamera::setFrameRate(double fps)
    {
        setBoolNode("AcquisitionFrameRateEnable", true);
        return setFloatNode("AcquisitionFrameRate", static_cast<float>(fps));
    }

    bool HikrobotCamera::setROI(int x, int y, int width, int height)
    {
        // Order is CRITICAL in GenICam: Always reduce Offset to 0 first,
        // then set new Size, then set new Offset. Otherwise you hit boundary limits.
        setIntNode("OffsetX", 0);
        setIntNode("OffsetY", 0);

        bool w = setIntNode("Width", width);
        bool h = setIntNode("Height", height);

        bool ox = setIntNode("OffsetX", x);
        bool oy = setIntNode("OffsetY", y);

        return w && h && ox && oy;
    }

    bool HikrobotCamera::setSmartReduction(int horizontal, int vertical)
    {
        // Try Binning first
        bool bH = setEnumNode("BinningHorizontal", horizontal);
        bool bV = setEnumNode("BinningVertical", vertical);

        if (bH && bV)
            return true;

        // Fallback to Decimation (Subsampling) if Binning fails or isn't supported
        qInfo() << "Hikrobot: Binning failed, falling back to Decimation.";
        bool dH = setEnumNode("DecimationHorizontal", horizontal);
        bool dV = setEnumNode("DecimationVertical", vertical);

        return dH && dV;
    }

    bool HikrobotCamera::setReverse(bool flipX, bool flipY)
    {
        bool x = setBoolNode("ReverseX", flipX);
        bool y = setBoolNode("ReverseY", flipY);
        return x && y;
    }

    // =========================================================================
    // MVS SDK HELPERS
    // =========================================================================

    bool HikrobotCamera::setFloatNode(const char *key, float value)
    {
        return m_cameraHandle && (MV_CC_SetFloatValue(m_cameraHandle, key, value) == MV_OK);
    }

    bool HikrobotCamera::setIntNode(const char *key, int64_t value)
    {
        return m_cameraHandle && (MV_CC_SetIntValueEx(m_cameraHandle, key, value) == MV_OK);
    }

    bool HikrobotCamera::setEnumNode(const char *key, int32_t value)
    {
        return m_cameraHandle && (MV_CC_SetEnumValue(m_cameraHandle, key, static_cast<uint32_t>(value)) == MV_OK);
    }

    bool HikrobotCamera::setBoolNode(const char *key, bool value)
    {
        return m_cameraHandle && (MV_CC_SetBoolValue(m_cameraHandle, key, value) == MV_OK);
    }

    bool HikrobotCamera::executeCommandNode(const char *key)
    {
        return m_cameraHandle && (MV_CC_SetCommandValue(m_cameraHandle, key) == MV_OK);
    }

    std::optional<int64_t> HikrobotCamera::getIntNode(const char *key) const
    {
        if (!m_cameraHandle)
            return std::nullopt;
        MVCC_INTVALUE_EX struValue = {0};
        if (MV_CC_GetIntValueEx(m_cameraHandle, key, &struValue) == MV_OK)
            return struValue.nCurValue;
        return std::nullopt;
    }

    std::optional<NodeBounds<int64_t>> HikrobotCamera::getROIBounds(const char *dimensionKey) const
    {
        if (!m_cameraHandle)
            return std::nullopt;
        MVCC_INTVALUE_EX struValue = {0};
        if (MV_CC_GetIntValueEx(m_cameraHandle, dimensionKey, &struValue) == MV_OK)
        {
            return NodeBounds<int64_t>{struValue.nMin, struValue.nMax, struValue.nInc};
        }
        return std::nullopt;
    }

} // namespace Kub3::HAL::Vision
