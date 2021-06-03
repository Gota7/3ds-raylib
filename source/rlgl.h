
/**********************************************************************************************
*
*   rlgl v3.7 - raylib OpenGL abstraction layer
*
*   rlgl is a wrapper for multiple OpenGL versions (1.1, 2.1, 3.3 Core, ES 2.0) to
*   pseudo-OpenGL 1.1 style functions (rlVertex, rlTranslate, rlRotate...).
*
*   When chosing an OpenGL version greater than OpenGL 1.1, rlgl stores vertex data on internal
*   VBO buffers (and VAOs if available). It requires calling 3 functions:
*       rlglInit()  - Initialize internal buffers and auxiliary resources
*       rlglClose() - De-initialize internal buffers data and other auxiliar resources
*
*   CONFIGURATION:
*
*   #define GRAPHICS_API_OPENGL_11
*   #define GRAPHICS_API_OPENGL_21
*   #define GRAPHICS_API_OPENGL_33
*   #define GRAPHICS_API_OPENGL_ES2
*       Use selected OpenGL graphics backend, should be supported by platform
*       Those preprocessor defines are only used on rlgl module, if OpenGL version is
*       required by any other module, use rlGetVersion() to check it
*
*   #define RLGL_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define RLGL_STANDALONE
*       Use rlgl as standalone library (no raylib dependency)
*
*   #define SUPPORT_GL_DETAILS_INFO
*       Show OpenGL extensions and capabilities detailed logs on init
*
*   DEPENDENCIES:
*       raymath     - 3D math functionality (Vector3, Matrix, Quaternion)
*       GLAD        - OpenGL extensions loading (OpenGL 3.3 Core only)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2021 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#define PICA200 1

#ifndef RLGL_H
#define RLGL_H

#if defined(PICA200)
    #include <3ds.h>
    #include <citro2d.h>
    #include <citro3d.h>
    #include <tex3ds.h>
    #include <stdlib.h>
#endif

#if defined(RLGL_STANDALONE)
    #define RAYMATH_STANDALONE
    #define RAYMATH_HEADER_ONLY

    #define RLAPI   // We are building or using rlgl as a static library (or Linux shared library)

    #if defined(_WIN32)
        #if defined(BUILD_LIBTYPE_SHARED)
            #define RLAPI __declspec(dllexport)         // We are building raylib as a Win32 shared library (.dll)
        #elif defined(USE_LIBTYPE_SHARED)
            #define RLAPI __declspec(dllimport)         // We are using raylib as a Win32 shared library (.dll)
        #endif
    #endif

    // Support TRACELOG macros
    #if !defined(TRACELOG)
        #define TRACELOG(level, ...) (void)0
        #define TRACELOGD(...) (void)0
    #endif

    // Allow custom memory allocators
    #ifndef RL_MALLOC
        #define RL_MALLOC(sz)       malloc(sz)
    #endif
    #ifndef RL_CALLOC
        #define RL_CALLOC(n,sz)     calloc(n,sz)
    #endif
    #ifndef RL_REALLOC
        #define RL_REALLOC(n,sz)    realloc(n,sz)
    #endif
    #ifndef RL_FREE
        #define RL_FREE(p)          free(p)
    #endif
#else
    #include "raylib.h"         // Required for: Shader, Texture2D
#endif

#include "raymath.h"            // Required for: Vector3, Matrix

#if defined(PICA200)
    #include <3ds.h>
    #include <citro2d.h>
    #include <citro3d.h>
    #include "vshader_shbin.h"
#endif

// Security check in case no GRAPHICS_API_OPENGL_* defined
#if !defined(GRAPHICS_API_OPENGL_11) && \
    !defined(GRAPHICS_API_OPENGL_21) && \
    !defined(GRAPHICS_API_OPENGL_33) && \
    !defined(GRAPHICS_API_OPENGL_ES2) && \
    !defined(PICA200)
        #define GRAPHICS_API_OPENGL_33
#endif

// Security check in case multiple GRAPHICS_API_OPENGL_* defined
#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(GRAPHICS_API_OPENGL_21)
        #undef GRAPHICS_API_OPENGL_21
    #endif
    #if defined(GRAPHICS_API_OPENGL_33)
        #undef GRAPHICS_API_OPENGL_33
    #endif
    #if defined(GRAPHICS_API_OPENGL_ES2)
        #undef GRAPHICS_API_OPENGL_ES2
    #endif
#endif

// OpenGL 2.1 uses most of OpenGL 3.3 Core functionality
// WARNING: Specific parts are checked with #if defines
#if defined(GRAPHICS_API_OPENGL_21)
    #define GRAPHICS_API_OPENGL_33
#endif

#define SUPPORT_RENDER_TEXTURES_HINT

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Default internal render batch limits
#ifndef DEFAULT_BATCH_BUFFER_ELEMENTS
    #if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
        // This is the maximum amount of elements (quads) per batch
        // NOTE: Be careful with text, every letter maps to a quad
        #define DEFAULT_BATCH_BUFFER_ELEMENTS   8192
    #endif
    #if defined(GRAPHICS_API_OPENGL_ES2)
        // We reduce memory sizes for embedded systems (RPI and HTML5)
        // NOTE: On HTML5 (emscripten) this is allocated on heap,
        // by default it's only 16MB!...just take care...
        #define DEFAULT_BATCH_BUFFER_ELEMENTS   2048
    #endif
    #if defined(PICA200)
        #define DEFAULT_BATCH_BUFFER_ELEMENTS   2048 // RANDOM NUMBER, IDK HOW MUCH IT IS!!!
    #endif
#endif
#ifndef DEFAULT_BATCH_BUFFERS
    #define DEFAULT_BATCH_BUFFERS            1      // Default number of batch buffers (multi-buffering)
#endif
#ifndef DEFAULT_BATCH_DRAWCALLS
    #define DEFAULT_BATCH_DRAWCALLS        256      // Default number of batch draw calls (by state changes: mode, texture)
#endif
#ifndef MAX_BATCH_ACTIVE_TEXTURES
    #define MAX_BATCH_ACTIVE_TEXTURES        4      // Maximum number of additional textures that can be activated on batch drawing (SetShaderValueTexture())
#endif

// Internal Matrix stack
#ifndef MAX_MATRIX_STACK_SIZE
    #define MAX_MATRIX_STACK_SIZE           32      // Maximum size of Matrix stack
#endif

// Vertex buffers id limit
#ifndef MAX_MESH_VERTEX_BUFFERS
    #define MAX_MESH_VERTEX_BUFFERS          7      // Maximum vertex buffers (VBO) per mesh
#endif

// Shader and material limits
#ifndef MAX_SHADER_LOCATIONS
    #define MAX_SHADER_LOCATIONS            32      // Maximum number of shader locations supported
#endif
#ifndef MAX_MATERIAL_MAPS
    #define MAX_MATERIAL_MAPS               12      // Maximum number of shader maps supported
#endif

// Projection matrix culling
#ifndef RL_CULL_DISTANCE_NEAR
    #define RL_CULL_DISTANCE_NEAR         0.01      // Default near cull distance
#endif
#ifndef RL_CULL_DISTANCE_FAR
    #define RL_CULL_DISTANCE_FAR        1000.0      // Default far cull distance
#endif

// Texture parameters (equivalent to OpenGL defines)
#define RL_TEXTURE_WRAP_S                       0x2802      // GL_TEXTURE_WRAP_S
#define RL_TEXTURE_WRAP_T                       0x2803      // GL_TEXTURE_WRAP_T
#define RL_TEXTURE_MAG_FILTER                   0x2800      // GL_TEXTURE_MAG_FILTER
#define RL_TEXTURE_MIN_FILTER                   0x2801      // GL_TEXTURE_MIN_FILTER

#define RL_TEXTURE_FILTER_NEAREST               0x2600      // GL_NEAREST
#define RL_TEXTURE_FILTER_LINEAR                0x2601      // GL_LINEAR
#define RL_TEXTURE_FILTER_MIP_NEAREST           0x2700      // GL_NEAREST_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR    0x2702      // GL_NEAREST_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST    0x2701      // GL_LINEAR_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_MIP_LINEAR            0x2703      // GL_LINEAR_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_ANISOTROPIC           0x3000      // Anisotropic filter (custom identifier)

#define RL_TEXTURE_WRAP_REPEAT                  0x2901      // GL_REPEAT
#define RL_TEXTURE_WRAP_CLAMP                   0x812F      // GL_CLAMP_TO_EDGE
#define RL_TEXTURE_WRAP_MIRROR_REPEAT           0x8370      // GL_MIRRORED_REPEAT
#define RL_TEXTURE_WRAP_MIRROR_CLAMP            0x8742      // GL_MIRROR_CLAMP_EXT

// Matrix modes (equivalent to OpenGL)
#define RL_MODELVIEW                            0x1700      // GL_MODELVIEW
#define RL_PROJECTION                           0x1701      // GL_PROJECTION
#define RL_TEXTURE                              0x1702      // GL_TEXTURE
#define RL_PROJECTION_BOTTOM                    0x1703      // 3ds projection bottom.

// Primitive assembly draw modes
#define RL_LINES                                0x0001      // GL_LINES
#define RL_TRIANGLES                            0x0004      // GL_TRIANGLES
#define RL_QUADS                                0x0007      // GL_QUADS

// GL equivalent data types
#define RL_UNSIGNED_BYTE                        0x1401      // GL_UNSIGNED_BYTE
#define RL_FLOAT                                0x1406      // GL_FLOAT

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { OPENGL_11 = 1, OPENGL_21, OPENGL_33, OPENGL_ES_20 } GlVersion;

typedef enum {
    RL_ATTACHMENT_COLOR_CHANNEL0 = 0,
    RL_ATTACHMENT_COLOR_CHANNEL1,
    RL_ATTACHMENT_COLOR_CHANNEL2,
    RL_ATTACHMENT_COLOR_CHANNEL3,
    RL_ATTACHMENT_COLOR_CHANNEL4,
    RL_ATTACHMENT_COLOR_CHANNEL5,
    RL_ATTACHMENT_COLOR_CHANNEL6,
    RL_ATTACHMENT_COLOR_CHANNEL7,
    RL_ATTACHMENT_DEPTH = 100,
    RL_ATTACHMENT_STENCIL = 200,
} FramebufferAttachType;

typedef enum {
    RL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_X,
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Y,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Y,
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Z,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Z,
    RL_ATTACHMENT_TEXTURE2D = 100,
    RL_ATTACHMENT_RENDERBUFFER = 200,
} FramebufferAttachTextureType;

// Dynamic vertex buffers (position + texcoords + colors + indices arrays)
typedef struct VertexBuffer {
    int elementsCount;          // Number of elements in the buffer (QUADS)

    int vCounter;               // Vertex position counter to process (and draw) from full buffer
    int tcCounter;              // Vertex texcoord counter to process (and draw) from full buffer
    int cCounter;               // Vertex color counter to process (and draw) from full buffer

    float *vertices;            // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;           // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    unsigned char *colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    unsigned int *indices;      // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    unsigned short *indices;    // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
    unsigned int vaoId;         // OpenGL Vertex Array Object id
    unsigned int vboId[4];      // OpenGL Vertex Buffer Objects id (4 types of vertex data)
} VertexBuffer;

// Draw call type
// NOTE: Only texture changes register a new draw, other state-change-related elements are not
// used at this moment (vaoId, shaderId, matrices), raylib just forces a batch draw call if any
// of those state-change happens (this is done in core module)
typedef struct DrawCall {
    int mode;                   // Drawing mode: LINES, TRIANGLES, QUADS
    int vertexCount;            // Number of vertex of the draw
    int vertexAlignment;        // Number of vertex required for index alignment (LINES, TRIANGLES)
    //unsigned int vaoId;       // Vertex array id to be used on the draw -> Using RLGL.currentBatch->vertexBuffer.vaoId
    //unsigned int shaderId;    // Shader id to be used on the draw -> Using RLGL.currentShader.id
    unsigned int textureId;     // Texture id to be used on the draw -> Use to create new draw call if changes

    //Matrix projection;        // Projection matrix for this draw -> Using RLGL.projection by default
    //Matrix modelview;         // Modelview matrix for this draw -> Using RLGL.modelview by default
} DrawCall;

// RenderBatch type
typedef struct RenderBatch {
    int buffersCount;           // Number of vertex buffers (multi-buffering support)
    int currentBuffer;          // Current buffer tracking in case of multi-buffering
    VertexBuffer *vertexBuffer; // Dynamic buffer(s) for vertex data

    DrawCall *draws;            // Draw calls array, depends on textureId
    int drawsCounter;           // Draw calls counter
    float currentDepth;         // Current depth value for next draw
} RenderBatch;

// Shader attribute data types
typedef enum {
    SHADER_ATTRIB_FLOAT = 0,
    SHADER_ATTRIB_VEC2,
    SHADER_ATTRIB_VEC3,
    SHADER_ATTRIB_VEC4
} ShaderAttributeDataType;

typedef struct TexLinkedList
{
    unsigned int id;
    C3D_Tex tex;
    struct TexLinkedList* next;
} TexLinkedList_t;

typedef struct rlglData {
    RenderBatch *currentBatch;              // Current render batch
    RenderBatch defaultBatch;               // Default internal render batch
#if defined(PICA200)
    C3D_RenderTarget* currentScreen;
    C3D_RenderTarget* topScreenLeft;
    C3D_RenderTarget* topScreenRight;
    C3D_RenderTarget* bottomScreen;
    bool bottom;
#endif

    struct {
        int currentMatrixMode;              // Current matrix mode
        Matrix *currentMatrix;              // Current matrix pointer
        Matrix modelview;                   // Default modelview matrix
        Matrix projection;                  // Default projection matrix
        Matrix projectionBottom;            // Default projection matrix bottom.
        Matrix transform;                   // Transform matrix to be used with rlTranslate, rlRotate, rlScale
        bool transformRequired;             // Require transform matrix application to current draw-call vertex (if required)
        Matrix stack[MAX_MATRIX_STACK_SIZE];// Matrix stack for push/pop
        int stackCounter;                   // Matrix stack counter

        unsigned int defaultTextureId;      // Default texture used on shapes/poly drawing (required by shader)
        unsigned int activeTextureId[MAX_BATCH_ACTIVE_TEXTURES];    // Active texture ids to be enabled on batch drawing (0 active by default)
        unsigned int defaultVShaderId;      // Default vertex shader id (used by default shader program)
        unsigned int defaultFShaderId;      // Default fragment shader Id (used by default shader program)
        Shader defaultShader;               // Basic shader, support vertex color and diffuse texture
        Shader currentShader;               // Shader to be used on rendering (by default, defaultShader)

        bool stereoRender;                  // Stereo rendering flag
        Matrix projectionStereo[2];         // VR stereo rendering eyes projection matrices
        Matrix viewOffsetStereo[2];         // VR stereo rendering eyes view offset matrices

        int currentBlendMode;               // Blending mode active
        int glBlendSrcFactor;               // Blending source factor
        int glBlendDstFactor;               // Blending destination factor
        int glBlendEquation;                // Blending equation

        int framebufferWidth;               // Default framebuffer width
        int framebufferHeight;              // Default framebuffer height

    } State;            // Renderer state
    struct {
        bool vao;                           // VAO support (OpenGL ES2 could not support VAO extension) (GL_ARB_vertex_array_object)
        bool instancing;                    // Instancing supported (GL_ANGLE_instanced_arrays, GL_EXT_draw_instanced + GL_EXT_instanced_arrays)
        bool texNPOT;                       // NPOT textures full support (GL_ARB_texture_non_power_of_two, GL_OES_texture_npot)
        bool texDepth;                      // Depth textures supported (GL_ARB_depth_texture, GL_WEBGL_depth_texture, GL_OES_depth_texture)
        bool texFloat32;                    // float textures support (32 bit per channel) (GL_OES_texture_float)
        bool texCompDXT;                    // DDS texture compression support (GL_EXT_texture_compression_s3tc, GL_WEBGL_compressed_texture_s3tc, GL_WEBKIT_WEBGL_compressed_texture_s3tc)
        bool texCompETC1;                   // ETC1 texture compression support (GL_OES_compressed_ETC1_RGB8_texture, GL_WEBGL_compressed_texture_etc1)
        bool texCompETC2;                   // ETC2/EAC texture compression support (GL_ARB_ES3_compatibility)
        bool texCompPVRT;                   // PVR texture compression support (GL_IMG_texture_compression_pvrtc)
        bool texCompASTC;                   // ASTC texture compression support (GL_KHR_texture_compression_astc_hdr, GL_KHR_texture_compression_astc_ldr)
        bool texMirrorClamp;                // Clamp mirror wrap mode supported (GL_EXT_texture_mirror_clamp)
        bool texAnisoFilter;                // Anisotropic texture filtering support (GL_EXT_texture_filter_anisotropic)

        float maxAnisotropyLevel;           // Maximum anisotropy level supported (minimum is 2.0f)
        int maxDepthBits;                   // Maximum bits for depth component

    } ExtSupported;     // Extensions supported flags

} rlglData;

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

//------------------------------------------------------------------------------------
// Functions Declaration - Matrix operations
//------------------------------------------------------------------------------------
RLAPI void rlMatrixMode(int mode);                    // Choose the current matrix to be transformed
RLAPI void rlPushMatrix(void);                        // Push the current matrix to stack
RLAPI void rlPopMatrix(void);                         // Pop lattest inserted matrix from stack
RLAPI void rlLoadIdentity(void);                      // Reset current matrix to identity matrix
RLAPI void rlTranslatef(float x, float y, float z);   // Multiply the current matrix by a translation matrix
RLAPI void rlRotatef(float angleDeg, float x, float y, float z);  // Multiply the current matrix by a rotation matrix
RLAPI void rlScalef(float x, float y, float z);       // Multiply the current matrix by a scaling matrix
RLAPI void rlMultMatrixf(float *matf);                // Multiply the current matrix by another matrix
RLAPI void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlViewport(int x, int y, int width, int height); // Set the viewport area

//------------------------------------------------------------------------------------
// Functions Declaration - Vertex level operations
//------------------------------------------------------------------------------------
RLAPI void rlBegin(int mode);                         // Initialize drawing mode (how to organize vertex)
RLAPI void rlEnd(void);                               // Finish vertex providing
RLAPI void rlVertex2i(int x, int y);                  // Define one vertex (position) - 2 int
RLAPI void rlVertex2f(float x, float y);              // Define one vertex (position) - 2 float
RLAPI void rlVertex3f(float x, float y, float z);     // Define one vertex (position) - 3 float
RLAPI void rlTexCoord2f(float x, float y);            // Define one vertex (texture coordinate) - 2 float
RLAPI void rlNormal3f(float x, float y, float z);     // Define one vertex (normal) - 3 float
RLAPI void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);  // Define one vertex (color) - 4 byte
RLAPI void rlColor3f(float x, float y, float z);          // Define one vertex (color) - 3 float
RLAPI void rlColor4f(float x, float y, float z, float w); // Define one vertex (color) - 4 float

//------------------------------------------------------------------------------------
// Functions Declaration - OpenGL style functions (common to 1.1, 3.3+, ES2)
// NOTE: This functions are used to completely abstract raylib code from OpenGL layer,
// some of them are direct wrappers over OpenGL calls, some others are custom
//------------------------------------------------------------------------------------

// Vertex buffers state
RLAPI bool rlEnableVertexArray(unsigned int vaoId);     // Enable vertex array (VAO, if supported)
RLAPI void rlDisableVertexArray(void);                  // Disable vertex array (VAO, if supported)
RLAPI void rlEnableVertexBuffer(unsigned int id);       // Enable vertex buffer (VBO)
RLAPI void rlDisableVertexBuffer(void);                 // Disable vertex buffer (VBO)
RLAPI void rlEnableVertexBufferElement(unsigned int id);// Enable vertex buffer element (VBO element)
RLAPI void rlDisableVertexBufferElement(void);          // Disable vertex buffer element (VBO element)
RLAPI void rlEnableVertexAttribute(unsigned int index); // Enable vertex attribute index
RLAPI void rlDisableVertexAttribute(unsigned int index);// Disable vertex attribute index
#if defined(GRAPHICS_API_OPENGL_11)
RLAPI void rlEnableStatePointer(int vertexAttribType, void *buffer);
RLAPI void rlDisableStatePointer(int vertexAttribType);
#endif

// Textures state
RLAPI void rlActiveTextureSlot(int slot);               // Select and active a texture slot
RLAPI void rlEnableTexture(unsigned int id);            // Enable texture
RLAPI void rlDisableTexture(void);                      // Disable texture
RLAPI void rlEnableTextureCubemap(unsigned int id);     // Enable texture cubemap
RLAPI void rlDisableTextureCubemap(void);               // Disable texture cubemap
RLAPI void rlTextureParameters(unsigned int id, int param, int value); // Set texture parameters (filter, wrap)

// Shader state
RLAPI void rlEnableShader(unsigned int id);             // Enable shader program
RLAPI void rlDisableShader(void);                       // Disable shader program

// Framebuffer state
RLAPI void rlEnableFramebuffer(unsigned int id);        // Enable render texture (fbo)
RLAPI void rlDisableFramebuffer(void);                  // Disable render texture (fbo), return to default framebuffer

// General render state
RLAPI void rlEnableDepthTest(void);                     // Enable depth test
RLAPI void rlDisableDepthTest(void);                    // Disable depth test
RLAPI void rlEnableDepthMask(void);                     // Enable depth write
RLAPI void rlDisableDepthMask(void);                    // Disable depth write
RLAPI void rlEnableBackfaceCulling(void);               // Enable backface culling
RLAPI void rlDisableBackfaceCulling(void);              // Disable backface culling
RLAPI void rlEnableScissorTest(void);                   // Enable scissor test
RLAPI void rlDisableScissorTest(void);                  // Disable scissor test
RLAPI void rlScissor(int x, int y, int width, int height); // Scissor test
RLAPI void rlEnableWireMode(void);                      // Enable wire mode
RLAPI void rlDisableWireMode(void);                     // Disable wire mode
RLAPI void rlSetLineWidth(float width);                 // Set the line drawing width
RLAPI float rlGetLineWidth(void);                       // Get the line drawing width
RLAPI void rlEnableSmoothLines(void);                   // Enable line aliasing
RLAPI void rlDisableSmoothLines(void);                  // Disable line aliasing
RLAPI void rlEnableStereoRender(void);                  // Enable stereo rendering
RLAPI void rlDisableStereoRender(void);                 // Disable stereo rendering
RLAPI bool rlIsStereoRenderEnabled(void);               // Check if stereo render is enabled

RLAPI void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a); // Clear color buffer with color
RLAPI void rlClearScreenBuffers(void);                  // Clear used screen buffers (color and depth)
RLAPI void rlCheckErrors(void);                         // Check and log OpenGL error codes
RLAPI void rlSetBlendMode(int mode);                    // Set blending mode
RLAPI void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation); // Set blending mode factor and equation (using OpenGL factors)

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
// rlgl initialization functions
RLAPI void rlglInit(int width, int height);           // Initialize rlgl (buffers, shaders, textures, states)
RLAPI void rlglClose(void);                           // De-inititialize rlgl (buffers, shaders, textures)
RLAPI void rlLoadExtensions(void *loader);            // Load OpenGL extensions (loader function required)
RLAPI int rlGetVersion(void);                         // Returns current OpenGL version
RLAPI int rlGetFramebufferWidth(void);                // Get default framebuffer width
RLAPI int rlGetFramebufferHeight(void);               // Get default framebuffer height

RLAPI Shader rlGetShaderDefault(void);                // Get default shader
RLAPI Texture2D rlGetTextureDefault(void);            // Get default texture

// Render batch management
// NOTE: rlgl provides a default render batch to behave like OpenGL 1.1 immediate mode
// but this render batch API is exposed in case of custom batches are required
RLAPI RenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements);  // Load a render batch system
RLAPI void rlUnloadRenderBatch(RenderBatch batch);                        // Unload render batch system
RLAPI void rlDrawRenderBatch(RenderBatch *batch);                         // Draw render batch data (Update->Draw->Reset)
RLAPI void rlSetRenderBatchActive(RenderBatch *batch);                    // Set the active render batch for rlgl (NULL for default internal)
RLAPI void rlDrawRenderBatchActive(void);                                 // Update and draw internal render batch
RLAPI bool rlCheckRenderBatchLimit(int vCount);                           // Check internal buffer overflow for a given number of vertex
RLAPI void rlSetTexture(unsigned int id);           // Set current texture for render batch and check buffers limits

//------------------------------------------------------------------------------------------------------------------------

// Vertex buffers management
RLAPI unsigned int rlLoadVertexArray(void);                               // Load vertex array (vao) if supported
RLAPI unsigned int rlLoadVertexBuffer(void *buffer, int size, bool dynamic);            // Load a vertex buffer attribute
RLAPI unsigned int rlLoadVertexBufferElement(void *buffer, int size, bool dynamic);     // Load a new attributes element buffer
RLAPI void rlUpdateVertexBuffer(int bufferId, void *data, int dataSize, int offset);    // Update GPU buffer with new data
RLAPI void rlUnloadVertexArray(unsigned int vaoId);
RLAPI void rlUnloadVertexBuffer(unsigned int vboId);
RLAPI void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, void *pointer);
RLAPI void rlSetVertexAttributeDivisor(unsigned int index, int divisor);
RLAPI void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count); // Set vertex attribute default value
RLAPI void rlDrawVertexArray(int offset, int count);
RLAPI void rlDrawVertexArrayElements(int offset, int count, void *buffer);
RLAPI void rlDrawVertexArrayInstanced(int offset, int count, int instances);
RLAPI void rlDrawVertexArrayElementsInstanced(int offset, int count, void *buffer, int instances);

// Textures management
RLAPI unsigned int rlLoadTexture(void *data, int width, int height, int format, int mipmapCount); // Load texture in GPU
RLAPI unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer);               // Load depth texture/renderbuffer (to be attached to fbo)
RLAPI unsigned int rlLoadTextureCubemap(void *data, int size, int format);                        // Load texture cubemap
RLAPI void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data);  // Update GPU texture with new data
RLAPI void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType);  // Get OpenGL internal formats
RLAPI void rlUnloadTexture(unsigned int id);                              // Unload texture from GPU memory
RLAPI void rlGenerateMipmaps(Texture2D *texture);                         // Generate mipmap data for selected texture
RLAPI void *rlReadTexturePixels(Texture2D texture);                       // Read texture pixel data
RLAPI unsigned char *rlReadScreenPixels(int width, int height);           // Read screen pixel data (color buffer)

// Framebuffer management (fbo)
RLAPI unsigned int rlLoadFramebuffer(int width, int height);              // Load an empty framebuffer
RLAPI void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel);  // Attach texture/renderbuffer to a framebuffer
RLAPI bool rlFramebufferComplete(unsigned int id);                        // Verify framebuffer is complete
RLAPI void rlUnloadFramebuffer(unsigned int id);                          // Delete framebuffer from GPU

// Shaders management
RLAPI unsigned int rlLoadShaderCode(const char *vsCode, const char *fsCode);    // Load shader from code strings
RLAPI unsigned int rlCompileShader(const char *shaderCode, int type);           // Compile custom shader and return shader id (type: GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
RLAPI unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId); // Load custom shader program
RLAPI void rlUnloadShaderProgram(unsigned int id);                              // Unload shader program
RLAPI int rlGetLocationUniform(unsigned int shaderId, const char *uniformName); // Get shader location uniform
RLAPI int rlGetLocationAttrib(unsigned int shaderId, const char *attribName);   // Get shader location attribute
RLAPI void rlSetUniform(int locIndex, const void *value, int uniformType, int count); // Set shader value uniform
RLAPI void rlSetUniformMatrix(int locIndex, Matrix mat);                        // Set shader value matrix
RLAPI void rlSetUniformSampler(int locIndex, unsigned int textureId);           // Set shader value sampler
RLAPI void rlSetShader(Shader shader);                                    // Set shader currently active

// Matrix state management
RLAPI Matrix rlGetMatrixModelview(void);                                  // Get internal modelview matrix
RLAPI Matrix rlGetMatrixProjection(void);                                 // Get internal projection matrix
RLAPI Matrix rlGetMatrixTransform(void);                                  // Get internal accumulated transform matrix
RLAPI Matrix rlGetMatrixProjectionStereo(int eye);                        // Get internal projection matrix for stereo render (selected eye)
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye);                        // Get internal view offset matrix for stereo render (selected eye)
RLAPI void rlSetMatrixProjection(Matrix proj);                            // Set a custom projection matrix (replaces internal projection matrix)
RLAPI void rlSetMatrixModelview(Matrix view);                             // Set a custom modelview matrix (replaces internal modelview matrix)
RLAPI void rlSetMatrixProjectionStereo(Matrix right, Matrix left);        // Set eyes projection matrices for stereo rendering
RLAPI void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left);        // Set eyes view offsets matrices for stereo rendering

// 3DS
RLAPI void rlSetDepth(float depth);
RLAPI void rlEnableBottomScreen(void);
RLAPI void rlSetCurrentScreen(int screen);
RLAPI int rlGetCurrentScreen();

// Quick and dirty cube/quad buffers load->draw->unload
RLAPI void rlLoadDrawCube(void);     // Load and draw a cube
RLAPI void rlLoadDrawQuad(void);     // Load and draw a quad
#if defined(__cplusplus)
}
#endif

#endif // RLGL_H

#if defined(RLGL_IMPLEMENTATION)

static rlglData RLGL = { 0 };

#define CLEAR_COLOR 0x68B0D8FF

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;
static int uLoc_projection, uLoc_modelView;
static C3D_Mtx projection;

static TexLinkedList_t* rootTex;

typedef enum Pica200ShaderParams
{
    SHD_POSITION,
    SHD_TEXCOORD,
    SHD_COLOR,
    SHD_NORMAL,
    SHD_MAX_PARAMS
};

extern rlglData RLGL;

static bool paramsUsed[SHD_MAX_PARAMS];
static Vector4 lastParams[SHD_MAX_PARAMS];
static Vector4 backupParams[SHD_MAX_PARAMS];
static int shaderNumParams = SHD_MAX_PARAMS;
static int paramMode;
static int paramNum;
static float currentDepth;

static void sceneInit(void)
{
	// Load the vertex shader, create a shader program and bind it
	vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
	C3D_BindProgram(&program);

	// Get the location of the uniforms
	uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
	uLoc_modelView = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

	// Configure attributes for use with the vertex shader
	C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
	AttrInfo_Init(attrInfo);
	AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
	AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texture
	AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 4); // v2=color
	//AttrInfo_AddLoader(attrInfo, 3, GPU_FLOAT, 3); // v3=normal

	// Compute the projection matrix
	Mtx_OrthoTilt(&projection, 0.0, 400.0, 0.0, 240.0, 0.0, 1.0, true);

	// Configure the first fragment shading substage to just pass through the vertex color
	// See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
	C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
    C3D_CullFace(GPU_CULL_BACK_CCW);
    //C3D_CullFace(GPU_CULL_NONE);
}

static void sceneRender(void)
{

	// Update the uniforms
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);

}

static void sceneExit(void)
{

	// Free the shader program
	shaderProgramFree(&program);
	DVLB_Free(vshader_dvlb);

}

// Initialize rlgl: OpenGL extensions, default buffers/shaders/textures, OpenGL states
void rlglInit(int width, int height)
{

    // Screen setup.
    RLGL.topScreenLeft = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    RLGL.topScreenRight = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    RLGL.bottomScreen = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    RLGL.currentScreen = RLGL.topScreenLeft;
    C3D_RenderTargetClear(RLGL.currentScreen, C3D_CLEAR_ALL, C2D_Color32(0, 0, 0, 255), 0);
    C3D_RenderTargetSetOutput(RLGL.currentScreen, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    sceneInit();

    // Load texture.
    unsigned char pixels[64] = {
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255
    };   // 8x8 pixel lightmap. (64 bytes)
    rootTex = NULL;
    RLGL.State.defaultTextureId = rlLoadTexture(pixels, 8, 8, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, 1);
    if (RLGL.State.defaultTextureId != 0) TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Default texture loaded successfully", RLGL.State.defaultTextureId);
    else TRACELOG(LOG_WARNING, "TEXTURE: Failed to load default texture");

    // Init default Shader (customized for GL 3.3 and ES2)
    //rlLoadShaderDefault(); // RLGL.State.defaultShader
    //RLGL.State.currentShader = RLGL.State.defaultShader;
    shaderNumParams = 3;

    // Init default vertex arrays buffers
    //RLGL.defaultBatch = rlLoadRenderBatch(DEFAULT_BATCH_BUFFERS, DEFAULT_BATCH_BUFFER_ELEMENTS);
    //RLGL.currentBatch = &RLGL.defaultBatch;

    // Init stack matrices (emulating OpenGL 1.1)
    for (int i = 0; i < MAX_MATRIX_STACK_SIZE; i++) RLGL.State.stack[i] = MatrixIdentity();

    // Init internal matrices
    RLGL.State.transform = MatrixIdentity();
    RLGL.State.projection = MatrixIdentity();
    RLGL.State.projectionBottom = MatrixIdentity();
    RLGL.State.modelview = MatrixIdentity();
    RLGL.State.currentMatrix = &RLGL.State.modelview;

    /*

    // Initialize OpenGL default states
    //----------------------------------------------------------
    // Init state: Depth test
    glDepthFunc(GL_LEQUAL);                                 // Type of depth testing to apply
    glDisable(GL_DEPTH_TEST);                               // Disable depth testing for 2D (only used for 3D)

    // Init state: Blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // Color blending function (how colors are mixed)
    glEnable(GL_BLEND);                                     // Enable color blending (required to work with transparencies)

    // Init state: Culling
    // NOTE: All shapes/models triangles are drawn CCW
    glCullFace(GL_BACK);                                    // Cull the back face (default)
    glFrontFace(GL_CCW);                                    // Front face are defined counter clockwise (default)
    glEnable(GL_CULL_FACE);                                 // Enable backface culling

    // Init state: Cubemap seamless
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);                 // Seamless cubemaps (not supported on OpenGL ES 2.0)

    // Init state: Color hints (deprecated in OpenGL 3.0+)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Improve quality of color and texture coordinate interpolation
    glShadeModel(GL_SMOOTH);                                // Smooth shading between vertex (vertex colors interpolation)
    */

    // Store screen size into global variables
    RLGL.State.framebufferWidth = width;
    RLGL.State.framebufferHeight = height;

    TRACELOG(LOG_INFO, "RLGL: Default PICA200 state initialized successfully");

    // Init state: Color/Depth buffers clear
    C3D_RenderTargetClear(RLGL.currentScreen, C3D_CLEAR_ALL, C2D_Color32(255, 0, 0, 0), 0);

}

