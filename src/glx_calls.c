#define _GNU_SOURCE
#include <dlfcn.h>
#include "gladio.h"
#include "gl_context.h"

#define MSG_DEBUG_UNIMPLEMENTED_GLXCALL "gladio: unimplemented call %s"
#define DEFAULT_FBCONFIG_ID 1

static int maxContextId = 1;

#define GLX_CALL_LOCK() pthread_mutex_lock(&gl_call_mutex)
#define GLX_CALL_UNLOCK() pthread_mutex_unlock(&gl_call_mutex)

GLXFBConfig* glXChooseFBConfig(Display* dpy, int screen, const int* attrib_list, int* nelements) {
    return glXGetFBConfigs(dpy, screen, nelements);
}

XVisualInfo* glXChooseVisual(Display* dpy, int screen, int* attribList) {
    XVisualInfo visualInfo = {0};
    visualInfo.depth = 32;
    visualInfo.class = TrueColor;

    int count;
    XVisualInfo* visuals = XGetVisualInfo(dpy, VisualDepthMask|VisualClassMask, &visualInfo, &count);
    if (!count) return NULL;

    return visuals;
}

void glXCopyContext(Display* dpy, GLXContext src, GLXContext dst, unsigned long mask) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXCopyContext");
}

GLXContext glXCreateContextAttribsARB(Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list) {
    if (!gladioInitOnce()) return NULL;
    GLX_CALL_LOCK();
    int contextId = maxContextId++;
    
    ArrayBuffer requestData = {0};
    ArrayBuffer_putInt(&requestData, contextId);
    ArrayBuffer_putInt(&requestData, DEFAULT_FBCONFIG_ID);
    ArrayBuffer_putInt(&requestData, 0);
    ArrayBuffer_putInt(&requestData, share_context ? share_context->id : 0);
    ArrayBuffer_put(&requestData, direct);
    ArrayBuffer_putBytes(&requestData, NULL, 3);
    
    int i = 0;
    int num_attribs = 0;
    while (attrib_list[i]) {
        num_attribs++;
        i += 2;
    }
    ArrayBuffer_putInt(&requestData, num_attribs);
    
    i = 0;
    while (attrib_list[i]) {
        ArrayBuffer_putInt(&requestData, attrib_list[i+0]);
        ArrayBuffer_putInt(&requestData, attrib_list[i+1]);
        i += 2;
    }
    
    if (!glx_send(serverFd, GLX_OPCODE_CREATE_CONTEXT_ATTRIBS_ARB, requestData.buffer, requestData.size)) {
        GLX_CALL_UNLOCK();
        return NULL;
    }
    
    ArrayBuffer replyData = {0};
    if (!glx_recv(serverFd, &replyData)) {
        GLX_CALL_UNLOCK();
        return NULL;
    }
    
    GLXContext context = createGLXContext(dpy, contextId, share_context);
    GLX_CALL_UNLOCK();
    return context;
}

GLXContext glXCreateContext(Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct) {
    if (!gladioInitOnce()) return NULL;
    GLX_CALL_LOCK();
    int contextId = maxContextId++;

    ArrayBuffer requestData = {0};
    ArrayBuffer_putInt(&requestData, contextId);
    ArrayBuffer_putInt(&requestData, 0);
    ArrayBuffer_putInt(&requestData, 0);
    ArrayBuffer_putInt(&requestData, shareList ? shareList->id : 0);
    ArrayBuffer_put(&requestData, direct);

    if (!glx_send(serverFd, GLX_OPCODE_CREATE_CONTEXT, requestData.buffer, requestData.size)) {
        GLX_CALL_UNLOCK();
        return NULL;
    }
    
    ArrayBuffer replyData = {0};
    if (!glx_recv(serverFd, &replyData)) {
        GLX_CALL_UNLOCK();
        return NULL;
    }    

    GLXContext context = createGLXContext(dpy, contextId, shareList);
    GLX_CALL_UNLOCK();
    return context;
}

GLXPixmap glXCreateGLXPixmap(Display* dpy, XVisualInfo* visual, Pixmap pixmap) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXCreateGLXPixmap");
    return 0;
}

