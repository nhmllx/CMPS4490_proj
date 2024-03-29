//Isaiah Malleaux
//program: background.cpp
//author:  Gordon Griesel
//date:    2017 - 2018
//
//The position of the background QUAD does not change.
//Just the texture coordinates change.
//In this example, only the x coordinates change.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

//24-bit color:  8 + 8 + 8 = 24
//               R   G   B
//how many colors?  256*256*256 = 16-million+
//
//32-bit color:  8 + 8 + 8     = 24
//               R   G   B   A
//               R   G   B     = 24
//
//char data[1000][3]  aaabbbcccdddeeefff
//char data[1000][4]  aaa bbb ccc ddd eee fff
//
//
class Image {
    public:
        int width, height;
        unsigned char *data;
        ~Image() { delete [] data; }
        Image(const char *fname) {
            if (fname[0] == '\0')
                return;
            //printf("fname **%s**\n", fname);
            char name[40];
            strcpy(name, fname);
            int slen = strlen(name);
            name[slen-4] = '\0';
            //printf("name **%s**\n", name);
            char ppmname[80];
            sprintf(ppmname,"%s.ppm", name);
            //printf("ppmname **%s**\n", ppmname);
            char ts[100];
            //system("convert eball.jpg eball.ppm");
            sprintf(ts, "convert %s %s", fname, ppmname);
            system(ts);
            //sprintf(ts, "%s", name);
            FILE *fpi = fopen(ppmname, "r");
            if (fpi) {
                char line[200];
                fgets(line, 200, fpi);
                fgets(line, 200, fpi);
                while (line[0] == '#')
                    fgets(line, 200, fpi);
                sscanf(line, "%i %i", &width, &height);
                fgets(line, 200, fpi);
                //get pixel data
                int n = width * height * 3;			
                data = new unsigned char[n];			
                for (int i=0; i<n; i++)
                    data[i] = fgetc(fpi);
                fclose(fpi);
            } else {
                printf("ERROR opening image: %s\n",ppmname);
                exit(0);
            }
            unlink(ppmname);
        }
};
//Image img[1] = {"seamless_back.jpg"};
//Image img[2] = {"seamless_back.jpg", "elpis.jpg"};
Image img[3] = {"ce.jpg", "elpis.png", "squid.png"};

class Texture {
    public:
        Image *backImage;
        GLuint backTexture;
        float xc[2];
        float yc[2];
};

class Global {
    public:
        int xres, yres;
        Texture tex;
        Texture elpis;
        Texture squid;
        int flag;

        Global() {
            xres=1000, yres=800;
            flag = 1;
        }
} g;

class X11_wrapper {
    public:
        Display *dpy;
        Window win;
        GLXContext glc;
        X11_wrapper() {
            GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
            //GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
            setup_screen_res(1000, 600);
            dpy = XOpenDisplay(NULL);
            if(dpy == NULL) {
                printf("\n\tcannot connect to X server\n\n");
                exit(EXIT_FAILURE);
            }
            Window root = DefaultRootWindow(dpy);
            XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
            if(vi == NULL) {
                printf("\n\tno appropriate visual found\n\n");
                exit(EXIT_FAILURE);
            } 
            Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
            XSetWindowAttributes swa;
            swa.colormap = cmap;
            swa.event_mask =
                ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask |
                ButtonPressMask | ButtonReleaseMask |
                StructureNotifyMask | SubstructureNotifyMask;
            win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
                    vi->depth, InputOutput, vi->visual,
                    CWColormap | CWEventMask, &swa);
            set_title();
            glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
            glXMakeCurrent(dpy, win, glc);
        }
        void cleanupXWindows() {
            XDestroyWindow(dpy, win);
            XCloseDisplay(dpy);
        }
        void setup_screen_res(const int w, const int h) {
            g.xres = w;
            g.yres = h;
        }
        void reshape_window(int width, int height) {
            //window has been resized.
            setup_screen_res(width, height);
            glViewport(0, 0, (GLint)width, (GLint)height);
            glMatrixMode(GL_PROJECTION); glLoadIdentity();
            glMatrixMode(GL_MODELVIEW); glLoadIdentity();
            glOrtho(0, g.xres, 0, g.yres, -1, 1);
            set_title();
        }
        void set_title() {
            //Set the window title bar.
            XMapWindow(dpy, win);
            XStoreName(dpy, win, "scrolling background (seamless)");
        }
        bool getXPending() {
            return XPending(dpy);
        }
        XEvent getXNextEvent() {
            XEvent e;
            XNextEvent(dpy, &e);
            return e;
        }
        void swapBuffers() {
            glXSwapBuffers(dpy, win);
        }
        void check_resize(XEvent *e) {
            //The ConfigureNotify is sent by the
            //server if the window is resized.
            if (e->type != ConfigureNotify)
                return;
            XConfigureEvent xce = e->xconfigure;
            if (xce.width != g.xres || xce.height != g.yres) {
                //Window size did change.
                reshape_window(xce.width, xce.height);
            }
        }
} x11;

