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

#include "VR_EngineWorkThread.h"

#include "VR_Log.h"
#include "VR_Def.h"
#include "BL_AutoSync.h"
#include "AplThreadName.h"

VR_EngineWorkThread::VR_EngineWorkThread()
{
    VR_LOGD_FUNC();
    m_taskQueue = VR_new VoiceQueue<sp_VR_EventBase>::type;
}

VR_EngineWorkThread::~VR_EngineWorkThread()
{
    VR_LOGD_FUNC();
    if (m_taskQueue) {
        delete m_taskQueue;
    }
}

/**
 * @brief VR_EngineWorkThread::Run
 */
void VR_EngineWorkThread::Run()
{
    VR_LOGD_FUNC();
    if (m_taskQueue == nullptr) {
        VR_LOGD("[DEBUG] ..... fatal error");
        return;
    }

    while (true) {
        if (m_taskQueue->size() == 0) {
            if(CL_TRUE == CheckQuit()) break;                   // check thread need quit
            Wait();
        }

        if(CL_TRUE == CheckQuit()) break;                       // check thread need quit

        while (!m_taskQueue->empty()) {
            if(CL_TRUE == CheckQuit()) break;                   // check thread need quit
            sp_VR_EventBase task;
            {
                BL_AutoSync lock(m_taskLock);
                task = m_taskQueue->front();
                m_taskQueue->pop();
            }

            if (task) {
                task->HandleEvent();
            }
        }
    }
}

// thread start
void VR_EngineWorkThread::ActiveThread()
{
    VR_LOGD_FUNC();
    BL_Thread::RegisterName(VR_DE_BAIDU);
    BL_Thread::StartRegistThread();
}

// thread stop
void VR_EngineWorkThread::DeactiveThread()
{
    VR_LOGD_FUNC();
    BL_Thread::StopThread();
}

void VR_EngineWorkThread::PushTask(sp_VR_EventBase evt)
{
    VR_LOGD_FUNC();
    if (m_taskQueue == nullptr) {
        VR_LOGD("[DEBUG] ..... fatal error");
        return;
    }

    BL_AutoSync lock(m_taskLock);
    m_taskQueue->push(evt);
    Notify();
}
