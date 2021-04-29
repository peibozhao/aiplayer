
#pragma once

#include "spdlog/spdlog.h"
#include <string>
#include <sstream>
#include <fstream>

#define _InitialWithStringFile \
    bool Init(const std::string &config) { \
        std::stringstream ss(config); \
        if (!Init(ss)) { \
            SPDLOG_ERROR(config); \
            return false; \
        } \
        return true; \
    } \
    bool InitWithFile(const std::string &file) { \
        std::ifstream ifs(file); \
        if (!ifs.is_open()) { \
            SPDLOG_ERROR("{} open failed", file); \
            return false; \
        } \
        return Init(ifs); \
    }

#define InitialBaseDefine \
    virtual bool Init(std::istream &is) = 0; \
    _InitialWithStringFile

#define InitialDefine \
    bool Init(std::istream &is); \
    _InitialWithStringFile
