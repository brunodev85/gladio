#include "gladio.h"
#include "gl_context.h"

#define MSG_DEBUG_UNIMPLEMENTED_GLCALL "gladio: unimplemented call %s"

pthread_mutex_t gl_call_mutex = PTHREAD_MUTEX_INITIALIZER;

void glAccum(GLenum op, GLfloat value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, op);
    ArrayBuffer_putFloat(&outputBuffer, value);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ACCUM, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glActiveTexture(GLenum texture) {
    GL_CALL_LOCK();
    currentGLContext->clientState->activeTexture = texture - GL_TEXTURE0;
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ACTIVE_TEXTURE, &texture, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glActiveTextureARB(GLenum texture) {
    glActiveTexture(texture);
}

void glAlphaFunc(GLenum func, GLfloat ref) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, func);
    ArrayBuffer_putFloat(&outputBuffer, ref);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ALPHA_FUNC, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

GLboolean glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences) {
    return true;
}

GLboolean glAreTexturesResidentEXT(GLsizei n, const GLuint* textures, GLboolean* residences) {
    return glAreTexturesResident(n, textures, residences);
}

void glArrayElement(GLint i) {
    GL_CALL_LOCK();
    if (currentGLContext->commandBuffer.begin) {
        ArrayBuffer_putShort(&currentGLContext->commandBuffer.data, REQUEST_CODE_GL_ARRAY_ELEMENT);
        ArrayBuffer_putInt(&currentGLContext->commandBuffer.data, i);
        currentGLContext->commandBuffer.minIndex = MIN(currentGLContext->commandBuffer.minIndex, i);
        currentGLContext->commandBuffer.maxIndex = MAX(currentGLContext->commandBuffer.maxIndex, i);
    }
    else {    
        ArrayBuffer_rewind(&outputBuffer);
        ArrayBuffer_putInt(&outputBuffer, i);
        GLClientState* clientState = currentGLContext->clientState;

        for (int j = 0; j < clientState->vao->maxEnabledAttribs; j++) {
            if (clientState->vao->attribs[j].state && clientState->vao->attribs[j].pointer) {
                void* values = clientState->vao->attribs[j].pointer + i * clientState->vao->attribs[j].stride;
                ArrayBuffer_putBytes(&outputBuffer, values, clientState->vao->attribs[j].stride);
            }
        }

        gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ARRAY_ELEMENT, outputBuffer.buffer, outputBuffer.size);
    }
    GL_CALL_UNLOCK();
}

void glArrayElementEXT(GLint i) {
    glArrayElement(i);
}

void glAttachObjectARB(GLhandleARB containerObj, GLhandleARB obj) {
    glAttachShader(containerObj, obj);
}

void glAttachShader(GLuint program, GLuint shader) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, shader);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ATTACH_SHADER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBegin(GLenum mode) {
    GL_CALL_LOCK();
    currentGLContext->commandBuffer.begin = true;
    currentGLContext->commandBuffer.mode = mode;
    currentGLContext->commandBuffer.minIndex = INT32_MAX;
    currentGLContext->commandBuffer.maxIndex = 0;
    ArrayBuffer_rewind(&currentGLContext->commandBuffer.data);
    GL_CALL_UNLOCK();
}

void glBeginConditionalRender(GLuint id, GLenum mode) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, id);
    ArrayBuffer_putInt(&outputBuffer, mode);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BEGIN_CONDITIONAL_RENDER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBeginQuery(GLenum target, GLuint id) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, id);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BEGIN_QUERY, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBeginQueryARB(GLenum target, GLuint id) {
    glBeginQuery(target, id);
}

void glBeginQueryEXT(GLenum target, GLuint id) {
    glBeginQuery(target, id);
}

void glBeginTransformFeedback(GLenum primitiveMode) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BEGIN_TRANSFORM_FEEDBACK, &primitiveMode, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glBeginTransformFeedbackEXT(GLenum primitiveMode) {
    glBeginTransformFeedback(primitiveMode);
}

void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, name, strlen(name) + 1);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_ATTRIB_LOCATION, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindAttribLocationARB(GLhandleARB programObj, GLuint index, const GLcharARB* name) {
    glBindAttribLocation(programObj, index, name);
}

void glBindBuffer(GLenum target, GLuint buffer) {
    GL_CALL_LOCK();
    GLBuffer_setBound(target, buffer);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, buffer);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_BUFFER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindBufferARB(GLenum target, GLuint buffer) {
    glBindBuffer(target, buffer);
}

void glBindBufferBase(GLenum target, GLuint index, GLuint buffer) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, buffer);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_BUFFER_BASE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindBufferBaseEXT(GLenum target, GLuint index, GLuint buffer) {
    glBindBufferBase(target, index, buffer);
}

void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, buffer);
    ArrayBuffer_putInt(&outputBuffer, offset);
    ArrayBuffer_putInt(&outputBuffer, size);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_BUFFER_RANGE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindBufferRangeEXT(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    glBindBufferRange(target, index, buffer, offset, size);
}

void glBindFragDataLocation(GLuint program, GLuint color, const GLchar* name) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, color);
    ArrayBuffer_putBytes(&outputBuffer, name, strlen(name) + 1);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_FRAG_DATA_LOCATION, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindFragDataLocationEXT(GLuint program, GLuint color, const GLchar* name) {
    glBindFragDataLocation(program, color, name);
}

void glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, colorNumber);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, name, strlen(name) + 1);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_FRAG_DATA_LOCATION_INDEXED, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindFragDataLocationIndexedEXT(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name) {
    glBindFragDataLocationIndexed(program, colorNumber, index, name);
}

void glBindFramebuffer(GLenum target, GLuint framebuffer) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, framebuffer);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_FRAMEBUFFER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindFramebufferEXT(GLenum target, GLuint framebuffer) {
    glBindFramebuffer(target, framebuffer);
}

void glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glBindTexture(target, texture);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glBindProgramARB(GLenum target, GLuint program) {
    GL_CALL_LOCK();
    currentGLContext->clientState->arbProgram[indexOfGLTarget(target)] = program;
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, program);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_PROGRAM_ARB, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, renderbuffer);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_RENDERBUFFER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindRenderbufferEXT(GLenum target, GLuint renderbuffer) {
    glBindRenderbuffer(target, renderbuffer);
}

void glBindSampler(GLuint unit, GLuint sampler) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, unit);
    ArrayBuffer_putInt(&outputBuffer, sampler);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_SAMPLER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindTexture(GLenum target, GLuint texture) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, texture);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_TEXTURE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBindTextureEXT(GLenum target, GLuint texture) {
    glBindTexture(target, texture);
}

void glBindVertexArray(GLuint array) {
    GL_CALL_LOCK();
    GLVertexArrayObject_setBound(currentGLContext->clientState, array);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BIND_VERTEX_ARRAY, &array, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* bitmap) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glBitmap");
}

void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, red);
    ArrayBuffer_putFloat(&outputBuffer, green);
    ArrayBuffer_putFloat(&outputBuffer, blue);
    ArrayBuffer_putFloat(&outputBuffer, alpha);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLEND_COLOR, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBlendColorEXT(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    glBlendColor(red, green, blue, alpha);
}

void glBlendEquation(GLenum mode) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLEND_EQUATION, &mode, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glBlendEquationEXT(GLenum mode) {
    glBlendEquation(mode);
}

void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, modeRGB);
    ArrayBuffer_putInt(&outputBuffer, modeAlpha);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLEND_EQUATION_SEPARATE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBlendEquationSeparateEXT(GLenum modeRGB, GLenum modeAlpha) {
    glBlendEquationSeparate(modeRGB, modeAlpha);
}

void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, buf);
    ArrayBuffer_putInt(&outputBuffer, modeRGB);
    ArrayBuffer_putInt(&outputBuffer, modeAlpha);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLEND_EQUATION_SEPARATEI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBlendEquationSeparateiARB(GLuint buf, GLenum modeRGB, GLenum modeAlpha) {
    glBlendEquationSeparatei(buf, modeRGB, modeAlpha);
}

void glBlendEquationSeparateiEXT(GLuint buf, GLenum modeRGB, GLenum modeAlpha) {
    glBlendEquationSeparatei(buf, modeRGB, modeAlpha);
}

void glBlendEquationi(GLuint buf, GLenum mode) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, buf);
    ArrayBuffer_putInt(&outputBuffer, mode);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLEND_EQUATIONI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBlendEquationiARB(GLuint buf, GLenum mode) {
    glBlendEquationi(buf, mode);
}

void glBlendEquationiEXT(GLuint buf, GLenum mode) {
    glBlendEquationi(buf, mode);
}

void glBlendFunc(GLenum sfactor, GLenum dfactor) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, sfactor);
    ArrayBuffer_putInt(&outputBuffer, dfactor);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLEND_FUNC, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, sfactorRGB);
    ArrayBuffer_putInt(&outputBuffer, dfactorRGB);
    ArrayBuffer_putInt(&outputBuffer, sfactorAlpha);
    ArrayBuffer_putInt(&outputBuffer, dfactorAlpha);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLEND_FUNC_SEPARATE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBlendFuncSeparateEXT(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
    glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, buf);
    ArrayBuffer_putInt(&outputBuffer, srcRGB);
    ArrayBuffer_putInt(&outputBuffer, dstRGB);
    ArrayBuffer_putInt(&outputBuffer, srcAlpha);
    ArrayBuffer_putInt(&outputBuffer, dstAlpha);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLEND_FUNC_SEPARATEI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBlendFuncSeparateiARB(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
    glBlendFuncSeparatei(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void glBlendFuncSeparateiEXT(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
    glBlendFuncSeparatei(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void glBlendFunci(GLuint buf, GLenum src, GLenum dst) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, buf);
    ArrayBuffer_putInt(&outputBuffer, src);
    ArrayBuffer_putInt(&outputBuffer, dst);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLEND_FUNCI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBlendFunciARB(GLuint buf, GLenum src, GLenum dst) {
    glBlendFunci(buf, src, dst);
}

void glBlendFunciEXT(GLuint buf, GLenum src, GLenum dst) {
    glBlendFunci(buf, src, dst);
}

void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, srcX0);
    ArrayBuffer_putInt(&outputBuffer, srcY0);
    ArrayBuffer_putInt(&outputBuffer, srcX1);
    ArrayBuffer_putInt(&outputBuffer, srcY1);
    ArrayBuffer_putInt(&outputBuffer, dstX0);
    ArrayBuffer_putInt(&outputBuffer, dstY0);
    ArrayBuffer_putInt(&outputBuffer, dstX1);
    ArrayBuffer_putInt(&outputBuffer, dstY1);
    ArrayBuffer_putInt(&outputBuffer, mask);
    ArrayBuffer_putInt(&outputBuffer, filter);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_BLIT_FRAMEBUFFER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glBlitFramebufferEXT(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
    glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    GL_CALL_LOCK();
    GLBuffer* buffer = GLBuffer_getBound(target);
    if (!buffer) {
        GL_CALL_UNLOCK();
        return;
    }

    buffer->usage = usage;
    if (size > buffer->size || !buffer->mappedData) {
        GL_CALL_UNLOCK();
        GLbitfield flags = usage == GL_DYNAMIC_DRAW ? GL_DYNAMIC_STORAGE_BIT : 0;
        glBufferStorage(target, size, data, flags);
        return;
    }

    if (data) memcpy(buffer->mappedData, data, size);
    if (target != GL_PIXEL_UNPACK_BUFFER) {
        ArrayBuffer_rewind(&outputBuffer);
        ArrayBuffer_putInt(&outputBuffer, target);
        ArrayBuffer_putInt(&outputBuffer, size);
        ArrayBuffer_putInt(&outputBuffer, usage);
        GL_SEND_CHECKED(REQUEST_CODE_GL_BUFFER_DATA, outputBuffer.buffer, outputBuffer.size);
        GL_RECV_CHECKED();
    }
    GL_CALL_UNLOCK();
}

void glBufferDataARB(GLenum target, GLsizeiptrARB size, const void* data, GLenum usage) {
    glBufferData(target, size, data, usage);
}

void glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags) {
    GL_CALL_LOCK();
    GLBuffer* buffer = GLBuffer_getBound(target);
    if (!buffer) {
        GL_CALL_UNLOCK();
        return;
    }
    
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, size);
    ArrayBuffer_putInt(&outputBuffer, flags);
    GL_SEND_CHECKED(REQUEST_CODE_GL_BUFFER_STORAGE, outputBuffer.buffer, outputBuffer.size);
    
    int fd, numFds;
    bool success = false;
    recv_fds(serverFd, &fd, &numFds, &success, sizeof(bool));
    if (numFds != 1 || !success || !GLBuffer_mapMemory(buffer, fd, size)) {
        GL_CALL_UNLOCK();
        return;
    }
    
    if ((flags & GL_MAP_PERSISTENT_BIT) && target != GL_PIXEL_UNPACK_BUFFER) {
        ArrayList_remove(&currentGLContext->clientState->persistentBuffers, buffer);
        ArrayList_add(&currentGLContext->clientState->persistentBuffers, buffer);
    }

    buffer->type = target;
    buffer->usage = (flags & GL_DYNAMIC_STORAGE_BIT) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    if (data) glBufferSubData(target, 0, size, data);
    GL_CALL_UNLOCK();
}

void glBufferStorageEXT(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags) {
    glBufferStorage(target, size, data, flags);
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
    GL_CALL_LOCK();
    GLBuffer* buffer = GLBuffer_getBound(target);
    if (!buffer || !buffer->mappedData) {
        GL_CALL_UNLOCK();
        return;
    }

    if (data) memcpy(buffer->mappedData + offset, data, size);
    if (target != GL_PIXEL_UNPACK_BUFFER) {
        ArrayBuffer_rewind(&outputBuffer);
        ArrayBuffer_putInt(&outputBuffer, target);
        ArrayBuffer_putInt(&outputBuffer, offset);
        ArrayBuffer_putInt(&outputBuffer, size);
        GL_SEND_CHECKED(REQUEST_CODE_GL_BUFFER_SUB_DATA, outputBuffer.buffer, outputBuffer.size);
        GL_RECV_CHECKED();
    }
    GL_CALL_UNLOCK();
}

void glBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const void* data) {
    glBufferSubData(target, offset, size, data);
}

void glCallList(GLuint list) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CALL_LIST, &list, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glCallLists(GLsizei n, GLenum type, const void* lists) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glCallLists");
}

GLenum glCheckFramebufferStatus(GLenum target) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_CHECK_FRAMEBUFFER_STATUS, &target, sizeof(GLenum), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLenum result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLenum glCheckFramebufferStatusEXT(GLenum target) {
    return glCheckFramebufferStatus(target);
}

GLenum glCheckNamedFramebufferStatusEXT(GLuint framebuffer, GLenum target) {
    GL_DSA_SAVE_BOUND_FRAMEBUFFER(framebuffer);
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    GL_DSA_RESTORE_BOUND_FRAMEBUFFER();
    return result;
}

void glClampColor(GLenum target, GLenum clamp) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, clamp);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLAMP_COLOR, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glClampColorARB(GLenum target, GLenum clamp) {
    glClampColor(target, clamp);
}

void glClear(GLbitfield mask) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLEAR, &mask, sizeof(GLbitfield));
    GL_CALL_UNLOCK();
}

void glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, red);
    ArrayBuffer_putFloat(&outputBuffer, green);
    ArrayBuffer_putFloat(&outputBuffer, blue);
    ArrayBuffer_putFloat(&outputBuffer, alpha);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLEAR_ACCUM, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, buffer);
    ArrayBuffer_putInt(&outputBuffer, drawbuffer);
    ArrayBuffer_putFloat(&outputBuffer, depth);
    ArrayBuffer_putInt(&outputBuffer, stencil);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLEAR_BUFFERFI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glClearBufferfv");
}

void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glClearBufferiv");
}

void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glClearBufferuiv");
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, red);
    ArrayBuffer_putFloat(&outputBuffer, green);
    ArrayBuffer_putFloat(&outputBuffer, blue);
    ArrayBuffer_putFloat(&outputBuffer, alpha);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLEAR_COLOR, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glClearDepth(GLdouble depth) {
    glClearDepthf((GLfloat)depth);
}

void glClearDepthf(GLfloat d) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLEAR_DEPTHF, &d, sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glClearIndex(GLfloat c) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLEAR_INDEX, &c, sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glClearStencil(GLint s) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLEAR_STENCIL, &s, sizeof(GLint));
    GL_CALL_UNLOCK();
}

void glClientActiveTexture(GLenum texture) {
    GL_CALL_LOCK();
    currentGLContext->clientState->activeTexCoord = texture - GL_TEXTURE0;
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLIENT_ACTIVE_TEXTURE, &texture, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glClientActiveTextureARB(GLenum texture) {
    glClientActiveTexture(texture);
}

void glClientAttribDefaultEXT(GLbitfield mask) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glClientAttribDefaultEXT");
}

GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putLong(&outputBuffer, (long)sync);
    ArrayBuffer_putInt(&outputBuffer, flags);
    ArrayBuffer_putLong(&outputBuffer, timeout);
    GL_SEND_CHECKED(REQUEST_CODE_GL_CLIENT_WAIT_SYNC, outputBuffer.buffer, outputBuffer.size, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLenum result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

void glClipPlane(GLenum plane, const GLdouble* equation) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, plane);
    ArrayBuffer_putBytes(&outputBuffer, equation, 4 * sizeof(double));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CLIP_PLANE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glColor3b(GLbyte red, GLbyte green, GLbyte blue) {
    glColor4f((GLfloat)red / INT8_MAX, (GLfloat)green / INT8_MAX, (GLfloat)blue / INT8_MAX, 1.0f);
}

void glColor3bv(const GLbyte* v) {
    glColor4f((GLfloat)v[0] / INT8_MAX, (GLfloat)v[1] / INT8_MAX, (GLfloat)v[2] / INT8_MAX, 1.0f);
}

void glColor3d(GLdouble red, GLdouble green, GLdouble blue) {
    glColor4f((GLfloat)red, (GLfloat)green, (GLfloat)blue, 1.0f);
}

void glColor3dv(const GLdouble* v) {
    glColor4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glColor3f(GLfloat red, GLfloat green, GLfloat blue) {
    glColor4f(red, green, blue, 1.0f);
}

void glColor3fv(const GLfloat* v) {
    glColor4f(v[0], v[1], v[2], 1.0f);
}

void glColor3i(GLint red, GLint green, GLint blue) {
    glColor4f((GLfloat)red / INT32_MAX, (GLfloat)green / INT32_MAX, (GLfloat)blue / INT32_MAX, 1.0f);
}

void glColor3iv(const GLint* v) {
    glColor4f((GLfloat)v[0] / INT32_MAX, (GLfloat)v[1] / INT32_MAX, (GLfloat)v[2] / INT32_MAX, 1.0f);
}

void glColor3s(GLshort red, GLshort green, GLshort blue) {
    glColor4f((GLfloat)red / INT16_MAX, (GLfloat)green / INT16_MAX, (GLfloat)blue / INT16_MAX, 1.0f);
}

void glColor3sv(const GLshort* v) {
    glColor4f((GLfloat)v[0] / INT16_MAX, (GLfloat)v[1] / INT16_MAX, (GLfloat)v[2] / INT16_MAX, 1.0f);
}

void glColor3ub(GLubyte red, GLubyte green, GLubyte blue) {
    glColor4f((GLfloat)red / UINT8_MAX, (GLfloat)green / UINT8_MAX, (GLfloat)blue / UINT8_MAX, 1.0f);
}

void glColor3ubv(const GLubyte* v) {
    glColor4f((GLfloat)v[0] / UINT8_MAX, (GLfloat)v[1] / UINT8_MAX, (GLfloat)v[2] / UINT8_MAX, 1.0f);
}

void glColor3ui(GLuint red, GLuint green, GLuint blue) {
    glColor4f((GLfloat)red / UINT32_MAX, (GLfloat)green / UINT32_MAX, (GLfloat)blue / UINT32_MAX, 1.0f);
}

void glColor3uiv(const GLuint* v) {
    glColor4f((GLfloat)v[0] / UINT32_MAX, (GLfloat)v[1] / UINT32_MAX, (GLfloat)v[2] / UINT32_MAX, 1.0f);
}

void glColor3us(GLushort red, GLushort green, GLushort blue) {
    glColor4f((GLfloat)red / UINT16_MAX, (GLfloat)green / UINT16_MAX, (GLfloat)blue / UINT16_MAX, 1.0f);
}

void glColor3usv(const GLushort* v) {
    glColor4f((GLfloat)v[0] / UINT16_MAX, (GLfloat)v[1] / UINT16_MAX, (GLfloat)v[2] / UINT16_MAX, 1.0f);
}

void glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha) {
    glColor4f((GLfloat)red / INT8_MAX, (GLfloat)green / INT8_MAX, (GLfloat)blue / INT8_MAX, (GLfloat)alpha / INT8_MAX);
}

void glColor4bv(const GLbyte* v) {
    glColor4f((GLfloat)v[0] / INT8_MAX, (GLfloat)v[1] / INT8_MAX, (GLfloat)v[2] / INT8_MAX, (GLfloat)v[3] / INT8_MAX);
}

void glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha) {
    glColor4f((GLfloat)red, (GLfloat)green, (GLfloat)blue, (GLfloat)alpha);
}

