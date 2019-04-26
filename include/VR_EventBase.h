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
 * @file VR_EventBase.h
 * @brief Header for work thread handle event
 *
 *
 * @attention used for C++ only.
 */

#ifndef VR_EVENTBASE_H
#define VR_EVENTBASE_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#include <string>
#include <boost/shared_ptr.hpp>

typedef struct __event_struct {
    std::string sender;
    std::string name;
    std::string mesg;
    int         mesg_id;

    __event_struct()
        : mesg_id (0)
    {

    }
}stEvent_info;

class VR_EventBase {
public:
    VR_EventBase(stEvent_info& event, void *ptr = nullptr);
    virtual ~VR_EventBase();

    void HandleEvent();

private:
    stEvent_info m_event;
    void*        m_pHandle;
};

typedef boost::shared_ptr<VR_EventBase> sp_VR_EventBase;

#endif // VR_EVENTBASE_H
