#define GL_SILENCE_DEPRECATION

#include <GLUT/glut.h>
#include <stdlib.h>

// Function to handle window resizing
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
}

// Function to render the scene
void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw a triangle
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.0, 0.0);  // Red color
    glVertex2f(100, 100);
    glColor3f(0.0, 1.0, 0.0);  // Green color
    glVertex2f(2460, 100);
    glColor3f(0.0, 0.0, 1.0);  // Blue color
    glVertex2f(100, 1500);
    glEnd();

    glFlush();
}

// Function to handle keyboard events
void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {  // ASCII code for Escape key
        exit(0);
    }
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    glutGameModeString("2560x1600:32");
    glutEnterGameMode();

    // Set callback functions
    glutReshapeFunc(reshape);
    glutDisplayFunc(render);
    glutKeyboardFunc(keyboard);

    // Enter the main loop
    glutMainLoop();

    return 0;
}