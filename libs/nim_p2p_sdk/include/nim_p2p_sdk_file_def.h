#ifndef NIM_P2P_SDK_FILE_DEF_H
#define NIM_P2P_SDK_FILE_DEF_H

#include "nim_p2p_def.h"
#include "nim_p2p_sdk_def.h"

// Callbacks for file transfer
typedef void (*OnTransferFileRequest)(const RemoteFlagType remote_flag, TransferFileSessionID session_id, const char* file_info);

typedef void (*TransferFileSessionStateChangeCallback)(const RemoteFlagType remote_flag, TransferFileSessionID session_id, enum TransferFileSessionState state);

typedef void (*TransferFileProgressCallback)(const RemoteFlagType remote_flag, TransferFileSessionID session_id, FilesizeType total, FilesizeType transferred);

#endif // NIM_P2P_SDK_FILE_DEF_H
