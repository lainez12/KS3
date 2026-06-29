#ifndef VISUALISATIONVIEWMODEL_H
#define VISUALISATIONVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/BaseVisionViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{

    class VisualisationViewModel final : public BaseVisionViewModel
    {
        Q_OBJECT

    public:
        explicit VisualisationViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~VisualisationViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Alignment

#endif