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
 * @file VR_BaiduDialogEngine.h
 * @brief Header file for baidu dialogengine 
 *
 *
 * @attention used for C++ only.
 */

#ifndef VR_BAIDUDIALOGENGINE_H
#define VR_BAIDUDIALOGENGINE_H

#include <atomic>
#include <boost/shared_ptr.hpp>
#include "VR_Def.h"
#include "VR_DialogEngineIF.h"
#include "NISMHStreamDataReply.h"
#include "NISMHStreamDataProxy.h"

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

class VR_EngineWorkThread;
class VR_BaiduAllWordsManager;
class VR_BaiduAudioListener;
class VR_BaiduAudioIn;
class VR_BaiduStateManager;
class VR_BaiduEngineTimer;
class VR_BaiduProtoTools;

typedef boost::shared_ptr<VR_EngineWorkThread>            sp_VR_EngineWorkThread;
typedef boost::shared_ptr<VR_BaiduAllWordsManager>        sp_VR_BaiduAllWordsManager;
typedef boost::shared_ptr<VR_BaiduAudioListener>          sp_VR_BaiduAudioListener;
typedef boost::shared_ptr<VR_BaiduAudioIn>                sp_VR_BaiduAudioIn;
typedef boost::shared_ptr<VR_BaiduStateManager>           sp_VR_BaiduStateManager;
typedef boost::shared_ptr<VR_BaiduEngineTimer>            sp_VR_BaiduEngineTimer;
typedef boost::shared_ptr<VR_BaiduProtoTools>             sp_VR_BaiduProtoTools;
typedef boost::shared_ptr<nutshell::NISMHStreamDataProxy> sp_NISMHStreamDataProxy;

class VR_BaiduDialogEngine : public VR_DialogEngineIF, public nutshell::NISMHStreamDataReply
{
public:
    VR_BaiduDialogEngine();
    virtual ~VR_BaiduDialogEngine();

    // VR dialogEngine interface
    virtual bool Initialize(VR_DialogEngineListener *listener, const VR_Settings &settings);
    virtual void UnInitialize();
    virtual bool Start();
    virtual void Stop();
    virtual bool SendMessage(const std::string& message, int actionSeqId = VR_ACTION_SEQ_ID_INVALID);
    virtual std::string getHints(const std::string& hintsParams);

    // SmartHome service interface
    virtual VOID onRecvStreamData(const nutshell::NCData& data);

    // Smart home telema interface wrapper
    bool SendAudioData(char* data, int len);       // data package
    bool SendParaData(bool again = false);         // para package
    bool SendLastData();                           // last package
    bool SendCloseConn(); // data package

    // reply message to dialogmanager
    bool ReplyMessage(const std::string& message, int id);

    void HandleEvent(std::string& evt, std::string& msg, int& msg_id);
    bool StartAudioIn(bool again = false);
    bool StopAudioIn();
    int GenerateId();
    std::string getImeiId();

private:
    VR_DialogEngineListener*   m_pListener;
    sp_VR_EngineWorkThread     m_spTaskThread;
    sp_VR_BaiduAllWordsManager m_spAllWordMgr;
    sp_VR_BaiduAudioListener   m_spAudioListener;
    sp_VR_BaiduAudioIn         m_spAudioIn;
    sp_VR_BaiduStateManager    m_spStateManager;
    sp_VR_BaiduEngineTimer     m_spEngineTimer;
    VoiceList<int>::type*      m_pActionIdList;
    sp_VR_BaiduProtoTools      m_spProtoTools;
    sp_NISMHStreamDataProxy    m_spSMHProxy;
    std::atomic<int>           m_id;
    bool                       m_debugmode;
};

#endif // VR_BAIDUDIALOGENGINE_H
