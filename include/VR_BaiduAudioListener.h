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
 * @file VR_BaiduAudioListener.h
 * @brief Header file for audio in callback
 *
 *
 * @attention used for C++ only.
 */

#ifndef VR_BAIDUAUDIOLISTENER_H
#define VR_BAIDUAUDIOLISTENER_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#include <boost/shared_ptr.hpp>

#include "VC_CommonIF.h"
#include "VC_AudioIn.h"
#include "ncore/NCTypesDefine.h"

class VR_BaiduDialogEngine;
typedef boost::shared_ptr<VR_BaiduDialogEngine> sp_VR_BaiduDialogEngine;


class VR_BaiduAudioListener : public VC_AudioIn::Listener{
public:
    VR_BaiduAudioListener(VR_BaiduDialogEngine* spEngine);
    virtual ~VR_BaiduAudioListener();

    // VC_AudioIn listener callback interface
    virtual VOID OnAudioInData(VOID* data, nutshell::INT len);
    virtual VOID OnAudioInCustom(int type, VOID* data, nutshell::INT len);
    virtual VOID OnAudioInStarted();
    virtual VOID OnAudioInStopped();

private:
    VR_BaiduDialogEngine*   m_pDialogEngine;
    char*                   m_ringbuff;
    char*                   m_buff;
    uint32_t                m_head;
    uint32_t                m_tail;
};

#endif // VR_BAIDUAUDIOLISTENER_H