void glColor4dv(const GLdouble* v) {
    glColor4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    GL_CALL_LOCK();
    if (currentGLContext->commandBuffer.begin) {
        ArrayBuffer_putShort(&currentGLContext->commandBuffer.data, REQUEST_CODE_GL_COLOR4F);
        ArrayBuffer_putFloat4(&currentGLContext->commandBuffer.data, red, green, blue, alpha);        
    }
    else {    
        ArrayBuffer_rewind(&outputBuffer);
        ArrayBuffer_putFloat4(&outputBuffer, red, green, blue, alpha);
        gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COLOR4F, outputBuffer.buffer, outputBuffer.size);
    }
    GL_CALL_UNLOCK();
}

void glColor4fv(const GLfloat* v) {
    glColor4f(v[0], v[1], v[2], v[3]);
}

void glColor4i(GLint red, GLint green, GLint blue, GLint alpha) {
    glColor4f((GLfloat)red / INT32_MAX, (GLfloat)green / INT32_MAX, (GLfloat)blue / INT32_MAX, (GLfloat)alpha / INT32_MAX);
}

void glColor4iv(const GLint* v) {
    glColor4f((GLfloat)v[0] / INT32_MAX, (GLfloat)v[1] / INT32_MAX, (GLfloat)v[2] / INT32_MAX, (GLfloat)v[3] / INT32_MAX);
}

void glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha) {
    glColor4f((GLfloat)red / INT16_MAX, (GLfloat)green / INT16_MAX, (GLfloat)blue / INT16_MAX, (GLfloat)alpha / INT16_MAX);
}

void glColor4sv(const GLshort* v) {
    glColor4f((GLfloat)v[0] / INT16_MAX, (GLfloat)v[1] / INT16_MAX, (GLfloat)v[2] / INT16_MAX, (GLfloat)v[3] / INT16_MAX);
}

void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) {
    glColor4f((GLfloat)red / UINT8_MAX, (GLfloat)green / UINT8_MAX, (GLfloat)blue / UINT8_MAX, (GLfloat)alpha / UINT8_MAX);
}

void glColor4ubv(const GLubyte* v) {
    glColor4f((GLfloat)v[0] / UINT8_MAX, (GLfloat)v[1] / UINT8_MAX, (GLfloat)v[2] / UINT8_MAX, (GLfloat)v[3] / UINT8_MAX);
}

void glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha) {
    glColor4f((GLfloat)red / UINT32_MAX, (GLfloat)green / UINT32_MAX, (GLfloat)blue / UINT32_MAX, (GLfloat)alpha / UINT32_MAX);
}

void glColor4uiv(const GLuint* v) {
    glColor4f((GLfloat)v[0] / UINT32_MAX, (GLfloat)v[1] / UINT32_MAX, (GLfloat)v[2] / UINT32_MAX, (GLfloat)v[3] / UINT32_MAX);
}

void glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha) {
    glColor4f((GLfloat)red / UINT16_MAX, (GLfloat)green / UINT16_MAX, (GLfloat)blue / UINT16_MAX, (GLfloat)alpha / UINT16_MAX);
}

void glColor4usv(const GLushort* v) {
    glColor4f((GLfloat)v[0] / UINT16_MAX, (GLfloat)v[1] / UINT16_MAX, (GLfloat)v[2] / UINT16_MAX, (GLfloat)v[3] / UINT16_MAX);
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_put(&outputBuffer, red);
    ArrayBuffer_put(&outputBuffer, green);
    ArrayBuffer_put(&outputBuffer, blue);
    ArrayBuffer_put(&outputBuffer, alpha);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COLOR_MASK, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_put(&outputBuffer, r);
    ArrayBuffer_put(&outputBuffer, g);
    ArrayBuffer_put(&outputBuffer, b);
    ArrayBuffer_put(&outputBuffer, a);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COLOR_MASKI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glColorMaskiEXT(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
    glColorMaski(index, r, g, b, a);
}

void glColorMaterial(GLenum face, GLenum mode) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, face);
    ArrayBuffer_putInt(&outputBuffer, mode);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COLOR_MATERIAL, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {
    GL_SEND_VERTEX_ARRAY(REQUEST_CODE_GL_COLOR_POINTER, COLOR_ARRAY_INDEX);
}

void glColorPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    glColorPointer(size, type, stride, pointer);
}

void glCompileShader(GLuint shader) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COMPILE_SHADER, &shader, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glCompileShaderARB(GLhandleARB shaderObj) {
    glCompileShader(shaderObj);
}

void glCompressedMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, bits);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCompressedMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, bits);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCompressedMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, bits);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCompressedMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, bits);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCompressedMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, bits);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCompressedMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data) {
    glCompressedTexImage2D(target, level, internalformat, width, 1, border, imageSize, data);
}

void glCompressedTexImage1DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data) {
    glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
}

void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, border);
    GL_SEND_TEXIMAGE(REQUEST_CODE_GL_COMPRESSED_TEX_IMAGE2D, internalformat, 0, width, height, 1, data, imageSize);
    GL_CALL_UNLOCK();
}

void glCompressedTexImage2DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data) {
    glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data) {
    glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void glCompressedTexImage3DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data) {
    glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data) {
    glCompressedTexSubImage2D(target, level, xoffset, 0, width, 1, format, imageSize, data);
}

void glCompressedTexSubImage1DARB(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data) {
    glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
}

void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, xoffset);
    ArrayBuffer_putInt(&outputBuffer, yoffset);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, format);
    GL_SEND_TEXIMAGE(REQUEST_CODE_GL_COMPRESSED_TEX_SUB_IMAGE2D, format, 0, width, height, 1, data, imageSize);
    GL_CALL_UNLOCK();    
}

void glCompressedTexSubImage2DARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) {
    glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data) {
    glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void glCompressedTexSubImage3DARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data) {
    glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

void glCompressedTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, bits);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCompressedTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, bits);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCompressedTextureImage3DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, bits);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCompressedTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, bits);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCompressedTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, bits);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCompressedTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, readTarget);
    ArrayBuffer_putInt(&outputBuffer, writeTarget);
    ArrayBuffer_putInt(&outputBuffer, readOffset);
    ArrayBuffer_putInt(&outputBuffer, writeOffset);
    ArrayBuffer_putInt(&outputBuffer, size);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COPY_BUFFER_SUB_DATA, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glCopyMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCopyTexImage1D(target, level, internalformat, x, y, width, border);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCopyMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCopyMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCopyTexSubImage1D(target, level, xoffset, x, y, width);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCopyMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCopyMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, type);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COPY_PIXELS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    glCopyTexImage2D(target, level, internalformat, x, y, width, 1, border);
}

void glCopyTexImage1DEXT(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    glCopyTexImage1D(target, level, internalformat, x, y, width, border);
}

void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, border);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COPY_TEX_IMAGE2D, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glCopyTexImage2DEXT(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    glCopyTexSubImage2D(target, level, xoffset, 0, x, y, width, 1);
}

void glCopyTexSubImage1DEXT(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    glCopyTexSubImage1D(target, level, xoffset, x, y, width);
}

void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, xoffset);
    ArrayBuffer_putInt(&outputBuffer, yoffset);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COPY_TEX_SUB_IMAGE2D, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glCopyTexSubImage2DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, xoffset);
    ArrayBuffer_putInt(&outputBuffer, yoffset);
    ArrayBuffer_putInt(&outputBuffer, zoffset);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_COPY_TEX_SUB_IMAGE3D, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glCopyTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

void glCopyTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCopyTexImage1D(target, level, internalformat, x, y, width, border);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCopyTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCopyTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCopyTexSubImage1D(target, level, xoffset, x, y, width);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCopyTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glCopyTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

GLuint glCreateProgram() {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_CREATE_PROGRAM, NULL, 0, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLuint result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLuint glCreateProgramObjectARB() {
    return glCreateProgram();
}

GLuint glCreateShader(GLenum type) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_CREATE_SHADER, &type, sizeof(GLenum), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLuint result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLuint glCreateShaderObjectARB(GLenum shaderType) {
    return glCreateShader(shaderType);
}

void glCullFace(GLenum mode) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_CULL_FACE, &mode, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glDeleteBuffers(GLsizei n, const GLuint* buffers) {
    GL_CALL_LOCK();
    for (int i = 0; i < n; i++) GLBuffer_delete(buffers[i]);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, n);
    ArrayBuffer_putBytes(&outputBuffer, buffers, n * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_BUFFERS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDeleteBuffersARB(GLsizei n, const GLuint* buffers) {
    glDeleteBuffers(n, buffers);
}

void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, n);
    ArrayBuffer_putBytes(&outputBuffer, framebuffers, n * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_FRAMEBUFFERS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDeleteFramebuffersEXT(GLsizei n, const GLuint* framebuffers) {
    glDeleteFramebuffers(n, framebuffers);
}

void glDeleteLists(GLuint list, GLsizei range) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, list);
    ArrayBuffer_putInt(&outputBuffer, range);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_LISTS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDeleteObjectARB(GLhandleARB obj) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_OBJECT_ARB, &obj, sizeof(GLhandleARB));
    GL_CALL_UNLOCK();
}

void glDeleteProgram(GLuint program) {
    GL_CALL_LOCK();
    if (program == currentGLContext->clientState->program) currentGLContext->clientState->program = 0;
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_PROGRAM, &program, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glDeleteProgramsARB(GLsizei n, const GLuint* programs) {
    GL_CALL_LOCK();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < MAX_ARB_PROGRAM_TARGETS; j++) {
            if (programs[i] == currentGLContext->clientState->arbProgram[j]) {
                currentGLContext->clientState->arbProgram[j] = 0;
                break;
            }
        }
    }
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, n);
    ArrayBuffer_putBytes(&outputBuffer, programs, n * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_PROGRAMS_ARB, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDeleteQueries(GLsizei n, const GLuint* ids) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, n);
    ArrayBuffer_putBytes(&outputBuffer, ids, n * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_QUERIES, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDeleteQueriesARB(GLsizei n, const GLuint* ids) {
    glDeleteQueries(n, ids);
}

void glDeleteQueriesEXT(GLsizei n, const GLuint* ids) {
    glDeleteQueries(n, ids);
}

void glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, n);
    ArrayBuffer_putBytes(&outputBuffer, renderbuffers, n * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_RENDERBUFFERS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDeleteRenderbuffersEXT(GLsizei n, const GLuint* renderbuffers) {
    glDeleteRenderbuffers(n, renderbuffers);
}

void glDeleteSamplers(GLsizei count, const GLuint* samplers) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, samplers, count * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_SAMPLERS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDeleteShader(GLuint shader) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_SHADER, &shader, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glDeleteSync(GLsync sync) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_SYNC, &sync, sizeof(long));
    GL_CALL_UNLOCK();
}

void glDeleteTextures(GLsizei n, const GLuint* textures) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, n);
    ArrayBuffer_putBytes(&outputBuffer, textures, n * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_TEXTURES, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDeleteTexturesEXT(GLsizei n, const GLuint* textures) {
    glDeleteTextures(n, textures);
}

void glDeleteVertexArrays(GLsizei n, const GLuint* arrays) {
    GL_CALL_LOCK();
    for (int i = 0; i < n; i++) GLVertexArrayObject_delete(currentGLContext->clientState, arrays[i]);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, n);
    ArrayBuffer_putBytes(&outputBuffer, arrays, n * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DELETE_VERTEX_ARRAYS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDepthFunc(GLenum func) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DEPTH_FUNC, &func, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glDepthMask(GLboolean flag) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DEPTH_MASK, &flag, sizeof(GLboolean));
    GL_CALL_UNLOCK();
}

void glDepthRange(GLdouble n, GLdouble f) {
    glDepthRangef((GLfloat)n, (GLfloat)f);
}

void glDepthRangef(GLfloat n, GLfloat f) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, n);
    ArrayBuffer_putFloat(&outputBuffer, f);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DEPTH_RANGEF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDetachObjectARB(GLhandleARB containerObj, GLhandleARB attachedObj) {
    glDetachShader(containerObj, attachedObj);
}

void glDetachShader(GLuint program, GLuint shader) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, shader);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DETACH_SHADER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDisable(GLenum cap) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DISABLE, &cap, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glDisableClientState(GLenum array) {
    GL_CALL_LOCK();
    GLClientState* clientState = currentGLContext->clientState;
    if (array == GL_TEXTURE_COORD_ARRAY) {
        GLVertexArrayObject_setAttribState(clientState, TEXCOORD_ARRAY_INDEX + clientState->activeTexCoord, VERTEX_ATTRIB_DISABLED, false);
    }
    else GLVertexArrayObject_setAttribState(clientState, array, VERTEX_ATTRIB_DISABLED, false);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DISABLE_CLIENT_STATE, &array, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glDisableClientStateIndexedEXT(GLenum array, GLuint index) {
    if (array != GL_TEXTURE_COORD_ARRAY) return;
    GL_CALL_LOCK();
    int oldIndex = currentGLContext->clientState->activeTexCoord;
    if (oldIndex != index) glClientActiveTexture(GL_TEXTURE0 + index);
    glDisableClientState(array);
    if (oldIndex != index) glClientActiveTexture(GL_TEXTURE0 + oldIndex);
    GL_CALL_UNLOCK();
}

void glDisableClientStateiEXT(GLenum array, GLuint index) {
    glDisableClientStateIndexedEXT(array, index);
}

void glDisableVertexArrayAttribEXT(GLuint vaobj, GLuint index) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glDisableVertexArrayAttribEXT");
}

void glDisableVertexArrayEXT(GLuint vaobj, GLenum array) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glDisableVertexArrayEXT");
}

void glDisableVertexAttribArray(GLuint index) {
    GL_CALL_LOCK();
    GLClientState* clientState = currentGLContext->clientState;
    if (clientState->arbProgram[0]) index += GENERIC_VERTEX_ARRAY_INDEX;
    GLVertexArrayObject_setAttribState(clientState, index, VERTEX_ATTRIB_DISABLED, false);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DISABLE_VERTEX_ATTRIB_ARRAY, &index, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glDisableVertexAttribArrayARB(GLuint index) {
    glDisableVertexAttribArray(index);
}

void glDisablei(GLenum target, GLuint index) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, index);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DISABLEI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDisableiEXT(GLenum target, GLuint index) {
    glDisablei(target, index);
}

void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, num_groups_x);
    ArrayBuffer_putInt(&outputBuffer, num_groups_y);
    ArrayBuffer_putInt(&outputBuffer, num_groups_z);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DISPATCH_COMPUTE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDispatchComputeIndirect(GLintptr indirect) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DISPATCH_COMPUTE_INDIRECT, &indirect, sizeof(GLintptr));
    GL_CALL_UNLOCK();
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    GL_CALL_LOCK();
    flushMappedPersistentBuffers(first, count, NULL, GL_NONE, 0);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, first);
    ArrayBuffer_putInt(&outputBuffer, count);
    writeUnboundVertexArrays(first, count, NULL, GL_NONE, 0);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_ARRAYS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDrawArraysEXT(GLenum mode, GLint first, GLsizei count) {
    glDrawArrays(mode, first, count);
}

void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
    GL_CALL_LOCK();
    flushMappedPersistentBuffers(first, count, NULL, GL_NONE, 0);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, first);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putInt(&outputBuffer, instancecount);
    writeUnboundVertexArrays(first, count, NULL, GL_NONE, 0);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_ARRAYS_INSTANCED, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDrawArraysInstancedARB(GLenum mode, GLint first, GLsizei count, GLsizei primcount) {
    glDrawArraysInstanced(mode, first, count, primcount);
}

void glDrawArraysInstancedEXT(GLenum mode, GLint start, GLsizei count, GLsizei primcount) {
    glDrawArraysInstanced(mode, start, count, primcount);
}

void glDrawBuffer(GLenum buf) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_BUFFER, &buf, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glDrawBuffers(GLsizei n, const GLenum* bufs) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, n);
    ArrayBuffer_putBytes(&outputBuffer, bufs, n * sizeof(GLenum));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_BUFFERS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDrawBuffersARB(GLsizei n, const GLenum* bufs) {
    glDrawBuffers(n, bufs);
}

void glDrawBuffersEXT(GLsizei n, const GLenum* bufs) {
    glDrawBuffers(n, bufs);
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    GL_CALL_LOCK();
    flushMappedPersistentBuffers(0, count, indices, type, 0);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putInt(&outputBuffer, type);
    writeUnboundVertexArrays(0, count, indices, type, 0);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_ELEMENTS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex) {
    GL_CALL_LOCK();
    flushMappedPersistentBuffers(0, count, indices, type, basevertex);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putInt(&outputBuffer, type);
    ArrayBuffer_putInt(&outputBuffer, basevertex);
    writeUnboundVertexArrays(0, count, indices, type, basevertex);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_ELEMENTS_BASE_VERTEX, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDrawElementsBaseVertexEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex) {
    glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
}

void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) {
    GL_CALL_LOCK();
    flushMappedPersistentBuffers(0, count, indices, type, 0);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putInt(&outputBuffer, type);
    ArrayBuffer_putInt(&outputBuffer, instancecount);
    writeUnboundVertexArrays(0, count, indices, type, 0);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_ELEMENTS_INSTANCED, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDrawElementsInstancedARB(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount) {
    glDrawElementsInstanced(mode, count, type, indices, primcount);
}

void glDrawElementsInstancedEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount) {
    glDrawElementsInstanced(mode, count, type, indices, primcount);
}

void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) {
    GL_CALL_LOCK();
    flushMappedPersistentBuffers(0, count, indices, type, basevertex);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putInt(&outputBuffer, type);
    ArrayBuffer_putInt(&outputBuffer, instancecount);
    ArrayBuffer_putInt(&outputBuffer, basevertex);
    writeUnboundVertexArrays(0, count, indices, type, basevertex);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_ELEMENTS_INSTANCED_BASE_VERTEX, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDrawElementsInstancedBaseVertexEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) {
    glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
}

void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, format);
    ArrayBuffer_putInt(&outputBuffer, type);
    GL_SEND_TEXIMAGE(REQUEST_CODE_GL_DRAW_PIXELS, format, type, width, height, 1, pixels, 0);
    GL_CALL_UNLOCK();
}

void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) {
    GL_CALL_LOCK();
    flushMappedPersistentBuffers(0, count, indices, type, 0);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, start);
    ArrayBuffer_putInt(&outputBuffer, end);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putInt(&outputBuffer, type);
    writeUnboundVertexArrays(0, count, indices, type, 0);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_RANGE_ELEMENTS, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDrawRangeElementsEXT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) {
    glDrawRangeElements(mode, start, end, count, type, indices);
}

void glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex) {
    GL_CALL_LOCK();
    flushMappedPersistentBuffers(0, count, indices, type, basevertex);
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, start);
    ArrayBuffer_putInt(&outputBuffer, end);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putInt(&outputBuffer, type);
    ArrayBuffer_putInt(&outputBuffer, basevertex);
    writeUnboundVertexArrays(0, count, indices, type, basevertex);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_DRAW_RANGE_ELEMENTS_BASE_VERTEX, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glDrawRangeElementsBaseVertexEXT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex) {
    glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
}

void glEdgeFlag(GLboolean flag) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_EDGE_FLAG, &flag, sizeof(GLboolean));
    GL_CALL_UNLOCK();
}

void glEdgeFlagPointer(GLsizei stride, const void* pointer) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glEdgeFlagPointer");
}

void glEdgeFlagPointerEXT(GLsizei stride, GLsizei count, const GLboolean* pointer) {
    glEdgeFlagPointer(stride, pointer);
}

void glEdgeFlagv(const GLboolean* flag) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glEdgeFlagv");
}

void glEnable(GLenum cap) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ENABLE, &cap, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glEnableClientState(GLenum array) {
    GL_CALL_LOCK();
    GLClientState* clientState = currentGLContext->clientState;
    if (!GLBuffer_getBound(GL_ARRAY_BUFFER) || !(clientState->program || clientState->arbProgram[0])) {
        if (array == GL_TEXTURE_COORD_ARRAY) {
            GLVertexArrayObject_setAttribState(clientState, TEXCOORD_ARRAY_INDEX + clientState->activeTexCoord, VERTEX_ATTRIB_LEGACY_ENABLED, false);
        }
        else GLVertexArrayObject_setAttribState(clientState, array, VERTEX_ATTRIB_LEGACY_ENABLED, false);        
    }
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ENABLE_CLIENT_STATE, &array, sizeof(GLenum));
    GL_CALL_UNLOCK();    
}

void glEnableClientStateIndexedEXT(GLenum array, GLuint index) {
    if (array != GL_TEXTURE_COORD_ARRAY) return;
    GL_CALL_LOCK();
    int oldIndex = currentGLContext->clientState->activeTexCoord;
    if (oldIndex != index) glClientActiveTexture(GL_TEXTURE0 + index);
    glEnableClientState(array);
    if (oldIndex != index) glClientActiveTexture(GL_TEXTURE0 + oldIndex);
    GL_CALL_UNLOCK();
}

void glEnableClientStateiEXT(GLenum array, GLuint index) {
    glEnableClientStateIndexedEXT(array, index);
}

void glEnableVertexArrayAttribEXT(GLuint vaobj, GLuint index) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glEnableVertexArrayAttribEXT");
}

void glEnableVertexArrayEXT(GLuint vaobj, GLenum array) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glEnableVertexArrayEXT");
}

