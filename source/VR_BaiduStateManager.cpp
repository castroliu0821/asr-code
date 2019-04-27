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

#include "VR_BaiduStateManager.h"

#include <boost/format.hpp>

#include "VR_Log.h"
#include "VR_Def.h"
#include "VR_BaiduDef.h"
#include "VR_BaiduDialogEngine.h"
#include "VR_ConfigureIF.h"
#include <pugixml.hpp>
#include <rapidjson/document.h>

using namespace std;
using namespace boost;
using namespace pugi;
using namespace rapidjson;

VR_BaiduStateManager::VR_BaiduStateManager(VR_BaiduDialogEngine *pEngine)
    : m_pEngine(pEngine)
    , m_CurState(em_Vr_Engine_idle)
    , m_IsContinue(false)
    , m_BeepActionId(0)
    , m_TtsActionId(0)
    , m_GuideTtsActionId(0)
    , m_StopTtsAciontId(0)
    , m_bNeedQuitVr(false)
    , m_bPlayGuide(false)
    , m_bStopGuide(false)
{
    VR_LOGD_FUNC();
}

VR_BaiduStateManager::~VR_BaiduStateManager()
{
    VR_LOGD_FUNC();
}

void VR_BaiduStateManager::Initilize()
{
    VR_LOGD_FUNC();
}
void VR_BaiduStateManager::Finalize()
{
    VR_LOGD_FUNC();
}

bool VR_BaiduStateManager::OnRecvEvent(string& evt,  string& msg, int& msg_id)
{
    VR_LOGD("[DEBUG] ..... handle evt: %s, mesg: %s, mesg_id: %d", evt.c_str(), msg.c_str(), msg_id);

    bool res = true;
    do {
        if (evt == EV_START_ENGINE) {
            res = HandleStartEngine(msg);
            break;
        }

        if (evt == EV_FINAL_RES) {
            res = HandleFinalResult();
            break;
        }

        if (evt == EV_THIRD_RES) {
            res = HandleThirdPartData(msg);
            break;
        }

        if (evt == EV_TTS_RESULT && msg_id == m_TtsActionId) {
            res = HandlePlayTtsResult();
            break;
        }

        if (evt == EV_TTS_RESULT && msg_id == m_GuideTtsActionId) {
            res = HandleGuideTtsResult();
            break;
        }

        if (evt == EV_STOPTTS_RESULT && msg_id == m_StopTtsAciontId) {
            res = HandleStopTtsResult();
            break;
        }

        if (evt == EV_BEEP_RESULT && msg_id == m_BeepActionId) {
            res = HandlePlayBeepResult();
            break;
        }

        if (evt == EV_CANCEL_ENGINE) {
            res = HandleCancelMessge(msg);
            break;
        }

        if (evt == EV_TIME_OUT) {
            res = HandleTimeOut();
            break;
        }

        if (evt == EV_NET_ABNORMAL) {
            res = HandleCommException(msg);
            break;
        }
    } while (0);

    return res;
}

bool VR_BaiduStateManager::IsRunning()
{
    return m_CurState == em_Vr_Engine_idle ? false : true;
}