void rlEnableBottomScreen(void)
{
    C3D_RenderTargetClear(RLGL.bottomScreen, C3D_CLEAR_ALL, C2D_Color32(0, 0, 0, 255), 0);
    C3D_RenderTargetSetOutput(RLGL.bottomScreen, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
}

GPU_TEXCOLOR rlGetPica200TextureFormat(int format)
{
    switch (format)
    {
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA:  return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA:  return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:       return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:      return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:      return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:      return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:       return GPU_ETC1;
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:  return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_COMPRESSED_ETC2_RGB:       return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_COMPRESSED_PVRT_RGB:       return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA:      return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:   return GPU_LA8;
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:    return GPU_L8;
        case PIXELFORMAT_UNCOMPRESSED_R32:          return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32:    return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: return (GPU_TEXCOLOR)-1;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:     return GPU_RGBA4;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:     return GPU_RGBA5551;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:       return GPU_RGB565;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:       return GPU_RGB8;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:     return GPU_RGBA8;
    }
}

int rlGetPica200TextureBpp(int format)
{
    switch (format)
    {
        case GPU_RGBA8:    return 32;
        case GPU_RGB8:     return 24;
        case GPU_RGBA5551: return 16;
        case GPU_RGB565:   return 16;
        case GPU_RGBA4:    return 16;
        case GPU_LA8:      return 16;
        case GPU_HILO8:    return 16;
        case GPU_L8:       return 8;
        case GPU_A8:       return 8;
        case GPU_LA4:      return 8;
        case GPU_L4:       return 4;
        case GPU_A4:       return 4;
        case GPU_ETC1:     return 4;
        case GPU_ETC1A4:   return 4;
    }
    return 0;
}

C3D_Tex* rlGetPica200Tex(unsigned int id)
{
    TexLinkedList_t* curr = rootTex;
    while (curr != NULL)
    {
        if (curr->id == id)
        {
            return &curr->tex;
        }
        curr = curr->next;
    }
    return NULL;
}

unsigned int rlNewPica200Tex()
{
    unsigned int newId = 1;
    if (rootTex == NULL)
    {
        rootTex = (TexLinkedList_t*)RL_MALLOC(sizeof(TexLinkedList_t));
        rootTex->id = newId;
        rootTex->next = NULL;
        return newId;
    }
    TexLinkedList_t* curr = rootTex->next;
    TexLinkedList_t* prev = rootTex;
    newId++;
    while (curr != NULL)
    {
        if (newId < curr->id)
        {
            TexLinkedList_t* next = curr->next;
            prev->next = (TexLinkedList_t*)RL_MALLOC(sizeof(TexLinkedList_t));
            prev->next->next = next;
            prev->next->id = newId;
            return newId;
        }
        newId = curr->id + 1;
        prev = curr;
        curr = curr->next;
    }
    prev->next = (TexLinkedList_t*)RL_MALLOC(sizeof(TexLinkedList_t));
    prev->next->id = newId;
    prev->next->next = NULL;
    return newId;
}

void rlRemovePica200Tex(unsigned int id)
{
    if (rootTex != NULL)
    {
        if (rootTex->id == id)
        {
            TexLinkedList_t* newRoot = rootTex->next;
            RL_FREE(rootTex);
            rootTex = newRoot;
        }
        else
        {
            TexLinkedList_t* prev = rootTex;
            TexLinkedList_t* curr = rootTex->next;
            while (curr != NULL)
            {
                if (curr->id == id)
                {
                    prev->next = curr->next;
                    RL_FREE(curr);
                    return;
                }
                else
                {
                    curr = curr->next;
                }
            }
        }
    }
}

const u8 swizzleArr[64] = {
     0,  1,  4,  5, 16, 17, 20, 21,
     2,  3,  6,  7, 18, 19, 22, 23,
     8,  9, 12, 13, 24, 25, 28, 29,
    10, 11, 14, 15, 26, 27, 30, 31,
    32, 33, 36, 37, 48, 49, 52, 53,
    34, 35, 38, 39, 50, 51, 54, 55,
    40, 41, 44, 45, 56, 57, 60, 61,
    42, 43, 46, 47, 58, 59, 62, 63
};

static inline bool checkTexSize(u32 size)
{
	if (size < 8 || size > 1024)
		return false;
	if (size & (size-1))
		return false;
	return true;
}

unsigned int rlLoadTexture(void *data, int width, int height, int format, int mipmapCount)
{ 
    GPU_TEXCOLOR texColor = rlGetPica200TextureFormat(format);
    if (texColor == -1)
    {
        TRACELOG(LOG_WARNING, "TEXTURE: Current format not supported (%i)", texColor);
        TRACELOG(LOG_WARNING, "TEXTURE: Failed to load texture");
        return 0;
    }
    unsigned int id = rlNewPica200Tex();
    C3D_Tex* tex = rlGetPica200Tex(id);
    if (!C3D_TexInit(tex, width, height, texColor))
    {
        TRACELOG(LOG_WARNING, "TEXTURE: Failed to load texture (invalid size): %d %d", checkTexSize(width), checkTexSize(height));
        return 0;
    }
    int bpp = rlGetPica200TextureBpp(texColor);
    u8* swizzled = RL_MALLOC(width * height * bpp / 8);
    /*
    for (int w = 0; w < width / 8; w++) // Each tile horizontally.
    {
        int pixelX = w * 8;
        for (int h = 0; h < height / 8; h++) // Each tile vertically.
        {
            int pixelY = h * 8;
            int tileNum = w + h * width / 8;
            for (int i = 0; i < 64; i += 2) // Copy every 2 pixels until all 64 are copied.
            {
                int pixelNum = pixelX + i % 8 + (pixelY + i / 8) * width;
                void* src = data + pixelNum * bpp / 8;                                      // Data in the source is stored left-right, top-bottom.
                void* dest = swizzled + tileNum * 64 * bpp / 8 + swizzleArr[i] * bpp / 8;   // Data in destination is stored in 8x8 tiles in the same ordering.
                memcpy(dest, src, bpp / 4);
            }
        }
    }*/
    for (int w = 0; w < width / 8; w++) // Each tile horizontally.
    {
        int pixelX = w * 8;
        for (int h = 0; h < height / 8; h++) // Each tile vertically.
        {
            int pixelY = h * 8;
            int tileNum = w + h * width / 8;
            for (int i = 0; i < 64; i += 2) // Copy every 2 pixels until all 64 are copied.
            {
                int pixelNum = pixelX + i % 8 + (height - (pixelY + i / 8) - 1) * width; // Flip along the y axis too for reasons.
                void* src = data + pixelNum * bpp / 8;                                      // Data in the source is stored left-right, top-bottom.
                void* dest = swizzled + tileNum * 64 * bpp / 8 + swizzleArr[i] * bpp / 8;   // Data in destination is stored in 8x8 tiles in the same ordering.
                //memcpy(dest, src, bpp / 4);
                int numPixels = 2;
                int numBytes = bpp / 8;
                if (numBytes == 0) { numBytes = 1; }
                for (int j = 0; j < numPixels; j++) // Swap order of bytes.
                {
                    for (int k = 0; k < numBytes; k++)
                    {
                        *(u8*)(dest + numBytes * j + numBytes - k - 1) = *(u8*)(src + k + numBytes * j);
                    }
                }
            }
        }
    }
    C3D_TexUpload(tex, swizzled);
    RL_FREE(swizzled);
    TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Texture loaded successfully (%ix%i - %i mipmaps)", id, width, height, mipmapCount);
    return id;
}

void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data)
{
    // TODO!!!
}