void glEnableVertexAttribArray(GLuint index) {
    GL_CALL_LOCK();
    if (currentGLContext->clientState->arbProgram[0]) index += GENERIC_VERTEX_ARRAY_INDEX;
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ENABLE_VERTEX_ATTRIB_ARRAY, &index, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glEnableVertexAttribArrayARB(GLuint index) {
    glEnableVertexAttribArray(index);
}

void glEnablei(GLenum target, GLuint index) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, index);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ENABLEI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glEnableiEXT(GLenum target, GLuint index) {
    glEnablei(target, index);
}

void glEnd() {
    GL_CALL_LOCK();
    CommandBuffer* commandBuffer = &currentGLContext->commandBuffer;
    if (commandBuffer->begin) {
        commandBuffer->begin = false;
        
        int indexCount = commandBuffer->minIndex != INT32_MAX ? (commandBuffer->maxIndex - commandBuffer->minIndex) + 1 : 0;
        
        ArrayBuffer_rewind(&outputBuffer);
        ArrayBuffer_putInt(&outputBuffer, commandBuffer->mode);
        ArrayBuffer_putInt(&outputBuffer, commandBuffer->minIndex);
        ArrayBuffer_putInt(&outputBuffer, indexCount);
        ArrayBuffer_putInt(&outputBuffer, commandBuffer->data.size);
        
        if (indexCount > 0) {
            GLClientState* clientState = currentGLContext->clientState;
            for (int i = 0; i < clientState->vao->maxEnabledAttribs; i++) {
                if (clientState->vao->attribs[i].state && clientState->vao->attribs[i].pointer) {
                    void* values = clientState->vao->attribs[i].pointer + commandBuffer->minIndex * clientState->vao->attribs[i].stride;
                    ArrayBuffer_putBytes(&outputBuffer, values, indexCount * clientState->vao->attribs[i].stride);
                }
            }
        }
        
        gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_END, outputBuffer.buffer, outputBuffer.size);
        if (commandBuffer->data.size > 0) {
            RingBuffer_write(currentGLContext->serverRing, commandBuffer->data.buffer, commandBuffer->data.size);
        }
    }
    else gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_END, NULL, 0);
    GL_CALL_UNLOCK();
}

void glEndConditionalRender() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_END_CONDITIONAL_RENDER, NULL, 0);
    GL_CALL_UNLOCK();
}

void glEndList() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_END_LIST, NULL, 0);
    GL_CALL_UNLOCK();
}

void glEndQuery(GLenum target) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_END_QUERY, &target, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glEndQueryARB(GLenum target) {
    glEndQuery(target);
}

void glEndQueryEXT(GLenum target) {
    glEndQuery(target);
}

void glEndTransformFeedback() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_END_TRANSFORM_FEEDBACK, NULL, 0);
    GL_CALL_UNLOCK();
}

void glEndTransformFeedbackEXT() {
    glEndTransformFeedback();
}

void glEvalCoord1d(GLdouble u) {
    glEvalCoord2f((GLfloat)u, 0.0f);
}

void glEvalCoord1dv(const GLdouble* u) {
    glEvalCoord2f((GLfloat)u[0], 0.0f);
}

void glEvalCoord1f(GLfloat u) {
    glEvalCoord2f(u, 0.0f);
}

void glEvalCoord1fv(const GLfloat* u) {
    glEvalCoord2f(u[0], 0.0f);
}

void glEvalCoord2d(GLdouble u, GLdouble v) {
    glEvalCoord2f((GLfloat)u, (GLfloat)v);
}

void glEvalCoord2dv(const GLdouble* u) {
    glEvalCoord2f((GLfloat)u[0], (GLfloat)u[1]);
}

void glEvalCoord2f(GLfloat u, GLfloat v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, u);
    ArrayBuffer_putFloat(&outputBuffer, v);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_EVAL_COORD2F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glEvalCoord2fv(const GLfloat* u) {
    glEvalCoord2f(u[0], u[1]);
}

void glEvalMesh1(GLenum mode, GLint i1, GLint i2) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, i1);
    ArrayBuffer_putInt(&outputBuffer, i2);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_EVAL_MESH1, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, mode);
    ArrayBuffer_putInt(&outputBuffer, i1);
    ArrayBuffer_putInt(&outputBuffer, i2);
    ArrayBuffer_putInt(&outputBuffer, j1);
    ArrayBuffer_putInt(&outputBuffer, j2);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_EVAL_MESH2, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glEvalPoint1(GLint i) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_EVAL_POINT1, &i, sizeof(GLint));
    GL_CALL_UNLOCK();
}

void glEvalPoint2(GLint i, GLint j) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, i);
    ArrayBuffer_putInt(&outputBuffer, j);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_EVAL_POINT2, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, size);
    ArrayBuffer_putInt(&outputBuffer, type);
    ArrayBuffer_putBytes(&outputBuffer, buffer, size * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FEEDBACK_BUFFER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

GLsync glFenceSync(GLenum condition, GLbitfield flags) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, condition);
    ArrayBuffer_putInt(&outputBuffer, flags);
    GL_SEND_CHECKED(REQUEST_CODE_GL_FENCE_SYNC, outputBuffer.buffer, outputBuffer.size, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLsync sync = (GLsync)ArrayBuffer_getLong(&inputBuffer);
    GL_CALL_UNLOCK();
    return sync;
}

void glFinish() {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_FINISH, NULL, 0);
    GL_RECV_CHECKED();
    GL_CALL_UNLOCK();
}

void glFlush() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FLUSH, NULL, 0);
    GL_CALL_UNLOCK();
}

void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) {
    if (target == GL_PIXEL_UNPACK_BUFFER || length == 0) return;
    GL_CALL_LOCK();
    GLBuffer* buffer = GLBuffer_getBound(target);
    if (!buffer) {
        GL_CALL_UNLOCK();
        return;
    }    
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, offset);
    ArrayBuffer_putInt(&outputBuffer, length);
    GL_SEND_CHECKED(REQUEST_CODE_GL_FLUSH_MAPPED_BUFFER_RANGE, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    GL_CALL_UNLOCK();
}

void glFlushMappedBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length) {
    glFlushMappedBufferRange(target, offset, length);
}

void glFlushMappedNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    glFlushMappedBufferRange(target, offset, length);
    GL_DSA_RESTORE_BOUND_BUFFER();
}

void glFogCoordPointer(GLenum type, GLsizei stride, const void* pointer) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glFogCoordPointer");
}

void glFogCoordPointerEXT(GLenum type, GLsizei stride, const void* pointer) {
    glFogCoordPointer(type, stride, pointer);
}

void glFogCoordd(GLdouble coord) {
    glFogCoordf((GLfloat)coord);
}

void glFogCoorddEXT(GLdouble coord) {
    glFogCoordd(coord);
}

void glFogCoorddv(const GLdouble* coord) {
    glFogCoordf((GLfloat)coord[0]);
}

void glFogCoorddvEXT(const GLdouble* coord) {
    glFogCoorddv(coord);
}

void glFogCoordf(GLfloat coord) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FOG_COORDF, &coord, sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glFogCoordfEXT(GLfloat coord) {
    glFogCoordf(coord);
}

void glFogCoordfv(const GLfloat* coord) {
    glFogCoordf(coord[0]);
}

void glFogCoordfvEXT(const GLfloat* coord) {
    glFogCoordfv(coord);
}

void glFogf(GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FOGF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glFogfv(GLenum pname, const GLfloat* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    int paramCount = getGLCallParamsCount(pname, NULL);
    ArrayBuffer_putBytes(&outputBuffer, params, paramCount * sizeof(float));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FOGFV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glFogi(GLenum pname, GLint param) {
    glFogf(pname, (GLfloat)param);
}

void glFogiv(GLenum pname, const GLint* params) {
    PARSE_PARAMSFV(pname, params);
    glFogfv(pname, paramsf);
}

void glFramebufferDrawBufferEXT(GLuint framebuffer, GLenum mode) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glFramebufferDrawBufferEXT");
}

void glFramebufferDrawBuffersEXT(GLuint framebuffer, GLsizei n, const GLenum* bufs) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glFramebufferDrawBuffersEXT");
}

void glFramebufferReadBufferEXT(GLuint framebuffer, GLenum mode) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glFramebufferReadBufferEXT");
}

void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, attachment);
    ArrayBuffer_putInt(&outputBuffer, renderbuffertarget);
    ArrayBuffer_putInt(&outputBuffer, renderbuffer);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FRAMEBUFFER_RENDERBUFFER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glFramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, attachment);
    ArrayBuffer_putInt(&outputBuffer, texture);
    ArrayBuffer_putInt(&outputBuffer, level);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FRAMEBUFFER_TEXTURE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glFramebufferTextureARB(GLenum target, GLenum attachment, GLuint texture, GLint level) {
    glFramebufferTexture(target, attachment, texture, level);
}

void glFramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level) {
    glFramebufferTexture(target, attachment, texture, level);
}

void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture1DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    glFramebufferTexture1D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, attachment);
    ArrayBuffer_putInt(&outputBuffer, textarget);
    ArrayBuffer_putInt(&outputBuffer, texture);
    ArrayBuffer_putInt(&outputBuffer, level);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FRAMEBUFFER_TEXTURE2D, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glFramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) {
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture3DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) {
    glFramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
}

void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, attachment);
    ArrayBuffer_putInt(&outputBuffer, texture);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, layer);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FRAMEBUFFER_TEXTURE_LAYER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glFramebufferTextureLayerARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    glFramebufferTextureLayer(target, attachment, texture, level, layer);
}

void glFramebufferTextureLayerEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    glFramebufferTextureLayer(target, attachment, texture, level, layer);
}

void glFrontFace(GLenum mode) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FRONT_FACE, &mode, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putDouble(&outputBuffer, left);
    ArrayBuffer_putDouble(&outputBuffer, right);
    ArrayBuffer_putDouble(&outputBuffer, bottom);
    ArrayBuffer_putDouble(&outputBuffer, top);
    ArrayBuffer_putDouble(&outputBuffer, zNear);
    ArrayBuffer_putDouble(&outputBuffer, zFar);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_FRUSTUM, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGenBuffers(GLsizei n, GLuint* buffers) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GEN_BUFFERS, &n, sizeof(GLsizei));
    GL_RECV_CHECKED();
    memcpy(buffers, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGenBuffersARB(GLsizei n, GLuint* buffers) {
    glGenBuffers(n, buffers);
}

void glGenFramebuffers(GLsizei n, GLuint* framebuffers) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GEN_FRAMEBUFFERS, &n, sizeof(GLsizei));
    GL_RECV_CHECKED();
    memcpy(framebuffers, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGenFramebuffersEXT(GLsizei n, GLuint* framebuffers) {
    glGenFramebuffers(n, framebuffers);
}

GLuint glGenLists(GLsizei range) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GEN_LISTS, &range, sizeof(GLsizei), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLuint result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

void glGenProgramsARB(GLsizei n, GLuint* programs) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GEN_PROGRAMS_ARB, &n, sizeof(GLsizei));
    GL_RECV_CHECKED();
    memcpy(programs, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGenQueries(GLsizei n, GLuint* ids) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GEN_QUERIES, &n, sizeof(GLsizei));
    GL_RECV_CHECKED();
    memcpy(ids, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGenQueriesARB(GLsizei n, GLuint* ids) {
    glGenQueries(n, ids);
}

void glGenQueriesEXT(GLsizei n, GLuint* ids) {
    glGenQueries(n, ids);
}

void glGenRenderbuffers(GLsizei n, GLuint* renderbuffers) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GEN_RENDERBUFFERS, &n, sizeof(GLsizei));
    GL_RECV_CHECKED();
    memcpy(renderbuffers, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGenRenderbuffersEXT(GLsizei n, GLuint* renderbuffers) {
    glGenRenderbuffers(n, renderbuffers);
}

void glGenSamplers(GLsizei count, GLuint* samplers) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GEN_SAMPLERS, &count, sizeof(GLsizei));
    GL_RECV_CHECKED();
    memcpy(samplers, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGenTextures(GLsizei n, GLuint* textures) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GEN_TEXTURES, &n, sizeof(GLsizei));
    GL_RECV_CHECKED();
    memcpy(textures, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGenTexturesEXT(GLsizei n, GLuint* textures) {
    glGenTextures(n, textures);
}

void glGenVertexArrays(GLsizei n, GLuint* arrays) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GEN_VERTEX_ARRAYS, &n, sizeof(GLsizei));
    GL_RECV_CHECKED();
    memcpy(arrays, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGenerateMipmap(GLenum target) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_GENERATE_MIPMAP, &target, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glGenerateMipmapEXT(GLenum target) {
    glGenerateMipmap(target);
}

void glGenerateMultiTexMipmapEXT(GLenum texunit, GLenum target) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGenerateMipmap(target);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGenerateTextureMipmapEXT(GLuint texture, GLenum target) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glGenerateMipmap(target);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, bufSize);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_ACTIVE_ATTRIB, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    if (length) *length = ArrayBuffer_getInt(&inputBuffer);
    else ArrayBuffer_skip(&inputBuffer, 4);
    if (size) *size = ArrayBuffer_getInt(&inputBuffer);
    else ArrayBuffer_skip(&inputBuffer, 4);
    if (type) *type = ArrayBuffer_getInt(&inputBuffer);
    else ArrayBuffer_skip(&inputBuffer, 4);
    memcpy(name, inputBuffer.buffer + inputBuffer.position, bufSize);
    GL_CALL_UNLOCK();
}

void glGetActiveAttribARB(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLcharARB* name) {
    glGetActiveAttrib(programObj, index, maxLength, length, size, type, name);
}

void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, bufSize);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_ACTIVE_UNIFORM, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    if (length) *length = ArrayBuffer_getInt(&inputBuffer);
    else ArrayBuffer_skip(&inputBuffer, 4);
    if (size) *size = ArrayBuffer_getInt(&inputBuffer);
    else ArrayBuffer_skip(&inputBuffer, 4);
    if (type) *type = ArrayBuffer_getInt(&inputBuffer);
    else ArrayBuffer_skip(&inputBuffer, 4);
    memcpy(name, inputBuffer.buffer + inputBuffer.position, bufSize);
    GL_CALL_UNLOCK();
}

void glGetActiveUniformARB(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLcharARB* name) {
    glGetActiveUniform(programObj, index, maxLength, length, size, type, name);
}

void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetActiveUniformBlockName");
}

void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetActiveUniformBlockiv");
}

void glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformName) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetActiveUniformName");
}

void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetActiveUniformsiv");
}

void glGetAttachedObjectsARB(GLhandleARB containerObj, GLsizei maxCount, GLsizei* count, GLhandleARB* obj) {
    glGetAttachedShaders(containerObj, maxCount, count, obj);
}

void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetAttachedShaders");
}

GLint glGetAttribLocation(GLuint program, const GLchar* name) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putBytes(&outputBuffer, name, strlen(name) + 1);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_ATTRIB_LOCATION, outputBuffer.buffer, outputBuffer.size, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLint result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLint glGetAttribLocationARB(GLhandleARB programObj, const GLcharARB* name) {
    return glGetAttribLocation(programObj, name);
}

void glGetBooleani_v(GLenum target, GLuint index, GLboolean* data) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetBooleani_v");
}

void glGetBooleanv(GLenum pname, GLboolean* data) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_BOOLEANV, &pname, sizeof(GLenum));
    GL_RECV_CHECKED();
    memcpy(data, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params) {
    GLint params32;
    glGetBufferParameteriv(target, pname, &params32);
    *params = params32;
}

void glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params) {
    GL_CALL_LOCK();
    GLBuffer_getParamsv(target, pname, params);
    GL_CALL_UNLOCK();
}

void glGetBufferParameterivARB(GLenum target, GLenum pname, GLint* params) {
    glGetBufferParameteriv(target, pname, params);
}

void glGetBufferPointerv(GLenum target, GLenum pname, void** params) {
    GL_CALL_LOCK();
    GLBuffer* buffer = GLBuffer_getBound(target);
    *params = buffer ? buffer->mappedData : NULL;
    GL_CALL_UNLOCK();
}

void glGetBufferPointervARB(GLenum target, GLenum pname, void** params) {
    glGetBufferPointerv(target, pname, params);
}

void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data) {
    GL_CALL_LOCK();
    GLBuffer* buffer = GLBuffer_getBound(target);
    if (buffer) memcpy(data, buffer->mappedData + offset, size);
    GL_CALL_UNLOCK();
}

void glGetBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, void* data) {
    glGetBufferSubData(target, offset, size, data);
}

void glGetClipPlane(GLenum plane, GLdouble* equation) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetClipPlane");
}

void glGetCompressedMultiTexImageEXT(GLenum texunit, GLenum target, GLint lod, void* img) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetCompressedTexImage(target, lod, img);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetCompressedTexImage(GLenum target, GLint level, void* img) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_COMPRESSED_TEX_IMAGE, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    GLint imageSize = ArrayBuffer_getInt(&inputBuffer);
    if (imageSize > 0) RingBuffer_read(currentGLContext->clientRing, img, imageSize);
    GL_CALL_UNLOCK();
}

void glGetCompressedTexImageARB(GLenum target, GLint level, void* img) {
    glGetCompressedTexImage(target, level, img);
}

void glGetCompressedTextureImageEXT(GLuint texture, GLenum target, GLint lod, void* img) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glGetCompressedTexImage(target, lod, img);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glGetDoubleIndexedvEXT(GLenum target, GLuint index, GLdouble* data) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetDoubleIndexedvEXT");
}

void glGetDoublei_vEXT(GLenum pname, GLuint index, GLdouble* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetDoublei_vEXT");
}

void glGetDoublev(GLenum pname, GLdouble* data) {
    GLfloat dataf;
    glGetFloatv(pname, &dataf);
    *data = dataf;
}

GLenum glGetError() {
    static int noError = -1;
    if (noError == -1) noError = getenv("GLADIO_NO_ERROR") && atoi(getenv("GLADIO_NO_ERROR"));
    if (noError) return GL_NO_ERROR;
    
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_ERROR, NULL, 0, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLenum result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

void glGetFloatIndexedvEXT(GLenum target, GLuint index, GLfloat* data) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetFloatIndexedvEXT");
}

void glGetFloati_vEXT(GLenum pname, GLuint index, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetFloati_vEXT");
}

void glGetFloatv(GLenum pname, GLfloat* data) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_FLOATV, &pname, sizeof(GLenum));
    GL_RECV_CHECKED();
    memcpy(data, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

GLint glGetFragDataIndex(GLuint program, const GLchar* name) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetFragDataIndex");
    return 0;
}

GLint glGetFragDataIndexEXT(GLuint program, const GLchar* name) {
    return glGetFragDataIndex(program, name);
}

GLint glGetFragDataLocation(GLuint program, const GLchar* name) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetFragDataLocation");
    return 0;
}

GLint glGetFragDataLocationEXT(GLuint program, const GLchar* name) {
    return glGetFragDataLocation(program, name);
}

void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, attachment);
    ArrayBuffer_putInt(&outputBuffer, pname);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_FRAMEBUFFER_ATTACHMENT_PARAMETERIV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    *params = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
}

void glGetFramebufferAttachmentParameterivEXT(GLenum target, GLenum attachment, GLenum pname, GLint* params) {
    glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void glGetFramebufferParameterivEXT(GLuint framebuffer, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetFramebufferParameterivEXT");
}

GLhandleARB glGetHandleARB(GLenum pname) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetHandleARB");
    return 0;
}

void glGetInfoLogARB(GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB* infoLog) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetInfoLogARB");
}

void glGetInteger64i_v(GLenum target, GLuint index, GLint64* data) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetInteger64i_v");
}

void glGetInteger64v(GLenum pname, GLint64* data) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetInteger64v");
}

void glGetInteger64vEXT(GLenum pname, GLint64* data) {
    glGetInteger64v(pname, data);
}

void glGetIntegeri_v(GLenum target, GLuint index, GLint* data) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetIntegeri_v");
}

void glGetIntegeri_vEXT(GLenum target, GLuint index, GLint* data) {
    glGetIntegeri_v(target, index, data);
}

void glGetIntegerv(GLenum pname, GLint* data) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_INTEGERV, &pname, sizeof(GLenum));
    GL_RECV_CHECKED();
    memcpy(data, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64* params) {
    GLint data[count]; 
    glGetInternalformativ(target, internalformat, pname, count, data);
    for (int i = 0; i < count; i++) params[i] = data[i];
}

void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putInt(&outputBuffer, count);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_INTERNALFORMATIV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    memcpy(params, inputBuffer.buffer, count * sizeof(GLint));
    GL_CALL_UNLOCK();
}

void glGetLightfv(GLenum light, GLenum pname, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetLightfv");
}

void glGetLightiv(GLenum light, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetLightiv");
}

void glGetMapdv(GLenum target, GLenum query, GLdouble* v) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetMapdv");
}

void glGetMapfv(GLenum target, GLenum query, GLfloat* v) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetMapfv");
}

void glGetMapiv(GLenum target, GLenum query, GLint* v) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetMapiv");
}

void glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetMaterialfv");
}

void glGetMaterialiv(GLenum face, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetMaterialiv");
}

void glGetMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexEnvfv(target, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexEnviv(target, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexGendv(coord, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexGenfv(coord, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, GLint* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexGeniv(coord, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultiTexImageEXT(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, void* pixels) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexImage(target, level, format, type, pixels);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultiTexLevelParameterfvEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexLevelParameterfv(target, level, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultiTexLevelParameterivEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexLevelParameteriv(target, level, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexParameterfv(target, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glGetTexParameteriv(target, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glGetMultisamplefv(GLenum pname, GLuint index, GLfloat* val) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetMultisamplefv");
}

void glGetNamedBufferParameterivEXT(GLuint buffer, GLenum pname, GLint* params) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    glGetBufferParameteriv(target, pname, params);
    GL_DSA_RESTORE_BOUND_BUFFER();
}

void glGetNamedBufferPointervEXT(GLuint buffer, GLenum pname, void** params) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    glGetBufferPointerv(target, pname, params);
    GL_DSA_RESTORE_BOUND_BUFFER();
}

void glGetNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    glGetBufferSubData(target, offset, size, data);
    GL_DSA_RESTORE_BOUND_BUFFER();
}

void glGetNamedFramebufferAttachmentParameterivEXT(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params) {
    GL_DSA_SAVE_BOUND_FRAMEBUFFER(framebuffer);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, pname, params);
    GL_DSA_RESTORE_BOUND_FRAMEBUFFER();
}

void glGetNamedProgramLocalParameterdvEXT(GLuint program, GLenum target, GLuint index, GLdouble* params) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glGetProgramLocalParameterdvARB(target, index, params);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glGetNamedProgramLocalParameterfvEXT(GLuint program, GLenum target, GLuint index, GLfloat* params) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glGetProgramLocalParameterfvARB(target, index, params);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glGetNamedProgramStringEXT(GLuint program, GLenum target, GLenum pname, void* string) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glGetProgramStringARB(target, pname, string);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glGetNamedProgramivEXT(GLuint program, GLenum target, GLenum pname, GLint* params) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glGetProgramivARB(target, pname, params);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glGetNamedRenderbufferParameterivEXT(GLuint renderbuffer, GLenum pname, GLint* params) {
    GL_DSA_SAVE_BOUND_RENDERBUFFER(renderbuffer);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, pname, params);
    GL_DSA_RESTORE_BOUND_RENDERBUFFER();
}

void glGetObjectParameterfvARB(GLhandleARB obj, GLenum pname, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetObjectParameterfvARB");
}

void glGetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetObjectParameterivARB");
}

void glGetPixelMapfv(GLenum map, GLfloat* values) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetPixelMapfv");
}

void glGetPixelMapuiv(GLenum map, GLuint* values) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetPixelMapuiv");
}

void glGetPixelMapusv(GLenum map, GLushort* values) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetPixelMapusv");
}

void glGetPointerIndexedvEXT(GLenum target, GLuint index, void** data) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetPointerIndexedvEXT");
}

void glGetPointeri_vEXT(GLenum pname, GLuint index, void** params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetPointeri_vEXT");
}

void glGetPointerv(GLenum pname, void** params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetPointerv");
}

void glGetPointervEXT(GLenum pname, void** params) {
    glGetPointerv(pname, params);
}

void glGetPolygonStipple(GLubyte* mask) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetPolygonStipple");
}

void glGetProgramEnvParameterdvARB(GLenum target, GLuint index, GLdouble* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetProgramEnvParameterdvARB");
}

void glGetProgramEnvParameterfvARB(GLenum target, GLuint index, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetProgramEnvParameterfvARB");
}

void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, bufSize);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_PROGRAM_INFO_LOG, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();

    int strSize = ArrayBuffer_getInt(&inputBuffer);
    memcpy(infoLog, inputBuffer.buffer + inputBuffer.position, strSize);
    infoLog[strSize] = '\0';
    if (length) *length = strSize;
    GL_CALL_UNLOCK();
}

void glGetProgramLocalParameterdvARB(GLenum target, GLuint index, GLdouble* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetProgramLocalParameterdvARB");
}

void glGetProgramLocalParameterfvARB(GLenum target, GLuint index, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetProgramLocalParameterfvARB");
}

void glGetProgramStringARB(GLenum target, GLenum pname, void* string) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetProgramStringARB");
}

void glGetProgramiv(GLuint program, GLenum pname, GLint* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, pname);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_PROGRAMIV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    *params = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
}

void glGetProgramivARB(GLenum target, GLenum pname, GLint* params) {
    glGetProgramiv(target, pname, params);
}

void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params) {
    GLuint params32;
    glGetQueryObjectuiv(id, pname, &params32);
    *params = params32;
}

void glGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64* params) {
    glGetQueryObjecti64v(id, pname, params);
}

void glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params) {
    glGetQueryObjectuiv(id, pname, (GLuint*)params);
}

void glGetQueryObjectivARB(GLuint id, GLenum pname, GLint* params) {
    glGetQueryObjectiv(id, pname, params);
}

void glGetQueryObjectivEXT(GLuint id, GLenum pname, GLint* params) {
    glGetQueryObjectiv(id, pname, params);
}

void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params) {
    GLuint params32;
    glGetQueryObjectuiv(id, pname, &params32);
    *params = params32;
}

void glGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64* params) {
    glGetQueryObjectui64v(id, pname, params);
}

void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, id);
    ArrayBuffer_putInt(&outputBuffer, pname);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_QUERY_OBJECTUIV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    memcpy(params, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGetQueryObjectuivARB(GLuint id, GLenum pname, GLuint* params) {
    glGetQueryObjectuiv(id, pname, params);
}

void glGetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint* params) {
    glGetQueryObjectuiv(id, pname, params);
}

void glGetQueryiv(GLenum target, GLenum pname, GLint* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, pname);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_QUERYIV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    memcpy(params, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGetQueryivARB(GLenum target, GLenum pname, GLint* params) {
    glGetQueryiv(target, pname, params);
}

void glGetQueryivEXT(GLenum target, GLenum pname, GLint* params) {
    glGetQueryiv(target, pname, params);
}

void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, pname);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_RENDERBUFFER_PARAMETERIV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    *params = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
}

void glGetRenderbufferParameterivEXT(GLenum target, GLenum pname, GLint* params) {
    glGetRenderbufferParameteriv(target, pname, params);
}

void glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint* params) {
    glGetSamplerParameteriv(sampler, pname, params);
}

void glGetSamplerParameterIivEXT(GLuint sampler, GLenum pname, GLint* params) {
    glGetSamplerParameterIiv(sampler, pname, params);
}

void glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint* params) {
    glGetSamplerParameteriv(sampler, pname, (GLint*)params);
}

void glGetSamplerParameterIuivEXT(GLuint sampler, GLenum pname, GLuint* params) {
    glGetSamplerParameterIuiv(sampler, pname, params);
}

void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, sampler);
    ArrayBuffer_putInt(&outputBuffer, pname);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_SAMPLER_PARAMETERFV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    memcpy(params, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params) {
    GLfloat paramsf;
    glGetSamplerParameterfv(sampler, pname, &paramsf);
    *params = (GLint)paramsf;
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, shader);
    ArrayBuffer_putInt(&outputBuffer, bufSize);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_SHADER_INFO_LOG, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();

    int strSize = ArrayBuffer_getInt(&inputBuffer);
    memcpy(infoLog, inputBuffer.buffer + inputBuffer.position, strSize);
    infoLog[strSize] = '\0';
    if (length) *length = strSize;
    GL_CALL_UNLOCK();
}

void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetShaderPrecisionFormat");
}

void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetShaderSource");
}

void glGetShaderSourceARB(GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB* source) {
    glGetShaderSource(obj, maxLength, length, source);
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, shader);
    ArrayBuffer_putInt(&outputBuffer, pname);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_SHADERIV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    *params = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
}

const GLubyte* glGetString(GLenum name) {
    char* cachedString = getCachedString(name);
    if (cachedString) return (const GLubyte*)cachedString;
    
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, name);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_STRING, outputBuffer.buffer, outputBuffer.size, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);

    const GLubyte* result = (const GLubyte*)putCachedString(name, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
    return result;
}

const GLubyte* glGetStringi(GLenum name, GLuint index) {
    static ArrayList cachedValues = {0};
    if (cachedValues.size == 0) strsplit((const char*)glGetString(name), ' ', &cachedValues);
    return index >= 0 && index < cachedValues.size ? cachedValues.elements[index] : NULL;
}

void glGetSynciv(GLsync sync, GLenum pname, GLsizei count, GLsizei* length, GLint* values) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetSynciv");
}

void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetTexEnvfv");
}

void glGetTexEnviv(GLenum target, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetTexEnviv");
}

void glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetTexGendv");
}

void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetTexGenfv");
}

void glGetTexGeniv(GLenum coord, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetTexGeniv");
}

void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, format);
    ArrayBuffer_putInt(&outputBuffer, type);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_TEX_IMAGE, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    GLint imageSize = ArrayBuffer_getInt(&inputBuffer);
    if (imageSize > 0) RingBuffer_read(currentGLContext->clientRing, pixels, imageSize);
    GL_CALL_UNLOCK();
}

void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, pname);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_TEX_LEVEL_PARAMETERFV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    *params = ArrayBuffer_getFloat(&inputBuffer);
    GL_CALL_UNLOCK();
}

void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) {
    GLfloat paramsf;
    glGetTexLevelParameterfv(target, level, pname, &paramsf);
    *params = (GLint)paramsf;
}

void glGetTexParameterIiv(GLenum target, GLenum pname, GLint* params) {
    glGetTexParameteriv(target, pname, params);
}

void glGetTexParameterIivEXT(GLenum target, GLenum pname, GLint* params) {
    glGetTexParameterIiv(target, pname, params);
}

void glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint* params) {
    glGetTexParameteriv(target, pname, (GLint*)params);
}

void glGetTexParameterIuivEXT(GLenum target, GLenum pname, GLuint* params) {
    glGetTexParameterIuiv(target, pname, params);
}

void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, pname);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_TEX_PARAMETERFV, outputBuffer.buffer, outputBuffer.size);
    GL_RECV_CHECKED();
    memcpy(params, inputBuffer.buffer, inputBuffer.size);
    GL_CALL_UNLOCK();
}

void glGetTexParameteriv(GLenum target, GLenum pname, GLint* params) {
    GLfloat paramsf;
    glGetTexParameterfv(target, pname, &paramsf);
    *params = (GLint)paramsf;
}

void glGetTextureImageEXT(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, void* pixels) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glGetTexImage(target, level, format, type, pixels);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glGetTextureLevelParameterfvEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat* params) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glGetTexLevelParameterfv(target, level, pname, params);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glGetTextureLevelParameterivEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLint* params) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glGetTexLevelParameteriv(target, level, pname, params);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glGetTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, GLfloat* params) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glGetTexParameterfv(target, pname, params);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glGetTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, GLint* params) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glGetTexParameteriv(target, pname, params);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetTransformFeedbackVarying");
}

void glGetTransformFeedbackVaryingEXT(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) {
    glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

GLuint glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putBytes(&outputBuffer, uniformBlockName, strlen(uniformBlockName) + 1);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_UNIFORM_BLOCK_INDEX, outputBuffer.buffer, outputBuffer.size, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLuint result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

void glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetUniformIndices");
}

GLint glGetUniformLocation(GLuint program, const GLchar* name) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putBytes(&outputBuffer, name, strlen(name) + 1);
    GL_SEND_CHECKED(REQUEST_CODE_GL_GET_UNIFORM_LOCATION, outputBuffer.buffer, outputBuffer.size, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLint result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLint glGetUniformLocationARB(GLhandleARB programObj, const GLcharARB* name) {
    return glGetUniformLocation(programObj, name);
}

void glGetUniformfv(GLuint program, GLint location, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetUniformfv");
}

void glGetUniformfvARB(GLhandleARB programObj, GLint location, GLfloat* params) {
    glGetUniformfv(programObj, location, params);
}

void glGetUniformiv(GLuint program, GLint location, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetUniformiv");
}

void glGetUniformivARB(GLhandleARB programObj, GLint location, GLint* params) {
    glGetUniformiv(programObj, location, params);
}

void glGetUniformuiv(GLuint program, GLint location, GLuint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetUniformuiv");
}

void glGetUniformuivEXT(GLuint program, GLint location, GLuint* params) {
    glGetUniformuiv(program, location, params);
}

void glGetVertexArrayIntegeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, GLint* param) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexArrayIntegeri_vEXT");
}

void glGetVertexArrayIntegervEXT(GLuint vaobj, GLenum pname, GLint* param) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexArrayIntegervEXT");
}

void glGetVertexArrayPointeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, void** param) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexArrayPointeri_vEXT");
}

void glGetVertexArrayPointervEXT(GLuint vaobj, GLenum pname, void** param) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexArrayPointervEXT");
}

void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexAttribIiv");
}

void glGetVertexAttribIivEXT(GLuint index, GLenum pname, GLint* params) {
    glGetVertexAttribIiv(index, pname, params);
}

void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexAttribIuiv");
}

void glGetVertexAttribIuivEXT(GLuint index, GLenum pname, GLuint* params) {
    glGetVertexAttribIuiv(index, pname, params);
}

void glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexAttribPointerv");
}

void glGetVertexAttribPointervARB(GLuint index, GLenum pname, void** pointer) {
    glGetVertexAttribPointerv(index, pname, pointer);
}

void glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexAttribdv");
}

void glGetVertexAttribdvARB(GLuint index, GLenum pname, GLdouble* params) {
    glGetVertexAttribdv(index, pname, params);
}

void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexAttribfv");
}

void glGetVertexAttribfvARB(GLuint index, GLenum pname, GLfloat* params) {
    glGetVertexAttribfv(index, pname, params);
}

void glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glGetVertexAttribiv");
}

void glGetVertexAttribivARB(GLuint index, GLenum pname, GLint* params) {
    glGetVertexAttribiv(index, pname, params);
}

void glHint(GLenum target, GLenum mode) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, mode);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_HINT, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glIndexMask(GLuint mask) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_INDEX_MASK, &mask, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glIndexPointer(GLenum type, GLsizei stride, const void* pointer) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glIndexPointer");
}

void glIndexPointerEXT(GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    glIndexPointer(type, stride, pointer);
}

void glIndexd(GLdouble c) {
    glIndexf((GLfloat)c);
}

void glIndexdv(const GLdouble* c) {
    glIndexf((GLfloat)c[0]);
}

void glIndexf(GLfloat c) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_INDEXF, &c, sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glIndexfv(const GLfloat* c) {
    glIndexf(c[0]);
}

void glIndexi(GLint c) {
    glIndexf((GLfloat)c);
}

void glIndexiv(const GLint* c) {
    glIndexf((GLfloat)c[0]);
}

void glIndexs(GLshort c) {
    glIndexf((GLfloat)c);
}

void glIndexsv(const GLshort* c) {
    glIndexf((GLfloat)c[0]);
}

void glIndexub(GLubyte c) {
    glIndexf((GLfloat)c);
}

void glIndexubv(const GLubyte* c) {
    glIndexf((GLfloat)c[0]);
}

void glInitNames() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_INIT_NAMES, NULL, 0);
    GL_CALL_UNLOCK();
}

void glInterleavedArrays(GLenum format, GLsizei stride, const void* pointer) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glInterleavedArrays");
}

GLboolean glIsBuffer(GLuint buffer) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_BUFFER, &buffer, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsBufferARB(GLuint buffer) {
    return glIsBuffer(buffer);
}

GLboolean glIsEnabled(GLenum cap) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_ENABLED, &cap, sizeof(GLenum), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsEnabledi(GLenum target, GLuint index) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, index);
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_ENABLEDI, outputBuffer.buffer, outputBuffer.size, GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsEnablediEXT(GLenum target, GLuint index) {
    return glIsEnabledi(target, index);
}

GLboolean glIsFramebuffer(GLuint framebuffer) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_FRAMEBUFFER, &framebuffer, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsFramebufferEXT(GLuint framebuffer) {
    return glIsFramebuffer(framebuffer);
}

GLboolean glIsList(GLuint list) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_LIST, &list, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsProgram(GLuint program) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_PROGRAM, &program, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsProgramARB(GLuint program) {
    return glIsProgram(program);
}

GLboolean glIsQuery(GLuint id) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_QUERY, &id, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsQueryARB(GLuint id) {
    return glIsQuery(id);
}

GLboolean glIsQueryEXT(GLuint id) {
    return glIsQuery(id);
}

GLboolean glIsRenderbuffer(GLuint renderbuffer) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_RENDERBUFFER, &renderbuffer, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsRenderbufferEXT(GLuint renderbuffer) {
    return glIsRenderbuffer(renderbuffer);
}

GLboolean glIsSampler(GLuint sampler) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_SAMPLER, &sampler, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsShader(GLuint shader) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_SHADER, &shader, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsSync(GLsync sync) {
    if (!sync) return GL_FALSE;
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_SYNC, &sync, sizeof(long), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsTexture(GLuint texture) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_TEXTURE, &texture, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

GLboolean glIsTextureEXT(GLuint texture) {
    return glIsTexture(texture);
}

GLboolean glIsVertexArray(GLuint array) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_IS_VERTEX_ARRAY, &array, sizeof(GLuint), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLboolean result = ArrayBuffer_get(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

void glLightModelf(GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LIGHT_MODELF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glLightModelfv(GLenum pname, const GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glLightModelfv");
}

void glLightModeli(GLenum pname, GLint param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putInt(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LIGHT_MODELI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glLightModeliv(GLenum pname, const GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glLightModeliv");
}

void glLightf(GLenum light, GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, light);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LIGHTF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glLightfv(GLenum light, GLenum pname, const GLfloat* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, light);
    ArrayBuffer_putInt(&outputBuffer, pname);
    int paramCount = getGLCallParamsCount(pname, NULL);
    ArrayBuffer_putBytes(&outputBuffer, params, paramCount * sizeof(float));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LIGHTFV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glLighti(GLenum light, GLenum pname, GLint param) {
    glLightf(light, pname, (GLfloat)param);
}

void glLightiv(GLenum light, GLenum pname, const GLint* params) {
    PARSE_PARAMSFV(pname, params);
    glLightfv(light, pname, paramsf);
}

void glLineStipple(GLint factor, GLushort pattern) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, factor);
    ArrayBuffer_putShort(&outputBuffer, pattern);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LINE_STIPPLE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glLineWidth(GLfloat width) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LINE_WIDTH, &width, sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glLinkProgram(GLuint program) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LINK_PROGRAM, &program, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glLinkProgramARB(GLhandleARB programObj) {
    glLinkProgram(programObj);
}

void glListBase(GLuint base) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LIST_BASE, &base, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glLoadIdentity() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LOAD_IDENTITY, NULL, 0);
    GL_CALL_UNLOCK();
}

void glLoadMatrixd(const GLdouble* m) {
    PARSE_MATRIXF(m);
    glLoadMatrixf(mf);
}

void glLoadMatrixf(const GLfloat* m) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LOAD_MATRIXF, m, 16 * sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glLoadName(GLuint name) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LOAD_NAME, &name, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glLoadTransposeMatrixd(const GLdouble* m) {
    PARSE_MATRIXF(m);
    glLoadTransposeMatrixf(mf);
}

void glLoadTransposeMatrixdARB(const GLdouble* m) {
    glLoadTransposeMatrixd(m);
}

void glLoadTransposeMatrixf(const GLfloat* m) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LOAD_TRANSPOSE_MATRIXF, m, 16 * sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glLoadTransposeMatrixfARB(const GLfloat* m) {
    glLoadTransposeMatrixf(m);
}

void glLogicOp(GLenum opcode) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_LOGIC_OP, &opcode, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMap1d");
}

void glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMap1f");
}

void glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMap2d");
}

void glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMap2f");
}

void* glMapBuffer(GLenum target, GLenum access) {
    return glMapBufferRange(target, 0, 0, access);
}

void* glMapBufferARB(GLenum target, GLenum access) {
    return glMapBuffer(target, access);
}

void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
    GL_CALL_LOCK();
    GLBuffer* buffer = GLBuffer_getBound(target);
    if (!buffer || buffer->mapped) {
        GL_CALL_UNLOCK();
        return NULL;
    }

    buffer->access = access;
    buffer->mapOffset = offset;
    buffer->mapLength = length;
    buffer->mapped = true;

    void* result = buffer->mappedData + offset;
    GL_CALL_UNLOCK();
    return result;
}

void* glMapBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
    return glMapBufferRange(target, offset, length, access);
}

void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, un);
    ArrayBuffer_putDouble(&outputBuffer, u1);
    ArrayBuffer_putDouble(&outputBuffer, u2);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MAP_GRID1D, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, un);
    ArrayBuffer_putFloat(&outputBuffer, u1);
    ArrayBuffer_putFloat(&outputBuffer, u2);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MAP_GRID1F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, un);
    ArrayBuffer_putDouble(&outputBuffer, u1);
    ArrayBuffer_putDouble(&outputBuffer, u2);
    ArrayBuffer_putInt(&outputBuffer, vn);
    ArrayBuffer_putDouble(&outputBuffer, v1);
    ArrayBuffer_putDouble(&outputBuffer, v2);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MAP_GRID2D, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, un);
    ArrayBuffer_putFloat(&outputBuffer, u1);
    ArrayBuffer_putFloat(&outputBuffer, u2);
    ArrayBuffer_putInt(&outputBuffer, vn);
    ArrayBuffer_putFloat(&outputBuffer, v1);
    ArrayBuffer_putFloat(&outputBuffer, v2);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MAP_GRID2F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void* glMapNamedBufferEXT(GLuint buffer, GLenum access) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    void* result = glMapBuffer(target, access);
    GL_DSA_RESTORE_BOUND_BUFFER();
    return result;
}

void* glMapNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    void* result = glMapBufferRange(target, offset, length, access);
    GL_DSA_RESTORE_BOUND_BUFFER();
    return result;
}

void glMaterialf(GLenum face, GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, face);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MATERIALF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glMaterialfv(GLenum face, GLenum pname, const GLfloat* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, face);
    ArrayBuffer_putInt(&outputBuffer, pname);
    int paramCount = getGLCallParamsCount(pname, NULL);
    ArrayBuffer_putBytes(&outputBuffer, params, paramCount * sizeof(float));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MATERIALFV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glMateriali(GLenum face, GLenum pname, GLint param) {
    glMaterialf(face, pname, (GLfloat)param);
}

void glMaterialiv(GLenum face, GLenum pname, const GLint* params) {
    PARSE_PARAMSFV(pname, params);
    glMaterialfv(face, pname, paramsf);
}

void glMatrixFrustumEXT(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixFrustumEXT");
}

void glMatrixLoadIdentityEXT(GLenum mode) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixLoadIdentityEXT");
}

void glMatrixLoadTransposedEXT(GLenum mode, const GLdouble* m) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixLoadTransposedEXT");
}

void glMatrixLoadTransposefEXT(GLenum mode, const GLfloat* m) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixLoadTransposefEXT");
}

void glMatrixLoaddEXT(GLenum mode, const GLdouble* m) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixLoaddEXT");
}

void glMatrixLoadfEXT(GLenum mode, const GLfloat* m) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixLoadfEXT");
}

void glMatrixMode(GLenum mode) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MATRIX_MODE, &mode, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glMatrixMultTransposedEXT(GLenum mode, const GLdouble* m) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixMultTransposedEXT");
}

void glMatrixMultTransposefEXT(GLenum mode, const GLfloat* m) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixMultTransposefEXT");
}

void glMatrixMultdEXT(GLenum mode, const GLdouble* m) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixMultdEXT");
}

void glMatrixMultfEXT(GLenum mode, const GLfloat* m) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixMultfEXT");
}

void glMatrixOrthoEXT(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixOrthoEXT");
}

void glMatrixPopEXT(GLenum mode) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixPopEXT");
}

void glMatrixPushEXT(GLenum mode) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixPushEXT");
}

void glMatrixRotatedEXT(GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixRotatedEXT");
}

void glMatrixRotatefEXT(GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixRotatefEXT");
}

void glMatrixScaledEXT(GLenum mode, GLdouble x, GLdouble y, GLdouble z) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixScaledEXT");
}

void glMatrixScalefEXT(GLenum mode, GLfloat x, GLfloat y, GLfloat z) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixScalefEXT");
}

void glMatrixTranslatedEXT(GLenum mode, GLdouble x, GLdouble y, GLdouble z) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixTranslatedEXT");
}

void glMatrixTranslatefEXT(GLenum mode, GLfloat x, GLfloat y, GLfloat z) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMatrixTranslatefEXT");
}

void glMinSampleShading(GLfloat value) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MIN_SAMPLE_SHADING, &value, sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glMinSampleShadingARB(GLfloat value) {
    glMinSampleShading(value);
}

void glMultMatrixd(const GLdouble* m) {
    PARSE_MATRIXF(m);
    glMultMatrixf(mf);
}

void glMultMatrixf(const GLfloat* m) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MULT_MATRIXF, m, 16 * sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glMultTransposeMatrixd(const GLdouble* m) {
    PARSE_MATRIXF(m);
    glMultTransposeMatrixf(mf);
}

void glMultTransposeMatrixdARB(const GLdouble* m) {
    glMultTransposeMatrixd(m);
}

void glMultTransposeMatrixf(const GLfloat* m) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MULT_TRANSPOSE_MATRIXF, m, 16 * sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glMultTransposeMatrixfARB(const GLfloat* m) {
    glMultTransposeMatrixf(m);
}

void glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount) {
    for (int i = 0; i < drawcount; i++) glDrawArrays(mode, first[i], count[i]);
}

void glMultiDrawArraysEXT(GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount) {
    glMultiDrawArrays(mode, first, count, primcount);
}

void glMultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount) {
    for (int i = 0; i < drawcount; i++) glDrawElements(mode, count[i], type, indices[i]);
}

void glMultiDrawElementsEXT(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei primcount) {
    glMultiDrawElements(mode, count, type, indices, primcount);
}

void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glMultiDrawElementsBaseVertex");
}

void glMultiDrawElementsBaseVertexEXT(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex) {
    glMultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
}

void glMultiTexCoord1d(GLenum target, GLdouble s) {
    glMultiTexCoord4f(target, (GLfloat)s, 0.0f, 0.0f, 1.0f);
}

void glMultiTexCoord1dARB(GLenum target, GLdouble s) {
    glMultiTexCoord1d(target, s);
}

void glMultiTexCoord1dv(GLenum target, const GLdouble* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], 0.0f, 0.0f, 1.0f);
}

void glMultiTexCoord1dvARB(GLenum target, const GLdouble* v) {
    glMultiTexCoord1dv(target, v);
}

void glMultiTexCoord1f(GLenum target, GLfloat s) {
    glMultiTexCoord4f(target, s, 0.0f, 0.0f, 1.0f);
}

void glMultiTexCoord1fARB(GLenum target, GLfloat s) {
    glMultiTexCoord1f(target, s);
}

void glMultiTexCoord1fv(GLenum target, const GLfloat* v) {
    glMultiTexCoord4f(target, v[0], 0.0f, 0.0f, 1.0f);
}

void glMultiTexCoord1fvARB(GLenum target, const GLfloat* v) {
    glMultiTexCoord1fv(target, v);
}

void glMultiTexCoord1i(GLenum target, GLint s) {
    glMultiTexCoord4f(target, (GLfloat)s, 0.0f, 0.0f, 1.0f);
}

void glMultiTexCoord1iARB(GLenum target, GLint s) {
    glMultiTexCoord1i(target, s);
}

void glMultiTexCoord1iv(GLenum target, const GLint* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], 0.0f, 0.0f, 1.0f);
}

void glMultiTexCoord1ivARB(GLenum target, const GLint* v) {
    glMultiTexCoord1iv(target, v);
}

void glMultiTexCoord1s(GLenum target, GLshort s) {
    glMultiTexCoord4f(target, (GLfloat)s, 0.0f, 0.0f, 1.0f);
}

void glMultiTexCoord1sARB(GLenum target, GLshort s) {
    glMultiTexCoord1s(target, s);
}

void glMultiTexCoord1sv(GLenum target, const GLshort* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], 0.0f, 0.0f, 1.0f);
}

void glMultiTexCoord1svARB(GLenum target, const GLshort* v) {
    glMultiTexCoord1sv(target, v);
}

void glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t) {
    glMultiTexCoord4f(target, (GLfloat)s, (GLfloat)t, 0.0f, 1.0f);
}

void glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t) {
    glMultiTexCoord2d(target, s, t);
}

void glMultiTexCoord2dv(GLenum target, const GLdouble* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glMultiTexCoord2dvARB(GLenum target, const GLdouble* v) {
    glMultiTexCoord2dv(target, v);
}

void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t) {
    glMultiTexCoord4f(target, s, t, 0.0f, 1.0f);
}

void glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t) {
    glMultiTexCoord2f(target, s, t);
}

void glMultiTexCoord2fv(GLenum target, const GLfloat* v) {
    glMultiTexCoord4f(target, v[0], v[1], 0.0f, 1.0f);
}

void glMultiTexCoord2fvARB(GLenum target, const GLfloat* v) {
    glMultiTexCoord2fv(target, v);
}

void glMultiTexCoord2i(GLenum target, GLint s, GLint t) {
    glMultiTexCoord4f(target, (GLfloat)s, (GLfloat)t, 0.0f, 1.0f);
}

void glMultiTexCoord2iARB(GLenum target, GLint s, GLint t) {
    glMultiTexCoord2i(target, s, t);
}

void glMultiTexCoord2iv(GLenum target, const GLint* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glMultiTexCoord2ivARB(GLenum target, const GLint* v) {
    glMultiTexCoord2iv(target, v);
}

void glMultiTexCoord2s(GLenum target, GLshort s, GLshort t) {
    glMultiTexCoord4f(target, (GLfloat)s, (GLfloat)t, 0.0f, 1.0f);
}

void glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t) {
    glMultiTexCoord2s(target, s, t);
}

void glMultiTexCoord2sv(GLenum target, const GLshort* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glMultiTexCoord2svARB(GLenum target, const GLshort* v) {
    glMultiTexCoord2sv(target, v);
}

void glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r) {
    glMultiTexCoord4f(target, (GLfloat)s, (GLfloat)t, (GLfloat)r, 1.0f);
}

void glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r) {
    glMultiTexCoord3d(target, s, t, r);
}

void glMultiTexCoord3dv(GLenum target, const GLdouble* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glMultiTexCoord3dvARB(GLenum target, const GLdouble* v) {
    glMultiTexCoord3dv(target, v);
}

void glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r) {
    glMultiTexCoord4f(target, s, t, r, 1.0f);
}

void glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r) {
    glMultiTexCoord3f(target, s, t, r);
}

void glMultiTexCoord3fv(GLenum target, const GLfloat* v) {
    glMultiTexCoord4f(target, v[0], v[1], v[2], 1.0f);
}

void glMultiTexCoord3fvARB(GLenum target, const GLfloat* v) {
    glMultiTexCoord3fv(target, v);
}

void glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r) {
    glMultiTexCoord4f(target, (GLfloat)s, (GLfloat)t, (GLfloat)r, 1.0f);
}

void glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r) {
    glMultiTexCoord3i(target, s, t, r);
}

void glMultiTexCoord3iv(GLenum target, const GLint* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glMultiTexCoord3ivARB(GLenum target, const GLint* v) {
    glMultiTexCoord3iv(target, v);
}

void glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r) {
    glMultiTexCoord4f(target, (GLfloat)s, (GLfloat)t, (GLfloat)r, 1.0f);
}

void glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r) {
    glMultiTexCoord3s(target, s, t, r);
}

void glMultiTexCoord3sv(GLenum target, const GLshort* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glMultiTexCoord3svARB(GLenum target, const GLshort* v) {
    glMultiTexCoord3sv(target, v);
}

void glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
    glMultiTexCoord4f(target, (GLfloat)s, (GLfloat)t, (GLfloat)r, (GLfloat)q);
}

void glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
    glMultiTexCoord4d(target, s, t, r, q);
}

void glMultiTexCoord4dv(GLenum target, const GLdouble* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glMultiTexCoord4dvARB(GLenum target, const GLdouble* v) {
    glMultiTexCoord4dv(target, v);
}

void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    GL_CALL_LOCK();
    if (currentGLContext->commandBuffer.begin) {
        ArrayBuffer_putShort(&currentGLContext->commandBuffer.data, REQUEST_CODE_GL_MULTI_TEX_COORD4F);
        ArrayBuffer_putInt(&currentGLContext->commandBuffer.data, target);
        ArrayBuffer_putFloat4(&currentGLContext->commandBuffer.data, s, t, r, q);        
    }
    else {    
        ArrayBuffer_rewind(&outputBuffer);
        ArrayBuffer_putInt(&outputBuffer, target);
        ArrayBuffer_putFloat4(&outputBuffer, s, t, r, q);
        gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_MULTI_TEX_COORD4F, outputBuffer.buffer, outputBuffer.size);
    }
    GL_CALL_UNLOCK();
}

void glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    glMultiTexCoord4f(target, s, t, r, q);
}

void glMultiTexCoord4fv(GLenum target, const GLfloat* v) {
    glMultiTexCoord4f(target, v[0], v[1], v[2], v[3]);
}

void glMultiTexCoord4fvARB(GLenum target, const GLfloat* v) {
    glMultiTexCoord4fv(target, v);
}

void glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q) {
    glMultiTexCoord4f(target, (GLfloat)s, (GLfloat)t, (GLfloat)r, (GLfloat)q);
}

void glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q) {
    glMultiTexCoord4i(target, s, t, r, q);
}

void glMultiTexCoord4iv(GLenum target, const GLint* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glMultiTexCoord4ivARB(GLenum target, const GLint* v) {
    glMultiTexCoord4iv(target, v);
}

void glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) {
    glMultiTexCoord4f(target, (GLfloat)s, (GLfloat)t, (GLfloat)r, (GLfloat)q);
}

void glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) {
    glMultiTexCoord4s(target, s, t, r, q);
}

void glMultiTexCoord4sv(GLenum target, const GLshort* v) {
    glMultiTexCoord4f(target, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glMultiTexCoord4svARB(GLenum target, const GLshort* v) {
    glMultiTexCoord4sv(target, v);
}

void glMultiTexCoordPointerEXT(GLenum texunit, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    GL_CALL_LOCK();
    GLenum oldTexunit = GL_TEXTURE0 + currentGLContext->clientState->activeTexCoord;
    if (oldTexunit != texunit) glClientActiveTexture(texunit);
    glTexCoordPointer(size, type, stride, pointer);
    if (oldTexunit != texunit) glClientActiveTexture(oldTexunit);
    GL_CALL_UNLOCK();
}

void glMultiTexEnvfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexEnvf(target, pname, param);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexEnvfv(target, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexEnviEXT(GLenum texunit, GLenum target, GLenum pname, GLint param) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexEnvi(target, pname, param);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexEnviv(target, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexGendEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble param) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexGend(coord, pname, param);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLdouble* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexGendv(coord, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexGenfEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat param) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexGenf(coord, pname, param);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLfloat* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexGenfv(coord, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexGeniEXT(GLenum texunit, GLenum coord, GLenum pname, GLint param) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexGeni(coord, pname, param);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, const GLint* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexGeniv(coord, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexImage1D(target, level, internalformat, width, border, format, type, pixels);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexParameterfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexParameterf(target, pname, param);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexParameterfv(target, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexParameteriEXT(GLenum texunit, GLenum target, GLenum pname, GLint param) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexParameteri(target, pname, param);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexParameteriv(target, pname, params);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_ACTIVE_TEXTURE(texunit);
    glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    GL_DSA_RESTORE_ACTIVE_TEXTURE();
}

void glNamedBufferDataEXT(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    glBufferData(target, size, data, usage);
    GL_DSA_RESTORE_BOUND_BUFFER();
}

void glNamedBufferStorageEXT(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    glBufferStorage(target, size, data, flags);
    GL_DSA_RESTORE_BOUND_BUFFER();
}

void glNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    glBufferSubData(target, offset, size, data);
    GL_DSA_RESTORE_BOUND_BUFFER();
}

void glNamedCopyBufferSubDataEXT(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glNamedCopyBufferSubDataEXT");
}

void glNamedFramebufferRenderbufferEXT(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    GL_DSA_SAVE_BOUND_FRAMEBUFFER(framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, renderbuffertarget, renderbuffer);
    GL_DSA_RESTORE_BOUND_FRAMEBUFFER();
}

void glNamedFramebufferTexture1DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    GL_DSA_SAVE_BOUND_FRAMEBUFFER(framebuffer);
    glFramebufferTexture1D(GL_FRAMEBUFFER, attachment, textarget, texture, level);
    GL_DSA_RESTORE_BOUND_FRAMEBUFFER();
}

void glNamedFramebufferTexture2DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    GL_DSA_SAVE_BOUND_FRAMEBUFFER(framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textarget, texture, level);
    GL_DSA_RESTORE_BOUND_FRAMEBUFFER();
}

void glNamedFramebufferTexture3DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) {
    GL_DSA_SAVE_BOUND_FRAMEBUFFER(framebuffer);
    glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, textarget, texture, level, zoffset);
    GL_DSA_RESTORE_BOUND_FRAMEBUFFER();
}

void glNamedProgramLocalParameter4dEXT(GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glProgramLocalParameter4dARB(target, index, x, y, z, w);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glNamedProgramLocalParameter4dvEXT(GLuint program, GLenum target, GLuint index, const GLdouble* params) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glProgramLocalParameter4dvARB(target, index, params);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glNamedProgramLocalParameter4fEXT(GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glProgramLocalParameter4fARB(target, index, x, y, z, w);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glNamedProgramLocalParameter4fvEXT(GLuint program, GLenum target, GLuint index, const GLfloat* params) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glProgramLocalParameter4fvARB(target, index, params);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glNamedProgramLocalParameters4fvEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat* params) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glProgramLocalParameters4fvEXT(target, index, count, params);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glNamedProgramStringEXT(GLuint program, GLenum target, GLenum format, GLsizei len, const void* string) {
    GL_DSA_SAVE_BOUND_ARB_PROGRAM(program);
    glProgramStringARB(target, format, len, string);
    GL_DSA_RESTORE_BOUND_ARB_PROGRAM();
}

void glNamedRenderbufferStorageEXT(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height) {
    GL_DSA_SAVE_BOUND_RENDERBUFFER(renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
    GL_DSA_RESTORE_BOUND_RENDERBUFFER();
}

void glNamedRenderbufferStorageMultisampleEXT(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
    GL_DSA_SAVE_BOUND_RENDERBUFFER(renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalformat, width, height);
    GL_DSA_RESTORE_BOUND_RENDERBUFFER();
}

void glNewList(GLuint list, GLenum mode) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, list);
    ArrayBuffer_putInt(&outputBuffer, mode);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_NEW_LIST, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz) {
    glNormal3f((GLfloat)nx, (GLfloat)ny, (GLfloat)nz);
}

void glNormal3bv(const GLbyte* v) {
    glNormal3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz) {
    glNormal3f((GLfloat)nx, (GLfloat)ny, (GLfloat)nz);
}

void glNormal3dv(const GLdouble* v) {
    glNormal3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
    GL_CALL_LOCK();
    if (currentGLContext->commandBuffer.begin) {
        ArrayBuffer_putShort(&currentGLContext->commandBuffer.data, REQUEST_CODE_GL_NORMAL3F);
        ArrayBuffer_putFloat3(&currentGLContext->commandBuffer.data, nx, ny, nz);
    }
    else {    
        ArrayBuffer_rewind(&outputBuffer);
        ArrayBuffer_putFloat3(&outputBuffer, nx, ny, nz);
        gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_NORMAL3F, outputBuffer.buffer, outputBuffer.size);
    }
    GL_CALL_UNLOCK();
}

void glNormal3fv(const GLfloat* v) {
    glNormal3f(v[0], v[1], v[2]);
}

void glNormal3i(GLint nx, GLint ny, GLint nz) {
    glNormal3f((GLfloat)nx, (GLfloat)ny, (GLfloat)nz);
}

void glNormal3iv(const GLint* v) {
    glNormal3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glNormal3s(GLshort nx, GLshort ny, GLshort nz) {
    glNormal3f((GLfloat)nx, (GLfloat)ny, (GLfloat)nz);
}

void glNormal3sv(const GLshort* v) {
    glNormal3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glNormalPointer(GLenum type, GLsizei stride, const void* pointer) {
    const GLint size = 3;
    GL_SEND_VERTEX_ARRAY(REQUEST_CODE_GL_NORMAL_POINTER, NORMAL_ARRAY_INDEX);
}

void glNormalPointerEXT(GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    glNormalPointer(type, stride, pointer);
}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putDouble(&outputBuffer, left);
    ArrayBuffer_putDouble(&outputBuffer, right);
    ArrayBuffer_putDouble(&outputBuffer, bottom);
    ArrayBuffer_putDouble(&outputBuffer, top);
    ArrayBuffer_putDouble(&outputBuffer, zNear);
    ArrayBuffer_putDouble(&outputBuffer, zFar);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ORTHO, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPassThrough(GLfloat token) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PASS_THROUGH, &token, sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glPatchParameterfv(GLenum pname, const GLfloat* values) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    int paramCount = getGLCallParamsCount(pname, NULL);
    ArrayBuffer_putBytes(&outputBuffer, values, paramCount * sizeof(float));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PATCH_PARAMETERFV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPatchParameteri(GLenum pname, GLint value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putInt(&outputBuffer, value);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PATCH_PARAMETERI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPatchParameteriEXT(GLenum pname, GLint value) {
    glPatchParameteri(pname, value);
}

void glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, map);
    ArrayBuffer_putInt(&outputBuffer, mapsize);
    ArrayBuffer_putBytes(&outputBuffer, values, mapsize * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PIXEL_MAPFV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, map);
    ArrayBuffer_putInt(&outputBuffer, mapsize);
    ArrayBuffer_putBytes(&outputBuffer, values, mapsize * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PIXEL_MAPUIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, map);
    ArrayBuffer_putInt(&outputBuffer, mapsize);
    ArrayBuffer_putBytes(&outputBuffer, values, mapsize * sizeof(GLushort));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PIXEL_MAPUSV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPixelStoref(GLenum pname, GLfloat param) {
    glPixelStorei(pname, (GLint)param);
}

void glPixelStorei(GLenum pname, GLint param) {
    GL_CALL_LOCK();
    if (pname == GL_UNPACK_ROW_LENGTH) {
        currentGLContext->clientState->pixelStore.unpackRowLength = param;
    }
    else if (pname == GL_UNPACK_IMAGE_HEIGHT) {
        currentGLContext->clientState->pixelStore.unpackImageHeight = param;
    }
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putInt(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PIXEL_STOREI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPixelTransferf(GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PIXEL_TRANSFERF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPixelTransferi(GLenum pname, GLint param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putInt(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PIXEL_TRANSFERI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPixelZoom(GLfloat xfactor, GLfloat yfactor) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, xfactor);
    ArrayBuffer_putFloat(&outputBuffer, yfactor);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PIXEL_ZOOM, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPointParameterf(GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POINT_PARAMETERF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPointParameterfARB(GLenum pname, GLfloat param) {
    glPointParameterf(pname, param);
}

void glPointParameterfEXT(GLenum pname, GLfloat param) {
    glPointParameterf(pname, param);
}

void glPointParameterfv(GLenum pname, const GLfloat* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    int paramCount = getGLCallParamsCount(pname, NULL);
    ArrayBuffer_putBytes(&outputBuffer, params, paramCount * sizeof(float));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POINT_PARAMETERFV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPointParameterfvARB(GLenum pname, const GLfloat* params) {
    glPointParameterfv(pname, params);
}

void glPointParameterfvEXT(GLenum pname, const GLfloat* params) {
    glPointParameterfv(pname, params);
}

void glPointParameteri(GLenum pname, GLint param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putInt(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POINT_PARAMETERI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPointParameteriv(GLenum pname, const GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glPointParameteriv");
}

void glPointSize(GLfloat size) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POINT_SIZE, &size, sizeof(GLfloat));
    GL_CALL_UNLOCK();
}

void glPolygonMode(GLenum face, GLenum mode) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, face);
    ArrayBuffer_putInt(&outputBuffer, mode);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POLYGON_MODE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPolygonOffset(GLfloat factor, GLfloat units) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, factor);
    ArrayBuffer_putFloat(&outputBuffer, units);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POLYGON_OFFSET, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPolygonOffsetEXT(GLfloat factor, GLfloat bias) {
    glPolygonOffset(factor, bias);
}

void glPolygonStipple(const GLubyte* mask) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glPolygonStipple");
}

void glPopAttrib() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POP_ATTRIB, NULL, 0);
    GL_CALL_UNLOCK();
}

void glPopClientAttrib() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POP_CLIENT_ATTRIB, NULL, 0);
    GL_CALL_UNLOCK();
}

void glPopMatrix() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POP_MATRIX, NULL, 0);
    GL_CALL_UNLOCK();
}

