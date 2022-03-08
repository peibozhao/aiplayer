
#pragma once

#include <string>

class IApplication {
public:
  virtual bool Init() { return true; }

  virtual void Start() = 0;

  virtual void Pause() = 0;

  virtual void Continue() = 0;

  virtual void Stop() = 0;

  virtual bool SetPlayer(const std::string &player) = 0;

  virtual std::string GetPlayer() = 0;

  virtual bool SetMode(const std::string &mode) = 0;

  virtual std::string GetMode() = 0;
};
