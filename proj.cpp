#include <GL/glut.h> // Include the GLUT header fil
#include <cstdio>

// Function to draw text using bitmap fonts
void drawBitmapString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y); // Set the position for drawing text
    for (const char* c = string; *c != '\0'; ++c) {
        glutBitmapCharacter(font, *c); // Render each character in the string
    }
}

// Function to handle window resizing
void reshape(int w, int h) {
    glViewport(0, 0, w, h); // Set the viewport to cover the entire window
    glMatrixMode(GL_PROJECTION); // Switch to the projection matrix
    glLoadIdentity(); // Reset the projection matrix
    gluOrtho2D(0, w, 0, h); // Set up a 2D orthographic projection
    glMatrixMode(GL_MODELVIEW); // Switch back to the modelview matrix
}

// Function to draw the title screen
void drawTitleScreen() {
    glClearColor(0.5, 0.5, 0.5, 1.0); // Set the clear color to gray
    glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

    // Draw the title text
    glColor3f(1.0, 1.0, 1.0); // Set the drawing color to white
    drawBitmapString(100, 200, GLUT_BITMAP_TIMES_ROMAN_24, "BRUH");//Font, text

    // Draw the start button
    glColor3f(0.0, 1.0, 0.0); // Set the drawing color to green
    glBegin(GL_QUADS); // Begin drawing quads
    glVertex2i(200, 100); // Bottom-left corner
    glVertex2i(400, 100); // Bottom-right corner
    glVertex2i(400, 150); // Top-right corner
    glVertex2i(200, 150); // Top-left corner
    glEnd(); // End drawing quads

    // Draw the text on the start button
    glColor3f(0.0, 0.0, 0.0); // Set the drawing color to black
    drawBitmapString(280, 120, GLUT_BITMAP_HELVETICA_12, "Start");

    glFlush(); // Flush the OpenGL pipeline
}

// Function to handle mouse events
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Check if the mouse click is within the bounds of the start button
        if (x >= 200 && x <= 400 && y >= 100 && y <= 150) {
            // Start the game or perform any action you want
            printf("Starting the game...\n");
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // Set up the display mode
    glutInitWindowSize(600, 400); // Set the window size
    glutCreateWindow("Title Screen"); // Create the window with the specified title

    glutReshapeFunc(reshape); // Set the reshape callback function
    glutDisplayFunc(drawTitleScreen); // Set the display callback function
    glutMouseFunc(mouse); // Set the mouse callback function

    glutMainLoop(); // Enter the GLUT event loop

    return 0;
}