void rlUnloadTexture(unsigned int id)
{
    C3D_Tex* tex = rlGetPica200Tex(id);
    if (tex == NULL)
    {
        TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Unable to unload texture");
        return;
    }
    C3D_TexDelete(tex);
    rlRemovePica200Tex(id);
}

// Get default internal texture (white texture)
Texture2D rlGetTextureDefault(void)
{
    Texture2D texture = { 0 };
    texture.id = RLGL.State.defaultTextureId;
    texture.width = 8;
    texture.height = 8;
    texture.mipmaps = 1;
    texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    return texture;
}

// Set texture parameters (wrap mode/filter mode)
void rlTextureParameters(unsigned int id, int param, int value)
{
    /*C3D_Tex* tex = rlGetPica200Tex(id);
    GPU_TEXTURE_FILTER_PARAM filter = 
    switch (param)
    {
        case RL_TEXTURE_WRAP_S:
        case RL_TEXTURE_WRAP_T:
        {
            if (value == RL_TEXTURE_WRAP_MIRROR_CLAMP)
            {
                if (RLGL.ExtSupported.texMirrorClamp) glTexParameteri(GL_TEXTURE_2D, param, value);
                else TRACELOG(LOG_WARNING, "GL: Clamp mirror wrap mode not supported (GL_MIRROR_CLAMP_EXT)");
            }
            else glTexParameteri(GL_TEXTURE_2D, param, value);

        } break;
        case RL_TEXTURE_MAG_FILTER:
        case RL_TEXTURE_MIN_FILTER:
        {
            switch (val)
        } break;
        case RL_TEXTURE_FILTER_ANISOTROPIC:
        {
            TRACELOG(LOG_WARNING, "GL: Anisotropic filtering not supported");
        } break;
        default: break;
    }

    glBindTexture(GL_TEXTURE_2D, 0);*/ // TODO!!!
}

