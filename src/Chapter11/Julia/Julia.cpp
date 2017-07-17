// Julia.cpp
// OpenGL SuperBible
// Julia Fragtal in the fragment shader
// Program by Graham Sellers

// OpenGL toolkit
#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <Stopwatch.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#endif

#ifdef _MSC_VER
#pragma comment (lib, "GLTools.lib")
#endif /* _MSC_VER */

static GLFrame              viewFrame;

static GLuint               juliaShader;        // The julia set renderer
static GLint                locC;               // The location of the Julia constant C uniform
static GLint                locZoom;            // Location of the zoom uniform
static GLint                locOffset;          // Location of the offset uniform

static GLuint               vao;                // The VAO
static GLuint               vertexBuffer;       // Geometry VBO

static GLuint               gradient_texture;   // The gradient texture

static bool                 paused = false;
static float                time_offset = 50.0f;
static float                zoom = 1.0f;
static float                x_offset = 0.0f;
static float                y_offset = 0.0f;

static const unsigned char palette_data[]  =
{
    0xFF, 0x00, 0x00, 0xFF, 0x0E, 0x03, 0xFF, 0x1C, 0x07, 0xFF, 0x2A, 0x0A, 0xFF, 0x38, 0x0E, 0xFF,
    0x46, 0x12, 0xFF, 0x54, 0x15, 0xFF, 0x62, 0x19, 0xFF, 0x71, 0x1D, 0xFF, 0x7F, 0x20, 0xFF, 0x88,
    0x22, 0xFF, 0x92, 0x25, 0xFF, 0x9C, 0x27, 0xFF, 0xA6, 0x2A, 0xFF, 0xB0, 0x2C, 0xFF, 0xBA, 0x2F,
    0xFF, 0xC4, 0x31, 0xFF, 0xCE, 0x34, 0xFF, 0xD8, 0x36, 0xFF, 0xE2, 0x39, 0xFF, 0xEC, 0x3B, 0xFF,
    0xF6, 0x3E, 0xFF, 0xFF, 0x40, 0xF8, 0xFE, 0x40, 0xF1, 0xFE, 0x40, 0xEA, 0xFE, 0x41, 0xE3, 0xFD,
    0x41, 0xDC, 0xFD, 0x41, 0xD6, 0xFD, 0x42, 0xCF, 0xFC, 0x42, 0xC8, 0xFC, 0x42, 0xC1, 0xFC, 0x43,
    0xBA, 0xFB, 0x43, 0xB4, 0xFB, 0x43, 0xAD, 0xFB, 0x44, 0xA6, 0xFA, 0x44, 0x9F, 0xFA, 0x45, 0x98,
    0xFA, 0x45, 0x92, 0xF9, 0x45, 0x8B, 0xF9, 0x46, 0x84, 0xF9, 0x46, 0x7D, 0xF8, 0x46, 0x76, 0xF8,
    0x46, 0x6F, 0xF8, 0x47, 0x68, 0xF8, 0x47, 0x61, 0xF7, 0x47, 0x5A, 0xF7, 0x48, 0x53, 0xF7, 0x48,
    0x4C, 0xF6, 0x48, 0x45, 0xF6, 0x49, 0x3E, 0xF6, 0x49, 0x37, 0xF6, 0x4A, 0x30, 0xF5, 0x4A, 0x29,
    0xF5, 0x4A, 0x22, 0xF5, 0x4B, 0x1B, 0xF5, 0x4B, 0x14, 0xF4, 0x4B, 0x0D, 0xF4, 0x4C, 0x06, 0xF4,
    0x4D, 0x04, 0xF1, 0x51, 0x0D, 0xE9, 0x55, 0x16, 0xE1, 0x59, 0x1F, 0xD9, 0x5D, 0x28, 0xD1, 0x61,
    0x31, 0xC9, 0x65, 0x3A, 0xC1, 0x69, 0x42, 0xB9, 0x6D, 0x4B, 0xB1, 0x71, 0x54, 0xA9, 0x75, 0x5D,
    0xA1, 0x79, 0x66, 0x99, 0x7D, 0x6F, 0x91, 0x81, 0x78, 0x89, 0x86, 0x80, 0x81, 0x8A, 0x88, 0x7A,
    0x8E, 0x90, 0x72, 0x92, 0x98, 0x6A, 0x96, 0xA1, 0x62, 0x9A, 0xA9, 0x5A, 0x9E, 0xB1, 0x52, 0xA2,
    0xBA, 0x4A, 0xA6, 0xC2, 0x42, 0xAA, 0xCA, 0x3A, 0xAE, 0xD3, 0x32, 0xB2, 0xDB, 0x2A, 0xB6, 0xE3,
    0x22, 0xBA, 0xEB, 0x1A, 0xBE, 0xF4, 0x12, 0xC2, 0xFC, 0x0A, 0xC6, 0xF5, 0x02, 0xCA, 0xE6, 0x09,
    0xCE, 0xD8, 0x18, 0xD1, 0xCA, 0x27, 0xD5, 0xBB, 0x36, 0xD8, 0xAD, 0x45, 0xDC, 0x9E, 0x54, 0xE0,
    0x90, 0x62, 0xE3, 0x82, 0x6F, 0xE6, 0x71, 0x7C, 0xEA, 0x61, 0x89, 0xEE, 0x51, 0x96, 0xF2, 0x40,
    0xA3, 0xF5, 0x30, 0xB0, 0xF9, 0x20, 0xBD, 0xFD, 0x0F, 0xE3, 0xFF, 0x01, 0xE9, 0xFF, 0x01, 0xEE,
    0xFF, 0x01, 0xF4, 0xFF, 0x00, 0xFA, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x0A, 0xFF, 0xFF,
    0x15, 0xFF, 0xFF, 0x20, 0xFF, 0xFF, 0x2B, 0xFF, 0xFF, 0x36, 0xFF, 0xFF, 0x41, 0xFF, 0xFF, 0x4C,
    0xFF, 0xFF, 0x57, 0xFF, 0xFF, 0x62, 0xFF, 0xFF, 0x6D, 0xFF, 0xFF, 0x78, 0xFF, 0xFF, 0x81, 0xFF,
    0xFF, 0x8A, 0xFF, 0xFF, 0x92, 0xFF, 0xFF, 0x9A, 0xFF, 0xFF, 0xA3, 0xFF, 0xFF, 0xAB, 0xFF, 0xFF,
    0xB4, 0xFF, 0xFF, 0xBC, 0xFF, 0xFF, 0xC4, 0xFF, 0xFF, 0xCD, 0xFF, 0xFF, 0xD5, 0xFF, 0xFF, 0xDD,
    0xFF, 0xFF, 0xE6, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF9,
    0xFF, 0xFF, 0xF3, 0xFF, 0xFF, 0xED, 0xFF, 0xFF, 0xE7, 0xFF, 0xFF, 0xE1, 0xFF, 0xFF, 0xDB, 0xFF,
    0xFF, 0xD5, 0xFF, 0xFF, 0xCF, 0xFF, 0xFF, 0xCA, 0xFF, 0xFF, 0xC4, 0xFF, 0xFF, 0xBE, 0xFF, 0xFF,
    0xB8, 0xFF, 0xFF, 0xB2, 0xFF, 0xFF, 0xAC, 0xFF, 0xFF, 0xA6, 0xFF, 0xFF, 0xA0, 0xFF, 0xFF, 0x9B,
    0xFF, 0xFF, 0x96, 0xFF, 0xFF, 0x90, 0xFF, 0xFF, 0x8B, 0xFF, 0xFF, 0x86, 0xFF, 0xFF, 0x81, 0xFF,
    0xFF, 0x7B, 0xFF, 0xFF, 0x76, 0xFF, 0xFF, 0x71, 0xFF, 0xFF, 0x6B, 0xFF, 0xFF, 0x66, 0xFF, 0xFF,
    0x61, 0xFF, 0xFF, 0x5C, 0xFF, 0xFF, 0x56, 0xFF, 0xFF, 0x51, 0xFF, 0xFF, 0x4C, 0xFF, 0xFF, 0x47,
    0xFF, 0xFF, 0x41, 0xF9, 0xFF, 0x40, 0xF0, 0xFF, 0x40, 0xE8, 0xFF, 0x40, 0xDF, 0xFF, 0x40, 0xD7,
    0xFF, 0x40, 0xCF, 0xFF, 0x40, 0xC6, 0xFF, 0x40, 0xBE, 0xFF, 0x40, 0xB5, 0xFF, 0x40, 0xAD, 0xFF,
    0x40, 0xA4, 0xFF, 0x40, 0x9C, 0xFF, 0x40, 0x95, 0xFF, 0x40, 0x8D, 0xFF, 0x40, 0x86, 0xFF, 0x40,
    0x7E, 0xFF, 0x40, 0x77, 0xFF, 0x40, 0x6F, 0xFF, 0x40, 0x68, 0xFF, 0x40, 0x60, 0xFF, 0x40, 0x59,
    0xFF, 0x40, 0x51, 0xFF, 0x40, 0x4A, 0xFA, 0x43, 0x42, 0xF3, 0x48, 0x3E, 0xED, 0x4E, 0x3D, 0xE6,
    0x53, 0x3B, 0xDF, 0x58, 0x39, 0xD8, 0x5E, 0x37, 0xD2, 0x63, 0x35, 0xCB, 0x68, 0x34, 0xC4, 0x6D,
    0x32, 0xBD, 0x73, 0x30, 0xB7, 0x78, 0x2E, 0xB0, 0x7D, 0x2D, 0xA9, 0x83, 0x2B, 0xA2, 0x88, 0x29,
    0x9C, 0x8D, 0x27, 0x95, 0x92, 0x25, 0x8E, 0x98, 0x24, 0x87, 0x9D, 0x22, 0x81, 0xA2, 0x20, 0x7A,
    0xA6, 0x1E, 0x74, 0xAB, 0x1D, 0x6E, 0xB0, 0x1B, 0x68, 0xB5, 0x1A, 0x61, 0xB9, 0x18, 0x5B, 0xBE,
    0x17, 0x55, 0xC3, 0x15, 0x4F, 0xC8, 0x13, 0x48, 0xCD, 0x12, 0x42, 0xD1, 0x10, 0x3C, 0xD6, 0x0F,
    0x36, 0xDB, 0x0D, 0x2F, 0xE0, 0x0C, 0x29, 0xE4, 0x0A, 0x23, 0xE9, 0x08, 0x1D, 0xEE, 0x07, 0x16,
    0xF3, 0x05, 0x10, 0xF7, 0x04, 0x0A, 0xFC, 0x02, 0x04, 0xFB, 0x01, 0x04, 0xEF, 0x00, 0x12, 0xE4,
    0x00, 0x1F, 0xD9, 0x00, 0x2D, 0xCE, 0x00, 0x3B, 0xC2, 0x00, 0x48, 0xB7, 0x00, 0x56, 0xAC, 0x00,
    0x64, 0xA0, 0x00, 0x72, 0x95, 0x00, 0x7F, 0x8A, 0x00, 0x88, 0x7F, 0x00, 0x92, 0x75, 0x00, 0x9C,
    0x6B, 0x00, 0xA6, 0x61, 0x00, 0xB0, 0x57, 0x00, 0xBA, 0x4E, 0x00, 0xC4, 0x44, 0x00, 0xCE, 0x3A,
    0x00, 0xD8, 0x30, 0x00, 0xE2, 0x27, 0x00, 0xEC, 0x1D, 0x00, 0xF6, 0x13, 0x00, 0xFF, 0x09, 0x00
};