bool VR_BaiduStateManager::HandlePlayBeepResult()
{
    VR_LOGD("[DEBUG] ..... handle play beep result");
    if (m_CurState == em_Vr_Engine_wait_beep_s) {
        bool res = m_pEngine->StartAudioIn(m_IsContinue);
        if (res) {
            VR_LOGD("[DEBUG] ..... trans to listening state");
            m_CurState = em_Vr_Engine_listening;

            string xml = (format(XML_ENGINE_STATE) % "listening").str();
            int id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);
        }

        m_IsContinue = false;
        return true;
    }

    if (m_CurState == em_Vr_Engine_wait_beep_r) {
        VR_LOGD("[DEBUG] ..... trans to process state");
        m_CurState = em_Vr_Engine_processing;
        string xml = (format(XML_ENGINE_STATE) % "processing").str();
        int id = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, id);

        if (m_CacheThirdMsg != "") {                                            // handle cache mesg that third data recv in play return beep
            HandleThirdPartData(m_CacheThirdMsg);
            m_CacheThirdMsg.clear();
        }

        return true;
    }

    if (m_CurState == em_Vr_Engine_wait_beep_e) {
        VR_LOGD("[DEBUG] ..... trans to idle state");
        m_CurState = em_Vr_Engine_idle;

        string xml = (format(XML_ENGINE_STATE) % "idle").str();                 // update engine state
        int id = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, id);

        m_pEngine->SendCloseConn();

        xml = XML_RELEASE_NETWORK;                                              // release communication right
        id = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, id);

        xml_document doc;
        for (auto message : m_CancelList) {
            doc.load_string(message.c_str());
            if (!doc) {
                VR_LOGD("[DEBUG] ..... unknown mesg: %s", message.c_str());
                continue;
            }

            xpath_node node;
            do {
                node = doc.select_node("/event[@name='StartBaiduEngine']");
                if (node) {
                    xml = (format(XML_START_ENGINE_RESP) % "false").str();
                    id = m_pEngine->GenerateId();
                    m_pEngine->ReplyMessage(xml, id);
                    break;
                }

                node = doc.select_node("/event[@name='cancel' and @option='smarthome']");
                if (node) {
                    xml = XML_CANCEL_ENGINE_RESP;
                    id = m_pEngine->GenerateId();
                    m_pEngine->ReplyMessage(xml, id);
                    break;
                }

            } while (0);
        }

        m_CancelList.clear();

        if (m_bNeedQuitVr) {
            string xml = XML_QUIT_APP;
            int id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);
        }

        return true;
    }

    return false;
}

bool VR_BaiduStateManager::HandlePlayTtsResult()
{
    if (m_CurState == em_Vr_Engine_wait_tts || m_CurState == em_Vr_Engine_wait_stoptts) {
        VR_LOGD("[DEBUG] ..... trans to listening state");

        if (m_IsContinue) { // multi session
            VR_LOGD("[DEBUG] ..... trans to wait start beep");
            m_pEngine->SendCloseConn();                             // first session close and than second session create

            m_CurState = em_Vr_Engine_wait_beep_s;

            string startBeep;
            startBeep = VR_ConfigureIF::Instance()->getDataPath() + START_BEEP_SUFFIX;

            string xml = (format(XML_ACTION_PLAYBEEP) % startBeep % "false").str();
            m_BeepActionId = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, m_BeepActionId);
            return true;
        }

        VR_LOGD("[DEBUG] ..... trans to wait end beep");
        m_bNeedQuitVr = true;
        m_CurState = em_Vr_Engine_wait_beep_e;

        string endBeep;
        endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;

        string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
        m_BeepActionId = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, m_BeepActionId);
        return true;
    }

    VR_LOGD("[DEBUG] ..... invalid status handle");
    return false;
}

bool VR_BaiduStateManager::HandleGuideTtsResult()
{
    m_bPlayGuide = false;
    m_IsContinue = false;
    m_bNeedQuitVr = false;
    m_CacheThirdMsg.clear();
    m_CurState = em_Vr_Engine_wait_beep_s;

    string xml = (format(XML_START_ENGINE_RESP) % "true").str();
    int id = m_pEngine->GenerateId();
    m_pEngine->ReplyMessage(xml, id);

    string startBeep;
    VR_ConfigureIF* pConf = VR_ConfigureIF::Instance();
    startBeep = pConf->getDataPath() + START_BEEP_SUFFIX;
    xml = (format(XML_ACTION_PLAYBEEP) % startBeep % "false").str();
    m_BeepActionId = m_pEngine->GenerateId();
    m_pEngine->ReplyMessage(xml, m_BeepActionId);
    return true;
}