// Generate mipmap data for selected texture
void rlGenerateMipmaps(Texture2D *texture)
{
    C3D_Tex* tex = rlGetPica200Tex(texture->id);
    C3D_TexGenerateMipmap(tex, GPU_TEXFACE_2D);
    texture->mipmaps++;
}

// Clear color buffer with color
void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    C3D_RenderTargetClear(RLGL.currentScreen, C3D_CLEAR_ALL, C2D_Color32(a, b, g, r), 0);
}

// Clear used screen buffers (color and depth)
void rlClearScreenBuffers(void)
{
    //C3D_FrameDrawOn(RLGL.currentScreen);
}

void rlSetCurrentScreen(int screen)
{
    if (screen == SCREEN_3DS_TOP)
    {
        RLGL.currentScreen = RLGL.topScreenLeft;
        RLGL.bottom = false;
    }
    else
    {
        RLGL.currentScreen = RLGL.bottomScreen;
        RLGL.bottom = true;
    }
    C3D_FrameDrawOn(RLGL.currentScreen);
}

int rlGetCurrentScreen(void)
{
    return RLGL.bottom ? SCREEN_3DS_BOTTOM : SCREEN_3DS_TOP;
}

// Draw render batch
// NOTE: We require a pointer to reset batch and increase current buffer (multi-buffer)
void rlDrawRenderBatch(RenderBatch *batch)
{
    // TODO!!!
}

