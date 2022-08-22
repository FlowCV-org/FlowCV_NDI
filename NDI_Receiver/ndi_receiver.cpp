//
// Plugin NDI Receiver
//

#include "ndi_receiver.hpp"
#include "imgui.h"
#include "imgui_instance_helper.hpp"

using namespace DSPatch;
using namespace DSPatchables;

int32_t global_inst_counter = 0;

namespace DSPatch::DSPatchables::internal
{
class NdiReceiver
{
};
}  // namespace DSPatch

NdiReceiver::NdiReceiver()
    : Component( ProcessOrder::OutOfOrder )
    , p( new internal::NdiReceiver() )
{
    // Name and Category
    SetComponentName_("NDI_Receiver");
    SetComponentCategory_(Category::Category_Source);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.0");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 0 inputs
    SetInputCount_(0);

    // add 1 output
    SetOutputCount_( 2, { "src", "fps" }, {IoType::Io_Type_CvMat, IoType::Io_Type_Int} );

    init_once_ = false;
    ndi_init_ = false;
    first_find_ = true;
    p_sources = nullptr;
    pNDI_recv = nullptr;
    pNDI_find = nullptr;
    ndi_source_count_ = 0;
    ndi_stream_idx_ = 0;
    fps_ = 30;
    has_recv = false;
    ndi_found_streams_.emplace_back("None");

    SetEnabled(true);

}

NdiReceiver::~NdiReceiver()
{
    if (pNDI_recv)
        NDIlib_recv_destroy(pNDI_recv);
    if (pNDI_find)
        NDIlib_find_destroy(pNDI_find);
    NDIlib_destroy();
}

void NdiReceiver::UpdateStreamList()
{
    std::lock_guard<std::mutex> lck(io_mutex_);
    ndi_found_streams_.clear();
    ndi_source_count_ = 0;

    ndi_found_streams_.emplace_back("None");

    if (pNDI_find) {
        NDIlib_find_destroy(pNDI_find);
    }

    pNDI_find = NDIlib_find_create_v2();
    if (!pNDI_find) {
        std::cout << "Error Creating NDI Finder" << std::endl;
        return;
    }

    NDIlib_find_wait_for_sources(pNDI_find, 5000);
    NDIlib_find_wait_for_sources(pNDI_find, 5000);
    p_sources = NDIlib_find_get_current_sources(pNDI_find, &ndi_source_count_);
    if (ndi_source_count_ > 0) {
        for (int i = 0; i < ndi_source_count_; i++) {
            ndi_found_streams_.emplace_back(p_sources[i].p_ndi_name);
        }
    }
}

bool NdiReceiver::FindSourceMatch(std::string &stream_name)
{

    if (pNDI_recv) {
        NDIlib_recv_destroy(pNDI_recv);
    }

    if (ndi_source_count_ > 0) {
        for (int i = 0; i < ndi_source_count_; i++) {
            if (stream_name == std::string(p_sources[i].p_ndi_name)) {
                pNDI_recv = NDIlib_recv_create_v3();
                if (!pNDI_recv)
                    return false;

                // Connect to our sources
                NDIlib_recv_connect(pNDI_recv, p_sources + i);
                return true;
            }
        }
    }
    return false;
}

bool NdiReceiver::InitNdiStream()
{
    has_recv = false;

    if (ndi_stream_idx_ > 0) {
        if (ndi_stream_idx_ < ndi_found_streams_.size()) {
            if (FindSourceMatch(ndi_found_streams_.at(ndi_stream_idx_))) {
                has_recv = true;
                return true;
            }
        }
    }

    return false;
}

void NdiReceiver::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    if (io_mutex_.try_lock()) { // Try lock so other threads will skip if locked instead of waiting
        if (!ndi_init_) {
            if (!NDIlib_initialize()) {
                std::cout << "Error Initializing NDI" << std::endl;
                io_mutex_.unlock();
                return;
            }
            ndi_init_ = true;
        }

        if (ndi_init_) {
            if (init_once_) {
                bool isInit = InitNdiStream();
                if (isInit)
                    init_once_ = false;
            }

            if (has_recv) {
                NDIlib_video_frame_v2_t video_frame;
                NDIlib_frame_type_e frameType = NDIlib_recv_capture_v2(pNDI_recv, &video_frame, nullptr, nullptr, 1000);
                if (frameType == NDIlib_frame_type_video) {
                    cv::Mat tmpFrame(cv::Size(video_frame.xres, video_frame.yres), CV_8UC2, video_frame.p_data, cv::Mat::AUTO_STEP);
                    fps_ = video_frame.frame_rate_N / video_frame.frame_rate_D;
                    cv::cvtColor(tmpFrame, frame_, cv::COLOR_YUV2BGR_UYVY);
                    NDIlib_recv_free_video_v2(pNDI_recv, &video_frame);
                    outputs.SetValue(0, frame_);
                    outputs.SetValue(1, fps_);
                }
            }
        }
        io_mutex_.unlock();
    }
}

bool NdiReceiver::HasGui(int interface)
{
    // This is where you tell the system if your node has any of the following interfaces: Main, Control or Other
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void NdiReceiver::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (ndi_init_) {
        if (first_find_) {
            UpdateStreamList();
            first_find_ = false;
        }
    }

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        if (ImGui::Button(CreateControlString("NDI List Refresh", GetInstanceName()).c_str())) {
            UpdateStreamList();
        }
        ImGui::Separator();
        ImGui::SetNextItemWidth(200);
        if (ImGui::Combo(CreateControlString("NDI Streams", GetInstanceName()).c_str(), &ndi_stream_idx_, [](void* data, int idx, const char** out_text) {
            *out_text = ((const std::vector<std::string>*)data)->at(idx).c_str();
            return true;
        }, (void*)&ndi_found_streams_, (int)ndi_found_streams_.size())) {
            init_once_ = true;
        }
    }
}

std::string NdiReceiver::GetState()
{
    using namespace nlohmann;

    json state;

    state["ndi_name"] = ndi_found_streams_.at(ndi_stream_idx_);

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void NdiReceiver::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("ndi_name")) {
        auto ndi_name = state["ndi_name"].get<std::string>();
        UpdateStreamList();
        for (int i = 0; i < ndi_found_streams_.size(); i++) {
            if (ndi_found_streams_.at(i) == ndi_name) {
                ndi_stream_idx_ = i;
                init_once_ = true;
            }
        }
    }

}