GLXContext glXCreateNewContext(Display* dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct) {
    return glXCreateContextAttribsARB(dpy, config, share_list, direct, NULL);
}

GLXPbuffer glXCreatePbuffer(Display* dpy, GLXFBConfig config, const int* attrib_list) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXCreatePbuffer");
    return 0;
}

GLXPixmap glXCreatePixmap(Display* dpy, GLXFBConfig config, Pixmap pixmap, const int* attrib_list) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXCreatePixmap");
    return 0;
}

GLXWindow glXCreateWindow(Display* dpy, GLXFBConfig config, Window win, const int* attrib_list) {
    return (GLXWindow)win;
}

void glXDestroyContext(Display* dpy, GLXContext ctx) {
    GLX_CALL_LOCK();
    glx_send(serverFd, GLX_OPCODE_DESTROY_CONTEXT, &ctx->id, sizeof(int));

    GLClientState_destroy(&ctx->clientState);
    SparseArray_remove(&glxContexts, ctx->id);
    free(ctx);
    GLX_CALL_UNLOCK();
}

void glXDestroyGLXPixmap(Display* dpy, GLXPixmap pixmap) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXDestroyGLXPixmap");
}

void glXDestroyPbuffer(Display* dpy, GLXPbuffer pbuf) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXDestroyPbuffer");
}

void glXDestroyPixmap(Display* dpy, GLXPixmap pixmap) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXDestroyPixmap");
}

void glXDestroyWindow(Display* dpy, GLXWindow win) {
    GLX_CALL_LOCK();
    for (int i = 0; i < glxContexts.size; i++) {
        GLXContext context = glxContexts.entries[i].value;
        if (context->drawable == win) context->drawable = 0;
    }
    GLX_CALL_UNLOCK();
}

const char* glXGetClientString(Display* dpy, int name) {
    return glXQueryServerString(dpy, 0, name);
}

int glXGetConfig(Display* dpy, XVisualInfo* visual, int attrib, int* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXGetConfig");
    return 0;
}

GLXContext glXGetCurrentContext() {
    return currentGLContext ? currentGLContext->glxContext : NULL;
}

Display* glXGetCurrentDisplay() {
    return currentGLContext && currentGLContext->glxContext ? currentGLContext->glxContext->dpy : NULL;
}

GLXDrawable glXGetCurrentDrawable() {
    return currentGLContext && currentGLContext->glxContext ? currentGLContext->glxContext->drawable : 0;
}

GLXDrawable glXGetCurrentReadDrawable() {
    return glXGetCurrentDrawable();
}

int glXGetFBConfigAttrib(Display* dpy, GLXFBConfig config, int attribute, int* value) {
    for (int i = 0; i < config->numProperties; i++) {
        if (config->attributes[i].name == attribute) {
            *value = config->attributes[i].value;
            return 0;
        }
    }
    
    *value = 0;
    return 1;
}

GLXFBConfig* glXGetFBConfigs(Display* dpy, int screen, int* nelements) {
    static struct __GLXFBConfigRec* globalFBConfigs = NULL;
    GLX_CALL_LOCK();
   
    if (!glx_send(serverFd, GLX_OPCODE_GET_FB_CONFIGS, &screen, sizeof(int))) {
        GLX_CALL_UNLOCK();
        return NULL;
    }
    
    ArrayBuffer replyData = {0};
    if (!glx_recv(serverFd, &replyData)) {
        GLX_CALL_UNLOCK();
        return NULL;
    }
    
    int numFBConfigs = ArrayBuffer_getInt(&replyData);
    int numProperties = ArrayBuffer_getInt(&replyData);
    ArrayBuffer_skip(&replyData, 16);
    *nelements = numFBConfigs;
    
    if (!globalFBConfigs) globalFBConfigs = calloc(numFBConfigs, sizeof(struct __GLXFBConfigRec));
    GLXFBConfig* fbConfigs = malloc(numFBConfigs * sizeof(GLXFBConfig));

    for (int i = 0, j; i < numFBConfigs; i++) {
        fbConfigs[i] = &globalFBConfigs[i];
        fbConfigs[i]->numProperties = numProperties;
        for (j = 0; j < numProperties; j++) {
            fbConfigs[i]->attributes[j].name = ArrayBuffer_getInt(&replyData);
            fbConfigs[i]->attributes[j].value = ArrayBuffer_getInt(&replyData);
        }        
    }

    GLX_CALL_UNLOCK();
    return fbConfigs;
}