void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics(void);
void render(void);
void render2(void);


//===========================================================================
//===========================================================================
int main()
{
    init_opengl();
    int done=0;
    while (!done) {
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);

            physics();
            if (g.flag == 1)
            {
                render();
            }
            x11.swapBuffers();
        }
    }
    return 0;
}

unsigned char *buildAlphaData(Image *img)
{
    //add 4th component to RGB stream...
    int i;
    int a,b,c;
    unsigned char *newdata, *ptr;
    unsigned char *data = (unsigned char *)img->data;
    newdata = (unsigned char *)malloc(img->width * img->height * 4);
    ptr = newdata;
    for (i=0; i<img->width * img->height * 3; i+=3) {
        a = *(data+0);
        b = *(data+1);
        c = *(data+2);
        *(ptr+0) = a;
        *(ptr+1) = b;
        *(ptr+2) = c;
        //-----------------------------------------------
        //get largest color component...
        //*(ptr+3) = (unsigned char)((
        //		(int)*(ptr+0) +
        //		(int)*(ptr+1) +
        //		(int)*(ptr+2)) / 3);
        //d = a;
        //if (b >= a && b >= c) d = b;
        //if (c >= a && c >= b) d = c;
        //*(ptr+3) = d;
        //-----------------------------------------------
        //this code optimizes the commented code above.
        //*(ptr+3) = (a|b|c);
        if (a == 0 && b == 0 && c == 0)
            *(ptr+3) = 0;
        else
            *(ptr+3) = 1;
        //-----------------------------------------------
        ptr += 4;
        data += 3;
    }
    return newdata;
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //This sets 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //Clear the screen
    glClearColor(1.0, 1.0, 1.0, 1.0);
    //glClear(GL_COLOR_BUFFER_BIT);
    //Do this to allow texture maps
    glEnable(GL_TEXTURE_2D);
    //
    //load the images file into a ppm structure.
    //
    g.tex.backImage = &img[0];
    //create opengl texture elements
    glGenTextures(1, &g.tex.backTexture);
    int w = g.tex.backImage->width;
    int h = g.tex.backImage->height;
    glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, g.tex.backImage->data);
    g.tex.xc[0] = 0.0;
    g.tex.xc[1] = 0.25;
    g.tex.yc[0] = 0.0;
    g.tex.yc[1] = 1.0;

    g.elpis.backImage = &img[1];
    glGenTextures(1, &g.elpis.backTexture);
    w = g.elpis.backImage->width;
    h = g.elpis.backImage->height;
    glBindTexture(GL_TEXTURE_2D, g.elpis.backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, g.elpis.backImage->data);
    g.elpis.xc[0] = 0.0;
    g.elpis.xc[1] = 1.0;
    g.elpis.yc[0] = 0.0;
    g.elpis.yc[1] = 1.0;

    g.squid.backImage = &img[2];
    glGenTextures(1, &g.squid.backTexture);
    w = g.squid.backImage->width;
    h = g.squid.backImage->height;
    glBindTexture(GL_TEXTURE_2D, g.squid.backTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    //glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
    //						GL_RGB, GL_UNSIGNED_BYTE, g.squid.backImage->data);
    unsigned char *data2 = buildAlphaData(&img[2]);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0,
            //GL_RGBA, GL_UNSIGNED_BYTE, g.squid.backImage->data);
        GL_RGBA, GL_UNSIGNED_BYTE, data2);
    g.squid.xc[0] = 0.0;
    g.squid.xc[1] = 1.0;
    g.squid.yc[0] = 0.0;
    g.squid.yc[1] = 1.0;
}

