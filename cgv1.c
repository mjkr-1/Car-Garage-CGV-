#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define PI 3.14159265358979323846

static float carX = 330.0f;
static float wheelAngle = 0.0f;
static float carScale = 1.0f;
static float carColor[3] = {0.86f, 0.12f, 0.10f};
static int headlightsOn = 0;
static int lightingOn = 1;
static int nightMode = 0;
static int revving = 0;
static int racing = 0;
static int raceFinished = 0;
static int raceStarted = 0;
static int raceTicks = 0;
static int animationEnabled = 1;
static int selectedCar = 0;
static GLuint garageList;

static void drawText(float x, float y, void *font, const char *text)
{
    glRasterPos2f(x, y);
    while (*text)
        glutBitmapCharacter(font, *text++);
}

static void setColor(float r, float g, float b)
{
    glColor3f(r, g, b);
}

static void drawRect(float x1, float y1, float x2, float y2,
                     float r, float g, float b)
{
    setColor(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static void drawCircle(float x, float y, float radius,
                       float r, float g, float b)
{
    int i;
    setColor(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (i = 0; i <= 48; ++i)
    {
        float angle = 2.0f * (float)PI * (float)i / 48.0f;
        glVertex2f(x + cosf(angle) * radius, y + sinf(angle) * radius);
    }
    glEnd();
}

static void drawGarageScenery(void)
{
    /* The wall, garage door, road, and lane markings never move. */
    glNewList(garageList, GL_COMPILE);

    drawRect(0, 0, 1000, 700, 0.72f, 0.79f, 0.83f);
    drawRect(0, 260, 1000, 700, 0.78f, 0.82f, 0.84f);
    drawRect(45, 365, 955, 655, 0.91f, 0.91f, 0.87f);
    drawRect(65, 385, 935, 625, 0.34f, 0.39f, 0.42f);
    drawRect(85, 405, 915, 605, 0.49f, 0.54f, 0.56f);

    /* Garage door slats */
    {
        int y;
        for (y = 425; y <= 585; y += 32)
        {
            drawRect(85, (float)y, 915, (float)y + 5, 0.30f, 0.35f, 0.37f);
        }
    }

    /* Workbench and wall shelves */
    drawRect(75, 320, 300, 350, 0.36f, 0.22f, 0.12f);
    drawRect(90, 350, 110, 415, 0.24f, 0.25f, 0.25f);
    drawRect(125, 350, 145, 395, 0.77f, 0.56f, 0.14f);
    drawRect(165, 350, 185, 425, 0.22f, 0.40f, 0.53f);
    drawRect(220, 350, 240, 405, 0.66f, 0.29f, 0.16f);

    /* Tool cabinet and wall light */
    drawRect(835, 300, 920, 420, 0.19f, 0.27f, 0.31f);
    drawRect(847, 340, 908, 345, 0.70f, 0.72f, 0.68f);
    drawRect(847, 375, 908, 380, 0.70f, 0.72f, 0.68f);
    drawRect(440, 640, 560, 650, 0.95f, 0.78f, 0.24f);
    drawRect(475, 625, 525, 640, 0.30f, 0.32f, 0.32f);

    /* Floor and road */
    drawRect(0, 0, 1000, 250, 0.48f, 0.48f, 0.46f);
    drawRect(0, 85, 1000, 225, 0.18f, 0.19f, 0.20f);
    drawRect(0, 78, 1000, 84, 0.88f, 0.76f, 0.35f);
    drawRect(0, 226, 1000, 232, 0.88f, 0.76f, 0.35f);

    {
        int x;
        for (x = 20; x < 1000; x += 125)
            drawRect((float)x, 151, (float)x + 62, 157,
                     0.91f, 0.88f, 0.72f);
    }

    /* Pit/parking marks */
    drawRect(265, 95, 270, 218, 0.77f, 0.77f, 0.70f);
    drawRect(735, 95, 740, 218, 0.77f, 0.77f, 0.70f);
    glEndList();
}

static void drawWheel(float x, float y)
{
    int i;

    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(wheelAngle, 0.0f, 0.0f, 1.0f);

    drawCircle(0, 0, 25, 0.07f, 0.08f, 0.09f);
    drawCircle(0, 0, 14, 0.65f, 0.68f, 0.67f);
    drawCircle(0, 0, 5, 0.22f, 0.24f, 0.24f);

    setColor(0.20f, 0.22f, 0.23f);
    glBegin(GL_LINES);
    for (i = 0; i < 8; ++i)
    {
        float angle = 2.0f * (float)PI * (float)i / 8.0f;
        glVertex2f(0, 0);
        glVertex2f(cosf(angle) * 12.0f, sinf(angle) * 12.0f);
    }
    glEnd();
    glPopMatrix();
}

static void drawCarBody(void)
{
    GLfloat material[4] = {carColor[0], carColor[1], carColor[2], 1.0f};

    if (lightingOn)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, material);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 38.0f);
    }
    else
    {
        glColor3fv(carColor);
    }

    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex2f(-105, -13);
    glVertex2f(-92, 12);
    glVertex2f(-62, 18);
    glVertex2f(-38, 53);
    glVertex2f(32, 53);
    glVertex2f(66, 17);
    glVertex2f(94, 10);
    glVertex2f(105, -12);
    glVertex2f(98, -27);
    glVertex2f(-93, -27);
    glEnd();

    /* Windows and trim are intentionally unlit for clear 2D contrast. */
    glDisable(GL_LIGHTING);
    setColor(0.19f, 0.31f, 0.37f);
    glBegin(GL_POLYGON);
    glVertex2f(-48, 20);
    glVertex2f(-30, 46);
    glVertex2f(-3, 46);
    glVertex2f(-3, 20);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(4, 20);
    glVertex2f(4, 46);
    glVertex2f(28, 46);
    glVertex2f(51, 20);
    glEnd();

    setColor(0.87f, 0.89f, 0.86f);
    glBegin(GL_LINES);
    glVertex2f(0, 18);
    glVertex2f(0, 49);
    glEnd();

    setColor(0.95f, 0.72f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(-96, -8);
    glVertex2f(-84, -8);
    glVertex2f(-84, 0);
    glVertex2f(-96, 0);
    glEnd();

    setColor(0.91f, 0.12f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(87, -7);
    glVertex2f(99, -7);
    glVertex2f(99, 2);
    glVertex2f(87, 2);
    glEnd();

    setColor(0.16f, 0.17f, 0.17f);
    glBegin(GL_QUADS);
    glVertex2f(-70, -18);
    glVertex2f(70, -18);
    glVertex2f(70, -23);
    glVertex2f(-70, -23);
    glEnd();

    if (headlightsOn)
    {
        setColor(1.0f, 0.91f, 0.39f);
        glBegin(GL_TRIANGLES);
        glVertex2f(98, -3);
        glVertex2f(195, 21);
        glVertex2f(195, -26);
        glEnd();
    }

    if (revving)
    {
        float flicker = 0.75f + 0.25f * sinf(wheelAngle * 0.12f);
        setColor(1.0f, 0.20f * flicker, 0.02f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-100, -11);
        glVertex2f(-137, -3);
        glVertex2f(-121, -17);
        glVertex2f(-101, -16);
        glVertex2f(-130, -25);
        glVertex2f(-111, -28);
        glEnd();

        drawCircle(-132, -4 + 3.0f * sinf(wheelAngle * 0.04f),
                   6, 0.55f, 0.57f, 0.58f);
        drawCircle(-142, 5 + 2.0f * sinf(wheelAngle * 0.03f),
                   8, 0.68f, 0.69f, 0.68f);
    }

    if (lightingOn)
        glEnable(GL_LIGHTING);
    else
        glDisable(GL_LIGHTING);
}

static void drawCar(void)
{
    glPushMatrix();
    glTranslatef(carX, 192.0f, 0.0f);
    glScalef(carScale, carScale, 1.0f);

    drawWheel(-58, -19);
    drawWheel(58, -19);
    drawCarBody();
    glPopMatrix();
}

static void drawColorSwatches(void)
{
    const float colors[5][3] = {
        {0.86f, 0.12f, 0.10f},
        {0.10f, 0.35f, 0.83f},
        {0.12f, 0.68f, 0.33f},
        {0.96f, 0.65f, 0.08f},
        {0.86f, 0.86f, 0.83f}
    };
    int i;

    drawText(48, 286, GLUT_BITMAP_HELVETICA_12, "PAINT");
    for (i = 0; i < 5; ++i)
    {
        setColor(colors[i][0], colors[i][1], colors[i][2]);
        glBegin(GL_QUADS);
        glVertex2f(48.0f + i * 36.0f, 255);
        glVertex2f(76.0f + i * 36.0f, 255);
        glVertex2f(76.0f + i * 36.0f, 278);
        glVertex2f(48.0f + i * 36.0f, 278);
        glEnd();
    }
}

static void drawInterface(void)
{
    char status[160];

    glDisable(GL_LIGHTING);
    setColor(0.07f, 0.09f, 0.11f);
    drawText(40, 673, GLUT_BITMAP_HELVETICA_18, "PIXEL PIT STOP  |  INTERACTIVE CAR GARAGE");
    drawText(40, 648, GLUT_BITMAP_HELVETICA_12,
             "Right-click for garage menus   |   Click paint chips to customize");

    drawColorSwatches();

    snprintf(status, sizeof(status),
             "Engine: %s    Headlights: %s    Car size: %.1fx    Lighting: %s",
             revving ? "REVVING" : "OFF",
             headlightsOn ? "ON" : "OFF",
             carScale,
             lightingOn ? "ON" : "OFF");
    drawText(450, 286, GLUT_BITMAP_HELVETICA_12, status);

    if (racing)
    {
        if (raceStarted)
            snprintf(status, sizeof(status), "PIT LANE SPRINT   |   Time: %.1f s",
                     (float)raceTicks / 10.0f);
        else
            snprintf(status, sizeof(status), "PIT LANE SPRINT   |   Drive right to start!");
        drawText(700, 286, GLUT_BITMAP_HELVETICA_12, status);
    }

    drawText(48, 42, GLUT_BITMAP_HELVETICA_12,
             "DRIVE: arrows / A,D   REV: R   LIGHTS: H   SIZE: +/-   COLOR: C   PAUSE ANIMATION: SPACE");
    drawText(48, 23, GLUT_BITMAP_HELVETICA_10,
             "OpenGL primitives | display-list scenery | transformations | GLUT menus | ambient/diffuse car lighting");

    if (selectedCar)
        drawText(410, 315, GLUT_BITMAP_HELVETICA_12, "CAR SELECTED");

    if (raceFinished)
        drawText(438, 560, GLUT_BITMAP_HELVETICA_18, "FINISH!  NICE DRIVE");
}

static void display(void)
{
    GLfloat lightPosition[] = {0.0f, 0.0f, 1.0f, 0.0f};
    GLfloat ambient[] = {0.42f, 0.42f, 0.42f, 1.0f};
    GLfloat diffuse[] = {0.80f, 0.80f, 0.80f, 1.0f};

    if (nightMode)
        glClearColor(0.035f, 0.055f, 0.10f, 1.0f);
    else
        glClearColor(0.68f, 0.78f, 0.83f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Draw static scene colors before enabling the car's lighting. */
    glDisable(GL_LIGHTING);
    glCallList(garageList);

    if (nightMode)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.025f, 0.04f, 0.13f, 0.48f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(1000, 0);
        glVertex2f(1000, 700);
        glVertex2f(0, 700);
        glEnd();
        glDisable(GL_BLEND);
        glColor3f(0.92f, 0.95f, 1.0f);
        drawCircle(875, 565, 25, 0.92f, 0.95f, 1.0f);
    }

    if (racing)
    {
        drawRect(850, 85, 857, 225, 0.91f, 0.18f, 0.12f);
        drawRect(857, 85, 864, 225, 0.93f, 0.92f, 0.83f);
    }

    if (lightingOn)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    }
    else
        glDisable(GL_LIGHTING);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    drawCar();

    glDisable(GL_LIGHTING);
    drawInterface();
    glutSwapBuffers();
}

static void reshape(int width, int height)
{
    if (height == 0)
        height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 1000.0, 0.0, 700.0);
    glMatrixMode(GL_MODELVIEW);
}

