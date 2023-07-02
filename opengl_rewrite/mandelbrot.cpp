#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cmath>

float zoomFactor = 1.0f;

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate color values based on time
    float time = glfwGetTime();
    float red = sin(time * 2.0);  // Varying value between -1.0 and 1.0
    float green = sin(time * 0.7); // Varying value between -1.0 and 1.0
    float blue = sin(time * 1.3);  // Varying value between -1.0 and 1.0

    // Draw a triangle with dynamically changing color
    glBegin(GL_TRIANGLES);
    glColor3f((red + 1.0) / 2.0, (green + 1.0) / 2.0, (blue + 1.0) / 2.0);
    glVertex2f(0, 0);
    glVertex2f(2560, 0);
    glVertex2f(2560/2, 1600);
    glEnd();

    glFlush();
}

void reshape(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    float zoomedWidth = width / zoomFactor;
    float zoomedHeight = height / zoomFactor;

    float center_real = width / 2;
    float center_imag = height / 2;

    float apothem_real = zoomedWidth / 2;
    float apothem_imag = zoomedHeight / 2;

    glOrtho(center_real - apothem_real, center_real + apothem_real,
            center_imag - apothem_imag, center_imag + apothem_imag, -1, 1);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void scroll(GLFWwindow* window, double xoffset, double yoffset) {
    if (yoffset > 0) {
        zoomFactor *= 1.1f; // Increase zoom factor by 10%
    } else {
        zoomFactor *= 0.9f; // Decrease zoom factor by 10%
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    reshape(window, width, height);
}

int main() {
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    GLFWwindow* window = glfwCreateWindow(400, 300, "Mandelbrot", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetKeyCallback(window, keyboard);
    glfwSetScrollCallback(window, scroll);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    reshape(window, width, height);

    while (!glfwWindowShouldClose(window)) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
