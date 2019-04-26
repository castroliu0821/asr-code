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

#include "VR_BaiduDialogEngine.h"

#include <algorithm>
#include <boost/assign.hpp>

#include "VR_Log.h"
#include "VR_Def.h"
#include "VR_EngineWorkThread.h"
#include "VR_BaiduAllWordsManager.h"
#include "VR_BaiduAudioListener.h"
#include "VR_BaiduAudioIn.h"
#include "VR_BaiduEngineTimer.h"
#include "VR_BaiduStateManager.h"
#include "VR_BaiduProtoTools.h"
#include "VR_EventBase.h"
#include "VR_BaiduDef.h"
#include "VR_Configure.h"
#include "VrSmartHomeData.pb.h"

#include "NISMHStreamDataReply.h"

#include "pugixml.hpp"

using namespace std;
using namespace pugi;
using namespace nutshell;
using namespace navi::VoiceRecog;

static const VoiceMap<string, string>::type g_s_MesageRoute = boost::assign::map_list_of
    ("/event[@name='buttonPressed']/keycode[@value='ptt_hard_key_long_press']"          , EV_CANCEL_ENGINE)
    ("/event[@name='buttonPressed']/keycode[@value='meter_hard_key_back_normal_press']" , EV_CANCEL_ENGINE)
    ("/event[@name='updateState']/item[@key='smartHomeStatus' and @value]"              , EV_CANCEL_ENGINE)
    ("/event[@name='changeLanguage']/language"              , EV_CANCEL_ENGINE)
    ("/event[@name='cancel' and @option='smarthome']"       , EV_CANCEL_ENGINE)
    ("/event[@name='cancel' and @option='ttscrash']"        , EV_CANCEL_ENGINE)
    ("/event[@name='StartBaiduEngine']"                     , EV_START_ENGINE)
    ("/event[@name='notifyAbnormal']"                       , EV_NET_ABNORMAL)
    ("/action-result[@op='playBeep']"                       , EV_BEEP_RESULT)
    ("/action-result[@op='playTts']"                        , EV_TTS_RESULT)
    ("/action-result[@op='stopTts']"                        , EV_STOPTTS_RESULT);

VR_BaiduDialogEngine::VR_BaiduDialogEngine()
    : VR_DialogEngineIF()
    , m_pListener(nullptr)
    , m_spTaskThread(VR_new VR_EngineWorkThread)
    , m_spAllWordMgr(VR_new VR_BaiduAllWordsManager)
    , m_spAudioListener(VR_new VR_BaiduAudioListener(this))
    , m_spStateManager(VR_new VR_BaiduStateManager(this))
    , m_spEngineTimer(VR_new VR_BaiduEngineTimer(m_spTaskThread, this))
    , m_pActionIdList (VR_new VoiceList<int>::type)
    , m_spProtoTools(VR_new VR_BaiduProtoTools(this, m_spTaskThread))
    , m_spSMHProxy(VR_new NISMHStreamDataProxy(this))
    , m_id(0)
    , m_debugmode(false)
{
    string file = "/tmp/baidu_debug.flag";
    ifstream fs(file, ios_base::in);
    if (fs.is_open()) {
        m_debugmode = true;
    }

    m_spAudioIn.reset(VR_new VR_BaiduAudioIn(m_debugmode));

    VR_LOGD_FUNC();
}

VR_BaiduDialogEngine::~VR_BaiduDialogEngine()
{
    VR_LOGD_FUNC();
    if (m_pActionIdList) {
        delete m_pActionIdList;
        m_pActionIdList = nullptr;
    }
}

// VR dialogEngine interface
bool VR_BaiduDialogEngine::Initialize(VR_DialogEngineListener *listener, const VR_Settings &settings)
{
    VR_LOGD_FUNC();
    VR_UNUSED_VAR(settings);

    bool res = true;
    m_pListener = listener;

    m_spAllWordMgr->Initialize();
    VR_LOGD("[DEBUG] ..... all words init complete");

    m_spStateManager->Initilize();
    VR_LOGD("[DEBUG] ..... state manager init complete");

    res = res & m_spSMHProxy->init();
    VR_LOGD("[DEBUG] ..... proxy init complete");

    m_spTaskThread->ActiveThread();
    VR_LOGD("[DEBUG] ..... main thread start");

    return res;
}

void VR_BaiduDialogEngine::UnInitialize()
{
    VR_LOGD_FUNC();

    m_pActionIdList->clear();

    m_spTaskThread->DeactiveThread();

    m_spSMHProxy->deinit();

    m_spAllWordMgr->Finalize();

    m_spStateManager->Finalize();
}

bool VR_BaiduDialogEngine::Start()
{
    VR_LOGD_FUNC();

    m_pListener->OnStarted();
    return true;
}

