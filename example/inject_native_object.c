/* File generated automatically by the QuickJS compiler. */

#include "cutils.h"
#include "quickjs-libc.h"
#include "quickjs.h"

static int eval_buf(JSContext *ctx, const void *buf, int buf_len,
                    const char *filename, int eval_flags) {
  JSValue val;
  int ret;

  if ((eval_flags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) {
    /* for the modules, we compile then run to be able to set
       import.meta */
    val = JS_Eval(ctx, buf, buf_len, filename,
                  eval_flags | JS_EVAL_FLAG_COMPILE_ONLY);
    if (!JS_IsException(val)) {
      js_module_set_import_meta(ctx, val, TRUE, TRUE);
      val = JS_EvalFunction(ctx, val);
    }
    val = js_std_await(ctx, val);
  } else {
    val = JS_Eval(ctx, buf, buf_len, filename, eval_flags);
  }
  if (JS_IsException(val)) {
    js_std_dump_error(ctx);
    ret = -1;
  } else {
    ret = 0;
  }
  JS_FreeValue(ctx, val);
  return ret;
}

static int eval_file(JSContext *ctx, const char *filename, int module) {
  uint8_t *buf;
  int ret, eval_flags;
  size_t buf_len;

  buf = js_load_file(ctx, &buf_len, filename);
  if (!buf) {
    perror(filename);
    exit(1);
  }

  if (module < 0) {
    module = (has_suffix(filename, ".mjs") ||
              JS_DetectModule((const char *)buf, buf_len));
  }
  if (module)
    eval_flags = JS_EVAL_TYPE_MODULE;
  else
    eval_flags = JS_EVAL_TYPE_GLOBAL;
  ret = eval_buf(ctx, buf, buf_len, filename, eval_flags);
  js_free(ctx, buf);
  return ret;
}

struct Point {
  int x;
  int y;
};

void Say(struct Point *p) { printf("Native Point:(%d, %d)", p->x, p->y); }

struct Point GlobalPoint = {-1, 1};

static JSValue JSSay(JSContext *ctx, JSValueConst this_val, int argc,
                     JSValueConst *argv) {
  struct Point *obj = JS_GetOpaque(this_val, 1);
  if (!obj) {
    printf("convert failed");
    return JS_ThrowTypeError(ctx, "Invalid Point object");
  }
  Say(obj);
  return JS_UNDEFINED;
}

static JSValue js_point_get_y(JSContext *ctx, JSValueConst this_val) {
  struct Point *p = JS_GetOpaque(this_val, 1);
  if (!p) return JS_ThrowTypeError(ctx, "Invalid Point object");
  JS_EXCEPTION;
  return JS_NewInt32(ctx, p->y);
}

static JSValue js_point_set_y(JSContext *ctx, JSValueConst this_val,
                              JSValueConst value) {
  struct Point *p = JS_GetOpaque(this_val, 1);
  if (!p) return JS_ThrowTypeError(ctx, "Invalid Point object");
  ;

  int y;
  if (JS_ToInt32(ctx, &y, value)) return JS_EXCEPTION;

  p->y = y;
  return JS_UNDEFINED;
}

static JSValue js_point_get_x(JSContext *ctx, JSValueConst this_val) {
  struct Point *p = JS_GetOpaque(this_val, 1);
  if (!p) return JS_ThrowTypeError(ctx, "Invalid Point object");
  ;
  return JS_NewInt32(ctx, p->x);
}

static JSValue js_point_set_x(JSContext *ctx, JSValueConst this_val,
                              JSValueConst value) {
  struct Point *p = JS_GetOpaque(this_val, 1);
  if (!p) return JS_ThrowTypeError(ctx, "Invalid Point object");
  ;

  int x;
  if (JS_ToInt32(ctx, &x, value)) return JS_EXCEPTION;

  p->x = x;
  return JS_UNDEFINED;
}

static const JSCFunctionListEntry js_point_props[] = {
    JS_CGETSET_DEF("x", js_point_get_x, js_point_set_x),
    JS_CGETSET_DEF("y", js_point_get_y, js_point_set_y),
    JS_CFUNC_DEF("Say", 0, JSSay),
};

void registerNativeObject(JSContext *ctx, JSValue *proto) {
  JS_SetPropertyFunctionList(ctx, *proto, js_point_props,
                             countof(js_point_props));
  JSValue jsObj = JS_NewObjectProto(ctx, *proto);
  JS_SetOpaque(jsObj, &GlobalPoint);
  JSValue global = JS_GetGlobalObject(ctx);
  JS_SetPropertyStr(ctx, global, "NPoint", jsObj);
  // JS_FreeValue(ctx, jsObj);
  // JS_FreeValue(ctx, proto);
  JS_FreeValue(ctx, global);
}

int main(int argc, char **argv) {
  JSRuntime *rt;
  JSContext *ctx;
  rt = JS_NewRuntime();
  js_std_init_handlers(rt);
  ctx = JS_NewContext(rt);
  JSValue proto = JS_NewObject(ctx);
  js_std_add_helpers(ctx, argc, argv);
  registerNativeObject(ctx, &proto);
  eval_file(ctx, "./inject_native_object.js", 0);
  JS_FreeValue(ctx, proto);
  js_std_loop(ctx);
  js_std_free_handlers(rt);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}
