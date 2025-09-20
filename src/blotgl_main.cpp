#include <GL/osmesa.h>  // OSMesa headers
#include <GL/gl.h>     // Standard OpenGL headers
#include <stdio.h>     // For printf
#include <stdlib.h>    // For malloc/exit

int main() {
    const int width = 640;
    const int height = 480;
    const int bytes_per_pixel = 3;  // For 24-bit RGB

    // Allocate buffer for raw pixel data (row-major, bottom-to-top)
    unsigned char *pixels = malloc(width * height * bytes_per_pixel);
    if (!pixels) {
        fprintf(stderr, "Failed to allocate pixel buffer\n");
        return 1;
    }

    // Create OSMesa context (use OSMESA_RGB for 24-bit; OSMESA_RGB_565 for 16-bit)
    OSMesaContext ctx = OSMesaCreateContext(OSMESA_RGB, NULL);
    if (!ctx) {
        fprintf(stderr, "OSMesaCreateContext failed\n");
        free(pixels);
        return 1;
    }

    // Bind the buffer to the context
    if (!OSMesaMakeCurrent(ctx, pixels, GL_UNSIGNED_BYTE, width, height)) {
        fprintf(stderr, "OSMesaMakeCurrent failed\n");
        OSMesaDestroyContext(ctx);
        free(pixels);
        return 1;
    }

    // Set up viewport and clear color
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Black background
    glClear(GL_COLOR_BUFFER_BIT);

    // Simple rendering: draw a colored triangle
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);  // Red
    glVertex2f(-0.5f, -0.5f);
    glColor3f(0.0f, 1.0f, 0.0f);  // Green
    glVertex2f(0.5f, -0.5f);
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue
    glVertex2f(0.0f, 0.5f);
    glEnd();

    // Finish rendering
    glFinish();

    // Now 'pixels' contains the raw 24-bit RGB data (bottom-to-top rows)
    // Feed this to your terminal canvas library here.
    // For demo, print the first few bytes (red pixel should be near bottom-left)
    printf("Sample pixels: %02x %02x %02x (should be near red)\n", pixels[0], pixels[1], pixels[2]);

    // Cleanup
    OSMesaDestroyContext(ctx);
    free(pixels);
    return 0;
}
