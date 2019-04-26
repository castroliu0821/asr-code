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


#include "BL_Timer.h"
#include <boost/shared_ptr.hpp>

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#ifndef VR_BAIDUENGINETIMER_H
#define VR_BAIDUENGINETIMER_H

class VR_EngineWorkThread;
class VR_BaiduDialogEngine;

typedef boost::shared_ptr<VR_EngineWorkThread>     sp_VR_EngineWorkThread;

class VR_BaiduEngineTimer : public BL_Timer
{
public:
    VR_BaiduEngineTimer(sp_VR_EngineWorkThread spWorkThread, VR_BaiduDialogEngine* spStateMgr);
    virtual ~VR_BaiduEngineTimer();

    virtual VOID OnTimer();

private:
    sp_VR_EngineWorkThread m_spTaskThread;
    VR_BaiduDialogEngine*  m_pEngine;
};

#endif // VR_BAIDUENGINETIMER_H
