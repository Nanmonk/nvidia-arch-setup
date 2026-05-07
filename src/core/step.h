#pragma once
#include "system_info.h"

#include <string>

class Step {
  public:
    virtual ~Step() = default;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual bool execute(const SystemInfo& info) = 0;
    virtual std::string preview(const SystemInfo& info) const {
        (void)info;
        return description();
    }
    virtual bool applicable(const SystemInfo& info) const {
        (void)info;
        return true;
    }
};
