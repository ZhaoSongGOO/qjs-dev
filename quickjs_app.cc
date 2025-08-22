#include <functional>
#include <string>
#include <map>
#include "jsi.h"

JSClassID JSIApp::class_id_ = 0;
std::map<JSRuntime *, bool> JSIApp::initialized_runtimes_;

JSIApp::JSIApp(JSContext *ctx) : ctx_(ctx) {
    ensureInitialized(ctx);
}

JSValue JSIApp::create(JSIObject *object) {
    JSValue obj = JS_NewObjectClass(ctx_, class_id_);
    if (JS_IsException(obj)) {
        return obj;
    }
    JS_SetOpaque(obj, object);
    return obj;
}

void JSIApp::setGlobal(const std::string &name, JSIObject *object) {
    JSValue global_obj = JS_GetGlobalObject(ctx_);
    JSValue obj = create(object);
    JS_SetPropertyStr(ctx_, global_obj, name.c_str(), obj);
    JS_FreeValue(ctx_, global_obj);
}

void JSIApp::finalizer(JSRuntime *rt, JSValue val) {
    JSIObject *object = (JSIObject *) JS_GetOpaque(val, class_id_);
    if (object) {
        delete object;
    }
}

JSValue JSIApp::callAdapter(JSContext *ctx, JSValueConst func_obj,
                            JSValueConst this_val, int argc, JSValueConst *argv,
                            int flags) {
    JSIObject *object = (JSIObject *) JS_GetOpaque(func_obj, class_id_);
    if (object) {
        return object->call(ctx, func_obj, this_val, argc, argv, flags);
    }
    return JS_EXCEPTION;
}

JSValue JSIApp::getAdapter(JSContext *ctx, JSValue obj, JSAtom prop, JSValue receiver) {
    JSIObject *object = (JSIObject *) JS_GetOpaque(obj, class_id_);
    if (object) {
        return object->get(ctx, obj, prop, receiver);
    }
    return JS_EXCEPTION;
}

int JSIApp::setAdapter(JSContext *ctx, JSValue obj, JSAtom prop, JSValue value, JSValue receiver, int flags) {
    JSIObject *object = (JSIObject *) JS_GetOpaque(obj, class_id_);
    if (object) {
        return object->set(ctx, obj, prop, value, receiver, flags);
    }
    return 0;
}

void JSIApp::ensureInitialized(JSContext *context) {
    JSRuntime *rt = JS_GetRuntime(context);
    static JSClassExoticMethods exotic = {
            .get_property = &JSIApp::getAdapter,
            .set_property = &JSIApp::setAdapter,
    };
    static JSClassDef class_def = {
            .class_name = "_native",
            .finalizer = &JSIApp::finalizer,
            .call = &JSIApp::callAdapter,
            .exotic = &exotic
    };
    if (initialized_runtimes_.find(rt) == initialized_runtimes_.end()) {
        JS_NewClassID(&class_id_);
        JS_NewClass(rt, class_id_, &class_def);
        initialized_runtimes_[rt] = true;
    }
}

JSValue JSIFunction::call(JSContext *ctx, JSValueConst func_obj,
                          JSValueConst this_val, int argc, JSValueConst *argv,
                          int flags) {
    if (callback_ == nullptr) {
        return JS_UNDEFINED;
    }
    return (*callback_)(ctx, func_obj, this_val, argc, argv, flags);
}
