//
// Plugin NDI Receiver
//

#ifndef FLOWCV_NDI_RECEIVER_HPP_
#define FLOWCV_NDI_RECEIVER_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include <Processing.NDI.Lib.h>
#include "json.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include <string>

namespace DSPatch::DSPatchables
{
namespace internal
{
class NdiReceiver;
}

class DLLEXPORT NdiReceiver final : public Component
{
  public:
    NdiReceiver();
    ~NdiReceiver() override;
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    bool InitNdiStream();
    void UpdateStreamList();
    bool FindSourceMatch(std::string &stream_name);

  private:
    std::unique_ptr<internal::NdiReceiver> p;
    cv::Mat frame_;
    std::mutex io_mutex_;
    std::vector<std::string> ndi_found_streams_;
    const NDIlib_source_t* p_sources;
    NDIlib_recv_instance_t pNDI_recv;
    NDIlib_find_instance_t pNDI_find;
    uint32_t ndi_source_count_;
    int fps_;
    int ndi_stream_idx_;
    bool first_find_;
    bool init_once_;
    bool has_recv;
    bool ndi_init_;
};

EXPORT_PLUGIN( NdiReceiver )

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_NDI_RECEIVER_HPP_
