#include "service_actor.h"

#include "helpers.h"

#include <cloud/filestore/libs/storage/api/service.h>
#include <cloud/filestore/libs/storage/api/tablet.h>
#include <cloud/filestore/libs/storage/api/tablet_proxy.h>
#include <cloud/filestore/libs/storage/core/public.h>
#include <cloud/filestore/private/api/protos/tablet.pb.h>

#include <library/cpp/actors/core/actor_bootstrapped.h>

#include <google/protobuf/util/json_util.h>

namespace NCloud::NFileStore::NStorage {

using namespace NActors;

using namespace NKikimr;

namespace {

////////////////////////////////////////////////////////////////////////////////

class TChangeStorageConfigActionActor final
    : public TActorBootstrapped<TChangeStorageConfigActionActor>
{
private:
    const TRequestInfoPtr RequestInfo;
    const TString Input;
    const TStorageConfigPtr Config;

public:
    TChangeStorageConfigActionActor(
        TRequestInfoPtr requestInfo,
        TString input,
        TStorageConfigPtr config);

    void Bootstrap(const TActorContext& ctx);

private:
    void ReplyAndDie(
        const TActorContext& ctx,
        NProtoPrivate::TChangeStorageConfigResponse response,
        NProto::TError error);

private:
    STFUNC(StateWork);

    void HandleChangeStorageConfigResponse(
        const TEvIndexTablet::TEvChangeStorageConfigResponse::TPtr& ev,
        const TActorContext& ctx);
};

////////////////////////////////////////////////////////////////////////////////

TChangeStorageConfigActionActor::TChangeStorageConfigActionActor(
        TRequestInfoPtr requestInfo,
        TString input,
        TStorageConfigPtr config)
    : RequestInfo(std::move(requestInfo))
    , Input(std::move(input))
    , Config(config)
{
    ActivityType = TFileStoreActivities::SERVICE_WORKER;
}

void TChangeStorageConfigActionActor::Bootstrap(const TActorContext& ctx)
{
    NProtoPrivate::TChangeStorageConfigRequest request;
    if (!google::protobuf::util::JsonStringToMessage(Input, &request).ok()) {
        ReplyAndDie(
            ctx,
            NProtoPrivate::TChangeStorageConfigResponse(),
            MakeError(
                E_ARGUMENT,
                "Failed to parse input"));
        return;
    }

    if (!request.GetFileSystemId()) {
        ReplyAndDie(
            ctx,
            NProtoPrivate::TChangeStorageConfigResponse(),
            MakeError(
                E_ARGUMENT,
                "FileSystem id should be supplied"));
        return;
    }

    LOG_INFO(ctx, TFileStoreComponents::SERVICE,
        "Start to change storage config of %s",
        request.GetFileSystemId().Quote().c_str());

    auto requestToTablets =
        std::make_unique<TEvIndexTablet::TEvChangeStorageConfigRequest>();
    auto& record = requestToTablets->Record;
    record.SetFileSystemId(request.GetFileSystemId());
    *record.MutableStorageConfig() = request.GetStorageConfig();
    record.SetMergeWithStorageConfigFromTabletDB(
        request.GetMergeWithStorageConfigFromTabletDB());

    NCloud::Send(
        ctx,
        MakeIndexTabletProxyServiceId(),
        std::move(requestToTablets));

    Become(&TThis::StateWork);
}

void TChangeStorageConfigActionActor::ReplyAndDie(
    const TActorContext& ctx,
    NProtoPrivate::TChangeStorageConfigResponse response,
    NProto::TError error)
{
    auto msg = std::make_unique<TEvService::TEvExecuteActionResponse>(
        std::move(error));

    google::protobuf::util::MessageToJsonString(
        std::move(response),
        msg->Record.MutableOutput());

    NCloud::Reply(ctx, *RequestInfo, std::move(msg));
    Die(ctx);
}

////////////////////////////////////////////////////////////////////////////////

void TChangeStorageConfigActionActor::HandleChangeStorageConfigResponse(
    const TEvIndexTablet::TEvChangeStorageConfigResponse::TPtr& ev,
    const TActorContext& ctx)
{
    auto* msg = ev->Get();
    if (SUCCEEDED(msg->GetStatus())) {
        NCloud::Send(
            ctx,
            ev->Sender,
            std::make_unique<TEvents::TEvPoisonPill>());
    }
    const auto error = msg->Record.GetError();
    ReplyAndDie(ctx, std::move(msg->Record), std::move(error));
}

////////////////////////////////////////////////////////////////////////////////

STFUNC(TChangeStorageConfigActionActor::StateWork)
{
    switch (ev->GetTypeRewrite()) {
        HFunc(TEvIndexTablet::TEvChangeStorageConfigResponse,
            HandleChangeStorageConfigResponse);

        default:
            HandleUnexpectedEvent(ev, TFileStoreComponents::SERVICE);
            break;
    }
}

}   // namespace

NActors::IActorPtr TStorageServiceActor::CreateChangeStorageConfigActionActor(
    TRequestInfoPtr requestInfo,
    TString input)
{
    return std::make_unique<TChangeStorageConfigActionActor>(
        std::move(requestInfo),
        std::move(input),
        StorageConfig);
}

}   // namespace NCloud::NFileStore::NStorage