static void moveCar(float distance)
{
    float oldX = carX;
    carX += distance;
    if (carX < 140.0f)
        carX = 140.0f;
    if (carX > 860.0f)
        carX = 860.0f;
    wheelAngle -= (carX - oldX) * 2.0f;

    if (racing && !raceStarted && carX > 350.0f)
    {
        raceStarted = 1;
        raceTicks = 0;
    }
    if (racing && raceStarted && carX >= 820.0f)
    {
        raceFinished = 1;
        racing = 0;
    }
    glutPostRedisplay();
}

static void timer(int value)
{
    (void)value;
    if (animationEnabled)
    {
        if (revving)
            wheelAngle -= 5.0f;
        if (racing && raceStarted)
            ++raceTicks;
        glutPostRedisplay();
    }
    glutTimerFunc(100, timer, 0);
}

static void keyboard(unsigned char key, int x, int y)
{
    (void)x;
    (void)y;
    switch (key)
    {
    case 27:
        exit(0);
        break;
    case 'a':
    case 'A':
        moveCar(-12.0f);
        break;
    case 'd':
    case 'D':
        moveCar(12.0f);
        break;
    case 'r':
    case 'R':
        revving = !revving;
        break;
    case 'h':
    case 'H':
        headlightsOn = !headlightsOn;
        break;
    case '+':
    case '=':
        if (carScale < 1.35f)
            carScale += 0.05f;
        break;
    case '-':
    case '_':
        if (carScale > 0.70f)
            carScale -= 0.05f;
        break;
    case 'c':
    case 'C':
        if (carColor[0] > 0.75f && carColor[1] < 0.25f)
        {
            carColor[0] = 0.10f; carColor[1] = 0.35f; carColor[2] = 0.83f;
        }
        else if (carColor[2] > 0.70f)
        {
            carColor[0] = 0.12f; carColor[1] = 0.68f; carColor[2] = 0.33f;
        }
        else if (carColor[1] > 0.60f)
        {
            carColor[0] = 0.96f; carColor[1] = 0.65f; carColor[2] = 0.08f;
        }
        else
        {
            carColor[0] = 0.86f; carColor[1] = 0.12f; carColor[2] = 0.10f;
        }
        break;
    case ' ':
        animationEnabled = !animationEnabled;
        break;
    case 'n':
    case 'N':
        nightMode = !nightMode;
        break;
    }
    glutPostRedisplay();
}