bool VR_BaiduStateManager::HandleStopTtsResult()
{
    if (m_bStopGuide) {
        VR_LOGD("[DEBUG] ..... stop guide tts");
        m_bStopGuide = false;
        m_bNeedQuitVr = true;

        m_CancelList.push_back(m_StopTtsMsg);
        m_CurState = em_Vr_Engine_wait_beep_e;

        string endBeep;

        endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
        string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
        m_BeepActionId = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, m_BeepActionId);

        return true;
    }

    if (m_CurState == em_Vr_Engine_wait_stoptts) {
        if (m_StopTtsMsg == "") {
            VR_LOGD("[DEBUG] ..... not care");
            return true;
        }
        xml_document doc;
        doc.load_string(m_StopTtsMsg.c_str());
        if (!doc) {
            VR_LOGD("[DEBUG] ..... invalid xml: %s", m_StopTtsMsg.c_str());
            return false;
        }

        if (m_CurState == em_Vr_Engine_listening) {
            m_pEngine->StopAudioIn();
        }

        string xml;
        xpath_node node;
        bool res = true;
        do {
            node = doc.select_node("/event[@name='cancel' and @option='smarthome']");
            if (node) {
                VR_LOGD("[DEBUG] ..... handle common cancel");

                m_bNeedQuitVr = true;

                m_CancelList.push_back(m_StopTtsMsg);
                m_CurState = em_Vr_Engine_wait_beep_e;

                string endBeep;
                endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
                string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
                m_BeepActionId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_BeepActionId);
                res = true;
                break;
            }

            node = doc.select_node("/event[@name='buttonPressed']/keycode[@value='ptt_hard_key_long_press']");
            if (node) {
                VR_LOGD("[DEBUG] ..... handle ptt long press");

                m_bNeedQuitVr = true;
                m_CurState = em_Vr_Engine_wait_beep_e;

                string endBeep;
                endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
                string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
                m_BeepActionId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_BeepActionId);
                break;
            }

            node = doc.select_node("/event[@name='buttonPressed']/keycode[@value='meter_hard_key_back_normal_press']");
            if (node) {
                VR_LOGD("[DEBUG] ..... handle meter back key press");

                m_bNeedQuitVr = true;
                m_CurState = em_Vr_Engine_wait_beep_e;

                string endBeep;
                endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
                string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
                m_BeepActionId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_BeepActionId);
                break;
            }

            node = doc.select_node("/event[@name='changeLanguage']/language");
            if (node) {
                m_bNeedQuitVr = true;
                m_CurState = em_Vr_Engine_wait_beep_e;

                string endBeep;
                endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
                string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
                m_BeepActionId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_BeepActionId);
                res = true;
                break;
            }

            node = doc.select_node("/event[@name='updateState']/item[@key='smartHomeStatus' and @value]");
            if (node) {
                VR_LOGD("[DEBUG] ..... handle smarthome state");
                m_bNeedQuitVr = true;
                m_CurState = em_Vr_Engine_wait_beep_e;

                string endBeep;
                endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
                string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
                m_BeepActionId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_BeepActionId);
                res = true;
                break;
            }
        } while (0);
        m_StopTtsMsg = "";
        VR_LOGD("[DEBUG] ..... handle stop tts action end");
        return res;
    }

    VR_LOGD("[DEBUG] ..... m_curstate = : %d", m_CurState);
    return false;
}

bool VR_BaiduStateManager::HandleFinalResult()
{
    if (m_CurState == em_Vr_Engine_listening) {
        VR_LOGD("[DEBUG] ..... trans to wait return beep end");
        m_CurState = em_Vr_Engine_wait_beep_r;

        bool res = m_pEngine->StopAudioIn();
        if (!res) {
            VR_LOGD("[DEBUG] ..... stop audio in failed");
        }

        string returnBeep;
        returnBeep = VR_ConfigureIF::Instance()->getDataPath() + RET_BEEP_SUFFIX;

        string xml = (format(XML_ACTION_PLAYBEEP) % returnBeep % "false").str();
        m_BeepActionId = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, m_BeepActionId);
        return true;
    }

    return false;
}

