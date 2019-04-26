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
 * @file VR_BaiduAudioIn.h
 * @brief Header file for audio in control
 *
 *
 * @attention used for C++ only.
 */

#ifndef VR_BAIDUAUDIOIN_H
#define VR_BAIDUAUDIOIN_H

#include <fstream>
#include "VC_AudioIn.h"
#include "ncore/NCTypesDefine.h"

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

class VR_BaiduAudioIn {
   public:
       VR_BaiduAudioIn(bool debug = false);
       virtual ~VR_BaiduAudioIn();

       bool Open(nutshell::INT* sampleRate, nutshell::INT* fragSize, nutshell::INT* fragCount, nutshell::INT mode);
       bool Start(VC_AudioIn::Listener* pListener);
       bool Stop(bool async = false);
       bool Close();
       bool IsRuning();

       bool write_to_file(const char* data, size_t len);
   private:
       std::ofstream m_fstream;
       VC_AudioIn*   m_audio_in;
       bool          m_bUsing;
   };

#endif // VR_BAIDUAUDIOIN_H