__GLXextFuncPtr glXGetProcAddressARB(const GLubyte* procName) {
    const char* name = (const char*)procName;
    if (strcmp(name, "glXChooseFBConfig") == 0) return (__GLXextFuncPtr)glXChooseFBConfig;
    else if (strcmp(name, "glXChooseVisual") == 0) return (__GLXextFuncPtr)glXChooseVisual;
    else if (strcmp(name, "glXCopyContext") == 0) return (__GLXextFuncPtr)glXCopyContext;
    else if (strcmp(name, "glXCreateContextAttribsARB") == 0) return (__GLXextFuncPtr)glXCreateContextAttribsARB;
    else if (strcmp(name, "glXCreateContext") == 0) return (__GLXextFuncPtr)glXCreateContext;
    else if (strcmp(name, "glXCreateGLXPixmap") == 0) return (__GLXextFuncPtr)glXCreateGLXPixmap;
    else if (strcmp(name, "glXCreateNewContext") == 0) return (__GLXextFuncPtr)glXCreateNewContext;
    else if (strcmp(name, "glXCreatePbuffer") == 0) return (__GLXextFuncPtr)glXCreatePbuffer;
    else if (strcmp(name, "glXCreatePixmap") == 0) return (__GLXextFuncPtr)glXCreatePixmap;
    else if (strcmp(name, "glXCreateWindow") == 0) return (__GLXextFuncPtr)glXCreateWindow;
    else if (strcmp(name, "glXDestroyContext") == 0) return (__GLXextFuncPtr)glXDestroyContext;
    else if (strcmp(name, "glXDestroyGLXPixmap") == 0) return (__GLXextFuncPtr)glXDestroyGLXPixmap;
    else if (strcmp(name, "glXDestroyPbuffer") == 0) return (__GLXextFuncPtr)glXDestroyPbuffer;
    else if (strcmp(name, "glXDestroyPixmap") == 0) return (__GLXextFuncPtr)glXDestroyPixmap;
    else if (strcmp(name, "glXDestroyWindow") == 0) return (__GLXextFuncPtr)glXDestroyWindow;
    else if (strcmp(name, "glXGetClientString") == 0) return (__GLXextFuncPtr)glXGetClientString;
    else if (strcmp(name, "glXGetConfig") == 0) return (__GLXextFuncPtr)glXGetConfig;
    else if (strcmp(name, "glXGetCurrentContext") == 0) return (__GLXextFuncPtr)glXGetCurrentContext;
    else if (strcmp(name, "glXGetCurrentDisplay") == 0) return (__GLXextFuncPtr)glXGetCurrentDisplay;
    else if (strcmp(name, "glXGetCurrentDrawable") == 0) return (__GLXextFuncPtr)glXGetCurrentDrawable;
    else if (strcmp(name, "glXGetCurrentReadDrawable") == 0) return (__GLXextFuncPtr)glXGetCurrentReadDrawable;
    else if (strcmp(name, "glXGetFBConfigAttrib") == 0) return (__GLXextFuncPtr)glXGetFBConfigAttrib;
    else if (strcmp(name, "glXGetFBConfigs") == 0) return (__GLXextFuncPtr)glXGetFBConfigs;
    else if (strcmp(name, "glXGetSelectedEvent") == 0) return (__GLXextFuncPtr)glXGetSelectedEvent;
    else if (strcmp(name, "glXGetVisualFromFBConfig") == 0) return (__GLXextFuncPtr)glXGetVisualFromFBConfig;
    else if (strcmp(name, "glXIsDirect") == 0) return (__GLXextFuncPtr)glXIsDirect;
    else if (strcmp(name, "glXMakeContextCurrent") == 0) return (__GLXextFuncPtr)glXMakeContextCurrent;
    else if (strcmp(name, "glXMakeCurrent") == 0) return (__GLXextFuncPtr)glXMakeCurrent;
    else if (strcmp(name, "glXQueryContext") == 0) return (__GLXextFuncPtr)glXQueryContext;
    else if (strcmp(name, "glXQueryDrawable") == 0) return (__GLXextFuncPtr)glXQueryDrawable;
    else if (strcmp(name, "glXQueryExtension") == 0) return (__GLXextFuncPtr)glXQueryExtension;
    else if (strcmp(name, "glXQueryExtensionsString") == 0) return (__GLXextFuncPtr)glXQueryExtensionsString;
    else if (strcmp(name, "glXQueryServerString") == 0) return (__GLXextFuncPtr)glXQueryServerString;
    else if (strcmp(name, "glXQueryVersion") == 0) return (__GLXextFuncPtr)glXQueryVersion;
    else if (strcmp(name, "glXSelectEvent") == 0) return (__GLXextFuncPtr)glXSelectEvent;
    else if (strcmp(name, "glXSwapBuffers") == 0) return (__GLXextFuncPtr)glXSwapBuffers;
    else if (strcmp(name, "glXUseXFont") == 0) return (__GLXextFuncPtr)glXUseXFont;
    else if (strcmp(name, "glXWaitGL") == 0) return (__GLXextFuncPtr)glXWaitGL;
    else if (strcmp(name, "glXWaitX") == 0) return (__GLXextFuncPtr)glXWaitX;

    // GL entry points (core and extensions) are exported by this library itself
    if (strncmp(name, "gl", 2) != 0) return NULL;
    static void* self = NULL;
    if (!self) {
        Dl_info info;
        if (dladdr((void*)glXGetProcAddressARB, &info) && info.dli_fname) self = dlopen(info.dli_fname, RTLD_NOW | RTLD_NOLOAD);
        if (!self) self = RTLD_DEFAULT;
    }
    return (__GLXextFuncPtr)dlsym(self, name);
}

