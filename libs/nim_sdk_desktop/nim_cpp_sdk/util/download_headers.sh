FILES=(
    "nim_data_sync_def.h"
    "nim_friend_def.h"
    "nim_session_def.h"
    "nim_client_def.h"
    "nim_user_def.h"
    "nim_team_def.h"
    "nim_msglog_def.h"
    "nim_talk_def.h"
    "nim_sysmsg_def.h"
    "nim_subscribe_event_def.h"
    "nim_robot_def.h"
    "nim_global_def.h"
    "nim_nos_def.h"
    "nim_plugin_in_def.h"
    "nim_tools_def.h"
    "nim_doc_trans_def.h"
    "nim_vchat_def.h"
    "nim_device_def.h"
    "nim_rts_def.h"
    "nim_signaling_def.h"
)

BASE_URL="https://raw.githubusercontent.com/netease-im/NIM_PC_SDK/master/nim_c_sdk/include/"
TARGET_DIR="/Volumes/m2/work/AiShu/github/im-c-front/libs/nim_sdk_desktop/nim_cpp_sdk/util/"

for file in "${FILES[@]}"; do
    echo "Downloading $file..."
    curl -s -o "$TARGET_DIR$file" "$BASE_URL$file"
done

echo "Downloading winsdk_config.h..."
curl -s -o "/Volumes/m2/work/AiShu/github/im-c-front/tool_kits/base/build/winsdk_config.h" "https://raw.githubusercontent.com/Washington-DC/GB28181-Service/master/base/build/winsdk_config.h"