bool VR_BaiduStateManager::HandleStartEngine(string& msg)
{
    VR_LOGD("[DEBUG] ..... handle start mesg, %d", m_CurState);
    VR_ConfigureIF* pConf = VR_ConfigureIF::Instance();

    bool bSMHCancel = true;
    bool bTTSCrash = true;
    pConf->getSMHCancelStatus(bSMHCancel, bTTSCrash);
    VR_LOGD("[DEBUG] ..... isRunning = %d, bSMHCancel = %d, bTTSCrash = %d", IsRunning(), bSMHCancel, bTTSCrash);

    if (IsRunning() || bSMHCancel || bTTSCrash) {
        string xml;
        if (bTTSCrash) {
            VR_LOGD("[DEBUG] ..... tts crash");

            m_CurState = em_Vr_Engine_idle;
            xml = XML_QUIT_APP;
            int id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);

            xml = (format(XML_START_ENGINE_RESP) % "false").str();
            id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);
            return true;
        }

        m_bNeedQuitVr = true;
        m_CancelList.push_back(msg);
        m_CurState = em_Vr_Engine_wait_beep_e;

        string endBeep;
        endBeep = pConf->getDataPath() + END_BEEP_SUFFIX;
        xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
        m_BeepActionId = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, m_BeepActionId);

        return true;
    }

    bool bState = pConf->getSMHActiveStatus();
    if (!bState) {
        m_bNeedQuitVr = true;
        m_CancelList.push_back(msg);
        m_CurState = em_Vr_Engine_wait_beep_e;

        string endBeep;
        endBeep = pConf->getDataPath() + END_BEEP_SUFFIX;
        string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
        m_BeepActionId = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, m_BeepActionId);
        return true;
    }

    VR_ConfigureIF::VR_AbnormalType err = VR_ConfigureIF::VR_AbnormalType::NO_ERROR;
    err = pConf->getSMHAbnormalType();
    if (VR_ConfigureIF::VR_AbnormalType::INNER == err) {
        m_CurState = em_Vr_Engine_error;
        string ons = (format(XML_ERROR_ONS) % "ONS-SMH-002").str();
        int id = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(ons, id);
        return true;
    }

    if (VR_ConfigureIF::VR_AbnormalType::ACCOUNT == err) {
        m_CurState = em_Vr_Engine_error;
        string ons = (format(XML_ERROR_ONS) % "ONS-SMH-003").str();
        int id = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(ons, id);
        return true;
    }

    string xml = (format(XML_ENGINE_STATE) % "idle").str();                     // update engine state
    int id = m_pEngine->GenerateId();
    m_pEngine->ReplyMessage(xml, id);

    int level = pConf->getVRPromptLevel();                              // level is 0, skip play tts
    if (0 != level) {
        std::string language = pConf->getVRLanguage();
        std::string isBargein = "false";
        const static string tts = "请说出您要进行的操作!";
        string xml = (format(XML_ACTION_PLAYTTS) % language % isBargein % tts).str();
        m_GuideTtsActionId = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, m_GuideTtsActionId);
        VR_LOGD("[DEBUG] ..... play tts : %s", tts.c_str());

        m_bPlayGuide = true;
        m_bStopGuide = false;
        return true;
    }

    m_bStopGuide = false;
    m_bPlayGuide = false;
    m_IsContinue = false;
    m_bNeedQuitVr = false;
    m_CacheThirdMsg.clear();
    m_CurState = em_Vr_Engine_wait_beep_s;

    xml = (format(XML_START_ENGINE_RESP) % "true").str();
    id = m_pEngine->GenerateId();
    m_pEngine->ReplyMessage(xml, id);

    string startBeep;
    startBeep = pConf->getDataPath() + START_BEEP_SUFFIX;
    xml = (format(XML_ACTION_PLAYBEEP) % startBeep % "false").str();
    m_BeepActionId = m_pEngine->GenerateId();
    m_pEngine->ReplyMessage(xml, m_BeepActionId);
    return true;
}

