//
// Plugin NDI Sender
//

#ifndef FLOWCV_NDI_SENDER_HPP_
#define FLOWCV_NDI_SENDER_HPP_
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
class NdiSender;
}

class DLLEXPORT NdiSender final : public Component
{
  public:
    NdiSender();
    ~NdiSender() override;
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;
    bool InitNdiStream();

  private:
    std::unique_ptr<internal::NdiSender> p;
    cv::Mat frame_;
    std::mutex io_mutex_;
    NDIlib_send_create_t NDI_video_desc_;
    NDIlib_video_frame_v2_t NDI_video_frame_;
    NDIlib_send_instance_t pNDI_Send_;
    cv::Point2i cur_res_;
    cv::Point2i last_res;
    std::string ndi_stream_name_;
    char tmp_name_buf[64];
    int cur_fps_;
    int last_fps_;
    bool init_once_;
    bool ndi_init_;
};

EXPORT_PLUGIN( NdiSender )

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_NDI_SENDER_HPP_