static void specialKeys(int key, int x, int y)
{
    (void)x;
    (void)y;
    if (key == GLUT_KEY_LEFT)
        moveCar(-12.0f);
    else if (key == GLUT_KEY_RIGHT)
        moveCar(12.0f);
}

static void mouse(int button, int state, int x, int y)
{
    float worldX, worldY;
    const float colors[5][3] = {
        {0.86f, 0.12f, 0.10f},
        {0.10f, 0.35f, 0.83f},
        {0.12f, 0.68f, 0.33f},
        {0.96f, 0.65f, 0.08f},
        {0.86f, 0.86f, 0.83f}
    };
    int i;

    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
        return;

    worldX = (float)x * 1000.0f / (float)glutGet(GLUT_WINDOW_WIDTH);
    worldY = 700.0f - (float)y * 700.0f / (float)glutGet(GLUT_WINDOW_HEIGHT);

    if (worldY >= 250.0f && worldY <= 282.0f &&
        worldX >= 46.0f && worldX <= 255.0f)
    {
        i = (int)((worldX - 48.0f) / 36.0f);
        if (i >= 0 && i < 5)
        {
            carColor[0] = colors[i][0];
            carColor[1] = colors[i][1];
            carColor[2] = colors[i][2];
        }
    }
    else if (worldX >= carX - 115.0f * carScale &&
             worldX <= carX + 115.0f * carScale &&
             worldY >= 150.0f && worldY <= 260.0f)
    {
        selectedCar = !selectedCar;
    }
    else if (worldX >= 430.0f && worldX <= 570.0f &&
             worldY >= 620.0f && worldY <= 655.0f)
    {
        nightMode = !nightMode;
    }
    glutPostRedisplay();
}

