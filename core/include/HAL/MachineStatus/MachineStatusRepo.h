#pragma once

#include <shared_mutex>
#include <unordered_map>

#include "IMachineStatusRepo.h"

namespace Kub3::HAL::MS
{

    class MachineStatusRepo : public IMachineStatusRepo
    {
        Q_OBJECT
    public:
        explicit MachineStatusRepo(QObject *parent = nullptr);
        ~MachineStatusRepo() override = default;

        void setValueRaw(const std::string &key, const MachineValue &value) override final;
        [[nodiscard]] Optional<MachineValue> getValueRaw(const std::string &key) const override final;
        [[nodiscard]] std::vector<std::string> getRegisteredKeys() const override final;

    private:
        std::unordered_map<std::string, MachineValue> m_sensors; // The dynamic, runtime-populated registry
        mutable std::shared_mutex m_mutex;                       // Reader-Writer lock  thread safety
    };

} // namespace Kub3::HAL::MS
