#include <functional>
#include "quickjs.h"
#include <string>
#include <iostream>
#include <map>

using JSCallback = std::function<JSValue(JSContext *ctx, JSValueConst func_obj,
                                         JSValueConst this_val, int argc, JSValueConst *argv,
                                         int flags)>;


class JSIFunction {
public:
    explicit JSIFunction(JSContext * ctx);

    JSValue create(JSCallback cpp_func);
    void setGlobal(const std::string& name, JSCallback cpp_func);
private:
    JSContext* ctx_;
    static JSClassID class_id_;
    static std::map<JSRuntime*, bool> initialized_runtimes_;

    static void finalizer(JSRuntime *rt, JSValue val);
    static JSValue callAdapter(JSContext *ctx, JSValueConst func_obj,
                               JSValueConst this_val, int argc, JSValueConst *argv,
                               int flags);

    static void ensureInitialized(JSContext * context);
};




int main(){
    JSRuntime* rt = JS_NewRuntime();
    JSContext* ctx = JS_NewContext(rt);
    JSIFunction wrapper(ctx);
    wrapper.setGlobal("print", [](JSContext *ctx, JSValueConst func_obj,
                                  JSValueConst this_val, int argc, JSValueConst *argv,
                                  int flags) -> JSValue {
        if (argc < 1) return JS_UNDEFINED;
        const char *str = JS_ToCString(ctx, argv[0]);
        if (!str) return JS_EXCEPTION;
        std::cout << "C++ says: " << str << std::endl;
        JS_FreeCString(ctx, str);
        return JS_UNDEFINED;
    });

    const char* jsCode = "print(\"hello world\")";
    JSValue result = JS_Eval(ctx, jsCode, strlen(jsCode), "<eval>", JS_EVAL_TYPE_GLOBAL);
}

