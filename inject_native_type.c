/* File generated automatically by the QuickJS compiler. */

#include "quickjs-libc.h"
#include "cutils.h"
#include "quickjs.h"
#include <stdio.h>


static int eval_buf(JSContext *ctx, const void *buf, int buf_len,
                    const char *filename, int eval_flags)
{
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


static int eval_file(JSContext *ctx, const char *filename, int module)
{
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

struct Point* point_create(int x, int y) {
    struct Point* p = (struct Point*)malloc(sizeof(struct Point));
    p->x = x;
    p->y = y;
    return p;
}

int point_get_x(struct Point* p) {
    return p->x;
}

int point_get_y(struct Point* p) {
    return p->y;
}

void point_set_x(struct Point* p, int x) {
    p->x = x;
}

void point_set_y(struct Point* p, int y) {
    p->y = y;
}

void point_free(struct Point* p) {
    free(p);
}

static JSValue js_point_create(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    int x, y;
    if (JS_ToInt32(ctx, &x, argv[0]))
        return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &y, argv[1]))
        return JS_EXCEPTION;

    struct Point* p = point_create(x, y);
    JSValue proto = JS_GetPropertyStr(ctx, this_val, "prototype");
    JSValue jsObj = JS_NewObjectProto(ctx, proto);
    JS_SetOpaque(jsObj, p);
    JS_FreeValue(ctx, proto);
    return jsObj;
}



static JSValue js_point_get_x(JSContext* ctx, JSValueConst this_val) {
    struct Point* p = JS_GetOpaque(this_val, 1); 
    if (!p)
        return JS_ThrowTypeError(ctx, "Invalid Point object");;
    return JS_NewInt32(ctx, p->x);
}

static JSValue js_point_set_x(JSContext* ctx, JSValueConst this_val, JSValueConst value) {
    struct Point* p = JS_GetOpaque(this_val, 1);
    if (!p)
        return JS_ThrowTypeError(ctx, "Invalid Point object");;

    int x;
    if (JS_ToInt32(ctx, &x, value))
        return JS_EXCEPTION;

    p->x = x;
    return JS_UNDEFINED;
}

static JSValue js_point_get_y(JSContext* ctx, JSValueConst this_val) {
    struct Point* p = JS_GetOpaque(this_val, 1); 
    if (!p)
        return JS_ThrowTypeError(ctx, "Invalid Point object");;
    return JS_NewInt32(ctx, p->y);
}

static JSValue js_point_set_y(JSContext* ctx, JSValueConst this_val, JSValueConst value) {
    struct Point* p = JS_GetOpaque(this_val, 1);
    if (!p)
        return JS_ThrowTypeError(ctx, "Invalid Point object");;

    int y;
    if (JS_ToInt32(ctx, &y, value))
        return JS_EXCEPTION;

    p->y = y;
    return JS_UNDEFINED;
}


static JSValue js_point_free(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    struct Point* p = JS_GetOpaque(this_val, 1);
    if (!p)
        return JS_EXCEPTION;

    point_free(p);
    JS_SetOpaque(this_val, NULL); 
    return JS_UNDEFINED;
}


static const JSCFunctionListEntry js_point_proto_funcs[] = {
    JS_CGETSET_DEF("x", js_point_get_x, js_point_set_x),
    JS_CGETSET_DEF("y", js_point_get_x, js_point_set_x),
    JS_CFUNC_DEF("free", 0, js_point_free),
};


static void js_point_init(JSContext* ctx) {
    JSValue point_proto, point_class;

    point_proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, point_proto, js_point_proto_funcs, countof(js_point_proto_funcs));

    /*
        point class 是一个函数，名称为XPoint的函数，函数实现是native的 js_point_create
        例如：
        ```
        let point_class = function XPoint(){
            [native code]
        }
        ```
    
        js中如果一个函数是构造函数，在使用 new name()的时候, 会传入函数原型，这里设置传入的原型是 point_proto.

        ```
            let p = new point_class();
        ```

        但是因为是c++的对象，所以我们还需要在js侧暴露一个名字，以便js逻辑调用，这里设置的名字是Point

        相当于下面的js代码
        ```
        var Point = functuion XPoint(){}

        let p = new Point();

        ```

    */

    point_class = JS_NewCFunction2(ctx, js_point_create, "XPoint", 2, JS_CFUNC_constructor, 0);
    printf("class1 : %d\n", JS_GetRefCount(point_class));
    printf("proto1 : %d\n", JS_GetRefCount(point_proto));
    JS_SetConstructor(ctx, point_class, point_proto);
    printf("class2 : %d\n", JS_GetRefCount(point_class));
    printf("proto2 : %d\n", JS_GetRefCount(point_proto));
    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "Point", point_class);
    printf("class3 : %d\n", JS_GetRefCount(point_class));
    printf("proto3 : %d\n", JS_GetRefCount(point_proto));
    JS_FreeValue(ctx, point_proto);
    /*
        这里不可以 free point_class: 否则会报错，如下。看报错原因是因为尝试对一个引用计数小于等于0的对象进行gc，即重复释放。
        ```
            Assertion failed: (p->ref_count > 0), function gc_decref_child, file quickjs.c, line 5751.
            [1]    35174 abort      ./out/inject_native_type
        ```
        1. 首先我增加了一个函数 JS_GetRefCount 用来打印对象的引用计数，在没有 free point_class 的时候，其引用计数为 2。看起来在正常的情况下，qjs vm 内部需要对这个对象 free 两次。
        2. 继续增加日志，发现在 JS_SetConstructor(ctx, point_class, point_proto); 后 class 和 proto 引用计数都增加了 1. 看起来这两个互相引用。
        3. 在 JS_SetPropertyStr 执行后，我预期是看到引用计数为 3， 但是事实上还是 2. 也就是说在 JS_SetPropertyStr 中挂载在 globalobj 上的对象是通过指针挂载的，而不是对象引用。所以我们不能 free。
        4. 

        ```
            global ---> class ---> proto
                         ^          |
                         | _ _ _ _  |
        ```
    */
    // JS_FreeValue(ctx, point_class);
    JS_FreeValue(ctx, global);

    printf("class : %d\n", JS_GetRefCount(point_class));
}



int main(int argc, char **argv)
{
  JSRuntime *rt;
  JSContext *ctx;
  rt = JS_NewRuntime();
  js_std_init_handlers(rt);
  ctx = JS_NewContext(rt);
  js_std_add_helpers(ctx, argc, argv);
  js_point_init(ctx);
  eval_file(ctx, "./inject_native_type.js", 0);
  js_std_loop(ctx);
  js_std_free_handlers(rt);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}