void VR_BaiduDialogEngine::Stop()
{
    VR_LOGD_FUNC();

    m_pListener->OnStopped();
}

bool VR_BaiduDialogEngine::SendMessage(const std::string& message, int actionSeqId)
{
    VR_LOGD("[DEBUG] ..... receive mesg form outside");

    xml_document doc;
    doc.load_string(message.c_str());
    if (!doc) {
        VR_LOGD("[DEBUG] ..... error mesg: %s, %d", message.c_str(), actionSeqId);
    }

    string evtName = "";
    for (auto route : g_s_MesageRoute) {
        xpath_node node = doc.select_node(route.first.c_str());
        if (!node) {
            continue;
        }

        evtName = route.second;
        break;
    }

    if (evtName == "") {
        VR_LOGD("[DEBUG] ..... filter message :[%s]-[%d]", message.c_str(), actionSeqId);
        return true;
    }

    stEvent_info info;
    info.sender = "DM";
    info.mesg = std::move(message);
    info.mesg_id = actionSeqId;
    info.name = evtName;
    sp_VR_EventBase evt(VR_new VR_EventBase(info, this));

    m_spTaskThread->PushTask(evt);
    return true;
}

std::string VR_BaiduDialogEngine::getHints(const std::string& hintsParams)
{
    VR_LOGD_FUNC();
    VR_UNUSED_VAR(hintsParams);
    return "";
}

// SmartHome service interface
VOID VR_BaiduDialogEngine::onRecvStreamData(const nutshell::NCData& data)
{
    VR_LOGD("[DEBUG] ..... receive down stream");

    bool res = m_spProtoTools->HandlePackage(data.getData(), data.getSize());
    if (!res) {
        VR_LOGD("[DEBUG] ..... down stream error");
    }
}

// Smart home telema interface wrapper
/**
 * @brief VR_BaiduDialogEngine::SendAudioData
 * @note ATTENTION:  This function invoke by AUDIO IN Thread
 *
 */
bool VR_BaiduDialogEngine::SendAudioData(char* data, int len)
{
    if (data == nullptr || len == 0) {
        VR_LOGD("[DEBUG] ..... Invalid para, data: %p, len:%d", data, len);
        return false;
    }

    m_spAudioIn->write_to_file(data, static_cast<size_t>(len));

    APIRequest api_req;
    api_req.set_api_req_type(API_REQ_TYPE_DATA);
    ApiData* audiodata = api_req.mutable_data();  //此处忽略异常
    audiodata->set_len(static_cast<uint32_t>(len));
    audiodata->set_post_data(data, static_cast<uint32_t>(len));

    std::string buffer;
    api_req.SerializeToString(&buffer);

    int protolen = static_cast<int>(buffer.length());
    int buflen = protolen +  static_cast<int>(sizeof(int));
    char* tmp = new char[buflen];
    ::memcpy(tmp, &protolen, sizeof(int));
    ::memcpy(tmp + sizeof(int), buffer.data(),  static_cast<size_t>(protolen));
    nutshell::NCData protodata(tmp, buflen);
    delete[] tmp;
    tmp = nullptr;

    if (m_debugmode) {
        usleep(160000);             // 5120 bytes
    }

    bool res = m_spSMHProxy->requestSendStreamData(false, &protodata);
    string str = res ? "success" : "failed";
    VR_LOGD("[Performance] ...... send audio end. res: %s", str.c_str());
    return res;
}

bool VR_BaiduDialogEngine::SendParaData(bool again)
{
    APIRequest param;
    ApiParam* p = nullptr;
    param.set_api_req_type(API_REQ_TYPE_PARAM);
    p = param.mutable_param();
    p->set_sample_rate(16000);
#ifdef OPUS_AUDIO_TYPE
    p->set_format("opus");
#else
    p->set_format("pcm");
#endif
    p->set_early_return(true);
    p->set_imei(getImeiId().c_str());
    p->set_home_link_type(1);
    p->set_asr_backend_type("iov");

    string buff;
    param.SerializeToString(&buff);

    auto proto_size = static_cast<int>(buff.length()) ;
    auto buff_size = proto_size + static_cast<int>(sizeof(int));
    auto tmp = new char[buff_size];
    memcpy(tmp, &proto_size, sizeof(int));
    memcpy(tmp + sizeof(int), const_cast<char*>(buff.data()), static_cast<size_t>(proto_size));

    NCData dat;
    dat.setData(tmp, buff_size);
    delete[] tmp;
    tmp = nullptr;

    bool last = false;
    bool res = m_spSMHProxy->requestSendStreamData(last, &dat, again);
    string str = res ? "success" : "failed";
    VR_LOGD("[Performance] ...... send param end. res: %s", str.c_str());
    VR_LOGD("[DEBUG] ...... send param end. res: %s", str.c_str());
    return res;
}

