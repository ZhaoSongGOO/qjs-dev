#include <functional>
#include <string>
#include <map>
#include "jsi.h"

JSClassID JSIFunction::class_id_ = 0;
std::map<JSRuntime*, bool> JSIFunction::initialized_runtimes_;

    JSIFunction::JSIFunction(JSContext * ctx):ctx_(ctx){
        ensureInitialized(ctx);
    }

    JSValue JSIFunction::create(JSCallback cpp_func) {
        JSValue func_obj = JS_NewObjectClass(ctx_, class_id_);
        if (JS_IsException(func_obj)) {
            return func_obj;
        }
        JSCallback* callback_ptr = new JSCallback(std::move(cpp_func));
        JS_SetOpaque(func_obj, callback_ptr);
        return func_obj;
    }
    void JSIFunction::setGlobal(const std::string& name, JSCallback cpp_func) {
        JSValue global_obj = JS_GetGlobalObject(ctx_);
        JSValue func_val = create(std::move(cpp_func));
        JS_SetPropertyStr(ctx_, global_obj, name.c_str(), func_val);
        JS_FreeValue(ctx_, global_obj);
    }

    void JSIFunction::finalizer(JSRuntime *rt, JSValue val) {
        JSCallback* callback = (JSCallback*)JS_GetOpaque(val, class_id_);
        if (callback) {
            delete callback;
        }
    }
    JSValue JSIFunction::callAdapter(JSContext *ctx, JSValueConst func_obj,
                               JSValueConst this_val, int argc, JSValueConst *argv,
                               int flags) {
        JSCallback* callback = (JSCallback*)JS_GetOpaque(func_obj, class_id_);
        if (callback && (*callback)) {
            return (*callback)(ctx, func_obj, this_val, argc, argv, flags);
        }
        return JS_EXCEPTION;
    }

    void JSIFunction::ensureInitialized(JSContext * context) {
        JSRuntime * rt = JS_GetRuntime(context);
        if (initialized_runtimes_.find(rt) == initialized_runtimes_.end()) {
            JS_NewClassID(&class_id_);
            JSClassDef class_def = {};
            class_def.class_name = "CppFunction";
            class_def.finalizer = &JSIFunction::finalizer;
            class_def.call = &JSIFunction::callAdapter;
            JS_NewClass(rt, class_id_, &class_def);
            initialized_runtimes_[rt] = true;
        }
    }
