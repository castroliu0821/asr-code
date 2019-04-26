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

#include "VR_BaiduProtoTools.h"

#include <boost/format.hpp>
#include "VR_Log.h"
#include "VR_Def.h"
#include "VR_BaiduDialogEngine.h"
#include "VR_EngineWorkThread.h"
#include "VrSmartHomeData.pb.h"
#include "VR_EventBase.h"
#include "VR_BaiduDef.h"

using namespace std;
using namespace boost;
using namespace navi::VoiceRecog;

VR_BaiduProtoTools::VR_BaiduProtoTools(VR_BaiduDialogEngine* pEngine, sp_VR_EngineWorkThread &spThread)
    : m_pEngine(pEngine)
    , m_spThread(spThread)
    , m_spData(VR_new RespStreamPack)
{
    VR_LOGD("[DEBUG] ..... Construct");
}

VR_BaiduProtoTools::~VR_BaiduProtoTools()
{
    VR_LOGD("[DEBUG] ..... Deconsturct");
}

void VR_BaiduProtoTools::PrepareParse()
{
    m_spData->clear();
}

bool VR_BaiduProtoTools::HandlePackage(const char* data, const int& len)
{
    const char* buffer = data;
    INT size = len;
    VR_LOGD("[DEBUG] ..... len = %d", len);

    char* buf = const_cast<char*>(buffer);

    while (buf < buffer + size) {
        bool last_pack_not_finished = m_spData->last_pack_not_finished();
        if (last_pack_not_finished) {   //上一个api-resp还不完整
            INT last_pack_need_len = m_spData->len - static_cast<INT>(m_spData->Catche.size());
            if (last_pack_need_len <= size) { //现在可以拼完整并解析了
                m_spData->Catche.append(buf, static_cast<size_t>(last_pack_need_len));

                bool ret = Parse(m_spData->Catche.c_str(), m_spData->len);
                m_spData->clear();
                if (!ret) {
                    return false;
                }

                buf += last_pack_need_len;
            } else {    //仍然不完整, 继续拼接
                m_spData->Catche.append(buf, static_cast<size_t>(size));
                buf += size;
            }
        } else { //新的api-resp, LV结构
            int len = reinterpret_cast<int*>(buf)[0];
            VR_LOGD("[DEBUG] ..... proto len = %d", len);
            if (len <= 0) {
                m_spData->clear();

                stEvent_info info;
                info.sender = "DE";
                info.mesg = "<event><status>Inner</status></event>";
                info.name = EV_NET_ABNORMAL;
                sp_VR_EventBase event(VR_new VR_EventBase(info, m_pEngine));
                m_spThread->PushTask(event);
                VR_LOGD("[DEBUG] ..... error length");
                return false;
            } else {
                buf += sizeof(int);

                m_spData->len = len;
                if (buf + len <= (const_cast<char*>(buffer) + size)) {  //现在就可以解析了
                    m_spData->Catche.append(buf, static_cast<size_t>(len));
                    bool ret = Parse(const_cast<char*>(m_spData->Catche.c_str()), m_spData->len);
                    m_spData->clear();
                    if (!ret) {
                        return false;
                    }

                    buf += len;
                } else {
                    m_spData->Catche.append(buf, static_cast<size_t>(buffer + size - buf));
                    buf += const_cast<char*>(buffer) + size - buf;
                }
            }
        }
    }
    return true;
}

bool VR_BaiduProtoTools::Parse(const char* data, const int& len)
{
    VR_LOGD_FUNC();
    if (data == nullptr || len <= 0) {
        VR_LOGD("param invalid");
        return false;
    }

    stEvent_info info;
    info.sender = "DE";

    APIResponse resp;
    bool res = resp.ParseFromArray(data, len);
    if (!res) {
        info.sender = "DE";
        info.mesg = "<event><status>Inner</status></event>";
        info.name = EV_NET_ABNORMAL;
        sp_VR_EventBase event(VR_new VR_EventBase(info, m_pEngine));
        m_spThread->PushTask(event);
        VR_LOGD("[DEBUG] ..... parse proto error");
        VR_LOGE("[ERROR] ..... parse proto error");
        return false;
    }

    if (resp.err_no() != 0) {
        info.sender = "DE";
        info.mesg = "<event><status>Inner</status></event>";
        info.name = EV_NET_ABNORMAL;
        sp_VR_EventBase event(VR_new VR_EventBase(info, m_pEngine));
        m_spThread->PushTask(event);
        VR_LOGD("[DEBUG] ..... err mesg : %s", info.mesg.c_str());
        VR_LOGE("[ERROR] ..... err mesg : %s", info.mesg.c_str());
        return true;
    }

    switch (resp.type()) {
    case API_RESP_TYPE_HEART:
        VR_LOGD("[DEBUG] ..... heart data");
        break;
    case API_RESP_TYPE_LAST:
        VR_LOGD("[DEBUG] ..... last data");
        break;
    case API_RESP_TYPE_MIDDLE: {
        string word;
        for (auto item : resp.result().word()) {
            word += item;
        }
        VR_LOGD("[DEBUG] ..... middle data: %s", word.c_str());
        break;
    }
    case API_RESP_TYPE_RES: {
        string word;
        for (auto item : resp.result().word()) {
            word += item;
        }

        info.mesg = std::move(word);
        info.name = EV_FINAL_RES;
        sp_VR_EventBase event(VR_new VR_EventBase(info, m_pEngine));
        m_spThread->PushTask(event);
        VR_LOGD("[DEBUG] ..... final res data: %s", info.mesg.c_str());
        break;
    }
    case API_RESP_TYPE_THIRD:
        VR_LOGD("[DEBUG] ..... third data");
        if (resp.third_data().len() != 0) {
            m_thirdData.append(resp.third_data().third_data().data(), resp.third_data().len());
            return true;
        }

        info.mesg = std::move(m_thirdData);
        info.name = EV_THIRD_RES;
        sp_VR_EventBase event(VR_new VR_EventBase(info, m_pEngine));
        m_spThread->PushTask(event);
        VR_LOGD("[DEBUG] ..... third data: %s", info.mesg.c_str());
        VR_LOGD("[Performance] ..... recv third data end");
        break;
    }
    m_thirdData.clear();

    return true;
}