static void startRace(void)
{
    racing = 1;
    raceStarted = 0;
    raceFinished = 0;
    raceTicks = 0;
    carX = 330.0f;
    glutPostRedisplay();
}

static void menuAction(int item)
{
    switch (item)
    {
    case 1:
        headlightsOn = !headlightsOn;
        break;
    case 2:
        revving = !revving;
        break;
    case 3:
        carScale = 1.0f;
        break;
    case 4:
        lightingOn = !lightingOn;
        break;
    case 5:
        nightMode = !nightMode;
        break;
    case 6:
        animationEnabled = !animationEnabled;
        break;
    case 7:
        startRace();
        break;
    case 8:
        racing = 0;
        raceStarted = 0;
        raceFinished = 0;
        carX = 330.0f;
        break;
    case 9:
        exit(0);
        break;
    }
    glutPostRedisplay();
}

static void createMenus(void)
{
    int carMenu = glutCreateMenu(menuAction);
    glutAddMenuEntry("Toggle Headlights (H)", 1);
    glutAddMenuEntry("Rev Engine (R)", 2);
    glutAddMenuEntry("Reset Car Size", 3);

    int environmentMenu = glutCreateMenu(menuAction);
    glutAddMenuEntry("Toggle Day / Night (N)", 5);
    glutAddMenuEntry("Pause / Resume Animation (Space)", 6);

    int lightingMenu = glutCreateMenu(menuAction);
    glutAddMenuEntry("Toggle Ambient / Diffuse Lighting", 4);

    int raceMenu = glutCreateMenu(menuAction);
    glutAddMenuEntry("Start Pit Lane Sprint", 7);
    glutAddMenuEntry("Reset Sprint", 8);

    glutCreateMenu(menuAction);
    glutAddSubMenu("Car", carMenu);
    glutAddSubMenu("Environment", environmentMenu);
    glutAddSubMenu("Lighting", lightingMenu);
    glutAddSubMenu("Race", raceMenu);
    glutAddMenuEntry("Quit", 9);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(80, 40);
    glutCreateWindow("Pixel Pit Stop - Interactive 2D Car Garage");

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);

    garageList = glGenLists(1);
    drawGarageScenery();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    createMenus();
    glutTimerFunc(100, timer, 0);

    glutMainLoop();
    return 0;
}