void glPopName() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_POP_NAME, NULL, 0);
    GL_CALL_UNLOCK();
}

void glPrimitiveRestartIndex(GLuint index) {
    if (index != UINT8_MAX && index != UINT16_MAX && index != UINT32_MAX) {
        println("gladio:glPrimitiveRestartIndex: invalid index %u", index);
    }
}

void glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLfloat* priorities) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, n);
    ArrayBuffer_putBytes(&outputBuffer, textures, n * sizeof(GLuint));
    ArrayBuffer_putBytes(&outputBuffer, priorities, n * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PRIORITIZE_TEXTURES, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glPrioritizeTexturesEXT(GLsizei n, const GLuint* textures, const GLclampf* priorities) {
    glPrioritizeTextures(n, textures, priorities);
}

void glProgramEnvParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    glProgramEnvParameter4fARB(target, index, (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glProgramEnvParameter4dvARB(GLenum target, GLuint index, const GLdouble* params) {
    glProgramEnvParameter4fARB(target, index, (GLfloat)params[0], (GLfloat)params[1], (GLfloat)params[2], (GLfloat)params[3]);
}

void glProgramEnvParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    ArrayBuffer_putFloat(&outputBuffer, z);
    ArrayBuffer_putFloat(&outputBuffer, w);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PROGRAM_ENV_PARAMETER4F_ARB, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glProgramEnvParameter4fvARB(GLenum target, GLuint index, const GLfloat* params) {
    glProgramEnvParameter4fARB(target, index, params[0], params[1], params[2], params[3]);
}

void glProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat* params) {
    for (int i = 0; i < count; i++) glProgramEnvParameter4fvARB(target, index + i, params + i * 4);
}

void glProgramLocalParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    glProgramLocalParameter4fARB(target, index, (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glProgramLocalParameter4dvARB(GLenum target, GLuint index, const GLdouble* params) {
    glProgramLocalParameter4fARB(target, index, (GLfloat)params[0], (GLfloat)params[1], (GLfloat)params[2], (GLfloat)params[3]);
}

void glProgramLocalParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    ArrayBuffer_putFloat(&outputBuffer, z);
    ArrayBuffer_putFloat(&outputBuffer, w);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PROGRAM_LOCAL_PARAMETER4F_ARB, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glProgramLocalParameter4fvARB(GLenum target, GLuint index, const GLfloat* params) {
    glProgramLocalParameter4fARB(target, index, params[0], params[1], params[2], params[3]);
}

void glProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat* params) {
    for (int i = 0; i < count; i++) glProgramLocalParameter4fvARB(target, index + i, params + i * 4);
}

void glProgramStringARB(GLenum target, GLenum format, GLsizei len, const void* string) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, format);
    ArrayBuffer_putInt(&outputBuffer, len);
    ArrayBuffer_putBytes(&outputBuffer, string, len);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PROGRAM_STRING_ARB, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glProgramUniform1fEXT(GLuint program, GLint location, GLfloat v0) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform1fEXT");
}

void glProgramUniform1fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform1fvEXT");
}

void glProgramUniform1iEXT(GLuint program, GLint location, GLint v0) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform1iEXT");
}

void glProgramUniform1ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform1ivEXT");
}

void glProgramUniform2fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform2fEXT");
}

void glProgramUniform2fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform2fvEXT");
}

void glProgramUniform2iEXT(GLuint program, GLint location, GLint v0, GLint v1) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform2iEXT");
}

void glProgramUniform2ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform2ivEXT");
}

void glProgramUniform3fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform3fEXT");
}

void glProgramUniform3fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform3fvEXT");
}

void glProgramUniform3iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform3iEXT");
}

void glProgramUniform3ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform3ivEXT");
}

void glProgramUniform4fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform4fEXT");
}

void glProgramUniform4fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform4fvEXT");
}

void glProgramUniform4iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform4iEXT");
}

void glProgramUniform4ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniform4ivEXT");
}

void glProgramUniformMatrix2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniformMatrix2fvEXT");
}

void glProgramUniformMatrix2x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniformMatrix2x3fvEXT");
}

void glProgramUniformMatrix2x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniformMatrix2x4fvEXT");
}

void glProgramUniformMatrix3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniformMatrix3fvEXT");
}

void glProgramUniformMatrix3x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniformMatrix3x2fvEXT");
}

void glProgramUniformMatrix3x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniformMatrix3x4fvEXT");
}

void glProgramUniformMatrix4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniformMatrix4fvEXT");
}

void glProgramUniformMatrix4x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniformMatrix4x2fvEXT");
}

void glProgramUniformMatrix4x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glProgramUniformMatrix4x3fvEXT");
}

void glProvokingVertex(GLenum mode) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PROVOKING_VERTEX, &mode, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glProvokingVertexEXT(GLenum mode) {
    glProvokingVertex(mode);
}

void glPushAttrib(GLbitfield mask) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PUSH_ATTRIB, &mask, sizeof(GLbitfield));
    GL_CALL_UNLOCK();
}

void glPushClientAttrib(GLbitfield mask) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PUSH_CLIENT_ATTRIB, &mask, sizeof(GLbitfield));
    GL_CALL_UNLOCK();
}

void glPushClientAttribDefaultEXT(GLbitfield mask) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glPushClientAttribDefaultEXT");
}

void glPushMatrix() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PUSH_MATRIX, NULL, 0);
    GL_CALL_UNLOCK();
}

void glPushName(GLuint name) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_PUSH_NAME, &name, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glQueryCounter(GLuint id, GLenum target) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, id);
    ArrayBuffer_putInt(&outputBuffer, target);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_QUERY_COUNTER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glQueryCounterEXT(GLuint id, GLenum target) {
    glQueryCounter(id, target);
}

void glRasterPos2d(GLdouble x, GLdouble y) {
    glRasterPos4f((GLfloat)x, (GLfloat)y, 0.0f, 1.0f);
}

void glRasterPos2dv(const GLdouble* v) {
    glRasterPos4f((GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glRasterPos2f(GLfloat x, GLfloat y) {
    glRasterPos4f(x, y, 0.0f, 1.0f);
}

void glRasterPos2fv(const GLfloat* v) {
    glRasterPos4f(v[0], v[1], 0.0f, 1.0f);
}

void glRasterPos2i(GLint x, GLint y) {
    glRasterPos4f((GLfloat)x, (GLfloat)y, 0.0f, 1.0f);
}

void glRasterPos2iv(const GLint* v) {
    glRasterPos4f((GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glRasterPos2s(GLshort x, GLshort y) {
    glRasterPos4f((GLfloat)x, (GLfloat)y, 0.0f, 1.0f);
}

void glRasterPos2sv(const GLshort* v) {
    glRasterPos4f((GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z) {
    glRasterPos4f((GLfloat)x, (GLfloat)y, (GLfloat)z, 1.0f);
}

void glRasterPos3dv(const GLdouble* v) {
    glRasterPos4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z) {
    glRasterPos4f(x, y, z, 1.0f);
}

void glRasterPos3fv(const GLfloat* v) {
    glRasterPos4f(v[0], v[1], v[2], 1.0f);
}

void glRasterPos3i(GLint x, GLint y, GLint z) {
    glRasterPos4f((GLfloat)x, (GLfloat)y, (GLfloat)z, 1.0f);
}

void glRasterPos3iv(const GLint* v) {
    glRasterPos4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glRasterPos3s(GLshort x, GLshort y, GLshort z) {
    glRasterPos4f((GLfloat)x, (GLfloat)y, (GLfloat)z, 1.0f);
}

void glRasterPos3sv(const GLshort* v) {
    glRasterPos4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    glRasterPos4f((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glRasterPos4dv(const GLdouble* v) {
    glRasterPos4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    ArrayBuffer_putFloat(&outputBuffer, z);
    ArrayBuffer_putFloat(&outputBuffer, w);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_RASTER_POS4F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glRasterPos4fv(const GLfloat* v) {
    glRasterPos4f(v[0], v[1], v[2], v[3]);
}

void glRasterPos4i(GLint x, GLint y, GLint z, GLint w) {
    glRasterPos4f((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glRasterPos4iv(const GLint* v) {
    glRasterPos4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w) {
    glRasterPos4f((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glRasterPos4sv(const GLshort* v) {
    glRasterPos4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glReadBuffer(GLenum src) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_READ_BUFFER, &src, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, format);
    ArrayBuffer_putInt(&outputBuffer, type);
    
    GLBuffer* pixelPackBuffer = GLBuffer_getBound(GL_PIXEL_PACK_BUFFER);
    if (pixelPackBuffer) {
        ArrayBuffer_putInt(&outputBuffer, (uint64_t)pixels);
        GL_SEND_CHECKED(REQUEST_CODE_GL_READ_PIXELS, outputBuffer.buffer, outputBuffer.size);
    }
    else {
        GL_SEND_CHECKED(REQUEST_CODE_GL_READ_PIXELS, outputBuffer.buffer, outputBuffer.size);
        GL_RECV_CHECKED();
        GLint imageSize = ArrayBuffer_getInt(&inputBuffer);
        if (imageSize > 0) RingBuffer_read(currentGLContext->clientRing, pixels, imageSize);
    }
    GL_CALL_UNLOCK();
}

void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) {
    glRectf((GLfloat)x1, (GLfloat)y1, (GLfloat)x2, (GLfloat)y2);
}

void glRectdv(const GLdouble* v1, const GLdouble* v2) {
    glRectf((GLfloat)v1[0], (GLfloat)v1[1], (GLfloat)v2[0], (GLfloat)v2[1]);
}

void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
    glBegin(GL_POLYGON);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void glRectfv(const GLfloat* v1, const GLfloat* v2) {
    glRectf(v1[0], v1[1], v2[0], v2[1]);
}

void glRecti(GLint x1, GLint y1, GLint x2, GLint y2) {
    glRectf((GLfloat)x1, (GLfloat)y1, (GLfloat)x2, (GLfloat)y2);
}

void glRectiv(const GLint* v1, const GLint* v2) {
    glRectf((GLfloat)v1[0], (GLfloat)v1[1], (GLfloat)v2[0], (GLfloat)v2[1]);
}

void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2) {
    glRectf((GLfloat)x1, (GLfloat)y1, (GLfloat)x2, (GLfloat)y2);
}

void glRectsv(const GLshort* v1, const GLshort* v2) {
    glRectf((GLfloat)v1[0], (GLfloat)v1[1], (GLfloat)v2[0], (GLfloat)v2[1]);
}

void glReleaseShaderCompiler() {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_RELEASE_SHADER_COMPILER, NULL, 0);
    GL_CALL_UNLOCK();
}

GLint glRenderMode(GLenum mode) {
    GL_CALL_LOCK();
    GL_SEND_CHECKED(REQUEST_CODE_GL_RENDER_MODE, &mode, sizeof(GLenum), GL_RETURN);
    GL_RECV_CHECKED(GL_RETURN);
    GLint result = ArrayBuffer_getInt(&inputBuffer);
    GL_CALL_UNLOCK();
    return result;
}

void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_RENDERBUFFER_STORAGE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glRenderbufferStorageEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    glRenderbufferStorage(target, internalformat, width, height);
}

void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, samples);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_RENDERBUFFER_STORAGE_MULTISAMPLE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glRenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
    glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
    glRotatef((GLfloat)angle, (GLfloat)x, (GLfloat)y, (GLfloat)z);
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, angle);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    ArrayBuffer_putFloat(&outputBuffer, z);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_ROTATEF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glSampleCoverage(GLfloat value, GLboolean invert) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, value);
    ArrayBuffer_put(&outputBuffer, invert);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SAMPLE_COVERAGE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glSampleCoverageARB(GLfloat value, GLboolean invert) {
    glSampleCoverage(value, invert);
}

void glSampleMaski(GLuint maskNumber, GLbitfield mask) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, maskNumber);
    ArrayBuffer_putInt(&outputBuffer, mask);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SAMPLE_MASKI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint* param) {
    PARSE_PARAMSIFV(pname, param);
    glSamplerParameterfv(sampler, pname, paramsf);
}

void glSamplerParameterIivEXT(GLuint sampler, GLenum pname, const GLint* param) {
    glSamplerParameterIiv(sampler, pname, param);
}

void glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint* param) {
    PARSE_PARAMSIFV(pname, param);
    glSamplerParameterfv(sampler, pname, paramsf);
}

void glSamplerParameterIuivEXT(GLuint sampler, GLenum pname, const GLuint* param) {
    glSamplerParameterIuiv(sampler, pname, param);
}

void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, sampler);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SAMPLER_PARAMETERF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, sampler);
    ArrayBuffer_putInt(&outputBuffer, pname);
    int paramCount = getGLCallParamsCount(pname, NULL);
    ArrayBuffer_putBytes(&outputBuffer, param, paramCount * sizeof(float));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SAMPLER_PARAMETERFV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param) {
    glSamplerParameterf(sampler, pname, (GLfloat)param);
}

void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param) {
    PARSE_PARAMSFV(pname, param);
    glSamplerParameterfv(sampler, pname, paramsf);
}

void glScaled(GLdouble x, GLdouble y, GLdouble z) {
    glScalef((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

void glScalef(GLfloat x, GLfloat y, GLfloat z) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    ArrayBuffer_putFloat(&outputBuffer, z);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SCALEF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SCISSOR, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue) {
    glSecondaryColor3f((GLfloat)red, (GLfloat)green, (GLfloat)blue);
}

void glSecondaryColor3bEXT(GLbyte red, GLbyte green, GLbyte blue) {
    glSecondaryColor3b(red, green, blue);
}

void glSecondaryColor3bv(const GLbyte* v) {
    glSecondaryColor3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glSecondaryColor3bvEXT(const GLbyte* v) {
    glSecondaryColor3bv(v);
}

void glSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue) {
    glSecondaryColor3f((GLfloat)red, (GLfloat)green, (GLfloat)blue);
}

void glSecondaryColor3dEXT(GLdouble red, GLdouble green, GLdouble blue) {
    glSecondaryColor3d(red, green, blue);
}

void glSecondaryColor3dv(const GLdouble* v) {
    glSecondaryColor3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glSecondaryColor3dvEXT(const GLdouble* v) {
    glSecondaryColor3dv(v);
}

void glSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, red);
    ArrayBuffer_putFloat(&outputBuffer, green);
    ArrayBuffer_putFloat(&outputBuffer, blue);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SECONDARY_COLOR3F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glSecondaryColor3fEXT(GLfloat red, GLfloat green, GLfloat blue) {
    glSecondaryColor3f(red, green, blue);
}

void glSecondaryColor3fv(const GLfloat* v) {
    glSecondaryColor3f(v[0], v[1], v[2]);
}

void glSecondaryColor3fvEXT(const GLfloat* v) {
    glSecondaryColor3fv(v);
}

void glSecondaryColor3i(GLint red, GLint green, GLint blue) {
    glSecondaryColor3f((GLfloat)red, (GLfloat)green, (GLfloat)blue);
}

void glSecondaryColor3iEXT(GLint red, GLint green, GLint blue) {
    glSecondaryColor3i(red, green, blue);
}

void glSecondaryColor3iv(const GLint* v) {
    glSecondaryColor3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glSecondaryColor3ivEXT(const GLint* v) {
    glSecondaryColor3iv(v);
}

void glSecondaryColor3s(GLshort red, GLshort green, GLshort blue) {
    glSecondaryColor3f((GLfloat)red, (GLfloat)green, (GLfloat)blue);
}

void glSecondaryColor3sEXT(GLshort red, GLshort green, GLshort blue) {
    glSecondaryColor3s(red, green, blue);
}

void glSecondaryColor3sv(const GLshort* v) {
    glSecondaryColor3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glSecondaryColor3svEXT(const GLshort* v) {
    glSecondaryColor3sv(v);
}

void glSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue) {
    glSecondaryColor3f((GLfloat)red, (GLfloat)green, (GLfloat)blue);
}

void glSecondaryColor3ubEXT(GLubyte red, GLubyte green, GLubyte blue) {
    glSecondaryColor3ub(red, green, blue);
}

void glSecondaryColor3ubv(const GLubyte* v) {
    glSecondaryColor3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glSecondaryColor3ubvEXT(const GLubyte* v) {
    glSecondaryColor3ubv(v);
}

void glSecondaryColor3ui(GLuint red, GLuint green, GLuint blue) {
    glSecondaryColor3f((GLfloat)red, (GLfloat)green, (GLfloat)blue);
}

void glSecondaryColor3uiEXT(GLuint red, GLuint green, GLuint blue) {
    glSecondaryColor3ui(red, green, blue);
}

void glSecondaryColor3uiv(const GLuint* v) {
    glSecondaryColor3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glSecondaryColor3uivEXT(const GLuint* v) {
    glSecondaryColor3uiv(v);
}

void glSecondaryColor3us(GLushort red, GLushort green, GLushort blue) {
    glSecondaryColor3f((GLfloat)red, (GLfloat)green, (GLfloat)blue);
}

void glSecondaryColor3usEXT(GLushort red, GLushort green, GLushort blue) {
    glSecondaryColor3us(red, green, blue);
}

void glSecondaryColor3usv(const GLushort* v) {
    glSecondaryColor3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glSecondaryColor3usvEXT(const GLushort* v) {
    glSecondaryColor3usv(v);
}

void glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glSecondaryColorPointer");
}

void glSecondaryColorPointerEXT(GLint size, GLenum type, GLsizei stride, const void* pointer) {
    glSecondaryColorPointer(size, type, stride, pointer);
}

void glSelectBuffer(GLsizei size, GLuint* buffer) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, size);
    ArrayBuffer_putBytes(&outputBuffer, buffer, size * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SELECT_BUFFER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glShadeModel(GLenum mode) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SHADE_MODEL, &mode, sizeof(GLenum));
    GL_CALL_UNLOCK();
}

void glShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryFormat, const void* binary, GLsizei length) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, shaders, count * sizeof(GLuint));
    ArrayBuffer_putInt(&outputBuffer, binaryFormat);
    ArrayBuffer_putBytes(&outputBuffer, binary, length);
    ArrayBuffer_putInt(&outputBuffer, length);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SHADER_BINARY, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, shader);
    ArrayBuffer_putInt(&outputBuffer, count);

    for (int i = 0; i < count; i++) {
        int strSize = length ? length[i] : strlen(string[i]);
        ArrayBuffer_putInt(&outputBuffer, strSize);
        ArrayBuffer_putBytes(&outputBuffer, string[i], strSize);
    }

    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_SHADER_SOURCE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glShaderSourceARB(GLhandleARB shaderObj, GLsizei count, const GLcharARB** string, const GLint* length) {
    glShaderSource(shaderObj, count, string, length);
}

void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, func);
    ArrayBuffer_putInt(&outputBuffer, ref);
    ArrayBuffer_putInt(&outputBuffer, mask);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_STENCIL_FUNC, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, face);
    ArrayBuffer_putInt(&outputBuffer, func);
    ArrayBuffer_putInt(&outputBuffer, ref);
    ArrayBuffer_putInt(&outputBuffer, mask);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_STENCIL_FUNC_SEPARATE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glStencilMask(GLuint mask) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_STENCIL_MASK, &mask, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glStencilMaskSeparate(GLenum face, GLuint mask) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, face);
    ArrayBuffer_putInt(&outputBuffer, mask);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_STENCIL_MASK_SEPARATE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, fail);
    ArrayBuffer_putInt(&outputBuffer, zfail);
    ArrayBuffer_putInt(&outputBuffer, zpass);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_STENCIL_OP, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, face);
    ArrayBuffer_putInt(&outputBuffer, sfail);
    ArrayBuffer_putInt(&outputBuffer, dpfail);
    ArrayBuffer_putInt(&outputBuffer, dppass);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_STENCIL_OP_SEPARATE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, buffer);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_BUFFER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexBufferARB(GLenum target, GLenum internalformat, GLuint buffer) {
    glTexBuffer(target, internalformat, buffer);
}

