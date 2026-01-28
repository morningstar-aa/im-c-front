#include "bubble_wallet.h"

using namespace ui;

namespace nim_comp
{
void MsgBubbleWallet::InitControl(bool bubble_right)
{
	__super::InitControl(bubble_right);

	msg_unknown_ = new HBox;
	if(bubble_right)
		GlobalManager::FillBoxWithCache(msg_unknown_, L"session/wallet_right.xml");
	else
		GlobalManager::FillBoxWithCache(msg_unknown_, L"session/wallet_left.xml");
	bubble_box_->Add(msg_unknown_);

	unknown_tip_ = (Label*) msg_unknown_->FindSubControl(L"wallet_tip");
}

void MsgBubbleWallet::InitInfo(const nim::IMMessage &msg)
{
	__super::InitInfo(msg);
	if (CustomMsgType_Wallet_send == isSendWallet_)
	{
		if (my_msg_)
		{
			unknown_tip_->SetText(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_SEND_WALLET_MSG"));
		}
		else
		{
			unknown_tip_->SetText(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_WALLET_MSG"));
		}
	}
	else if (CustomMsgType_Wallet_receive == isSendWallet_)
	{
		if (my_msg_)
		{
			unknown_tip_->SetText(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_SEND_WALLET_MSG_RECEIVER"));
		}
		else
		{
			unknown_tip_->SetText(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_WALLET_MSG_RECEIVER"));
		}
	}

	/*if (msg.type_ == nim::kNIMMessageTypeCustom)
		unknown_tip_->SetText(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_WALLET_MSG"));*/
}

void MsgBubbleWallet::SetWalletType(int isSendWallet)
{
	isSendWallet_ = isSendWallet;
}
}