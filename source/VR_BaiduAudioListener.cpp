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

#include "VR_BaiduAudioListener.h"

#include "VR_Log.h"
#include "VR_Def.h"
#include "ncore/NCTypesDefine.h"
#include "VR_BaiduDialogEngine.h"

using namespace nutshell;

static const int FRAME_LENGTH = 5120;
static const int RING_LEN = FRAME_LENGTH * 2;

VR_BaiduAudioListener::VR_BaiduAudioListener(VR_BaiduDialogEngine *spEngine)
    : m_pDialogEngine(spEngine)
{
    VR_LOGD_FUNC();
    m_ringbuff = VR_new char[RING_LEN];
    memset(m_ringbuff, 0, RING_LEN);
    m_buff = VR_new char[FRAME_LENGTH];
    memset(m_buff, 0, FRAME_LENGTH);
    m_head = 0;
    m_tail = 0;
}

VR_BaiduAudioListener::~VR_BaiduAudioListener()
{
    VR_LOGD_FUNC();
    delete[] m_buff;
    delete[] m_ringbuff;
}

/**
 * @brief VR_BaiduAudioListener::OnAudioInData
 * @param data
 * @param len
 *
 * @details After start audioin device, This function will be
 *
 * call when
 */
VOID VR_BaiduAudioListener::OnAudioInData(VOID* data, INT len)
{
    VR_LOGD("[DEBUG] ..... send audio data");
    uint32_t sum = 0;
    if (m_tail == m_head && m_head == 0) { // ring buffer empty
        memcpy(m_ringbuff, data, static_cast<uint32_t>(len));
        m_tail = static_cast<uint32_t>(len);
    }
    else {
        uint32_t l = RING_LEN - m_tail;
        if (static_cast<uint32_t>(len) < l) {                   // not full data
            memcpy(m_ringbuff + m_tail, data, static_cast<uint32_t>(len));
            m_tail = m_tail + static_cast<uint32_t>(len);
        }
        else {                           // full data
            memcpy(m_ringbuff + m_tail, data, l);
            memcpy(m_ringbuff, reinterpret_cast<char *>(data) + l, static_cast<uint32_t>(len) - l);
            m_tail = static_cast<uint32_t>(len) - l;
            m_head = m_head < m_tail ? m_tail + 1 : m_head;
        }
    }

    if (m_tail > m_head) {
        sum = m_tail - m_head;
    }
    else {
        sum = m_tail + RING_LEN - m_head;
    }

    if (sum >= FRAME_LENGTH) {
        if (m_head < m_tail) {
            memcpy(m_buff, m_ringbuff + m_head, FRAME_LENGTH);
            m_head += FRAME_LENGTH;
        }
        else {
            uint32_t l = RING_LEN - m_head;
            if (l >= FRAME_LENGTH) {
                memcpy(m_buff, m_ringbuff + m_head, FRAME_LENGTH);
                m_head = m_head + FRAME_LENGTH;
            }
            else {
                memcpy(m_buff, m_ringbuff + m_head, l);
                memcpy(m_buff + l, m_ringbuff, FRAME_LENGTH - l);
                m_head = FRAME_LENGTH - l;
            }
        }

        if (m_pDialogEngine) {
            m_pDialogEngine->SendAudioData(m_buff, FRAME_LENGTH);
        }
    }
}

VOID VR_BaiduAudioListener::OnAudioInCustom(int type, VOID* data, nutshell::INT len)
{
    VR_LOGD_FUNC();
    VR_UNUSED_VAR(type);
    VR_UNUSED_VAR(data);
    VR_UNUSED_VAR(len);
}

VOID VR_BaiduAudioListener::OnAudioInStarted()
{
    VR_LOGD_FUNC();
}

VOID VR_BaiduAudioListener::OnAudioInStopped()
{
    VR_LOGD_FUNC();
}