bool VR_BaiduDialogEngine::SendLastData()
{
    APIRequest api_req;
    api_req.set_api_req_type(API_REQ_TYPE_LAST);

    std::string buffer;
    api_req.SerializeToString(&buffer);

    int protolen = static_cast<int>(buffer.length());
    int buflen = protolen +  static_cast<int>(sizeof(int));
    char* tmp = new char[buflen];
    ::memcpy(tmp, &protolen, sizeof(int));
    ::memcpy(tmp + sizeof(int), buffer.data(),  static_cast<size_t>(protolen));
    NCData dat(tmp, buflen);
    delete[] tmp;
    tmp = nullptr;

    nutshell::iov::NDIOVHttpUpStream req;
    bool last = false;
    bool res = m_spSMHProxy->requestSendStreamData(last, &dat);
    string str = res ? "success" : "failed";
    VR_LOGD("[Performance] ...... send last end. res: %s", str.c_str());
    VR_LOGD("[DEBUG] ...... send last end. res: %s", str.c_str());
    return res;
}

bool VR_BaiduDialogEngine::SendCloseConn() {
    NCData dat;
    bool res = m_spSMHProxy->requestSendStreamData(true, &dat);
    string str = res ? "success" : "failed";
    VR_LOGD("[Performance] ...... send end. res: %s", str.c_str());
    VR_LOGD("[DEBUG] ...... send end. res: %s", str.c_str());
    return res;
}

// reply message to dialogmanager
bool VR_BaiduDialogEngine::ReplyMessage(const std::string& message, int id)
{
    VR_LOGD_FUNC();

    m_pListener->OnRequestAction(message, id);

    VR_LOGD("[DEBUG] ..... reply mssage : [%s] [%d]", message.c_str(), id);

    return true;
}

void VR_BaiduDialogEngine::HandleEvent(string& evt, string& msg, int& msg_id)
{
    m_spStateManager->OnRecvEvent(evt, msg, msg_id);
}

bool VR_BaiduDialogEngine::StartAudioIn(bool again)
{
    static int Bitrate = 16000;
    static int BuffLen = 4096;
    static int Count = 4;
    bool res = m_spAudioIn->Open(&Bitrate, &BuffLen, &Count, VC_AUDIO_MODE_NORMAL);
    if (!res) {
        VR_LOGD("[DEBUG] ..... open device failed");
        return false;
    }

    SendParaData(again);

    res = m_spAudioIn->Start(m_spAudioListener.get());
    if (!res) {
        VR_LOGD("[DEBUG] ..... start device failed");
        m_spAudioIn->Close();
    }

    m_spEngineTimer->Start();
    VR_LOGD("[DEBUG] ..... start audio in");
    return res;
}

bool VR_BaiduDialogEngine::StopAudioIn()
{
    m_spEngineTimer->Stop();

    bool res = m_spAudioIn->IsRuning();
    if (res) {
        m_spAudioIn->Stop();
    }

    res = m_spAudioIn->Close();

    VR_LOGD("[DEBUG] ..... sotp audio in, %d", res);
    return res;
}

int VR_BaiduDialogEngine::GenerateId()
{
    ++m_id;
    VR_LOGD("[DEBUG] ..... generate id : %d", m_id.load());
    return m_id;
}

string VR_BaiduDialogEngine::getImeiId()
{
    string imei;
    ifstream ifs("/var/imei.txt", ifstream::in);

    if (!ifs.is_open()) {
        VR_LOGD("[DEBUG] ..... open failed /var/imei.txt");
        return "";
    }

    getline(ifs, imei);
    VR_LOGD("[DEBUG] ..... get imei: %s", imei.c_str());
    return  imei;
}

extern "C" VR_API VR_DialogEngineIF* VR_CreateOnlineDialogEngine()
{
    VR_DialogEngineIF* pEngine = nullptr;
    VR_ConfigureIF* pConf = VR_ConfigureIF::Instance();
    std::string region = pConf->getVRRegion();

    VR_PRODUCT_TYPE emType = pConf->getVRProduct();
    if (!(("China" == region) && (
        VR_PRODUCT_TYPE_L2 == emType ||
        VR_PRODUCT_TYPE_L1 == emType ||
        VR_PRODUCT_TYPE_L1_5 == emType))) {
        VR_LOGD("[DEBUG] ..... invalid model");
        return nullptr;
    }

    pEngine = VR_new VR_BaiduDialogEngine;
    if (pEngine == nullptr) {
        VR_LOGD("[DEBUG] ..... allocate object failed");
    }

    VR_LOGD("[DEBUG] ..... create online engine success");
    return pEngine;
}