// Update and draw internal render batch
void rlDrawRenderBatchActive(void)
{
    rlDrawRenderBatch(RLGL.currentBatch);
}

// Check internal buffer overflow for a given number of vertex
// and force a RenderBatch draw call if required
bool rlCheckRenderBatchLimit(int vCount)
{
    return false;
    /*bool overflow = false;

    if ((RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter + vCount) >=
        (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementsCount*4))
    {
        overflow = true;
        rlDrawRenderBatch(RLGL.currentBatch);    // NOTE: Stereo rendering is checked inside
    }

    return overflow;*/
}

void rlSetTexture(unsigned int id)
{
    if (id == 0)
    {
        rlDisableTexture();
    }
    else
    {
        rlEnableTexture(id);
    }
}

// Enable texture
void rlEnableTexture(unsigned int id)
{
    C3D_Tex* tex = rlGetPica200Tex(id);
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexSetFilter(tex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
    C3D_TexBind(0, tex);
}

// Disable texture
void rlDisableTexture(void)
{
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
    C3D_TexBind(0, NULL);
}

// Set blend mode
void rlSetBlendMode(int mode)
{
    /*if (RLGL.State.currentBlendMode != mode)
    {
        rlDrawRenderBatch(RLGL.currentBatch);
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        switch (mode)
        {
            case BLEND_ALPHA: C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE); break;
            case BLEND_ADDITIVE: C3D_TexEnvFunc(env, C3D_Alpha, GPU_MULTIPLY_ADD); break;
            case BLEND_MULTIPLIED: C3D_TexEnvFunc(env, C3D_Both, GPU_MULTIPLY_ADD); break;
            case BLEND_ADD_COLORS: C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD); break;
            case BLEND_SUBTRACT_COLORS: C3D_TexEnvFunc(env, C3D_RGB, GPU_SUBTRACT); break;
            default: break;
        }

        RLGL.State.currentBlendMode = mode;
    }*/
}

// Enable depth test
void rlEnableDepthTest(void) {  }

// Disable depth test
void rlDisableDepthTest(void) {  }

// Choose the current matrix to be transformed
void rlMatrixMode(int mode)
{
    if (mode == RL_PROJECTION) RLGL.State.currentMatrix = &RLGL.State.projection;
    else if (mode == RL_PROJECTION_BOTTOM) RLGL.State.currentMatrix = &RLGL.State.projectionBottom;
    else if (mode == RL_MODELVIEW) RLGL.State.currentMatrix = &RLGL.State.modelview;
    //else if (mode == RL_TEXTURE) // Not supported

    RLGL.State.currentMatrixMode = mode;
}

// Push the current matrix into RLGL.State.stack
void rlPushMatrix(void)
{
    if (RLGL.State.stackCounter >= MAX_MATRIX_STACK_SIZE) TRACELOG(LOG_ERROR, "RLGL: Matrix stack overflow (MAX_MATRIX_STACK_SIZE)");

    if (RLGL.State.currentMatrixMode == RL_MODELVIEW)
    {
        RLGL.State.transformRequired = true;
        RLGL.State.currentMatrix = &RLGL.State.transform;
    }

    RLGL.State.stack[RLGL.State.stackCounter] = *RLGL.State.currentMatrix;
    RLGL.State.stackCounter++;
}

// Pop lattest inserted matrix from RLGL.State.stack
void rlPopMatrix(void)
{
    if (RLGL.State.stackCounter > 0)
    {
        Matrix mat = RLGL.State.stack[RLGL.State.stackCounter - 1];
        *RLGL.State.currentMatrix = mat;
        RLGL.State.stackCounter--;
    }

    if ((RLGL.State.stackCounter == 0) && (RLGL.State.currentMatrixMode == RL_MODELVIEW))
    {
        RLGL.State.currentMatrix = &RLGL.State.modelview;
        RLGL.State.transformRequired = false;
    }
}

// Reset current matrix to identity matrix
void rlLoadIdentity(void)
{
    *RLGL.State.currentMatrix = MatrixIdentity();
}

// Multiply the current matrix by a translation matrix
void rlTranslatef(float x, float y, float z)
{
    Matrix matTranslation = MatrixTranslate(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *RLGL.State.currentMatrix = MatrixMultiply(matTranslation, *RLGL.State.currentMatrix);
}

// Multiply the current matrix by a rotation matrix
void rlRotatef(float angleDeg, float x, float y, float z)
{
    Matrix matRotation = MatrixIdentity();

    Vector3 axis = (Vector3){ x, y, z };
    matRotation = MatrixRotate(Vector3Normalize(axis), angleDeg*DEG2RAD);

    // NOTE: We transpose matrix with multiplication order
    *RLGL.State.currentMatrix = MatrixMultiply(matRotation, *RLGL.State.currentMatrix);
}

// Multiply the current matrix by a scaling matrix
void rlScalef(float x, float y, float z)
{
    Matrix matScale = MatrixScale(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *RLGL.State.currentMatrix = MatrixMultiply(matScale, *RLGL.State.currentMatrix);
}

// Multiply the current matrix by another matrix
void rlMultMatrixf(float *matf)
{
    // Matrix creation from array
    Matrix mat = { matf[0], matf[4], matf[8], matf[12],
                   matf[1], matf[5], matf[9], matf[13],
                   matf[2], matf[6], matf[10], matf[14],
                   matf[3], matf[7], matf[11], matf[15] };

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, mat);
}

// Multiply the current matrix by a perspective matrix generated by parameters
void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    Matrix matPerps = MatrixFrustum(left, right, bottom, top, znear, zfar);

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, matPerps);
}

