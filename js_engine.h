

#include "quickjs-libc.h"
#include "cutils.h"
#include "quickjs.h"
#include <memory>

#ifndef __JS_ENGINE_H__
#define __JS_ENGINE_H__

class JSEngine{
public:
    JSEngine() = default;
    void InitRuntime();
    void RunScript(const std::string & file);
private:
    std::shared_ptr<JSRuntime> rt_ = nullptr;
};


#endif
