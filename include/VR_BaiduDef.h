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
 * @file VR_BaiduEngineTimer.h
 * @brief Header for baidu engine timer
 *
 *
 * @attention used for C++ only.
 */


#ifndef VR_BAIDUDEF_H
#define VR_BAIDUDEF_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#define EV_CANCEL_ENGINE    "CANCEL_ENGINE"
#define EV_TTS_RESULT       "TTS_RESULT"
#define EV_STOPTTS_RESULT   "STOPTTS_RESULT"
#define EV_BEEP_RESULT      "BEEP_RESULT"
#define EV_TIME_OUT         "TIMEOUT"
#define EV_START_ENGINE     "START_ENGINE"
#define EV_FINAL_RES        "FINAL_RESULT"
#define EV_NET_ABNORMAL     "ABNORMAL"
#define EV_THIRD_RES        "THIRD_RESULT"
#define END_BEEP_SUFFIX     "beep/endVR.wav"
#define START_BEEP_SUFFIX   "beep/startVR.wav"
#define RET_BEEP_SUFFIX     "beep/returnVR.wav"

/**
  * @section state node:
  * 1: idle
  * 2: listening
  * 3: processing
  * 4: promptPlaying
  *
  **/
#define XML_ENGINE_STATE    \
    "<display agent='Common' content='SMHState'> "\
      "<state>%1%</state>" \
    "</display>"

#define XML_NOTIFY_HINT \
    "<display agent='Common' content='SMHHintsDisplay'>" \
        "<count>%1%</count>" \
        "<items>" \
            "<item> " \
                "<hint>%1%</hint>" \
            "</item>" \
        "</items>" \
    "</display>"

/**
  * @section state node:
  * 1: VR-SMH-001
  * 2: ONS-SMH-002
  * 3: ONS-SMH-003
  * 4: ONS-SMH-004
  **/
#define XML_ERROR_ONS \
    "<display agent='Common' content='ShowPopupMessage'>" \
        "<messageId>%1%</messageId>" \
        "<type>smarthome</type>" \
        "<prompt></prompt>" \
    "</display>"

#define XML_QUIT_APP \
    "<display agent='Common' content='QuitVRApp'></display>"

#define XML_START_ENGINE_RESP \
    "<event-result name='StartBaiduEngine errcode=%1%'/>"

#define XML_PTT_LONG_PRESS_RESP \
    "<event-result name='buttonPressed'><keycode value='ptt_hard_key_long_press'/></event-result>"

#define XML_METER_BACK_RESP \
    "<event-result name='buttonPressed'><keycode value='meter_hard_key_back_normal_press'/></event-result>"

#define XML_CHANGE_LANG_RESP \
    "<event-result name='changeLanguage'><language>%1%</language></event-result>"

#define XML_CANCEL_ENGINE_RESP \
    "<event-result name='cancel' option='smarthome'/>"

#define XML_CANCEL_TTSCRASH_RESP \
    "<event-result name='cancel' option='ttscrash'/>"

#define XML_UPDATE_STATE_RESP \
    "<event-result name='updateState'/>"

#define XML_ACTION_PLAYBEEP \
    "<action agent=\"prompt\" op=\"playBeep\">"\
        "<beepFile>%1%</beepFile>"\
        "<bargein>%2%</bargein>"\
    "</action>"

#define XML_ACTION_PLAYTTS \
    "<action agent=\"prompt\" op=\"playTts\">"\
        "<language>%1%</language>"\
        "<bargein>%2%</bargein>"\
        "<text>%3%</text>"\
    "</action>"

#define XML_ACTION_STOPTTS \
    "<action agent='prompt' op='stopTts'><reqId>%1%</reqId></action>"

#define XML_RELEASE_NETWORK \
    "<action agent='smartHome' op='ReqHttpDisConnect'></action>"

#endif // VR_BAIDUDEF_H
