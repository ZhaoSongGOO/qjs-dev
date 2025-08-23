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

int main(int argc, char **argv) {
  JSRuntime *rt;
  JSContext *ctx;
  rt = JS_NewRuntime();
  js_std_init_handlers(rt);
  ctx = JS_NewContext(rt);
  js_std_add_helpers(ctx, argc, argv);
  eval_file(ctx, "./demo.js", 0);

  JSAtom atom = JS_NewAtom(ctx, "CustomFunction");
  JSValue global_obj = JS_GetGlobalObject(ctx);

  JSValue v = JS_GetProperty(ctx, global_obj, atom);

  if (JS_IsFunction(ctx, v)) {
    JSValue arg[1] = {JS_NewString(ctx, "Hello World!")};
    JSValue ret = JS_Call(ctx, v, JS_UNDEFINED, 1, arg);
    if (JS_IsException(ret)) {
      // 调用失败，打印错误信息
      js_std_dump_error(ctx);
    } else {
      // 调用成功，处理返回值
      const char *str = JS_ToCString(ctx, ret);
      printf("Function returned: %s\n", str);
      JS_FreeCString(ctx, str);
    }
    JS_FreeValue(ctx, arg[0]);
    JS_FreeValue(ctx, ret);
  }

  JS_FreeValue(ctx, v);
  JS_FreeValue(ctx, global_obj);
  JS_FreeAtom(ctx, atom);
  js_std_loop(ctx);
  js_std_free_handlers(rt);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}
