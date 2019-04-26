/**
 * Copyright @ 2013 - 2019 iAuto Software(Shanghai) Co., Ltd.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted except as agreed by
 * iAuto Software(Shanghai) Co., Ltd.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include "VR_BaiduEngineTimer.h"
#include "VR_Log.h"
#include "VR_Def.h"

#include "VR_EventBase.h"
#include "VR_EngineWorkThread.h"
#include "VR_BaiduDialogEngine.h"
#include "VR_BaiduDef.h"

const static long MAX_AUDIO_TIME = 14 * 1000; // effective audio duration 14s = 14 * 1000 ms

VR_BaiduEngineTimer::VR_BaiduEngineTimer(sp_VR_EngineWorkThread spWorkThread, VR_BaiduDialogEngine *pEngine)
    : BL_Timer (MAX_AUDIO_TIME)
    , m_spTaskThread(spWorkThread)
    , m_pEngine(pEngine)
{
    VR_LOGD_FUNC();
}

VR_BaiduEngineTimer::~VR_BaiduEngineTimer()
{
    VR_LOGD_FUNC();
}

VOID VR_BaiduEngineTimer::OnTimer()
{
    VR_LOGD("[DEBUG] ..... Time out");
    stEvent_info info;
    info.sender = "DE";
    info.mesg = "";
    info.mesg_id = 0;
    info.name = EV_TIME_OUT;
    sp_VR_EventBase event(VR_new VR_EventBase(info, m_pEngine));

    m_spTaskThread->PushTask(event);
}