bool VR_BaiduStateManager::HandleThirdPartData(string &mesg)
{
    VR_LOGD("[DEBUG] ..... handle third data.  mesg: %s", mesg.c_str());
    if (m_CurState == em_Vr_Engine_wait_beep_r) {
	VR_LOGD("[DEBUG] ..... cache mesg");
        m_CacheThirdMsg = mesg;
        return true;
    }

    if (m_CurState == em_Vr_Engine_processing) {

        std::string tts;
        int err = GetTtsAndContinue(mesg, tts, m_IsContinue);
        VR_LOGD("error : %d,  tts: %s,  continue: %d", err, tts.c_str(), m_IsContinue);
        if (err == 0) {

            if (m_IsContinue) {
                string hints = (format(XML_NOTIFY_HINT) % tts).str();
                int id = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(hints, id);
            }

            VR_ConfigureIF* pConf = VR_ConfigureIF::Instance();
            int level = pConf->getVRPromptLevel();                              // level is 0, skip play tts
            if (0 == level) {
                if (m_IsContinue) {                                             // second session

                    m_pEngine->SendCloseConn();                                 // first session close and than second session create

                    m_CurState = em_Vr_Engine_wait_beep_s;

                    string startBeep;
                    startBeep = VR_ConfigureIF::Instance()->getDataPath() + START_BEEP_SUFFIX;

                    string xml = (format(XML_ACTION_PLAYBEEP) % startBeep % "false").str();
                    m_BeepActionId = m_pEngine->GenerateId();
                    m_pEngine->ReplyMessage(xml, m_BeepActionId);
                    VR_LOGD("[DEBUG] ..... skip tts, and second session");
                    return true;
                }

                m_pEngine->SendLastData();                                      // send LAST package when is last session third package back

                m_bNeedQuitVr = true;
                m_CurState = em_Vr_Engine_wait_beep_e;                          // final session

                string endBeep;
                endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;

                string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
                m_BeepActionId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_BeepActionId);
                VR_LOGD("[DEBUG] ..... skip tts and play end beep");
                return true;
            }

            m_CurState = em_Vr_Engine_wait_tts;                                 // level is not 0, normal play tts

            m_bNeedQuitVr = m_IsContinue ? false : true;                        // final session quit vr
            std::string language = VR_ConfigureIF::Instance()->getVRLanguage();
            std::string isBargein = "false";

            string xml = (format(XML_ACTION_PLAYTTS) % language % isBargein % tts).str();
            m_TtsActionId = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, m_TtsActionId);
            VR_LOGD("[DEBUG] ..... play tts : %s", tts.c_str());

            xml = (format(XML_ENGINE_STATE) % "promptPlaying").str();           // update engine state
            int id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);
        } else if (err == -3014) {                                              // Ref: Home連携Error該当Ons一覧1221.xlsx
            m_bNeedQuitVr = true;
            m_CurState = em_Vr_Engine_wait_beep_e;

            string endBeep;
            endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;

            string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
            m_BeepActionId = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, m_BeepActionId);

            VR_LOGD("[DEBUG] ..... json error -3014");
        } else if (err == -3004) {                                              // Ref: Home連携Error該当Ons一覧1221.xlsx
            m_CurState = em_Vr_Engine_error;
            string ons = (format(XML_ERROR_ONS) % "ONS-SMH-003").str();
            int id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(ons, id);
            VR_LOGD("[DEBUG] ..... json error -3004");
        } else {                                                                // Default ONS: ONS-SMH-004
            m_CurState = em_Vr_Engine_error;
            string ons = (format(XML_ERROR_ONS) % "ONS-SMH-004").str();
            int id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(ons, id);
            VR_LOGD("[DEBUG] ..... json error");
        }
    }

    return true;
}

