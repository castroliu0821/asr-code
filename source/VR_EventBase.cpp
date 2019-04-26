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

#include "VR_EventBase.h"

#include "VR_Log.h"
#include "VR_Def.h"
#include "VR_BaiduDef.h"
#include "VR_BaiduStateManager.h"
#include "VR_BaiduDialogEngine.h"

VR_EventBase::VR_EventBase(stEvent_info& event, void *ptr)
    : m_event(event)
    , m_pHandle(ptr)
{
    VR_LOGD_FUNC();
}

VR_EventBase::~VR_EventBase()
{
    VR_LOGD_FUNC();
}

void VR_EventBase::HandleEvent()
{
    VR_BaiduDialogEngine* pEngine = reinterpret_cast<VR_BaiduDialogEngine*>(m_pHandle);

    VR_LOGD("[DEBUG] ..... Handle event : %s", m_event.name.c_str());

    if (pEngine == nullptr) {
        VR_LOGD("[DEBUG] ..... invalid para");
        return;
    }

    pEngine->HandleEvent(m_event.name, m_event.mesg, m_event.mesg_id);
}