void glTexBufferEXT(GLenum target, GLenum internalformat, GLuint buffer) {
    glTexBuffer(target, internalformat, buffer);
}

void glTexCoord1d(GLdouble s) {
    glTexCoord4f((GLfloat)s, 0.0f, 0.0f, 1.0f);
}

void glTexCoord1dv(const GLdouble* v) {
    glTexCoord4f((GLfloat)v[0], 0.0f, 0.0f, 1.0f);
}

void glTexCoord1f(GLfloat s) {
    glTexCoord4f(s, 0.0f, 0.0f, 1.0f);
}

void glTexCoord1fv(const GLfloat* v) {
    glTexCoord4f(v[0], 0.0f, 0.0f, 1.0f);
}

void glTexCoord1i(GLint s) {
    glTexCoord4f((GLfloat)s, 0.0f, 0.0f, 1.0f);
}

void glTexCoord1iv(const GLint* v) {
    glTexCoord4f((GLfloat)v[0], 0.0f, 0.0f, 1.0f);
}

void glTexCoord1s(GLshort s) {
    glTexCoord4f((GLfloat)s, 0.0f, 0.0f, 1.0f);
}

void glTexCoord1sv(const GLshort* v) {
    glTexCoord4f((GLfloat)v[0], 0.0f, 0.0f, 1.0f);
}

void glTexCoord2d(GLdouble s, GLdouble t) {
    glTexCoord4f((GLfloat)s, (GLfloat)t, 0.0f, 1.0f);
}

void glTexCoord2dv(const GLdouble* v) {
    glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glTexCoord2f(GLfloat s, GLfloat t) {
    glTexCoord4f(s, t, 0.0f, 1.0f);
}

void glTexCoord2fv(const GLfloat* v) {
    glTexCoord4f(v[0], v[1], 0.0f, 1.0f);
}

void glTexCoord2i(GLint s, GLint t) {
    glTexCoord4f((GLfloat)s, (GLfloat)t, 0.0f, 1.0f);
}

void glTexCoord2iv(const GLint* v) {
    glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glTexCoord2s(GLshort s, GLshort t) {
    glTexCoord4f((GLfloat)s, (GLfloat)t, 0.0f, 1.0f);
}

void glTexCoord2sv(const GLshort* v) {
    glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r) {
    glTexCoord4f((GLfloat)s, (GLfloat)t, (GLfloat)r, 1.0f);
}

void glTexCoord3dv(const GLdouble* v) {
    glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r) {
    glTexCoord4f(s, t, r, 1.0f);
}

void glTexCoord3fv(const GLfloat* v) {
    glTexCoord4f(v[0], v[1], v[2], 1.0f);
}

void glTexCoord3i(GLint s, GLint t, GLint r) {
    glTexCoord4f((GLfloat)s, (GLfloat)t, (GLfloat)r, 1.0f);
}

void glTexCoord3iv(const GLint* v) {
    glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glTexCoord3s(GLshort s, GLshort t, GLshort r) {
    glTexCoord4f((GLfloat)s, (GLfloat)t, (GLfloat)r, 1.0f);
}

void glTexCoord3sv(const GLshort* v) {
    glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
    glTexCoord4f((GLfloat)s, (GLfloat)t, (GLfloat)r, (GLfloat)q);
}

void glTexCoord4dv(const GLdouble* v) {
    glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    GL_CALL_LOCK();
    if (currentGLContext->commandBuffer.begin) {
        ArrayBuffer_putShort(&currentGLContext->commandBuffer.data, REQUEST_CODE_GL_TEX_COORD4F);
        ArrayBuffer_putFloat4(&currentGLContext->commandBuffer.data, s, t, r, q);
    }
    else {
        ArrayBuffer_rewind(&outputBuffer);
        ArrayBuffer_putFloat4(&outputBuffer, s, t, r, q);
        gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_COORD4F, outputBuffer.buffer, outputBuffer.size);
    }
    GL_CALL_UNLOCK();
}

void glTexCoord4fv(const GLfloat* v) {
    glTexCoord4f(v[0], v[1], v[2], v[3]);
}

void glTexCoord4i(GLint s, GLint t, GLint r, GLint q) {
    glTexCoord4f((GLfloat)s, (GLfloat)t, (GLfloat)r, (GLfloat)q);
}

void glTexCoord4iv(const GLint* v) {
    glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q) {
    glTexCoord4f((GLfloat)s, (GLfloat)t, (GLfloat)r, (GLfloat)q);
}

void glTexCoord4sv(const GLshort* v) {
    glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {
    GL_SEND_VERTEX_ARRAY(REQUEST_CODE_GL_TEX_COORD_POINTER, TEXCOORD_ARRAY_INDEX);
}

void glTexCoordPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    glTexCoordPointer(size, type, stride, pointer);
}

void glTexEnvf(GLenum target, GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_ENVF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, pname);
    int paramCount = getGLCallParamsCount(pname, NULL);
    ArrayBuffer_putBytes(&outputBuffer, params, paramCount * sizeof(float));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_ENVFV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexEnvi(GLenum target, GLenum pname, GLint param) {
    glTexEnvf(target, pname, (GLfloat)param);
}

void glTexEnviv(GLenum target, GLenum pname, const GLint* params) {
    PARSE_PARAMSFV(pname, params);
    glTexEnvfv(target, pname, paramsf);
}

void glTexGend(GLenum coord, GLenum pname, GLdouble param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, coord);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putDouble(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_GEND, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexGendv(GLenum coord, GLenum pname, const GLdouble* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glTexGendv");
}

void glTexGenf(GLenum coord, GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, coord);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_GENF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glTexGenfv");
}

void glTexGeni(GLenum coord, GLenum pname, GLint param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, coord);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putInt(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_GENI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexGeniv(GLenum coord, GLenum pname, const GLint* params) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glTexGeniv");
}

void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) {
    glTexImage2D(target, level, internalformat, width, 1, border, format, type, pixels);
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, border);
    ArrayBuffer_putInt(&outputBuffer, format);
    ArrayBuffer_putInt(&outputBuffer, type);
    GL_SEND_TEXIMAGE(REQUEST_CODE_GL_TEX_IMAGE2D, format, type, width, height, 1, pixels, 0);
    GL_CALL_UNLOCK();
}

void glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, samples);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_put(&outputBuffer, fixedsamplelocations);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_IMAGE2DMULTISAMPLE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, depth);
    ArrayBuffer_putInt(&outputBuffer, border);
    ArrayBuffer_putInt(&outputBuffer, format);
    ArrayBuffer_putInt(&outputBuffer, type);
    GL_SEND_TEXIMAGE(REQUEST_CODE_GL_TEX_IMAGE3D, format, type, width, height, depth, pixels, 0);
    GL_CALL_UNLOCK();
}

void glTexImage3DEXT(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
    glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, samples);
    ArrayBuffer_putInt(&outputBuffer, internalformat);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, depth);
    ArrayBuffer_put(&outputBuffer, fixedsamplelocations);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_IMAGE3DMULTISAMPLE, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexParameterIiv(GLenum target, GLenum pname, const GLint* params) {
    PARSE_PARAMSIFV(pname, params);
    glTexParameterfv(target, pname, paramsf);
}

void glTexParameterIivEXT(GLenum target, GLenum pname, const GLint* params) {
    glTexParameterIiv(target, pname, params);
}

void glTexParameterIuiv(GLenum target, GLenum pname, const GLuint* params) {
    PARSE_PARAMSIFV(pname, params);
    glTexParameterfv(target, pname, paramsf);
}

void glTexParameterIuivEXT(GLenum target, GLenum pname, const GLuint* params) {
    glTexParameterIuiv(target, pname, params);
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, pname);
    ArrayBuffer_putFloat(&outputBuffer, param);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_PARAMETERF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, pname);
    int paramCount = getGLCallParamsCount(pname, NULL);
    ArrayBuffer_putBytes(&outputBuffer, params, paramCount * sizeof(float));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TEX_PARAMETERFV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glTexParameteri(GLenum target, GLenum pname, GLint param) {
    glTexParameterf(target, pname, (GLfloat)param);
}

void glTexParameteriv(GLenum target, GLenum pname, const GLint* params) {
    PARSE_PARAMSFV(pname, params);
    glTexParameterfv(target, pname, paramsf);
}

void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) {
    glTexSubImage2D(target, level, xoffset, 0, width, 1, format, type, pixels);
}

void glTexSubImage1DEXT(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) {
    glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, xoffset);
    ArrayBuffer_putInt(&outputBuffer, yoffset);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, format);
    ArrayBuffer_putInt(&outputBuffer, type);
    GL_SEND_TEXIMAGE(REQUEST_CODE_GL_TEX_SUB_IMAGE2D, format, type, width, height, 1, pixels, 0);
    GL_CALL_UNLOCK();
}

void glTexSubImage2DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, target);
    ArrayBuffer_putInt(&outputBuffer, level);
    ArrayBuffer_putInt(&outputBuffer, xoffset);
    ArrayBuffer_putInt(&outputBuffer, yoffset);
    ArrayBuffer_putInt(&outputBuffer, zoffset);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    ArrayBuffer_putInt(&outputBuffer, depth);
    ArrayBuffer_putInt(&outputBuffer, format);
    ArrayBuffer_putInt(&outputBuffer, type);
    GL_SEND_TEXIMAGE(REQUEST_CODE_GL_TEX_SUB_IMAGE3D, format, type, width, height, depth, pixels, 0);
    GL_CALL_UNLOCK();
}

void glTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
    glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void glTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexImage1D(target, level, internalformat, width, border, format, type, pixels);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTextureImage3DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTextureParameterfEXT(GLuint texture, GLenum target, GLenum pname, GLfloat param) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexParameterf(target, pname, param);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, const GLfloat* params) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexParameterfv(target, pname, params);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTextureParameteriEXT(GLuint texture, GLenum target, GLenum pname, GLint param) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexParameteri(target, pname, param);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, const GLint* params) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexParameteriv(target, pname, params);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
    GL_DSA_SAVE_BOUND_TEXTURE(texture);
    glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    GL_DSA_RESTORE_BOUND_TEXTURE();
}

void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glTransformFeedbackVaryings");
}

void glTransformFeedbackVaryingsEXT(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode) {
    glTransformFeedbackVaryings(program, count, varyings, bufferMode);
}