bool VR_BaiduStateManager::HandleCancelMessge(std::string& mesg)
{
    VR_LOGD("[DEBUG] ..... handle cancel mesg, state: %d", m_CurState);

    xml_document doc;
    doc.load_string(mesg.c_str());
    if (!doc) {
        VR_LOGD("[DEBUG] ..... invalid xml: %s", mesg.c_str());
        return false;
    }

    if (m_CurState == em_Vr_Engine_listening) {
        m_pEngine->StopAudioIn();
    }

    m_IsContinue = false;                                                       // there no mutli session come again;

    int id;
    string xml;
    xpath_node node;
    bool res = true;
    do {
        node = doc.select_node("/event[@name='cancel' and @option='ttscrash']"); // special case, don't play beep and tts, directly exit vr
        if (node) {
            VR_LOGD("[DEBUG] ..... handle tts crash");
            xml = XML_CANCEL_TTSCRASH_RESP;
            id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);

            if (m_CurState == em_Vr_Engine_idle || m_bPlayGuide || m_bStopGuide) {
                break;
            }

            xml_document doc;
            for (auto message : m_CancelList) {
                doc.load_string(message.c_str());
                if (!doc) {
                    VR_LOGD("[DEBUG] ..... xml error. %s", message.c_str());
                    return false;
                }

                xpath_node nd;
                do {
                    nd = doc.select_node("/event[@name='cancel' and @option='smarthome']");
                    if (nd) {
                        xml = XML_CANCEL_ENGINE_RESP;
                        id = m_pEngine->GenerateId();
                        m_pEngine->ReplyMessage(xml, id);
                        break;
                    }

                } while (0);
            }

            m_CancelList.clear();
            
            if (!m_bPlayGuide && !m_bStopGuide) {
                m_pEngine->SendCloseConn();

                xml = XML_RELEASE_NETWORK;                                          // release communication right
                id = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, id);
            }

            m_CurState = em_Vr_Engine_idle;
            xml = XML_QUIT_APP;
            id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);
            m_bPlayGuide = false;                                               // update play guide tts state
            break;
        }

        node = doc.select_node("/event[@name='cancel' and @option='smarthome']");
        if (node) {
            VR_LOGD("[DEBUG] ..... handle common cancel");

            if (m_bPlayGuide) {
                xml = (format(XML_ACTION_STOPTTS) % m_GuideTtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                m_bStopGuide = true;
                m_bPlayGuide = false;
                break;
            }

            if (m_CurState == em_Vr_Engine_idle ) {
                xml = XML_CANCEL_ENGINE_RESP;
                id = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, id);

                break;
            }

            if (m_CurState == em_Vr_Engine_wait_beep_e ||
                m_CurState == em_Vr_Engine_wait_stoptts) {
                VR_LOGD("[DEBUG] ..... cancel in end beep and in stop tts");
                m_CancelList.push_back(mesg);                                   // cache multi cancel message
                break;
            }

            if (m_CurState == em_Vr_Engine_wait_tts) {
                m_CurState = em_Vr_Engine_wait_stoptts;
                xml = (format(XML_ACTION_STOPTTS) % m_TtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                break;
            }

            m_bNeedQuitVr = true;
            m_CancelList.push_back(mesg);
            m_CurState = em_Vr_Engine_wait_beep_e;

            string endBeep;
            endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
            string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
            m_BeepActionId = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, m_BeepActionId);
            break;
        }

        node = doc.select_node("/event[@name='buttonPressed']/keycode[@value='ptt_hard_key_long_press']");
        if (node) {
            VR_LOGD("[DEBUG] ..... handle ptt long press");
            xml = XML_PTT_LONG_PRESS_RESP;
            id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);

            if (m_bPlayGuide) {
                xml = (format(XML_ACTION_STOPTTS) % m_GuideTtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                m_bStopGuide = true;
                m_bPlayGuide = false;
                break;
            }

            if (m_CurState == em_Vr_Engine_idle ||
                m_CurState == em_Vr_Engine_wait_stoptts ||
                m_CurState == em_Vr_Engine_wait_beep_e) {
                break;
            }

            if (m_CurState == em_Vr_Engine_wait_tts) {
                m_CurState = em_Vr_Engine_wait_stoptts;
                xml = (format(XML_ACTION_STOPTTS) % m_TtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                break;
            }

            m_bNeedQuitVr = true;
            m_CurState = em_Vr_Engine_wait_beep_e;

            string endBeep;
            endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
            string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
            m_BeepActionId = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, m_BeepActionId);
            break;
        }

        node = doc.select_node("/event[@name='buttonPressed']/keycode[@value='meter_hard_key_back_normal_press']");
        if (node) {
            VR_LOGD("[DEBUG] ..... handle meter back key press");
            xml = XML_METER_BACK_RESP;
            id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);

            if (m_bPlayGuide) {
                xml = (format(XML_ACTION_STOPTTS) % m_GuideTtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                m_bStopGuide = true;
                m_bPlayGuide = false;
                break;
            }

            if (m_CurState == em_Vr_Engine_idle ||
                m_CurState == em_Vr_Engine_wait_stoptts ||
                m_CurState == em_Vr_Engine_wait_beep_e) {
                break;
            }

            if (m_CurState == em_Vr_Engine_wait_tts) {
                m_CurState = em_Vr_Engine_wait_stoptts;
                xml = (format(XML_ACTION_STOPTTS) % m_TtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                break;
            }

            m_bNeedQuitVr = true;
            m_CurState = em_Vr_Engine_wait_beep_e;

            string endBeep;
            endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
            string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
            m_BeepActionId = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, m_BeepActionId);
            break;
        }

        node = doc.select_node("/event[@name='changeLanguage']/language");
        if (node) {
            VR_LOGD("[DEBUG] ..... handle change lang");
            string lang = node.node().text().as_string();
            xml = (format(XML_CHANGE_LANG_RESP) % lang).str();
            id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);

            if (m_bPlayGuide) {
                xml = (format(XML_ACTION_STOPTTS) % m_GuideTtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                m_bStopGuide = true;
                m_bPlayGuide = false;
                break;
            }

            if (m_CurState == em_Vr_Engine_idle ||
                m_CurState == em_Vr_Engine_wait_stoptts ||
                m_CurState == em_Vr_Engine_wait_beep_e) {
                break;
            }

            if (m_CurState == em_Vr_Engine_wait_tts) {
                m_CurState = em_Vr_Engine_wait_stoptts;
                xml = (format(XML_ACTION_STOPTTS) % m_TtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                break;
            }

            m_bNeedQuitVr = true;
            m_CurState = em_Vr_Engine_wait_beep_e;

            string endBeep;
            endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
            string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
            m_BeepActionId = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, m_BeepActionId);
            break;
        }

        node = doc.select_node("/event[@name='updateState']/item[@key='smartHomeStatus' and @value]");
        if (node) {
            VR_LOGD("[DEBUG] ..... handle smarthome state");
            xml = XML_UPDATE_STATE_RESP;
            id = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, id);

            string val = node.node().attribute("value").value();
            if (val != "false") {
                break;
            }

            if (m_bPlayGuide) {
                xml = (format(XML_ACTION_STOPTTS) % m_GuideTtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                m_bStopGuide = true;
                m_bPlayGuide = false;
                break;
            }

            if (m_CurState == em_Vr_Engine_idle ||
                m_CurState == em_Vr_Engine_wait_stoptts ||
                m_CurState == em_Vr_Engine_wait_beep_e) {
                break;
            }

            if (m_CurState == em_Vr_Engine_wait_tts) {
                m_CurState = em_Vr_Engine_wait_stoptts;
                xml = (format(XML_ACTION_STOPTTS) % m_TtsActionId).str();
                m_StopTtsAciontId = m_pEngine->GenerateId();
                m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
                m_StopTtsMsg = mesg;
                break;
            }

            m_bNeedQuitVr = true;
            m_CurState = em_Vr_Engine_wait_beep_e;

            string endBeep;
            endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;
            string xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
            m_BeepActionId = m_pEngine->GenerateId();
            m_pEngine->ReplyMessage(xml, m_BeepActionId);
            break;
        }
    } while (0);

    return res;
}

