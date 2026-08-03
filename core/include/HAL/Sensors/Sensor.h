#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <HAL/Sensors/ISensor.h>

namespace Kub3::HAL::Sensors
{

    template <typename T>
    class Sensor : public ISensor
    {
    public:
        using Mapper = std::function<T(const QByteArray &)>;

        Sensor(Shared<MS::IMachineStatusRepo> repo, std::string key, T initialValue, Mapper mapper, QObject *parent = nullptr) :
            ISensor(parent),
            m_key(std::move(key)),
            m_mapper(mapper),
            m_repo(std::move(repo))
        {
            m_repo->setValue<T>(m_key, initialValue);
            if (!m_mapper)
                qWarning().nospace() << "Sensor [" << m_key << "] was provided a null mapper function.";
        }

        // The implementation of the base interface
        void processData(const QByteArray &data) override
        {
            if (!m_mapper)
            {
                qWarning().nospace() << "Attempt to update sensor [" << m_key << "] without a mapper function.";
                return;
            }

            T value = m_mapper(data);

            m_repo->setValue<T>(m_key, value);
        }

    protected:
        Mapper m_mapper;
        Shared<MS::IMachineStatusRepo> m_repo;

    private:
        const std::string m_key;
    };

}