void check_mouse(XEvent *e)
{
    //Did the mouse move?
    //Was a mouse button clicked?
    static int savex = 0;
    static int savey = 0;
    //
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {

            //printf("look: %i\n", savex);
            if (savex > g.xres*0.445 && savex < g.xres*0.605){
                printf("look: %i\n", savex);
                // x11.cleanupXWindows();
                render2();
                printf("hi");
                // XDestroyWindow(x11.dpy, x11.win);
                g.flag = 0;

            }
            //Left button is down
        }
        if (e->xbutton.button==3) {
            //Right button is down
        }
    }
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        //Mouse moved
        savex = e->xbutton.x;
        savey = e->xbutton.y;
    }
}

int check_keys(XEvent *e)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }
    }
    return 0;
}

void physics()
{
    //move the background
    g.tex.xc[0] += 0.001;
    g.tex.xc[1] += 0.001;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    //draw background
    glBindTexture(GL_TEXTURE_2D, g.tex.backTexture);
    glBegin(GL_QUADS);//background
    glTexCoord2f(g.tex.xc[0], g.tex.yc[1]); glVertex2i(0, 0);
    glTexCoord2f(g.tex.xc[0], g.tex.yc[0]); glVertex2i(0, g.yres);
    glTexCoord2f(g.tex.xc[1], g.tex.yc[0]); glVertex2i(g.xres, g.yres);
    glTexCoord2f(g.tex.xc[1], g.tex.yc[1]); glVertex2i(g.xres, 0);
    glEnd();
    //draw background
    /*	glBindTexture(GL_TEXTURE_2D, g.elpis.backTexture);
        glBegin(GL_QUADS);
        glTexCoord2f(g.elpis.xc[0], g.elpis.yc[1]); glVertex2i(0, 0);
        glTexCoord2f(g.elpis.xc[0], g.elpis.yc[0]); glVertex2i(0, 100);
        glTexCoord2f(g.elpis.xc[1], g.elpis.yc[0]); glVertex2i(100, 100);
        glTexCoord2f(g.elpis.xc[1], g.elpis.yc[1]); glVertex2i(100, 0);
        glEnd();*/

    glBindTexture(GL_TEXTURE_2D, g.squid.backTexture);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glColor4ub(255,255,255,255);
    glBegin(GL_QUADS); //start/placeholder
    glTexCoord2f(g.squid.xc[0], g.squid.yc[1]); glVertex2i(g.xres/3, 30);
    glTexCoord2f(g.squid.xc[0], g.squid.yc[0]); glVertex2i(g.xres/3, g.yres*0.50);
    glTexCoord2f(g.squid.xc[1], g.squid.yc[0]); glVertex2i(g.xres*0.70, g.yres*0.50);
    glTexCoord2f(g.squid.xc[1], g.squid.yc[1]); glVertex2i(g.xres * 0.70, 30);
    //glTexCoord2f(g.squid.xc[0], g.squid.yc[1]); glVertex2i(100, 0);
    //glTexCoord2f(g.squid.xc[0], g.squid.yc[0]); glVertex2i(100, 100);
    //glTexCoord2f(g.squid.xc[1], g.squid.yc[0]); glVertex2i(200, 100);
    //glTexCoord2f(g.squid.xc[1], g.squid.yc[1]); glVertex2i(200, 0);
    glEnd();
    glDisable(GL_ALPHA_TEST);
}



void render2()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.5f, 0.5f, 0.5f); // Set grid color

    // Draw vertical lines
    glBegin(GL_LINES);
    for (int x = 0; x <= g.xres; x += 20) { // Adjust 20 for spacing
        glVertex2i(x, 0);
        glVertex2i(x, g.yres);
    }
    glEnd();

    // Draw horizontal lines
    glBegin(GL_LINES);
    for (int y = 0; y <= g.yres; y += 20) { // Adjust 20 for spacing
        glVertex2i(0, y);
        glVertex2i(g.xres, y);
    }
    glEnd();

    // Swap buffers after drawing
    x11.swapBuffers();
}