nbool VR_BaiduStateManager::HandleCommException(string& mesg)
{
    VR_LOGD("[DEBUG] ..... handle abnormal msg, state: %d", m_CurState);

    if ((m_CurState == em_Vr_Engine_idle ||
        m_CurState == em_Vr_Engine_error ||
        m_CurState == em_Vr_Engine_wait_beep_e) && m_bPlayGuide) {
        return false;
    }

    if (m_bPlayGuide) {
        string xml = (format(XML_ACTION_STOPTTS) % m_GuideTtsActionId).str();   // don't care stop tts result
        m_GuideTtsActionId = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, m_GuideTtsActionId);
        m_StopTtsMsg = "";
        m_bStopGuide = true;
        m_bPlayGuide = false;
    }

    if (m_CurState == em_Vr_Engine_listening) {
        bool res = m_pEngine->StopAudioIn();
        if (!res) {
            VR_LOGD("[DEBUG] ..... stop audio in failed");
        }
    }

    string xml = (format(XML_ENGINE_STATE) % "idle").str();                     // update engine state
    int id = m_pEngine->GenerateId();
    m_pEngine->ReplyMessage(xml, id);


    if (m_CurState == em_Vr_Engine_wait_tts) {
        m_CurState = em_Vr_Engine_wait_stoptts;
        xml = (format(XML_ACTION_STOPTTS) % m_TtsActionId).str();
        m_StopTtsAciontId = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, m_StopTtsAciontId);
        m_StopTtsMsg = "";
    }

    xml_document doc;
    doc.load_string(mesg.c_str());
    xpath_node node = doc.select_node("/event/status");
    string err = node.node().text().as_string();
    VR_LOGD("[DEBUG] ..... error: %s", err.c_str());
    if (err == "Inner") {
        m_CurState = em_Vr_Engine_error;
        string ons = (format(XML_ERROR_ONS) % "ONS-SMH-002").str();
        int id = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(ons, id);

    } else {
        m_CurState = em_Vr_Engine_error;
        string ons = (format(XML_ERROR_ONS) % "ONS-SMH-003").str();
        int id = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(ons, id);
    }

    return true;
}

