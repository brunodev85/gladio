#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/mman.h>

#include "gladio.h"
#include "gl_context.h"

int serverFd = -1;
ArrayBuffer outputBuffer = {0};
ArrayBuffer inputBuffer = {0};
SparseArray glxContexts = {0};
thread_local GLContext* currentGLContext = NULL;

static SparseArray cachedStrings = {0};

char* getCachedString(int name) {
    return SparseArray_get(&cachedStrings, name);
}

char* putCachedString(int name, char* string, int length) {
    char* cachedString = malloc(length + 1);
    memcpy(cachedString, string, length);
    cachedString[length] = '\0';
    SparseArray_put(&cachedStrings, name, cachedString);
    return cachedString;
}

void getRangeIndices(const void* indices, GLsizei count, GLenum indexType, GLint basevertex, GLuint* range) {
    range[0] = INT32_MAX;
    range[1] = 0;
    switch (indexType) {
        case GL_UNSIGNED_BYTE: {
            GLubyte* values = indices;
            for (int i = 0; i < count; i++) {
                range[0] = MIN(values[i] + basevertex, range[0]);
                range[1] = MAX(values[i] + basevertex, range[1]);
            }
            break;
        }
        case GL_UNSIGNED_SHORT: {
            GLushort* values = indices;
            for (int i = 0; i < count; i++) {
                range[0] = MIN(values[i] + basevertex, range[0]);
                range[1] = MAX(values[i] + basevertex, range[1]);
            }
            break;
        }
        case GL_UNSIGNED_INT: {
            GLuint* values = indices;
            for (int i = 0; i < count; i++) {
                range[0] = MIN(values[i] + basevertex, range[0]);
                range[1] = MAX(values[i] + basevertex, range[1]);
            }
            break;
        }
    }
}

void writeUnboundVertexArrays(GLint first, GLsizei count, const void* indices, GLenum indexType, GLint basevertex) {
    GLClientState* clientState = currentGLContext->clientState;
    if (indexType != GL_NONE) {
        GLBuffer* elementArrayBuffer = GLBuffer_getBound(GL_ELEMENT_ARRAY_BUFFER);
        if (elementArrayBuffer) {
            ArrayBuffer_putInt(&outputBuffer, (uint64_t)indices);
            indices = elementArrayBuffer->mappedData + (uint64_t)indices;
        }
        else ArrayBuffer_putBytes(&outputBuffer, indices, count * sizeofGLType(indexType));

        int indexCount = 0;
        for (int i = 0, j, index; i < clientState->vao->maxEnabledAttribs; i++) {
            if (clientState->vao->attribs[i].state && clientState->vao->attribs[i].pointer) {
                short stride = clientState->vao->attribs[i].stride;
                if (clientState->vao->attribs[i].state == VERTEX_ATTRIB_ENABLED || GLClientState_isLegacyEnabledWithProgram(clientState, i)) {
                    if (indexCount == 0) {
                        GLuint range[2];
                        getRangeIndices(indices, count, indexType, basevertex, range);
                        indexCount = range[1] + 1;
                    }
                    ArrayBuffer_putInt(&outputBuffer, indexCount * stride);
                    ArrayBuffer_putBytes(&outputBuffer, clientState->vao->attribs[i].pointer, indexCount * stride);
                }
                else if (clientState->vao->attribs[i].state == VERTEX_ATTRIB_LEGACY_ENABLED) {
                    for (j = 0; j < count; j++) {
                        switch (indexType) {
                            case GL_UNSIGNED_BYTE:
                                index = ((GLubyte*)indices)[j] + basevertex;
                                break;
                            case GL_UNSIGNED_SHORT:
                                index = ((GLushort*)indices)[j] + basevertex;
                                break;
                            case  GL_UNSIGNED_INT:
                                index = ((GLuint*)indices)[j] + basevertex;
                                break;
                            default:
                                index = 0;
                                break;
                        }

                        ArrayBuffer_putBytes(&outputBuffer, clientState->vao->attribs[i].pointer + (index * stride), stride);
                    }
                }
            }
        }
    }
    else {
        for (int i = 0; i < clientState->vao->maxEnabledAttribs; i++) {
            if (clientState->vao->attribs[i].state && clientState->vao->attribs[i].pointer) {
                bool legacyEnabledWithProgram = clientState->vao->attribs[i].state == VERTEX_ATTRIB_ENABLED && clientState->program > 0;
                short stride = clientState->vao->attribs[i].stride;
                if (clientState->vao->attribs[i].state == VERTEX_ATTRIB_ENABLED || legacyEnabledWithProgram) {
                    ArrayBuffer_putInt(&outputBuffer, (first + count) * stride);
                    ArrayBuffer_putBytes(&outputBuffer, clientState->vao->attribs[i].pointer, (first + count) * stride);
                }
                else if (clientState->vao->attribs[i].state == VERTEX_ATTRIB_LEGACY_ENABLED) {
                    ArrayBuffer_putBytes(&outputBuffer, clientState->vao->attribs[i].pointer + (first * stride), count * stride);
                }
            }
        }
    }
}

bool setupRingBuffers(GLContext* context) {
    if (context->serverRing && context->clientRing) return true;

    int shmFds[2] = {-1, -1};
    int numFds;
    recv_fds(serverFd, shmFds, &numFds, NULL, 0);
    if (numFds != 2) goto error;
    
    context->serverRing = RingBuffer_create(shmFds[0], SERVER_RING_BUFFER_SIZE);
    if (!context->serverRing) goto error;
    
    context->clientRing = RingBuffer_create(shmFds[1], CLIENT_RING_BUFFER_SIZE);
    if (!context->clientRing) goto error;
    
    CLOSEFD(shmFds[0]);
    CLOSEFD(shmFds[1]);
    return true;
    
error:
    if (context->serverRing) {
        RingBuffer_free(context->serverRing);
        context->serverRing = NULL;
    }
    if (context->clientRing) {
        RingBuffer_free(context->clientRing);
        context->clientRing = NULL;
    }
    CLOSEFD(shmFds[0]);
    CLOSEFD(shmFds[1]);    
    return false;
}

static int gladioServerConnect() {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;

    strncpy(server_addr.sun_path, X11_SERVER_PATH, sizeof(server_addr.sun_path) - 1);

    int res;
    do {
        res = 0;
        if (connect(fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) < 0) res = -errno;
    } 
    while (res == -EINTR);    

    if (res < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static bool sendX11AuthRequest() {
    char requestData[12] = {0};
    *(short*)(requestData + 2) = 11;
    
    int result;
    result = sock_write(serverFd, requestData, 12);
    if (result != 12) return false;
    
    char replyData[8] = {0};
    result = sock_read(serverFd, replyData, 8);
    if (result != 8) return false;
    
    int additionalDataLength = *(short*)(replyData + 6) * 4;
    char additionalData[additionalDataLength];
    result = sock_read(serverFd, additionalData, additionalDataLength);
    if (result != additionalDataLength) return false;
    
    return true;
}

bool gladioInitOnce() {
    if (serverFd == -1) {
        serverFd = gladioServerConnect();
        
        if (!sendX11AuthRequest()) {
            CLOSEFD(serverFd);
            return false;
        }
    }
    return serverFd > 0;    
}