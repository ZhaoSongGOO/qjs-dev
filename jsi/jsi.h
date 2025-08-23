#include <functional>
#include <map>
#include <string>

#include "quickjs.h"

class JSIObject {
 public:
  virtual JSValue get(JSContext *ctx, JSValueConst obj, JSAtom prop,
                      JSValueConst receiver) {
    return JS_UNDEFINED;
  }

  virtual int set(JSContext *ctx, JSValueConst obj, JSAtom prop,
                  JSValueConst value, JSValueConst receiver, int flags) {
    return 0;
  }

  virtual JSValue call(JSContext *ctx, JSValueConst func_obj,
                       JSValueConst this_val, int argc, JSValueConst *argv,
                       int flags) {
    return JS_UNDEFINED;
  }

  virtual ~JSIObject() {}
};

using JSCallback = std::function<JSValue(JSContext *ctx, JSValueConst func_obj,
                                         JSValueConst this_val, int argc,
                                         JSValueConst *argv, int flags)>;

class JSIFunction : public JSIObject {
 public:
  explicit JSIFunction(JSCallback callback)
      : callback_(new JSCallback(std::move(callback))) {}

  JSValue call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_val,
               int argc, JSValueConst *argv, int flags);

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
                             JSValueConst this_val, int argc,
                             JSValueConst *argv, int flags);

  static JSValue getAdapter(JSContext *ctx, JSValueConst obj, JSAtom prop,
                            JSValueConst receiver);

  static int setAdapter(JSContext *ctx, JSValueConst obj, JSAtom prop,
                        JSValueConst value, JSValueConst receiver, int flags);

  JSContext *ctx_;

  static void ensureInitialized(JSContext *context);
};
