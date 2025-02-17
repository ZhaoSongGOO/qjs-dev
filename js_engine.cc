#include "js_engine.h"
#include "quickjs.h"
#include <memory>


void JSEngine::InitRuntime(){
    rt_ =  std::make_shared<JSRuntime>(JS_NewRuntime());
}


void JSEngine::RunScript(const std::string &file) {
    
}




