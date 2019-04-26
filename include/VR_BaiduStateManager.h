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
 * @file VR_BaiduStateManager.h
 * @brief Header for state manager
 *
 *
 * @attention used for C++ only.
 */

#ifndef VR_BAIDUSTATEMANAGER_H
#define VR_BAIDUSTATEMANAGER_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

class VR_BaiduDialogEngine;

#include "string"
#include "VR_Def.h"

class VR_BaiduStateManager {

    enum {
        em_Vr_Engine_idle              = 0,
        em_Vr_Engine_wait_beep_s       = 1,
        em_Vr_Engine_wait_beep_r       = 2,
        em_Vr_Engine_wait_beep_e       = 3,
        em_Vr_Engine_processing        = 4,
        em_Vr_Engine_listening         = 5,
        em_Vr_Engine_wait_tts          = 6,
        em_Vr_Engine_wait_stoptts      = 7,
        em_Vr_Engine_error             = 8,
    };

public:
    VR_BaiduStateManager(VR_BaiduDialogEngine* pEngine);
    virtual ~VR_BaiduStateManager();

    void Initilize();
    void Finalize();

    bool OnRecvEvent(std::string& evt,  std::string& msg, int& msg_id);
    bool IsRunning();

private:
    bool HandlePlayBeepResult();
    bool HandlePlayTtsResult();
    bool HandleGuideTtsResult();
    bool HandleStopTtsResult();
    bool HandleFinalResult();
    bool HandleStartEngine(std::string& mesg);
    bool HandleThirdPartData(std::string& mesg);
    bool HandleCancelMessge(std::string& mesg);
    bool HandleCommException(std::string& mesg);
    bool HandleTimeOut();
    int  GetTtsAndContinue(std::string& mesg, std::string& tts, bool& isContinue);

private:
    VR_BaiduDialogEngine*        m_pEngine;
    int                          m_CurState;         // current state
    bool                         m_IsContinue;       // Second session flag
    int                          m_BeepActionId;     // Beep action id
    int                          m_TtsActionId;      // Tts action id
    int                          m_GuideTtsActionId; // Guide tts action id
    int                          m_StopTtsAciontId;  // Stop Tts action id
    VoiceList<std::string>::type m_CancelList;       // Store cancel type message
    std::string                  m_StopTtsMsg;       // Store cause to stop message
    bool                         m_bNeedQuitVr;      // Need service quit vr flag after play end beep
    bool                         m_bPlayGuide;       // Is in playing guide tts
    std::string                  m_CacheThirdMsg;    // Cache third in return beep phases
};

#endif // VR_BAIDUSTATEMANAGER_H
