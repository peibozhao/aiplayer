
#pragma once

#define InitialBaseDefine \
    virtual bool Init(std::istream &is) = 0; \
    bool Init(const std::string &config) { \
        std::stringstream ss(config); \
        return Init(ss); \
    } \
    bool InitWithFile(const std::string &file) { \
        std::ifstream ifs(file); \
        if (!ifs.is_open()) { \
            return false; \
        } \
        return Init(ifs); \
    }

#define InitialDefine \
    bool Init(std::istream &is); \
    bool Init(const std::string &config) { \
        std::stringstream ss(config); \
        return Init(ss); \
    } \
    bool InitWithFile(const std::string &file) { \
        std::ifstream ifs(file); \
        if (!ifs.is_open()) { \
            return false; \
        } \
        return Init(ifs); \
    }

