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
 * @file VR_BaiduAllWordsManager.h
 * @brief Header file for manager all words in smart home specification
 *
 *
 * @attention used for C++ only.
 */

#ifndef VR_BAIDUALLWORDSMANAGER_H
#define VR_BAIDUALLWORDSMANAGER_H

#include <string>
#include <VR_Def.h>

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

class VR_BaiduAllWordsManager {
public:
    VR_BaiduAllWordsManager();
    ~VR_BaiduAllWordsManager();

    void Initialize();
    void Finalize();

    std::string QueryPromptById(std::string id);

private:
    std::string GetResourcePath();

private:
    VoiceMap<std::string, std::string>* m_PmtMapper;
};

#endif // VR_BAIDUALLWORDSMANAGER_H