__GLXextFuncPtr glXGetProcAddress(const GLubyte* procName) {
    return glXGetProcAddressARB(procName);
}

void glXGetSelectedEvent(Display* dpy, GLXDrawable draw, unsigned long* event_mask) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXGetSelectedEvent");
}

XVisualInfo* glXGetVisualFromFBConfig(Display* dpy, GLXFBConfig config) {
    XVisualInfo visualInfo = {0};
    visualInfo.depth = 32;
    visualInfo.class = TrueColor;
    
    int count;
    XVisualInfo *visuals = XGetVisualInfo(dpy, VisualDepthMask|VisualClassMask, &visualInfo, &count);
    if (!count) return NULL;
    
    return visuals;
}

Bool glXIsDirect(Display* dpy, GLXContext ctx) {
    return true;
}

Bool glXMakeContextCurrent(Display* dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx) {
    return glXMakeCurrent(dpy, draw, ctx);
}

Bool glXMakeCurrent(Display* dpy, GLXDrawable drawable, GLXContext ctx) {
    GLX_CALL_LOCK();
    if (ctx) {
        ctx->drawable = drawable;
        createGLContextForCallingThread();
        if (currentGLContext) {
            currentGLContext->glxContext = ctx;
            currentGLContext->clientState = &ctx->clientState;
        }
    }
    else {
        destroyGLContextForCallingThread();
        GLX_CALL_UNLOCK();
        return true;
    }

    ArrayBuffer requestData = {0};
    ArrayBuffer_putInt(&requestData, (int)drawable);
    ArrayBuffer_putInt(&requestData, ctx->id);
    GL_SEND_CHECKED(REQUEST_CODE_SET_CURRENT_RENDER_WINDOW, requestData.buffer, requestData.size, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLX_CALL_UNLOCK();
    return true;
}

int glXQueryContext(Display* dpy, GLXContext ctx, int attribute, int* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXQueryContext");
    return 0;
}

void glXQueryDrawable(Display* dpy, GLXDrawable draw, int attribute, unsigned int* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXQueryDrawable");
}

Bool glXQueryExtension(Display* dpy, int* errorb, int* event) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXQueryExtension");
    return 0;
}