void glTranslated(GLdouble x, GLdouble y, GLdouble z) {
    glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    ArrayBuffer_putFloat(&outputBuffer, z);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_TRANSLATEF, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform1f(GLint location, GLfloat v0) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putFloat(&outputBuffer, v0);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM1F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform1fARB(GLint location, GLfloat v0) {
    glUniform1f(location, v0);
}

void glUniform1fv(GLint location, GLsizei count, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM1FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform1fvARB(GLint location, GLsizei count, const GLfloat* value) {
    glUniform1fv(location, count, value);
}

void glUniform1i(GLint location, GLint v0) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, v0);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM1I, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform1iARB(GLint location, GLint v0) {
    glUniform1i(location, v0);
}

void glUniform1iv(GLint location, GLsizei count, const GLint* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * sizeof(GLint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM1IV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform1ivARB(GLint location, GLsizei count, const GLint* value) {
    glUniform1iv(location, count, value);
}

void glUniform1ui(GLint location, GLuint v0) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, v0);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM1UI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform1uiEXT(GLint location, GLuint v0) {
    glUniform1ui(location, v0);
}

void glUniform1uiv(GLint location, GLsizei count, const GLuint* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM1UIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform1uivEXT(GLint location, GLsizei count, const GLuint* value) {
    glUniform1uiv(location, count, value);
}

void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putFloat(&outputBuffer, v0);
    ArrayBuffer_putFloat(&outputBuffer, v1);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM2F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform2fARB(GLint location, GLfloat v0, GLfloat v1) {
    glUniform2f(location, v0, v1);
}

void glUniform2fv(GLint location, GLsizei count, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 2 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM2FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform2fvARB(GLint location, GLsizei count, const GLfloat* value) {
    glUniform2fv(location, count, value);
}

void glUniform2i(GLint location, GLint v0, GLint v1) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, v0);
    ArrayBuffer_putInt(&outputBuffer, v1);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM2I, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform2iARB(GLint location, GLint v0, GLint v1) {
    glUniform2i(location, v0, v1);
}

void glUniform2iv(GLint location, GLsizei count, const GLint* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 2 * sizeof(GLint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM2IV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform2ivARB(GLint location, GLsizei count, const GLint* value) {
    glUniform2iv(location, count, value);
}

void glUniform2ui(GLint location, GLuint v0, GLuint v1) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, v0);
    ArrayBuffer_putInt(&outputBuffer, v1);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM2UI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform2uiEXT(GLint location, GLuint v0, GLuint v1) {
    glUniform2ui(location, v0, v1);
}

void glUniform2uiv(GLint location, GLsizei count, const GLuint* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 2 * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM2UIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform2uivEXT(GLint location, GLsizei count, const GLuint* value) {
    glUniform2uiv(location, count, value);
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putFloat(&outputBuffer, v0);
    ArrayBuffer_putFloat(&outputBuffer, v1);
    ArrayBuffer_putFloat(&outputBuffer, v2);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM3F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform3fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    glUniform3f(location, v0, v1, v2);
}

void glUniform3fv(GLint location, GLsizei count, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 3 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM3FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform3fvARB(GLint location, GLsizei count, const GLfloat* value) {
    glUniform3fv(location, count, value);
}

void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, v0);
    ArrayBuffer_putInt(&outputBuffer, v1);
    ArrayBuffer_putInt(&outputBuffer, v2);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM3I, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform3iARB(GLint location, GLint v0, GLint v1, GLint v2) {
    glUniform3i(location, v0, v1, v2);
}

void glUniform3iv(GLint location, GLsizei count, const GLint* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 3 * sizeof(GLint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM3IV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform3ivARB(GLint location, GLsizei count, const GLint* value) {
    glUniform3iv(location, count, value);
}

void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, v0);
    ArrayBuffer_putInt(&outputBuffer, v1);
    ArrayBuffer_putInt(&outputBuffer, v2);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM3UI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform3uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2) {
    glUniform3ui(location, v0, v1, v2);
}

void glUniform3uiv(GLint location, GLsizei count, const GLuint* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 3 * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM3UIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform3uivEXT(GLint location, GLsizei count, const GLuint* value) {
    glUniform3uiv(location, count, value);
}

void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putFloat(&outputBuffer, v0);
    ArrayBuffer_putFloat(&outputBuffer, v1);
    ArrayBuffer_putFloat(&outputBuffer, v2);
    ArrayBuffer_putFloat(&outputBuffer, v3);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM4F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform4fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    glUniform4f(location, v0, v1, v2, v3);
}

void glUniform4fv(GLint location, GLsizei count, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 4 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM4FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform4fvARB(GLint location, GLsizei count, const GLfloat* value) {
    glUniform4fv(location, count, value);
}

void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, v0);
    ArrayBuffer_putInt(&outputBuffer, v1);
    ArrayBuffer_putInt(&outputBuffer, v2);
    ArrayBuffer_putInt(&outputBuffer, v3);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM4I, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform4iARB(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    glUniform4i(location, v0, v1, v2, v3);
}

void glUniform4iv(GLint location, GLsizei count, const GLint* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 4 * sizeof(GLint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM4IV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform4ivARB(GLint location, GLsizei count, const GLint* value) {
    glUniform4iv(location, count, value);
}

void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, v0);
    ArrayBuffer_putInt(&outputBuffer, v1);
    ArrayBuffer_putInt(&outputBuffer, v2);
    ArrayBuffer_putInt(&outputBuffer, v3);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM4UI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform4uiEXT(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
    glUniform4ui(location, v0, v1, v2, v3);
}

void glUniform4uiv(GLint location, GLsizei count, const GLuint* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 4 * sizeof(GLuint));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM4UIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniform4uivEXT(GLint location, GLsizei count, const GLuint* value) {
    glUniform4uiv(location, count, value);
}

void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, program);
    ArrayBuffer_putInt(&outputBuffer, uniformBlockIndex);
    ArrayBuffer_putInt(&outputBuffer, uniformBlockBinding);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_BLOCK_BINDING, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_put(&outputBuffer, transpose);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 4 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_MATRIX2FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniformMatrix2fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    glUniformMatrix2fv(location, count, transpose, value);
}

void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_put(&outputBuffer, transpose);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 6 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_MATRIX2X3FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_put(&outputBuffer, transpose);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 8 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_MATRIX2X4FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_put(&outputBuffer, transpose);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 9 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_MATRIX3FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniformMatrix3fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    glUniformMatrix3fv(location, count, transpose, value);
}

void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_put(&outputBuffer, transpose);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 6 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_MATRIX3X2FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_put(&outputBuffer, transpose);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 12 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_MATRIX3X4FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_put(&outputBuffer, transpose);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 16 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_MATRIX4FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniformMatrix4fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    glUniformMatrix4fv(location, count, transpose, value);
}

void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_put(&outputBuffer, transpose);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 8 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_MATRIX4X2FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, location);
    ArrayBuffer_putInt(&outputBuffer, count);
    ArrayBuffer_put(&outputBuffer, transpose);
    ArrayBuffer_putBytes(&outputBuffer, value, count * 12 * sizeof(GLfloat));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_UNIFORM_MATRIX4X3FV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

GLboolean glUnmapBuffer(GLenum target) {
    GL_CALL_LOCK();
    GLBuffer* buffer = GLBuffer_getBound(target);
    if (!buffer) {
        GL_CALL_UNLOCK();
        return GL_FALSE;
    }

    if (buffer->mapLength == 0 && (buffer->access == GL_WRITE_ONLY || buffer->access == GL_READ_WRITE)) {
        glFlushMappedBufferRange(target, 0, buffer->size);
    }
    else if (buffer->mapLength > 0 && (buffer->access & GL_MAP_WRITE_BIT)) {
        glFlushMappedBufferRange(target, buffer->mapOffset, buffer->mapLength);
    }

    buffer->mapOffset = 0;
    buffer->mapLength = 0;
    buffer->access = 0;
    buffer->mapped = false;
    GL_CALL_UNLOCK();
    return GL_TRUE;
}

GLboolean glUnmapBufferARB(GLenum target) {
    return glUnmapBuffer(target);
}

GLboolean glUnmapNamedBufferEXT(GLuint buffer) {
    GL_DSA_SAVE_BOUND_BUFFER(buffer);
    GLboolean result = glUnmapBuffer(target);
    GL_DSA_RESTORE_BOUND_BUFFER();
    return result;
}

void glUseProgram(GLuint program) {
    GL_CALL_LOCK();
    currentGLContext->clientState->program = program;
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_USE_PROGRAM, &program, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glUseProgramObjectARB(GLhandleARB programObj) {
    glUseProgram(programObj);
}

void glValidateProgram(GLuint program) {
    GL_CALL_LOCK();
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VALIDATE_PROGRAM, &program, sizeof(GLuint));
    GL_CALL_UNLOCK();
}

void glValidateProgramARB(GLhandleARB programObj) {
    glValidateProgram(programObj);
}

void glVertex2d(GLdouble x, GLdouble y) {
    glVertex4f((GLfloat)x, (GLfloat)y, 0.0f, 1.0f);
}

void glVertex2dv(const GLdouble* v) {
    glVertex4f((GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glVertex2f(GLfloat x, GLfloat y) {
    glVertex4f(x, y, 0.0f, 1.0f);
}

void glVertex2fv(const GLfloat* v) {
    glVertex4f(v[0], v[1], 0.0f, 1.0f);
}

void glVertex2i(GLint x, GLint y) {
    glVertex4f((GLfloat)x, (GLfloat)y, 0.0f, 1.0f);
}

void glVertex2iv(const GLint* v) {
    glVertex4f((GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glVertex2s(GLshort x, GLshort y) {
    glVertex4f((GLfloat)x, (GLfloat)y, 0.0f, 1.0f);
}

void glVertex2sv(const GLshort* v) {
    glVertex4f((GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f);
}

void glVertex3d(GLdouble x, GLdouble y, GLdouble z) {
    glVertex4f((GLfloat)x, (GLfloat)y, (GLfloat)z, 1.0f);
}

void glVertex3dv(const GLdouble* v) {
    glVertex4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z) {
    glVertex4f(x, y, z, 1.0f);
}

void glVertex3fv(const GLfloat* v) {
    glVertex4f(v[0], v[1], v[2], 1.0f);
}

void glVertex3i(GLint x, GLint y, GLint z) {
    glVertex4f((GLfloat)x, (GLfloat)y, (GLfloat)z, 1.0f);
}

void glVertex3iv(const GLint* v) {
    glVertex4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glVertex3s(GLshort x, GLshort y, GLshort z) {
    glVertex4f((GLfloat)x, (GLfloat)y, (GLfloat)z, 1.0f);
}

void glVertex3sv(const GLshort* v) {
    glVertex4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f);
}

void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    glVertex4f((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glVertex4dv(const GLdouble* v) {
    glVertex4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    GL_CALL_LOCK();
    if (currentGLContext->commandBuffer.begin) {
        ArrayBuffer_putShort(&currentGLContext->commandBuffer.data, REQUEST_CODE_GL_VERTEX4F);
        ArrayBuffer_putFloat4(&currentGLContext->commandBuffer.data, x, y, z, w);        
    }
    else {
        ArrayBuffer_rewind(&outputBuffer);
        ArrayBuffer_putFloat4(&outputBuffer, x, y, z, w);
        gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX4F, outputBuffer.buffer, outputBuffer.size);        
    }
    GL_CALL_UNLOCK();
}

void glVertex4fv(const GLfloat* v) {
    glVertex4f(v[0], v[1], v[2], v[3]);
}

void glVertex4i(GLint x, GLint y, GLint z, GLint w) {
    glVertex4f((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glVertex4iv(const GLint* v) {
    glVertex4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w) {
    glVertex4f((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glVertex4sv(const GLshort* v) {
    glVertex4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertexArrayColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayColorOffsetEXT");
}

void glVertexArrayEdgeFlagOffsetEXT(GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayEdgeFlagOffsetEXT");
}

void glVertexArrayFogCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayFogCoordOffsetEXT");
}

void glVertexArrayIndexOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayIndexOffsetEXT");
}

void glVertexArrayMultiTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayMultiTexCoordOffsetEXT");
}

void glVertexArrayNormalOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayNormalOffsetEXT");
}

void glVertexArraySecondaryColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArraySecondaryColorOffsetEXT");
}

void glVertexArrayTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayTexCoordOffsetEXT");
}

void glVertexArrayVertexAttribDivisorEXT(GLuint vaobj, GLuint index, GLuint divisor) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayVertexAttribDivisorEXT");
}

void glVertexArrayVertexAttribIOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayVertexAttribIOffsetEXT");
}

void glVertexArrayVertexAttribLOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayVertexAttribLOffsetEXT");
}

void glVertexArrayVertexAttribOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayVertexAttribOffsetEXT");
}

void glVertexArrayVertexOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexArrayVertexOffsetEXT");
}

void glVertexAttrib1d(GLuint index, GLdouble x) {
    glVertexAttrib1f(index, (GLfloat)x);
}

void glVertexAttrib1dARB(GLuint index, GLdouble x) {
    glVertexAttrib1d(index, x);
}

void glVertexAttrib1dv(GLuint index, const GLdouble* v) {
    glVertexAttrib1f(index, (GLfloat)v[0]);
}

void glVertexAttrib1dvARB(GLuint index, const GLdouble* v) {
    glVertexAttrib1dv(index, v);
}

void glVertexAttrib1f(GLuint index, GLfloat x) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putFloat(&outputBuffer, x);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB1F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib1fARB(GLuint index, GLfloat x) {
    glVertexAttrib1f(index, x);
}

void glVertexAttrib1fv(GLuint index, const GLfloat* v) {
    glVertexAttrib1f(index, v[0]);
}

void glVertexAttrib1fvARB(GLuint index, const GLfloat* v) {
    glVertexAttrib1fv(index, v);
}

void glVertexAttrib1s(GLuint index, GLshort x) {
    glVertexAttrib1f(index, (GLfloat)x);
}

void glVertexAttrib1sARB(GLuint index, GLshort x) {
    glVertexAttrib1s(index, x);
}

void glVertexAttrib1sv(GLuint index, const GLshort* v) {
    glVertexAttrib1f(index, (GLfloat)v[0]);
}

void glVertexAttrib1svARB(GLuint index, const GLshort* v) {
    glVertexAttrib1sv(index, v);
}

void glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y) {
    glVertexAttrib2f(index, (GLfloat)x, (GLfloat)y);
}

void glVertexAttrib2dARB(GLuint index, GLdouble x, GLdouble y) {
    glVertexAttrib2d(index, x, y);
}

void glVertexAttrib2dv(GLuint index, const GLdouble* v) {
    glVertexAttrib2f(index, (GLfloat)v[0], (GLfloat)v[1]);
}

void glVertexAttrib2dvARB(GLuint index, const GLdouble* v) {
    glVertexAttrib2dv(index, v);
}

void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB2F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib2fARB(GLuint index, GLfloat x, GLfloat y) {
    glVertexAttrib2f(index, x, y);
}

void glVertexAttrib2fv(GLuint index, const GLfloat* v) {
    glVertexAttrib2f(index, v[0], v[1]);
}

void glVertexAttrib2fvARB(GLuint index, const GLfloat* v) {
    glVertexAttrib2fv(index, v);
}

void glVertexAttrib2s(GLuint index, GLshort x, GLshort y) {
    glVertexAttrib2f(index, (GLfloat)x, (GLfloat)y);
}

void glVertexAttrib2sARB(GLuint index, GLshort x, GLshort y) {
    glVertexAttrib2s(index, x, y);
}

void glVertexAttrib2sv(GLuint index, const GLshort* v) {
    glVertexAttrib2f(index, (GLfloat)v[0], (GLfloat)v[1]);
}

void glVertexAttrib2svARB(GLuint index, const GLshort* v) {
    glVertexAttrib2sv(index, v);
}

void glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    glVertexAttrib3f(index, (GLfloat)x, (GLfloat)y, (GLfloat)z);
}

void glVertexAttrib3dARB(GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    glVertexAttrib3d(index, x, y, z);
}

void glVertexAttrib3dv(GLuint index, const GLdouble* v) {
    glVertexAttrib3f(index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glVertexAttrib3dvARB(GLuint index, const GLdouble* v) {
    glVertexAttrib3dv(index, v);
}

void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    ArrayBuffer_putFloat(&outputBuffer, z);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB3F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib3fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
    glVertexAttrib3f(index, x, y, z);
}

void glVertexAttrib3fv(GLuint index, const GLfloat* v) {
    glVertexAttrib3f(index, v[0], v[1], v[2]);
}

void glVertexAttrib3fvARB(GLuint index, const GLfloat* v) {
    glVertexAttrib3fv(index, v);
}

void glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z) {
    glVertexAttrib3f(index, (GLfloat)x, (GLfloat)y, (GLfloat)z);
}

void glVertexAttrib3sARB(GLuint index, GLshort x, GLshort y, GLshort z) {
    glVertexAttrib3s(index, x, y, z);
}

void glVertexAttrib3sv(GLuint index, const GLshort* v) {
    glVertexAttrib3f(index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glVertexAttrib3svARB(GLuint index, const GLshort* v) {
    glVertexAttrib3sv(index, v);
}

void glVertexAttrib4Nbv(GLuint index, const GLbyte* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(char));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB4NBV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib4NbvARB(GLuint index, const GLbyte* v) {
    glVertexAttrib4Nbv(index, v);
}

void glVertexAttrib4Niv(GLuint index, const GLint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB4NIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib4NivARB(GLuint index, const GLint* v) {
    glVertexAttrib4Niv(index, v);
}

void glVertexAttrib4Nsv(GLuint index, const GLshort* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(short));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB4NSV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib4NsvARB(GLuint index, const GLshort* v) {
    glVertexAttrib4Nsv(index, v);
}

void glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_put(&outputBuffer, x);
    ArrayBuffer_put(&outputBuffer, y);
    ArrayBuffer_put(&outputBuffer, z);
    ArrayBuffer_put(&outputBuffer, w);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB4NUB, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib4NubARB(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) {
    glVertexAttrib4Nub(index, x, y, z, w);
}

void glVertexAttrib4Nubv(GLuint index, const GLubyte* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(char));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB4NUBV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib4NubvARB(GLuint index, const GLubyte* v) {
    glVertexAttrib4Nubv(index, v);
}

void glVertexAttrib4Nuiv(GLuint index, const GLuint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB4NUIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib4NuivARB(GLuint index, const GLuint* v) {
    glVertexAttrib4Nuiv(index, v);
}

void glVertexAttrib4Nusv(GLuint index, const GLushort* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(short));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB4NUSV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib4NusvARB(GLuint index, const GLushort* v) {
    glVertexAttrib4Nusv(index, v);
}

void glVertexAttrib4bv(GLuint index, const GLbyte* v) {
    glVertexAttrib4f(index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertexAttrib4bvARB(GLuint index, const GLbyte* v) {
    glVertexAttrib4bv(index, v);
}

void glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    glVertexAttrib4f(index, (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glVertexAttrib4dARB(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    glVertexAttrib4d(index, x, y, z, w);
}

void glVertexAttrib4dv(GLuint index, const GLdouble* v) {
    glVertexAttrib4f(index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertexAttrib4dvARB(GLuint index, const GLdouble* v) {
    glVertexAttrib4dv(index, v);
}

void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    ArrayBuffer_putFloat(&outputBuffer, z);
    ArrayBuffer_putFloat(&outputBuffer, w);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB4F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttrib4fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    glVertexAttrib4f(index, x, y, z, w);
}

void glVertexAttrib4fv(GLuint index, const GLfloat* v) {
    glVertexAttrib4f(index, v[0], v[1], v[2], v[3]);
}

void glVertexAttrib4fvARB(GLuint index, const GLfloat* v) {
    glVertexAttrib4fv(index, v);
}

void glVertexAttrib4iv(GLuint index, const GLint* v) {
    glVertexAttrib4f(index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertexAttrib4ivARB(GLuint index, const GLint* v) {
    glVertexAttrib4iv(index, v);
}

void glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) {
    glVertexAttrib4f(index, (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

void glVertexAttrib4sARB(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) {
    glVertexAttrib4s(index, x, y, z, w);
}

void glVertexAttrib4sv(GLuint index, const GLshort* v) {
    glVertexAttrib4f(index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertexAttrib4svARB(GLuint index, const GLshort* v) {
    glVertexAttrib4sv(index, v);
}

void glVertexAttrib4ubv(GLuint index, const GLubyte* v) {
    glVertexAttrib4f(index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertexAttrib4ubvARB(GLuint index, const GLubyte* v) {
    glVertexAttrib4ubv(index, v);
}

void glVertexAttrib4uiv(GLuint index, const GLuint* v) {
    glVertexAttrib4f(index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertexAttrib4uivARB(GLuint index, const GLuint* v) {
    glVertexAttrib4uiv(index, v);
}

void glVertexAttrib4usv(GLuint index, const GLushort* v) {
    glVertexAttrib4f(index, (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

void glVertexAttrib4usvARB(GLuint index, const GLushort* v) {
    glVertexAttrib4usv(index, v);
}

void glVertexAttribDivisor(GLuint index, GLuint divisor) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, divisor);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_DIVISOR, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribDivisorARB(GLuint index, GLuint divisor) {
    glVertexAttribDivisor(index, divisor);
}

void glVertexAttribDivisorEXT(GLuint index, GLuint divisor) {
    glVertexAttribDivisor(index, divisor);
}

void glVertexAttribI1i(GLuint index, GLint x) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, x);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I1I, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI1iEXT(GLuint index, GLint x) {
    glVertexAttribI1i(index, x);
}

void glVertexAttribI1iv(GLuint index, const GLint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 1 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I1IV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI1ivEXT(GLuint index, const GLint* v) {
    glVertexAttribI1iv(index, v);
}

void glVertexAttribI1ui(GLuint index, GLuint x) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, x);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I1UI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI1uiEXT(GLuint index, GLuint x) {
    glVertexAttribI1ui(index, x);
}

void glVertexAttribI1uiv(GLuint index, const GLuint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 1 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I1UIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI1uivEXT(GLuint index, const GLuint* v) {
    glVertexAttribI1uiv(index, v);
}

void glVertexAttribI2i(GLuint index, GLint x, GLint y) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I2I, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI2iEXT(GLuint index, GLint x, GLint y) {
    glVertexAttribI2i(index, x, y);
}

void glVertexAttribI2iv(GLuint index, const GLint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 2 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I2IV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI2ivEXT(GLuint index, const GLint* v) {
    glVertexAttribI2iv(index, v);
}

void glVertexAttribI2ui(GLuint index, GLuint x, GLuint y) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I2UI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI2uiEXT(GLuint index, GLuint x, GLuint y) {
    glVertexAttribI2ui(index, x, y);
}

void glVertexAttribI2uiv(GLuint index, const GLuint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 2 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I2UIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI2uivEXT(GLuint index, const GLuint* v) {
    glVertexAttribI2uiv(index, v);
}

void glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, z);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I3I, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI3iEXT(GLuint index, GLint x, GLint y, GLint z) {
    glVertexAttribI3i(index, x, y, z);
}

void glVertexAttribI3iv(GLuint index, const GLint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 3 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I3IV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI3ivEXT(GLuint index, const GLint* v) {
    glVertexAttribI3iv(index, v);
}

void glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, z);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I3UI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI3uiEXT(GLuint index, GLuint x, GLuint y, GLuint z) {
    glVertexAttribI3ui(index, x, y, z);
}

void glVertexAttribI3uiv(GLuint index, const GLuint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 3 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I3UIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI3uivEXT(GLuint index, const GLuint* v) {
    glVertexAttribI3uiv(index, v);
}

void glVertexAttribI4bv(GLuint index, const GLbyte* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(char));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I4BV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI4bvEXT(GLuint index, const GLbyte* v) {
    glVertexAttribI4bv(index, v);
}

void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, z);
    ArrayBuffer_putInt(&outputBuffer, w);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I4I, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI4iEXT(GLuint index, GLint x, GLint y, GLint z, GLint w) {
    glVertexAttribI4i(index, x, y, z, w);
}

void glVertexAttribI4iv(GLuint index, const GLint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I4IV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI4ivEXT(GLuint index, const GLint* v) {
    glVertexAttribI4iv(index, v);
}

void glVertexAttribI4sv(GLuint index, const GLshort* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(short));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I4SV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI4svEXT(GLuint index, const GLshort* v) {
    glVertexAttribI4sv(index, v);
}

void glVertexAttribI4ubv(GLuint index, const GLubyte* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(char));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I4UBV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI4ubvEXT(GLuint index, const GLubyte* v) {
    glVertexAttribI4ubv(index, v);
}

void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, z);
    ArrayBuffer_putInt(&outputBuffer, w);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I4UI, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI4uiEXT(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
    glVertexAttribI4ui(index, x, y, z, w);
}

void glVertexAttribI4uiv(GLuint index, const GLuint* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(int));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I4UIV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI4uivEXT(GLuint index, const GLuint* v) {
    glVertexAttribI4uiv(index, v);
}

void glVertexAttribI4usv(GLuint index, const GLushort* v) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putBytes(&outputBuffer, v, 4 * sizeof(short));
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_I4USV, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribI4usvEXT(GLuint index, const GLushort* v) {
    glVertexAttribI4usv(index, v);
}

void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    println(MSG_DEBUG_UNIMPLEMENTED_GLCALL, "glVertexAttribIPointer");
}

void glVertexAttribIPointerEXT(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    glVertexAttribIPointer(index, size, type, stride, pointer);
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
    GL_CALL_LOCK();
    GLClientState* clientState = currentGLContext->clientState;
    GLBuffer* arrayBuffer = GLBuffer_getBound(GL_ARRAY_BUFFER);
    if (index < VERTEX_ATTRIB_COUNT) {
        if (clientState->arbProgram[0]) index += GENERIC_VERTEX_ARRAY_INDEX;
        if (!arrayBuffer || size == GL_BGRA) {
            clientState->vao->attribs[index].stride = stride > 0 ? stride : MIN(4, size) * sizeofGLType(type);
            clientState->vao->attribs[index].pointer = arrayBuffer ? arrayBuffer->mappedData : pointer;
            GLVertexArrayObject_setAttribState(clientState, index, VERTEX_ATTRIB_ENABLED, false);
            if (!arrayBuffer) pointer = (void*)0;
        }
        else GLVertexArrayObject_setAttribState(clientState, index, VERTEX_ATTRIB_DISABLED, true);
    }
    
    if (arrayBuffer && arrayBuffer->mapped) {
        short drawStride = stride > 0 ? stride : (MIN(4, size) * sizeofGLType(type) + (uint64_t)pointer);
        arrayBuffer->drawStride = MAX(drawStride, arrayBuffer->drawStride);
    }

    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, index);
    ArrayBuffer_putInt(&outputBuffer, size);
    ArrayBuffer_putInt(&outputBuffer, type);
    ArrayBuffer_put(&outputBuffer, normalized);
    ArrayBuffer_putInt(&outputBuffer, stride);
    ArrayBuffer_putInt(&outputBuffer, (uint64_t)pointer);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VERTEX_ATTRIB_POINTER, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glVertexAttribPointerARB(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {
    GL_SEND_VERTEX_ARRAY(REQUEST_CODE_GL_VERTEX_POINTER, POSITION_ARRAY_INDEX);
}

void glVertexPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    glVertexPointer(size, type, stride, pointer);
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putInt(&outputBuffer, x);
    ArrayBuffer_putInt(&outputBuffer, y);
    ArrayBuffer_putInt(&outputBuffer, width);
    ArrayBuffer_putInt(&outputBuffer, height);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_VIEWPORT, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putLong(&outputBuffer, (long)sync);
    ArrayBuffer_putInt(&outputBuffer, flags);
    ArrayBuffer_putLong(&outputBuffer, timeout);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_WAIT_SYNC, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glWindowPos2d(GLdouble x, GLdouble y) {
    glWindowPos3f((GLfloat)x, (GLfloat)y, 0.0f);
}

void glWindowPos2dARB(GLdouble x, GLdouble y) {
    glWindowPos2d(x, y);
}

void glWindowPos2dv(const GLdouble* v) {
    glWindowPos3f((GLfloat)v[0], (GLfloat)v[1], 0.0f);
}

void glWindowPos2dvARB(const GLdouble* v) {
    glWindowPos2dv(v);
}

void glWindowPos2f(GLfloat x, GLfloat y) {
    glWindowPos3f(x, y, 0.0f);
}

void glWindowPos2fARB(GLfloat x, GLfloat y) {
    glWindowPos2f(x, y);
}

void glWindowPos2fv(const GLfloat* v) {
    glWindowPos3f(v[0], v[1], 0.0f);
}

void glWindowPos2fvARB(const GLfloat* v) {
    glWindowPos2fv(v);
}

void glWindowPos2i(GLint x, GLint y) {
    glWindowPos3f((GLfloat)x, (GLfloat)y, 0.0f);
}

void glWindowPos2iARB(GLint x, GLint y) {
    glWindowPos2i(x, y);
}

void glWindowPos2iv(const GLint* v) {
    glWindowPos3f((GLfloat)v[0], (GLfloat)v[1], 0.0f);
}

void glWindowPos2ivARB(const GLint* v) {
    glWindowPos2iv(v);
}

void glWindowPos2s(GLshort x, GLshort y) {
    glWindowPos3f((GLfloat)x, (GLfloat)y, 0.0f);
}

void glWindowPos2sARB(GLshort x, GLshort y) {
    glWindowPos2s(x, y);
}

void glWindowPos2sv(const GLshort* v) {
    glWindowPos3f((GLfloat)v[0], (GLfloat)v[1], 0.0f);
}

void glWindowPos2svARB(const GLshort* v) {
    glWindowPos2sv(v);
}

void glWindowPos3d(GLdouble x, GLdouble y, GLdouble z) {
    glWindowPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

void glWindowPos3dARB(GLdouble x, GLdouble y, GLdouble z) {
    glWindowPos3d(x, y, z);
}

void glWindowPos3dv(const GLdouble* v) {
    glWindowPos3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glWindowPos3dvARB(const GLdouble* v) {
    glWindowPos3dv(v);
}

void glWindowPos3f(GLfloat x, GLfloat y, GLfloat z) {
    GL_CALL_LOCK();
    ArrayBuffer_rewind(&outputBuffer);
    ArrayBuffer_putFloat(&outputBuffer, x);
    ArrayBuffer_putFloat(&outputBuffer, y);
    ArrayBuffer_putFloat(&outputBuffer, z);
    gl_send(currentGLContext->serverRing, REQUEST_CODE_GL_WINDOW_POS3F, outputBuffer.buffer, outputBuffer.size);
    GL_CALL_UNLOCK();
}

void glWindowPos3fARB(GLfloat x, GLfloat y, GLfloat z) {
    glWindowPos3f(x, y, z);
}

void glWindowPos3fv(const GLfloat* v) {
    glWindowPos3f(v[0], v[1], v[2]);
}

void glWindowPos3fvARB(const GLfloat* v) {
    glWindowPos3fv(v);
}

void glWindowPos3i(GLint x, GLint y, GLint z) {
    glWindowPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

void glWindowPos3iARB(GLint x, GLint y, GLint z) {
    glWindowPos3i(x, y, z);
}

void glWindowPos3iv(const GLint* v) {
    glWindowPos3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glWindowPos3ivARB(const GLint* v) {
    glWindowPos3iv(v);
}

void glWindowPos3s(GLshort x, GLshort y, GLshort z) {
    glWindowPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

void glWindowPos3sARB(GLshort x, GLshort y, GLshort z) {
    glWindowPos3s(x, y, z);
}

void glWindowPos3sv(const GLshort* v) {
    glWindowPos3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void glWindowPos3svARB(const GLshort* v) {
    glWindowPos3sv(v);
}

