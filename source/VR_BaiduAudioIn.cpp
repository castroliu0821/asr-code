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

#include "VC_CommonIF.h"
#include "VR_BaiduAudioIn.h"

#include "VR_Log.h"

using namespace std;

VR_BaiduAudioIn::VR_BaiduAudioIn(bool debug)
    : m_audio_in(nullptr)
    , m_bUsing(false)
{
    m_audio_in = VC_CommonIF::Instance()->CreateAudioIn(debug);
    if (m_audio_in == nullptr) {
        VR_LOGD("create audio in failed");
    }
}

VR_BaiduAudioIn::~VR_BaiduAudioIn()
{
    delete m_audio_in;
    m_audio_in = nullptr;
}

bool VR_BaiduAudioIn::Open(nutshell::INT* sampleRate, nutshell::INT* fragSize, nutshell::INT* fragCount, nutshell::INT mode)
{
    VR_LOGD_FUNC();
    if (m_audio_in == nullptr) {
        return false;
    }

    string name = "/var/audio_data/";
    static int i = 1;

#ifdef OPUS_AUDIO_TYPE
    name = name + "audio_" + to_string(i++) + ".opus";
#else
    name = name + "audio_" + to_string(i++) + ".pcm";
#endif
    m_fstream.open(name.c_str(), ifstream::out);

    return CL_TRUE == m_audio_in->Open(sampleRate, fragSize, fragCount, mode) ? true : false;
}

bool VR_BaiduAudioIn::Start(VC_AudioIn::Listener* pListener)
{
    VR_LOGD_FUNC();
    if (m_audio_in == nullptr) {
        return false;
    }

    CL_BOOL res = m_audio_in->Start(pListener);
    m_bUsing = res;
    return res == CL_TRUE ? true : false;
}

bool VR_BaiduAudioIn::Stop(bool async)
{
    VR_LOGD_FUNC();
    if (m_audio_in == nullptr) {
        return false;
    }

    BOOL res = m_audio_in->Stop(async);

    m_bUsing = res ? true : false;
    return CL_TRUE == res ? true : false;
}

bool VR_BaiduAudioIn::Close()
{
     VR_LOGD_FUNC();
     if (m_audio_in == nullptr) {
         return false;
     }

     if (m_fstream.is_open()) {
         m_fstream.close();
     }

     return CL_TRUE == m_audio_in->Close() ? true : false;
 }

bool VR_BaiduAudioIn::IsRuning()
{
    VR_LOGD_FUNC();
    return m_bUsing;
}

 bool VR_BaiduAudioIn::write_to_file(const char *data, size_t len)
 {
     VR_LOGD_FUNC();
     if (m_fstream.is_open()) {
         m_fstream.write(data, static_cast<int>(len));
     }

     return true;
 }
