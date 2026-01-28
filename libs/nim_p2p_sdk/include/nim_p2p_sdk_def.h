#ifndef NIM_P2P_SDK_DEF_H
#define NIM_P2P_SDK_DEF_H

#include "nim_p2p_def.h"

// Enum for transfer state
enum TransferFileSessionState
{
    TransferFileSessionState_NULL = 0,
    TransferFileSessionState_Wait,
    TransferFileSessionState_Consulting,
    TransferFileSessionState_Transferring,
    TransferFileSessionState_Succeeded,
    TransferFileSessionState_Failed,
    TransferFileSessionState_CMDTimeout,
    TransferFileSessionState_ReceiverRejected,
    TransferFileSessionState_Rejected,
    TransferFileSessionState_ReceiverCancel,
    TransferFileSessionState_SenderCancel
};

// Callback for receiving custom commands
typedef void (*OnChannelCommand)(const RemoteFlagType remote_flag, const char* command);

// Callback for sending commands
typedef bool (*SendCommandChannel)(const RemoteFlagType remote_flag, const char* command);

#endif // NIM_P2P_SDK_DEF_H
