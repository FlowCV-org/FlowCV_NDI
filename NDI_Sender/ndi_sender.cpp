//
// Plugin NDI Sender
//

#include "ndi_sender.hpp"
#include "imgui.h"
#include "imgui_instance_helper.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class NdiSender
{
};
}  // namespace DSPatch

NdiSender::NdiSender()
    : Component( ProcessOrder::OutOfOrder )
    , p( new internal::NdiSender() )
{
    // Name and Category
    SetComponentName_("NDI_Sender");
    SetComponentCategory_(Category::Category_Output);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 1 inputs
    SetInputCount_( 2, {"in", "fps"}, {IoType::Io_Type_CvMat, IoType::Io_Type_Int} );

    init_once_ = false;
    last_fps_ = 30;
    last_res.x = 0;
    last_res.y = 0;
    NDI_video_frame_.p_data = nullptr;
    pNDI_Send_ = nullptr;
    ndi_init_ = false;
    memset(tmp_name_buf, '\0', 64);

    SetEnabled(true);

}

NdiSender::~NdiSender()
{
    // Free the video frame
    if (NDI_video_frame_.p_data)
        free(NDI_video_frame_.p_data);
    // Destroy the NDI sender
    NDIlib_send_destroy(pNDI_Send_);
}

bool NdiSender::InitNdiStream()
{
    if (!ndi_stream_name_.empty()) {
        if (!ndi_init_) {
            if (!NDIlib_initialize()) {
                std::cout << "Error Initializing NDI" << std::endl;
                return false;
            }
            ndi_init_ = true;
        }
        if (init_once_) {
            if (pNDI_Send_) {
                NDIlib_send_destroy(pNDI_Send_);
            }

            NDI_video_desc_.p_ndi_name = ndi_stream_name_.c_str();
            pNDI_Send_ = NDIlib_send_create(&NDI_video_desc_);
            if (!pNDI_Send_) {
                std::cout << "Error Creating NDI Sender" << std::endl;
                return false;
            }
            NDI_video_frame_.xres = cur_res_.x;
            NDI_video_frame_.yres = cur_res_.y;
            NDI_video_frame_.frame_rate_N = cur_fps_ * 1000;
            NDI_video_frame_.frame_rate_D = 1000;
            NDI_video_frame_.FourCC = NDIlib_FourCC_type_BGRX;

            if (NDI_video_frame_.p_data)
                free(NDI_video_frame_.p_data);

            NDI_video_frame_.p_data = (uint8_t *) malloc(cur_res_.x * cur_res_.y * 4);
            NDI_video_frame_.line_stride_in_bytes = cur_res_.x * 4;
            last_res.x = cur_res_.x;
            last_res.y = cur_res_.y;
            return true;
        }
    }
    return false;
}

void NdiSender::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    if (io_mutex_.try_lock()) { // Try lock so other threads will skip if locked instead of waiting
        auto in1 = inputs.GetValue<cv::Mat>(0);
        auto in2 = inputs.GetValue<int>(1);
        if (!in1) {
            io_mutex_.unlock();
            return;
        }

        cur_fps_ = 30;

        if (in2)
            cur_fps_ = *in2;

        if (!in1->empty()) {
            cur_res_.x = in1->cols;
            cur_res_.y = in1->rows;

            if (cur_res_.x != last_res.x || cur_res_.y != last_res.y)
                init_once_ = true;

            if (cur_fps_ != last_fps_) {
                NDI_video_frame_.frame_rate_N = cur_fps_ * 1000;
                last_fps_ = cur_fps_;
            }
            if (init_once_) {
                if(InitNdiStream())
                    init_once_ = false;
            }
            cv::cvtColor(*in1, frame_, cv::COLOR_BGR2BGRA);
            if (ndi_init_) {
                memcpy((void *) NDI_video_frame_.p_data, frame_.data, cur_res_.x * cur_res_.y * 4);
                NDIlib_send_send_video_v2(pNDI_Send_, &NDI_video_frame_);
            }

            last_res.x = cur_res_.x;
            last_res.y = cur_res_.y;
        }
        io_mutex_.unlock();
    }
}

bool NdiSender::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void NdiSender::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        ImGui::SetNextItemWidth(200);
        if (ImGui::InputText(CreateControlString("NDI Name", GetInstanceName()).c_str(), tmp_name_buf, 64, ImGuiInputTextFlags_EnterReturnsTrue)) {
            ndi_stream_name_ = std::string(tmp_name_buf);
            init_once_ = true;
        }
    }
}

std::string NdiSender::GetState()
{
    using namespace nlohmann;

    json state;

    state["ndi_name"] = ndi_stream_name_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void NdiSender::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("ndi_name")) {
        ndi_stream_name_ = state["ndi_name"].get<std::string>();
        sprintf(tmp_name_buf, "%s", ndi_stream_name_.c_str());
    }

    if (!ndi_stream_name_.empty())
        init_once_ = true;

}