const char* glXQueryExtensionsString(Display* dpy, int screen) {
    GLX_CALL_LOCK();
    char* cachedString = getCachedString(GLX_EXTENSIONS);
    if (cachedString) {
        GLX_CALL_UNLOCK();
        return cachedString;
    }
    
    if (!glx_send(serverFd, GLX_OPCODE_QUERY_EXTENSIONS_STRING, &screen, sizeof(int))) {
        GLX_CALL_UNLOCK();
        return false;
    }
    
    ArrayBuffer replyData = {0};
    if (!glx_recv(serverFd, &replyData)) {
        GLX_CALL_UNLOCK();
        return false;
    }
    
    ArrayBuffer_skip(&replyData, 4);
    int length = ArrayBuffer_getInt(&replyData);
    ArrayBuffer_skip(&replyData, 16);
    
    char* string = ArrayBuffer_getBytes(&replyData, length);
    char* result = putCachedString(GLX_EXTENSIONS, string, length);
    GLX_CALL_UNLOCK();
    return result;
}

const char* glXQueryServerString(Display* dpy, int screen, int name) {
    GLX_CALL_LOCK();
    char* cachedString = getCachedString(name);
    if (cachedString) {
        GLX_CALL_UNLOCK();
        return cachedString;
    }
    
    ArrayBuffer requestData = {0};
    ArrayBuffer_putInt(&requestData, screen);
    ArrayBuffer_putInt(&requestData, name);
    
    if (!glx_send(serverFd, GLX_OPCODE_QUERY_SERVER_STRING, requestData.buffer, requestData.size)) {
        GLX_CALL_UNLOCK();
        return false;
    }
    
    ArrayBuffer replyData = {0};
    if (!glx_recv(serverFd, &replyData)) {
        GLX_CALL_UNLOCK();
        return false;
    }
    
    ArrayBuffer_skip(&replyData, 4);
    int length = ArrayBuffer_getInt(&replyData);
    ArrayBuffer_skip(&replyData, 16);
    
    char* string = ArrayBuffer_getBytes(&replyData, length);
    char* result = putCachedString(name, string, length);
    GLX_CALL_UNLOCK();
    return result;
}

Bool glXQueryVersion(Display* dpy, int* maj, int* min) {
    GLX_CALL_LOCK();
    ArrayBuffer requestData = {0};
    ArrayBuffer_putInt(&requestData, *maj);
    ArrayBuffer_putInt(&requestData, *min);
    
    if (!glx_send(serverFd, GLX_OPCODE_QUERY_VERSION, requestData.buffer, requestData.size)) {
        GLX_CALL_UNLOCK();
        return false;
    }
    
    ArrayBuffer replyData = {0};
    if (!glx_recv(serverFd, &replyData)) {
        GLX_CALL_UNLOCK();
        return false;
    }
    
    *maj = ArrayBuffer_getInt(&replyData);
    *min = ArrayBuffer_getInt(&replyData);
    GLX_CALL_UNLOCK();
    return true;
}

void glXSelectEvent(Display* dpy, GLXDrawable draw, unsigned long event_mask) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXSelectEvent");
}

void glXSwapBuffers(Display* dpy, GLXDrawable drawable) {
    GL_CALL_LOCK();
    int drawableId = (int)drawable;
    GL_SEND_CHECKED(REQUEST_CODE_SWAP_DISPLAY_BUFFERS, &drawableId, sizeof(int));
    GL_RECV_CHECKED();
    GL_CALL_UNLOCK();
}

void glXUseXFont(Font font, int first, int count, int list) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXUseXFont");
}

void glXWaitGL() {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXWaitGL");
}

void glXWaitX() {
    println(MSG_DEBUG_UNIMPLEMENTED_GLXCALL, "glXWaitX");
}

