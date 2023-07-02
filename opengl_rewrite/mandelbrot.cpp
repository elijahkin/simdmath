#define GL_SILENCE_DEPRECATION

#include <GLUT/glut.h>
#include <stdlib.h>
#include <cmath>

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate color values based on time
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;  // Get elapsed time in seconds
    float red = sin(time * 2.0);  // Varying value between -1.0 and 1.0
    float green = sin(time * 0.7); // Varying value between -1.0 and 1.0
    float blue = sin(time * 1.3);  // Varying value between -1.0 and 1.0

    // Draw a triangle with dynamically changing color
    glBegin(GL_TRIANGLES);
    glColor3f((red + 1.0) / 2.0, (green + 1.0) / 2.0, (blue + 1.0) / 2.0);
    glVertex2f(100, 100);
    glVertex2f(2460, 100);
    glVertex2f(100, 1500);
    glEnd();

    glFlush();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {  // esc
        exit(0);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

	glutInitWindowSize(400,300);
	glutCreateWindow("Mandelbrot");
    // glutGameModeString("2560x1600:32");
    // glutEnterGameMode();

    glutDisplayFunc(render);
    glutIdleFunc(render);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}