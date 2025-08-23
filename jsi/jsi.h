#include <functional> 
#include <map>      
#include <string>  

#include "quickjs.h"

/**
 * @brief Base class for JavaScript interface objects
 * 
 * Provides virtual methods for handling property get/set operations
 * and function calls when interacting with JavaScript through QuickJS
 */
class JSIObject {
 public:
  /**
   * @brief Handle property retrieval from JavaScript
   * @param ctx QuickJS context
   * @param obj The object being accessed
   * @param prop The property atom being retrieved
   * @param receiver The receiver object
   * @return JSValue representing the property value (default: undefined)
   */
  virtual JSValue get(JSContext *ctx, JSValueConst obj, JSAtom prop,
                      JSValueConst receiver) {
    return JS_UNDEFINED;
  }

  /**
   * @brief Handle property assignment from JavaScript
   * @param ctx QuickJS context
   * @param obj The object being modified
   * @param prop The property atom being set
   * @param value The new value for the property
   * @param receiver The receiver object
   * @param flags Additional flags for the operation
   * @return 0 on success, non-zero on error (default: 0)
   */
  virtual int set(JSContext *ctx, JSValueConst obj, JSAtom prop,
                  JSValueConst value, JSValueConst receiver, int flags) {
    return 0;
  }

  /**
   * @brief Handle function calls from JavaScript
   * @param ctx QuickJS context
   * @param func_obj The function object being called
   * @param this_val The 'this' value for the call
   * @param argc Number of arguments
   * @param argv Array of arguments
   * @param flags Additional flags for the call
   * @return JSValue representing the return value (default: undefined)
   */
  virtual JSValue call(JSContext *ctx, JSValueConst func_obj,
                       JSValueConst this_val, int argc, JSValueConst *argv,
                       int flags) {
    return JS_UNDEFINED;
  }

  /**
   * @brief Virtual destructor for proper cleanup of derived classes
   */
  virtual ~JSIObject() {}
};

/**
 * @brief Function type for JavaScript callback handlers
 * 
 * Matches the signature required for QuickJS function callbacks
 */
using JSCallback = std::function<JSValue(JSContext *ctx, JSValueConst func_obj,
                                         JSValueConst this_val, int argc,
                                         JSValueConst *argv, int flags)>;

/**
 * @brief Specialized JSIObject for function callbacks
 * 
 * Wraps a std::function to handle JavaScript function invocations
 */
class JSIFunction : public JSIObject {
 public:
  /**
   * @brief Construct a new JSIFunction object
   * @param callback The function to be called when invoked from JavaScript
   */
  explicit JSIFunction(JSCallback callback)
      : callback_(new JSCallback(std::move(callback))) {}

  /**
   * @brief Override call method to invoke the stored callback
   * @param ctx QuickJS context
   * @param func_obj The function object being called
   * @param this_val The 'this' value for the call
   * @param argc Number of arguments
   * @param argv Array of arguments
   * @param flags Additional flags for the call
   * @return JSValue representing the return value
   */
  JSValue call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_val,
               int argc, JSValueConst *argv, int flags);

 private:
  JSCallback *callback_;  // Stored callback function
};

/**
 * @brief JavaScript interface application class
 * 
 * Manages the interaction between C++ objects and the QuickJS engine
 */
class JSIApp {
 public:
  /**
   * @brief Construct a new JSIApp object
   * @param ctx QuickJS context to associate with this instance
   */
  explicit JSIApp(JSContext *ctx);

  /**
   * @brief Create a JavaScript value wrapping a JSIObject
   * @param object The C++ object to wrap
   * @return JSValue representing the wrapped object
   */
  JSValue create(JSIObject *object);

  /**
   * @brief Expose a JSIObject as a global variable in JavaScript
   * @param name The global variable name
   * @param object The C++ object to expose
   */
  void setGlobal(const std::string &name, JSIObject *object);

 private:
  static JSClassID class_id_;  // QuickJS class identifier for JSI objects
  static std::map<JSRuntime *, bool> initialized_runtimes_;  // Track initialized runtimes

  /**
   * @brief Finalizer callback for cleaning up JSI objects
   * @param rt QuickJS runtime
   * @param val The JSValue being finalized
   */
  static void finalizer(JSRuntime *rt, JSValue val);

  /**
   * @brief Adapter for handling function calls from JavaScript
   * @param ctx QuickJS context
   * @param func_obj The function object being called
   * @param this_val The 'this' value for the call
   * @param argc Number of arguments
   * @param argv Array of arguments
   * @param flags Additional flags for the call
   * @return JSValue representing the return value
   */
  static JSValue callAdapter(JSContext *ctx, JSValueConst func_obj,
                             JSValueConst this_val, int argc,
                             JSValueConst *argv, int flags);

  /**
   * @brief Adapter for handling property get operations from JavaScript
   * @param ctx QuickJS context
   * @param obj The object being accessed
   * @param prop The property atom being retrieved
   * @param receiver The receiver object
   * @return JSValue representing the property value
   */
  static JSValue getAdapter(JSContext *ctx, JSValueConst obj, JSAtom prop,
                            JSValueConst receiver);

  /**
   * @brief Adapter for handling property set operations from JavaScript
   * @param ctx QuickJS context
   * @param obj The object being modified
   * @param prop The property atom being set
   * @param value The new value for the property
   * @param receiver The receiver object
   * @param flags Additional flags for the operation
   * @return 0 on success, non-zero on error
   */
  static int setAdapter(JSContext *ctx, JSValueConst obj, JSAtom prop,
                        JSValueConst value, JSValueConst receiver, int flags);

  JSContext *ctx_;  // Associated QuickJS context

  /**
   * @brief Ensure the QuickJS runtime is properly initialized
   * @param context QuickJS context to initialize
   */
  static void ensureInitialized(JSContext *context);
};