// Multiply the current matrix by an orthographic matrix generated by parameters
void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    // NOTE: If left-right and top-botton values are equal it could create
    // a division by zero on MatrixOrtho(), response to it is platform/compiler dependant
    Matrix matOrtho = MatrixOrtho(left, right, bottom, top, znear, zfar);

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, matOrtho);
}

// Multiply the current matrix by an orthographic matrix generated by parameters, with tilting for the 3ds screen.
void rlOrthoTilt(double left, double right, double bottom, double top, double znear, double zfar)
{
    Matrix matOrthoTilt = MatrixOrthoTilt(left, right, bottom, top, znear, zfar);

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, matOrthoTilt);
}

// Set the viewport area (transformation from normalized device coordinates to window coordinates)
void rlViewport(int x, int y, int width, int height)
{
    C3D_SetViewport(x, y, width, height);
}

void rlBegin(int mode)
{
    switch (mode)
    {
        case RL_LINES: C3D_ImmDrawBegin(GPU_TRIANGLE_STRIP); break;
        case RL_TRIANGLES: C3D_ImmDrawBegin(GPU_TRIANGLES); break;
        case RL_QUADS: C3D_ImmDrawBegin(GPU_TRIANGLE_STRIP); break;
        default: break;
    }
    paramMode = mode;
    paramNum = 0;
    Matrix* pro = RLGL.bottom ? &RLGL.State.projectionBottom : &RLGL.State.projection;
    C3D_Mtx tmp;
    tmp.m[3] = pro->m0; // ACCOUNT FOR TILTING!!!
    tmp.m[2] = pro->m1;
    tmp.m[1] = pro->m2;
    tmp.m[0] = pro->m3;
    tmp.m[7] = pro->m4;
    tmp.m[6] = pro->m5;
    tmp.m[5] = pro->m6;
    tmp.m[4] = pro->m7;
    tmp.m[11] = pro->m8;
    tmp.m[10] = pro->m9;
    tmp.m[9] = pro->m10;
    tmp.m[8] = pro->m11;
    tmp.m[15] = pro->m12;
    tmp.m[14] = pro->m13;
    tmp.m[13] = pro->m14;
    tmp.m[12] = pro->m15;
    //Mtx_RotateZ(&tmp, PI / 2, true);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &tmp);
    tmp.m[3] = RLGL.State.modelview.m0;
    tmp.m[2] = RLGL.State.modelview.m1;
    tmp.m[1] = RLGL.State.modelview.m2;
    tmp.m[0] = RLGL.State.modelview.m3;
    tmp.m[7] = RLGL.State.modelview.m4;
    tmp.m[6] = RLGL.State.modelview.m5;
    tmp.m[5] = RLGL.State.modelview.m6;
    tmp.m[4] = RLGL.State.modelview.m7;
    tmp.m[11] = RLGL.State.modelview.m8;
    tmp.m[10] = RLGL.State.modelview.m9;
    tmp.m[9] = RLGL.State.modelview.m10;
    tmp.m[8] = RLGL.State.modelview.m11;
    tmp.m[15] = RLGL.State.modelview.m12;
    tmp.m[14] = RLGL.State.modelview.m13;
    tmp.m[13] = RLGL.State.modelview.m14;
    tmp.m[12] = RLGL.State.modelview.m15;
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &tmp);
    for (int i = 0; i < SHD_MAX_PARAMS; i++)
    {
        paramsUsed[i] = false;
    }
    lastParams[SHD_POSITION] = (Vector4){ 0, 0, 0, 0 };
    lastParams[SHD_TEXCOORD] = (Vector4){ 0, 0, 0, 0 };
    lastParams[SHD_COLOR] = (Vector4){ 1, 1, 1, 1 };
    lastParams[SHD_NORMAL] = (Vector4){ 0, 0, 1, 0 };
}

