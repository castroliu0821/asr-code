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

/**
 * @file VR_EngineWorkThread.h
 * @brief Header for baidu engine work thread
 *
 *
 * @attention used for C++ only.
 */

#ifndef VR_ENGINEWORKTHREAD_H
#define VR_ENGINEWORKTHREAD_H

#include <queue>
#include "VR_Def.h"
#include "VR_EventBase.h"
#include "BL_Thread.h"
#include "BL_SyncObject.h"

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

class VR_EventBase;

typedef boost::shared_ptr<VR_EventBase> sp_VR_EventBase;

class VR_EngineWorkThread : public BL_Thread {
public:
    VR_EngineWorkThread();
    virtual ~VR_EngineWorkThread();

    // thread loop
    virtual void Run();

    // thread start
    virtual void ActiveThread();

    // thread stop
    virtual void DeactiveThread();

    void PushTask(sp_VR_EventBase evt);

private:
    VoiceQueue<sp_VR_EventBase>::type* m_taskQueue;
    BL_SyncObject                      m_taskLock;
};

#endif // VR_ENGINEWORKTHREAD_H
