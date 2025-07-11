// SPDX-FileCopyrightText: Copyright 2024 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "core/core_timing.h"
#include "core/hle/service/acc/profile_manager.h"
#include "core/hle/service/am/applet_data_broker.h"
#include "core/hle/service/am/applet_manager.h"
#include "core/hle/service/am/frontend/applets.h"
#include "core/hle/service/am/service/library_applet_self_accessor.h"
#include "core/hle/service/am/service/storage.h"
#include "core/hle/service/cmif_serialization.h"

#include "core/hle/service/glue/glue_manager.h"
#include "core/hle/service/ns/application_manager_interface.h"
#include "core/hle/service/ns/service_getter_interface.h"
#include "core/hle/service/sm/sm.h"

namespace Service::AM {

namespace {

AppletIdentityInfo GetCallerIdentity(Applet& applet) {
    if (const auto caller_applet = applet.caller_applet.lock(); caller_applet) {
        // TODO: is this actually the application ID?
        return {
            .applet_id = caller_applet->applet_id,
            .application_id = caller_applet->program_id,
        };
    } else {
        return {
            .applet_id = AppletId::QLaunch,
            .application_id = 0x0100000000001000ull,
        };
    }
}

} // namespace

ILibraryAppletSelfAccessor::ILibraryAppletSelfAccessor(Core::System& system_,
                                                       std::shared_ptr<Applet> applet)
    : ServiceFramework{system_, "ILibraryAppletSelfAccessor"}, m_applet{std::move(applet)},
      m_broker{m_applet->caller_applet_broker} {
    // clang-format off
    static const FunctionInfo functions[] = {
        {0, D<&ILibraryAppletSelfAccessor::PopInData>, "PopInData"},
        {1, D<&ILibraryAppletSelfAccessor::PushOutData>, "PushOutData"},
        {2, D<&ILibraryAppletSelfAccessor::PopInteractiveInData>, "PopInteractiveInData"},
        {3, D<&ILibraryAppletSelfAccessor::PushInteractiveOutData>, "PushInteractiveOutData"},
        {5, D<&ILibraryAppletSelfAccessor::GetPopInDataEvent>, "GetPopInDataEvent"},
        {6, D<&ILibraryAppletSelfAccessor::GetPopInteractiveInDataEvent>, "GetPopInteractiveInDataEvent"},
        {10, D<&ILibraryAppletSelfAccessor::ExitProcessAndReturn>, "ExitProcessAndReturn"},
        {11, D<&ILibraryAppletSelfAccessor::GetLibraryAppletInfo>, "GetLibraryAppletInfo"},
        {12, D<&ILibraryAppletSelfAccessor::GetMainAppletIdentityInfo>, "GetMainAppletIdentityInfo"},
        {13, D<&ILibraryAppletSelfAccessor::CanUseApplicationCore>, "CanUseApplicationCore"},
        {14, D<&ILibraryAppletSelfAccessor::GetCallerAppletIdentityInfo>, "GetCallerAppletIdentityInfo"},
        {15, D<&ILibraryAppletSelfAccessor::GetMainAppletApplicationControlProperty>, "GetMainAppletApplicationControlProperty"},
        {16, D<&ILibraryAppletSelfAccessor::GetMainAppletStorageId>, "GetMainAppletStorageId"},
        {17, D<&ILibraryAppletSelfAccessor::GetCallerAppletIdentityInfoStack>, "GetCallerAppletIdentityInfoStack"},
        {18, nullptr, "GetNextReturnDestinationAppletIdentityInfo"},
        {19, D<&ILibraryAppletSelfAccessor::GetDesirableKeyboardLayout>, "GetDesirableKeyboardLayout"},
        {20, nullptr, "PopExtraStorage"},
        {25, nullptr, "GetPopExtraStorageEvent"},
        {30, nullptr, "UnpopInData"},
        {31, nullptr, "UnpopExtraStorage"},
        {40, nullptr, "GetIndirectLayerProducerHandle"},
        {50, D<&ILibraryAppletSelfAccessor::ReportVisibleError>, "ReportVisibleError"},
        {51, D<&ILibraryAppletSelfAccessor::ReportVisibleErrorWithErrorContext>, "ReportVisibleErrorWithErrorContext"},
        {60, D<&ILibraryAppletSelfAccessor::GetMainAppletApplicationDesiredLanguage>, "GetMainAppletApplicationDesiredLanguage"},
        {70, D<&ILibraryAppletSelfAccessor::GetCurrentApplicationId>, "GetCurrentApplicationId"},
        {80, nullptr, "RequestExitToSelf"},
        {90, nullptr, "CreateApplicationAndPushAndRequestToLaunch"},
        {100, nullptr, "CreateGameMovieTrimmer"},
        {101, nullptr, "ReserveResourceForMovieOperation"},
        {102, nullptr, "UnreserveResourceForMovieOperation"},
        {110, D<&ILibraryAppletSelfAccessor::GetMainAppletAvailableUsers>, "GetMainAppletAvailableUsers"},
        {120, nullptr, "GetLaunchStorageInfoForDebug"},
        {130, nullptr, "GetGpuErrorDetectedSystemEvent"},
        {140, nullptr, "SetApplicationMemoryReservation"},
        {150, D<&ILibraryAppletSelfAccessor::ShouldSetGpuTimeSliceManually>, "ShouldSetGpuTimeSliceManually"},
        {160, D<&ILibraryAppletSelfAccessor::Cmd160>, "Cmd160"},
    };
    // clang-format on
    RegisterHandlers(functions);
}

ILibraryAppletSelfAccessor::~ILibraryAppletSelfAccessor() = default;

Result ILibraryAppletSelfAccessor::PopInData(Out<SharedPointer<IStorage>> out_storage) {
    LOG_INFO(Service_AM, "called");
    R_RETURN(m_broker->GetInData().Pop(out_storage));
}

Result ILibraryAppletSelfAccessor::PushOutData(SharedPointer<IStorage> storage) {
    LOG_INFO(Service_AM, "called");
    m_broker->GetOutData().Push(storage);
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::PopInteractiveInData(Out<SharedPointer<IStorage>> out_storage) {
    LOG_INFO(Service_AM, "called");
    R_RETURN(m_broker->GetInteractiveInData().Pop(out_storage));
}

Result ILibraryAppletSelfAccessor::PushInteractiveOutData(SharedPointer<IStorage> storage) {
    LOG_INFO(Service_AM, "called");
    m_broker->GetInteractiveOutData().Push(storage);
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetPopInDataEvent(
    OutCopyHandle<Kernel::KReadableEvent> out_event) {
    LOG_INFO(Service_AM, "called");
    *out_event = m_broker->GetInData().GetEvent();
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetPopInteractiveInDataEvent(
    OutCopyHandle<Kernel::KReadableEvent> out_event) {
    LOG_INFO(Service_AM, "called");
    *out_event = m_broker->GetInteractiveInData().GetEvent();
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetLibraryAppletInfo(
    Out<LibraryAppletInfo> out_library_applet_info) {
    LOG_INFO(Service_AM, "called");
    *out_library_applet_info = {
        .applet_id = m_applet->applet_id,
        .library_applet_mode = m_applet->library_applet_mode,
    };
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetMainAppletIdentityInfo(
    Out<AppletIdentityInfo> out_identity_info) {
    LOG_WARNING(Service_AM, "(STUBBED) called");
    *out_identity_info = {
        .applet_id = AppletId::QLaunch,
        .application_id = 0x0100000000001000ull,
    };
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::CanUseApplicationCore(Out<bool> out_can_use_application_core) {
    // TODO: This appears to read the NPDM from state and check the core mask of the applet.
    LOG_WARNING(Service_AM, "(STUBBED) called");
    *out_can_use_application_core = false;
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetMainAppletApplicationControlProperty(
    OutLargeData<std::array<u8, 0x4000>, BufferAttr_HipcMapAlias> out_nacp) {
    LOG_WARNING(Service_AM, "(STUBBED) called");
    UNIMPLEMENTED();
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetMainAppletStorageId(Out<FileSys::StorageId> out_storage_id) {
    LOG_INFO(Service_AM, "(STUBBED) called");
    UNIMPLEMENTED();
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::ExitProcessAndReturn() {
    LOG_INFO(Service_AM, "called");
    system.GetAppletManager().TerminateAndRemoveApplet(m_applet->aruid);
    m_broker->SignalCompletion();
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetCallerAppletIdentityInfo(
    Out<AppletIdentityInfo> out_identity_info) {
    LOG_INFO(Service_AM, "called");
    *out_identity_info = GetCallerIdentity(*m_applet);
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetCallerAppletIdentityInfoStack(
    Out<s32> out_count, OutArray<AppletIdentityInfo, BufferAttr_HipcMapAlias> out_identity_info) {
    LOG_INFO(Service_AM, "called");

    std::shared_ptr<Applet> applet = m_applet;
    *out_count = 0;

    do {
        if (*out_count >= static_cast<s32>(out_identity_info.size())) {
            break;
        }
        out_identity_info[(*out_count)++] = GetCallerIdentity(*applet);
    } while ((applet = applet->caller_applet.lock()));

    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetDesirableKeyboardLayout(Out<u32> out_desirable_layout) {
    LOG_WARNING(Service_AM, "(STUBBED) called");
    *out_desirable_layout = 0;
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::ReportVisibleError(ErrorCode error_code) {
    LOG_WARNING(Service_AM, "(STUBBED) called, error {}-{}", error_code.category,
                error_code.number);
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::ReportVisibleErrorWithErrorContext(
    ErrorCode error_code, InLargeData<ErrorContext, BufferAttr_HipcMapAlias> error_context) {
    LOG_WARNING(Service_AM, "(STUBBED) called, error {}-{}", error_code.category,
                error_code.number);
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetMainAppletApplicationDesiredLanguage(
    Out<u64> out_desired_language) {
    UNIMPLEMENTED();
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetCurrentApplicationId(Out<u64> out_application_id) {
    LOG_WARNING(Service_AM, "(STUBBED) called");

    // TODO: this should be the main applet, not the caller applet
    const auto main_applet = GetCallerIdentity(*m_applet);
    *out_application_id = main_applet.application_id;

    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::GetMainAppletAvailableUsers(
    Out<bool> out_can_select_any_user, Out<s32> out_users_count,
    OutArray<Common::UUID, BufferAttr_HipcMapAlias> out_users) {
    const Service::Account::ProfileManager manager{};

    *out_can_select_any_user = false;
    *out_users_count = -1;

    LOG_INFO(Service_AM, "called");

    if (manager.GetUserCount() > 0) {
        *out_can_select_any_user = true;
        *out_users_count = static_cast<s32>(manager.GetUserCount());

        const auto users = manager.GetAllUsers();
        for (size_t i = 0; i < users.size() && i < out_users.size(); i++) {
            out_users[i] = users[i];
        }
    }

    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::ShouldSetGpuTimeSliceManually(
    Out<bool> out_should_set_gpu_time_slice_manually) {
    LOG_INFO(Service_AM, "(STUBBED) called");
    *out_should_set_gpu_time_slice_manually = false;
    R_SUCCEED();
}

Result ILibraryAppletSelfAccessor::Cmd160(Out<u64> out_unknown0) {
    LOG_WARNING(Service_AM, "(STUBBED) called");
    *out_unknown0 = 0;
    R_SUCCEED();
}

} // namespace Service::AM
