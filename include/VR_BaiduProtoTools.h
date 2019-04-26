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
 * @file VR_BaiduProtoTools.h
 * @brief Header for tool class tcp / ip package to proto
 *
 *
 * @attention used for C++ only.
 */

#ifndef VR_BAIDUPROTOTOOLS_H
#define VR_BAIDUPROTOTOOLS_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif


#include <boost/shared_ptr.hpp>

class RespStreamPack;
class VR_EngineWorkThread;
class VR_BaiduDialogEngine;

typedef  boost::shared_ptr<RespStreamPack>        sp_RespStreamPack;
typedef boost::shared_ptr<VR_EngineWorkThread>     sp_VR_EngineWorkThread;

class VR_BaiduProtoTools {
public:
    VR_BaiduProtoTools(VR_BaiduDialogEngine* pEngine, sp_VR_EngineWorkThread& spThread);
    ~VR_BaiduProtoTools();

    void PrepareParse();
    bool HandlePackage(const char* data, const int& len);

private:
    bool Parse(const char* data, const int& len);

private:
    VR_BaiduDialogEngine*  m_pEngine;
    sp_VR_EngineWorkThread m_spThread;
    sp_RespStreamPack      m_spData;
    std::string            m_thirdData;
};

class RespStreamPack {
public:
    RespStreamPack() : len(0) {
        clear();
    }

    void clear() {
        len = 0;
        Catche.clear();
    }

    bool last_pack_not_finished() {
        return (Catche.size() > 0 && Catche.size() < static_cast<uint32_t>(len));
    }

public:
    int len;
    std::string Catche;
};

#endif // VR_BAIDUPROTOTOOLS_H