RLAPI void rlSetDepth(float depth)
{
    currentDepth = depth;
}

void rlBackupParams()
{
    for (int i = 0; i < SHD_MAX_PARAMS; i++)
    {
        backupParams[i].x = lastParams[i].x;
        backupParams[i].y = lastParams[i].y;
        backupParams[i].z = lastParams[i].z;
        backupParams[i].w = lastParams[i].w;
    }
}

void rlSendAttr(Vector4* attribs)
{
    for (int i = 0; i < shaderNumParams; i++)
    {
        Vector4* attr = &attribs[i];
        C3D_ImmSendAttrib(attr->x, attr->y, attr->z, attr->w);
        paramsUsed[i] = false;
    }
}

// So what happens is raylib doesn't call the position and texture immeduates in the correct order for the shader.
// Typically, a color and normal are given before anything else, than a texture coordinate and position for each vertex.
// This means that each call has to be cached, then have everything sent to the GPU in the correct order when a duplicate call is encountered to start a new one.
void rlPicaAttrFlush()
{
    if (paramMode == RL_LINES)
    {
        paramNum = !paramNum;
    }
    else if (paramMode == RL_QUADS)
    {
        if (paramNum == 2)
        {
            rlBackupParams();
            for (int i = 0; i < shaderNumParams; i++)
            {
                paramsUsed[i] = false;
            }
        }
        else if (paramNum == 3)
        {
            rlSendAttr(lastParams);
            rlSendAttr(backupParams);
        }
        else
        {
            rlSendAttr(lastParams);
        }
        paramNum++;
        paramNum %= 4;
    }
    else
    {
        rlSendAttr(lastParams);
    }
}