// This function does any needed initialization on the rendering
// context. 
void SetupRC(void)
{
    // Background
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f );

    viewFrame.MoveForward(5.0f);

    juliaShader = gltLoadShaderPairWithAttributes("../../Data/Shaders/Chapter11/Julia.vs",
                                                  "../../Data/Shaders/Chapter11/Julia.fs",
                                                  1,
                                                  GLT_ATTRIBUTE_VERTEX, "vVertex");

    locC = glGetUniformLocation(juliaShader, "C");
    locZoom = glGetUniformLocation(juliaShader, "zoom");
    locOffset = glGetUniformLocation(juliaShader, "offset");

    // Geometry for a simple quad
    static const GLfloat quad[] =
    {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };

    // Greate a vertex array object and a vertex buffer for the quad
    // including position and texture coordinates
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);

    // Setup the gradient texture
    glGenTextures(1, &gradient_texture);
    glBindTexture(GL_TEXTURE_1D, gradient_texture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, palette_data);
    glGenerateMipmap(GL_TEXTURE_1D);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

// Cleanup
void ShutdownRC(void)
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &gradient_texture);
}

// Called to draw scene
void RenderScene(void)
{
    static CStopWatch stopwatch;
    static float t = 0.0f;
    float r;

    if (!paused)
        t = stopwatch.GetElapsedSeconds();

    r = t + time_offset;

    float C[2] = { (sinf(r * 0.1f) + cosf(r * 0.23f)) * 0.5f, (cosf(r * 0.13f) + sinf(r * 0.21f)) * 0.5f };
    float offset[2] = { x_offset, y_offset };

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(juliaShader);
    glUniform2fv(locC, 1, C);
    glUniform2fv(locOffset, 1, offset);
    glUniform1f(locZoom, zoom);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glutSwapBuffers();
    glutPostRedisplay();
}

void ChangeSize(int w, int h)
{
    // Prevent a divide by zero
    if(h == 0)
        h = 1;

    // Set Viewport to window dimensions
    glViewport(0, 0, w, h);
}

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case ' ': paused = !paused;
            break;
        case '+': time_offset -= 0.00001f;
            break;
        case '-': time_offset += 0.00001f;
            break;
        case '9': time_offset += 0.0001f;
            break;
        case '3': time_offset -= 0.0001f;
            break;
        case '8': time_offset += 0.01f;
            break;
        case '2': time_offset -= 0.01f;
            break;
        case '7': time_offset += 1.0f;
            break;
        case '1': time_offset -= 1.0f;
            break;
        case ']': zoom *= 1.1f;
            break;
        case '[': zoom /= 1.1f;
            break;
        case 'w': y_offset += zoom;
            break;
        case 'a': x_offset += zoom;
            break;
        case 's': y_offset -= zoom;
            break;
        case 'd': x_offset -= zoom;
            break;
        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);

    glutInit(&argc, argv);
    glutInitContextVersion(3, 2);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Julia Set Renderer");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(Keyboard);
    glutDisplayFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s/n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();

    glutMainLoop();
    ShutdownRC();

    return 0;
}