bool VR_BaiduStateManager::HandleTimeOut()
{
    VR_LOGD("[DEBUG] ..... handle timeout msg, state: %d", m_CurState);
    if (m_CurState == em_Vr_Engine_listening) {
        bool res = m_pEngine->StopAudioIn();
        if (!res) {
            VR_LOGD("[DEBUG] ..... stop audio in failed");
        }

        string xml = (format(XML_ENGINE_STATE) % "idle").str();                 // update engine state
        int id = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, id);

        m_bNeedQuitVr = true;
        m_CurState = em_Vr_Engine_wait_beep_e;

        string endBeep;
        endBeep = VR_ConfigureIF::Instance()->getDataPath() + END_BEEP_SUFFIX;

        xml = (format(XML_ACTION_PLAYBEEP) % endBeep % "false").str();
        m_BeepActionId = m_pEngine->GenerateId();
        m_pEngine->ReplyMessage(xml, m_BeepActionId);
    }

    return true;
}

int VR_BaiduStateManager::GetTtsAndContinue(std::string& mesg, std::string& tts, bool& isContinue)
{
    Document doc;
    doc.Parse<0>(mesg.c_str());
    tts = "";
    isContinue = false;

    if (doc.HasParseError()) {
        VR_LOGD("[DEBUG] ..... invalid json format. : %s", mesg.c_str());
        tts = "";
        isContinue = false;
        return -1;
    }

    int error = 0;

    if (!doc.HasMember("errno")) {
        VR_LOGD("[DEBUG] ..... no errno node");
        return -3008;
    }

    const Value& dat1 = doc["errno"];
    if (!dat1.IsInt()) {
        VR_LOGD("[DEBUG] ..... invalid errno");
        return -3008;
    }

    error = dat1.GetInt();                                                      // get json 'errno' node content

    if (error != 0) {
        VR_LOGD("[DEBUG] server error : %d", error);
        const Value& mesg = doc["message"];
        if (!mesg.IsString()) {
            VR_LOGD("[DEBUG] ..... mesg node error");
            return  -3008;
        }
        VR_LOGD("[DEBUG] server error mesg : %s", mesg.GetString());
        return error;
    }

    if (!doc.HasMember("result")) {
        VR_LOGD("[DEBUG] ..... no result node");
        return  -3008;
    }

    if (!doc["result"].IsArray()) {
        VR_LOGD("[DEBUG] ..... result not array");
        return  -3008;
    }

    const Value& arr = doc["result"];
    if (1 != arr.Capacity()) {
        VR_LOGD("[DEBUG] ..... array size is not : %d", arr.Size());
        return  -3008;
    }

    if (!doc["result"][static_cast<SizeType>(0)].HasMember("tts_status")) {
        VR_LOGD("[DEBUG] ..... no tts_status");
        return  -3008;
    }

    if (!doc["result"][static_cast<SizeType>(0)]["tts_status"].IsObject()) {
        VR_LOGD("[DEBUG] ..... tts_status type error");
        return  -3008;
    }

    if (!doc["result"][static_cast<SizeType>(0)]["tts_status"].HasMember("tts")) {
        VR_LOGD("[DEBUG] ..... no tts");
        return  -3008;
    }

    const Value& dat2 = doc["result"][static_cast<SizeType>(0)]["tts_status"]["tts"];
    if (!dat2.IsString()) {
        VR_LOGD("[DEBUG] ..... tts error");
        return -3008;
    }

    tts = dat2.GetString();

    if (!doc["result"][static_cast<SizeType>(0)].HasMember("data")) {
        VR_LOGD("[DEBUG] ..... no data");
        return  -3008;
    }

    if (!doc["result"][static_cast<SizeType>(0)]["data"].IsObject()) {
        VR_LOGD("[DEBUG] ..... data type error");
        return  -3008;
    }

    if (!doc["result"][static_cast<SizeType>(0)]["data"].HasMember("hint_wakeup")) {
        VR_LOGD("[DEBUG] ..... no hint wakeup");
        return  -3008;
    }

    const Value& dat3 = doc["result"][static_cast<SizeType>(0)]["data"]["hint_wakeup"];
    if (!dat3.IsInt()) {
        VR_LOGD("[DEBUG] ..... hint error");
        return -3008;       // parse json error, ONS-004
    }

    isContinue = dat3.GetInt() == 1 ? false: true;                              // hint_wakeup 0: not finish  1: finish
    return error;
}