void rlVertex2i(int x, int y)
{
    if (paramsUsed[SHD_POSITION] && SHD_POSITION < shaderNumParams)
    {
        rlPicaAttrFlush();
    }
    lastParams[SHD_POSITION] = (Vector4){ (float)x, (float)y, 0, 0 };
    paramsUsed[SHD_POSITION] = true;
}

void rlVertex2f(float x, float y)
{
    if (paramsUsed[SHD_POSITION] && SHD_POSITION < shaderNumParams)
    {
        rlPicaAttrFlush();
    }
    lastParams[SHD_POSITION] = (Vector4){ x, y, currentDepth, 0 };
    paramsUsed[SHD_POSITION] = true;
}

void rlVertex3f(float x, float y, float z)
{
    if (paramsUsed[SHD_POSITION] && SHD_POSITION < shaderNumParams)
    {
        rlPicaAttrFlush();
    }
    lastParams[SHD_POSITION] = (Vector4){ x, y, z, 0 };
    paramsUsed[SHD_POSITION] = true;
}

void rlTexCoord2f(float x, float y)
{
    if (paramsUsed[SHD_TEXCOORD] && SHD_TEXCOORD < shaderNumParams)
    {
        rlPicaAttrFlush();
    }
    lastParams[SHD_TEXCOORD] = (Vector4){ x, y, 0, 0 };
    paramsUsed[SHD_TEXCOORD] = true;
}

void rlNormal3f(float x, float y, float z)
{
    if (paramsUsed[SHD_NORMAL] && SHD_NORMAL < shaderNumParams)
    {
        rlPicaAttrFlush();
    }
    lastParams[SHD_NORMAL] = (Vector4){ x, y, z, 0 };
    paramsUsed[SHD_NORMAL] = true;
}

void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    if (paramsUsed[SHD_COLOR] && SHD_COLOR < shaderNumParams)
    {
        rlPicaAttrFlush();
    }
    lastParams[SHD_COLOR] = (Vector4){ (float)r/255, (float)g/255, (float)b/255, (float)a/255 };
    paramsUsed[SHD_COLOR] = true;
}

void rlColor3f(float x, float y, float z)
{
    if (paramsUsed[SHD_COLOR] && SHD_COLOR < shaderNumParams)
    {
        rlPicaAttrFlush();
    }
    lastParams[SHD_COLOR] = (Vector4){ x, y, z, 0 };
    paramsUsed[SHD_COLOR] = true;
}

void rlColor4f(float x, float y, float z, float w)
{
    if (paramsUsed[SHD_COLOR] && SHD_COLOR < shaderNumParams)
    {
        rlPicaAttrFlush();
    }
    lastParams[SHD_COLOR] = (Vector4){ x, y, z, w };
    paramsUsed[SHD_COLOR] = true;
}

void rlEnd()
{
    rlPicaAttrFlush();
    C3D_ImmDrawEnd();
    currentDepth -= (1.0f/20000.0f);
}

void rlDummyRender()
{

    // Lazy.
    /*rlBegin(RL_TRIANGLES);
    rlVertex2f(100, 200);
    rlColor4ub(255, 0, 0, 255);
    rlVertex2f(100, 40);
    rlColor4ub(255, 255, 0, 255);
    rlVertex2f(300, 200);
    rlColor4ub(0, 255, 0, 255);
    rlVertex2f(300, 200);
    rlColor4ub(0, 255, 0, 255);
    rlVertex2f(100, 40);
    rlColor4ub(255, 255, 0, 255);
    rlVertex2f(300, 40);
    rlColor4ub(0, 0, 255, 255);
    rlEnd();*/

    // Full.
    /*rlBegin(RL_TRIANGLES);
    rlVertex3f(200, 200, 0.5);
    rlTexCoord2f(0, 0);
    rlColor4ub(255, 0, 0, 255);
    rlVertex3f(100, 40, 0.5);
    rlTexCoord2f(0, 0);
    rlColor4ub(255, 0, 0, 255);
    rlVertex3f(300, 40, 0.5);
    rlTexCoord2f(0, 0);
    rlColor4ub(255, 0, 0, 255);
    rlEnd();*/

    // Fuller. Crashes?
    /*rlBegin(RL_TRIANGLES);
    C3D_ImmSendAttrib(200, 200, 0.5, 0);
    C3D_ImmSendAttrib(0, 0, 0, 0);
    C3D_ImmSendAttrib(1, 0, 0, 1);
    C3D_ImmSendAttrib(100, 40, 0.5, 0);
    C3D_ImmSendAttrib(0, 0, 0, 0);
    C3D_ImmSendAttrib(1, 0, 0, 1);
    C3D_ImmSendAttrib(300, 40, 0.5, 0);
    C3D_ImmSendAttrib(0, 0, 0, 0);
    C3D_ImmSendAttrib(1, 0, 0, 1);
    rlEnd();*/

    //sceneRender();

}

void rlglClose()
{
    sceneExit();
    C3D_Fini();
}

#endif  // RLGL_IMPLEMENTATION