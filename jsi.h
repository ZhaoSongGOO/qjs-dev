#include <functional>
#include "quickjs.h"
#include <string>
#include <iostream>
#include <map>
#include "quickjs-libc.h"


class JSIObject {
public:
    virtual JSValue get(JSContext *ctx, JSValueConst obj, JSAtom prop, JSValueConst receiver) { return JS_UNDEFINED; }

    virtual int
    set(JSContext *ctx, JSValueConst obj, JSAtom prop, JSValueConst value, JSValueConst receiver,
        int flags) { return 0; }

    virtual JSValue
    call(JSContext *ctx, JSValueConst func_obj,
         JSValueConst this_val, int argc, JSValueConst *argv,
         int flags) { return JS_UNDEFINED; }

    virtual ~JSIObject() {}
};


using JSCallback = std::function<JSValue(JSContext *ctx, JSValueConst func_obj,
                                         JSValueConst this_val, int argc, JSValueConst *argv,
                                         int flags)>;


class JSIFunction : public JSIObject {
public:
    explicit JSIFunction(JSCallback callback) : callback_(
            new JSCallback(std::move(callback))) {}

    JSValue call(JSContext *ctx, JSValueConst func_obj,
                 JSValueConst this_val, int argc, JSValueConst *argv,
                 int flags);

private:
    JSCallback *callback_;
};

class JSIApp {
public:
    explicit JSIApp(JSContext *ctx);

    JSValue create(JSIObject *object);

    void setGlobal(const std::string &name, JSIObject *object);

private:
    static JSClassID class_id_;
    static std::map<JSRuntime *, bool> initialized_runtimes_;

    static void finalizer(JSRuntime *rt, JSValue val);

    static JSValue callAdapter(JSContext *ctx, JSValueConst func_obj,
                               JSValueConst this_val, int argc, JSValueConst *argv,
                               int flags);

    static JSValue getAdapter(JSContext *ctx, JSValueConst obj, JSAtom prop, JSValueConst receiver);

    static int
    setAdapter(JSContext *ctx, JSValueConst obj, JSAtom prop, JSValueConst value, JSValueConst receiver, int flags);

    JSContext *ctx_;

    static void ensureInitialized(JSContext *context);

};

class MyJNIObject : public JSIObject {
public:
    JSValue get(JSContext *ctx, JSValue obj, JSAtom prop, JSValue receiver) override {
        std::cout << " call get here\n";
        return JS_NewInt32(ctx, value);
    }

    int set(JSContext *ctx, JSValue obj, JSAtom prop, JSValue value, JSValue receiver, int flags) override {
        std::cout << " call set here\n";
        if (JS_ToInt32(ctx, &this->value, value) != 0) {
            return 0;
        }
        return 0;
    }

private:
    int value = 10;

};


int main() {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);
    js_std_add_helpers(ctx, 0, nullptr);
    JSIFunction *func = new JSIFunction([](JSContext *ctx, JSValueConst func_obj,
                                           JSValueConst this_val, int argc, JSValueConst *argv,
                                           int flags) -> JSValue {
        if (argc < 1) return JS_UNDEFINED;
        const char *str = JS_ToCString(ctx, argv[0]);
        if (!str) return JS_EXCEPTION;
        std::cout << "C++ says: " << str << std::endl;
        JS_FreeCString(ctx, str);
        return JS_UNDEFINED;
    });

    JSIApp *app = new JSIApp(ctx);
    app->setGlobal("print", func);
    app->setGlobal("nn", new MyJNIObject());

    const char *jsCode = "console.log(nn.value);nn.value=88;console.log(nn.value)";
    JS_Eval(ctx, jsCode, strlen(jsCode), "<eval>", JS_EVAL_TYPE_GLOBAL);
}

