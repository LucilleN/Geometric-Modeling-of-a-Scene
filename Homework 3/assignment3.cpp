/***
 Assignment-3: Geometric Modeling of a Scene
 
 Name: Njoo, Lucille
 
 Collaborators: N/A
 ** Note: although the assignment should be completed individually
 you may speak with classmates on high level algorithmic concepts. Please
 list their names in this section
 
 Project Summary: A short paragraph (3-4 sentences) describing the work you
 did for the project.
 ***/


#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#pragma GCC diagnostic pop

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
using namespace std;

// If a float is < EPSILON or > -EPILSON then it should be 0
float EPSILON = 0.000001;
// theta is the angle to rotate the scene
float THETA = 0.0;
// Vector placeholders for the scene and color array
vector<GLfloat> SCENE;
vector<GLfloat> COLOR;

/**************************************************
 *  Rectangular Prisms via Hierarchical Modeling  *
 *                                                *
 *  using planes as building blocks, build a unit *
 *  cube with transformations that will serve as  *
 *  a primitive for modeling objects in the scene *
 *                                                *
 *************************************************/

// Initializes a square plane of unit lengths
vector<GLfloat> init_plane() {
    vector<GLfloat> vertices = {
        +0.5,   +0.5,   +0.0,
        -0.5,   +0.5,   +0.0,
        -0.5,   -0.5,   +0.0,
        +0.5,   -0.5,   +0.0
    };
    return vertices;
}

// Converts degrees to radians for rotation
float deg2rad(float d) {
    return (d*M_PI) / 180.0;
}

// Converts a vector to an array
GLfloat* vector2array(vector<GLfloat> vec) {
    GLfloat* arr = new GLfloat[vec.size()];
    for (int i = 0; i < vec.size(); i++) {
        arr[i] = vec[i];
    }
    return arr;
}

// Converts Cartesian coordinates to homogeneous coordinates
vector<GLfloat> to_homogeneous_coord(vector<GLfloat> cartesian_coords) {
    vector<GLfloat> result;
    for (int i = 0; i < cartesian_coords.size(); i++) {
        result.push_back(cartesian_coords[i]);
        if ((i+1) % 3 == 0) {
            result.push_back(1.0);
        }
    }
    return result;
}

// Converts Cartesian coordinates to homogeneous coordinates
vector<GLfloat> to_cartesian_coord(vector<GLfloat> homogeneous_coords) {
    vector<GLfloat> result;
    for (int i = 0; i < homogeneous_coords.size(); i++) {
        if ((i+1) % 4 == 0) {
            continue;
        } else {
            result.push_back(homogeneous_coords[i]);
        }
    }
    return result;
}

// Definition of a translation matrix
vector<GLfloat> translation_matrix (float dx, float dy, float dz) {
    vector<GLfloat> translate_mat = {
        +1.0,       +0.0,       +0.0,       dx,
        +0.0,       +1.0,       +0.0,       dy,
        +0.0,       +0.0,       +1.0,       dz,
        +0.0,       +0.0,       +0.0,       +1.0
    };
    return translate_mat;
}

// Definition of a scaling matrix
vector<GLfloat> scaling_matrix (float sx, float sy, float sz) {
    vector<GLfloat> scale_mat = {
        sx,         +0.0,       +0.0,       +0.0,
        +0.0,       sy,         +0.0,       +0.0,
        +0.0,       +0.0,       sz,         +0.0,
        +0.0,       +0.0,       +0.0,       +1.0
    };
    return scale_mat;
}

// Definition of a rotation matrix about the x-axis by theta degrees
vector<GLfloat> rotation_matrix_x (float theta) {
    float r = deg2rad(theta);
    vector<GLfloat> rotate_mat_x = {
        +1.0,       +0.0,       +0.0,       +0.0,
        +0.0,       cos(r),     -sin(r),    +0.0,
        +0.0,       sin(r),     cos(r),     +0.0,
        +0.0,       +0.0,       +0.0,       +1.0
    };
    return rotate_mat_x;
}

// Definition of a rotation matrix along the y-axis by theta degrees
vector<GLfloat> rotation_matrix_y (float theta) {
    float r = deg2rad(theta);
    vector<GLfloat> rotate_mat_y = {
        cos(r),     +0.0,       sin(r),     +0.0,
        +0.0,       +1.0,       +0.0,       +0.0,
        -sin(r),    +0.0,       cos(r),     +0.0,
        +0.0,       +0.0,       +0.0,       +1.0
    };
    return rotate_mat_y;
}

// Definition of a rotation matrix along the z-axis by theta degrees
vector<GLfloat> rotation_matrix_z (float theta) {
    float r = deg2rad(theta);
    vector<GLfloat> rotate_mat_z = {
        cos(r),     -sin(r),    +0.0,       +0.0,
        sin(r),     cos(r),     +0.0,       +0.0,
        +0.0,       +0.0,       +1.0,       +0.0,
        +0.0,       +0.0,       +0.0,       +1.0
    };
    return rotate_mat_z;
}

// Perform matrix multiplication for A B
vector<GLfloat> mat_mult(vector<GLfloat> A, vector<GLfloat> B) {
    vector<GLfloat> result;
    for (int i = 0; i < B.size(); i += 4) {
        for (int j = 0; j < A.size(); j += 4) {
            GLfloat dot_product = 0.0;
            for (int k = 0; k < 4; k++){
                GLfloat value = A[j+k] * B[i+k];
                if (value < EPSILON && value > -1.0*EPSILON) {
                    value = 0.0;
                }
                dot_product += value;
            }
            result.push_back(dot_product);
        }
    }
    return result;
}

// Builds a unit cube centered at the origin
vector<GLfloat> build_cube() {
    vector<GLfloat> result = {};
    
    vector<GLfloat> front = mat_mult(translation_matrix(0, 0, 0.5), to_homogeneous_coord(init_plane()));
    vector<GLfloat> back = mat_mult(translation_matrix(0, 0, -0.5), mat_mult(rotation_matrix_y(180), to_homogeneous_coord(init_plane())));
    
    vector<GLfloat> right = mat_mult(translation_matrix(0.5, 0, 0), mat_mult(rotation_matrix_y(90), to_homogeneous_coord(init_plane())));
    vector<GLfloat> left = mat_mult(translation_matrix(-0.5, 0, 0), mat_mult(rotation_matrix_y(-90), to_homogeneous_coord(init_plane())));

    vector<GLfloat> top = mat_mult(translation_matrix(0, 0.5, 0), mat_mult(rotation_matrix_x(-90), to_homogeneous_coord(init_plane())));
    vector<GLfloat> bottom = mat_mult(translation_matrix(0, -0.5, 0), mat_mult(rotation_matrix_x(90), to_homogeneous_coord(init_plane())));
    
    vector<GLfloat> planes[6] = {front, back, right, left, top, bottom};
    
    for (vector<GLfloat> plane : planes) {
        result.insert(result.end(), plane.begin(), plane.end());
    }
    
    return to_cartesian_coord(result);
//    return left;
}

/**************************************************
 *            Camera and World Modeling           *
 *                                                *
 *  create a scene by applying transformations to *
 *  the objects built from planes and position    *
 *  the camera to view the scene by setting       *
 *  the projection/viewing matrices               *
 *                                                *
 *************************************************/

void setup() {
    // Enable the vertex array functionality
    glEnableClientState(GL_VERTEX_ARRAY);
    // Enable the color array functionality (so we can specify a color for each vertex)
    glEnableClientState(GL_COLOR_ARRAY);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Set up some default base color
    glColor3f(0.5, 0.5, 0.5);
    // Set up white background
    glClearColor(1.0, 1.0, 1.0, 0.0);
}

void init_camera() {
    // Camera parameters
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // TODO: Setup your camera here
    
    // Define a 50 degree field of view, 1:1 aspect ratio, near and far planes at 3 and 7
    gluPerspective(50.0, 1.0, 2.0, 10.0);
    // Position camera at (2, 3, 5), attention at (0, 0, 0), up at (0, 1, 0)
    gluLookAt(2.0, 6.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
}

// Construct the scene using objects built from cubes/prisms
vector<GLfloat> init_scene() {
    vector<GLfloat> scene = build_cube();
    
    // TODO: Build your scene here
    
    return scene;
}

// Construct the color mapping of the scene
vector<GLfloat> init_color(vector<GLfloat> scene) {
    vector<GLfloat> colors;
    for (int i = 0; i < scene.size(); i++) {
        colors.push_back(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    }
    return colors;
}

void display_func() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // TODO: Rotate the scene using the scene vector
    vector<GLfloat> scene = SCENE;
    
    GLfloat* scene_vertices = vector2array(scene);
    GLfloat* color_vertices = vector2array(COLOR);
    // Pass the scene vertex pointer
    glVertexPointer(3,                // 3 components (x, y, z)
                    GL_FLOAT,         // Vertex type is GL_FLOAT
                    0,                // Start position in referenced memory
                    scene_vertices);  // Pointer to memory location to read from
    
    // Pass the color vertex pointer
    glColorPointer(3,                   // 3 components (r, g, b)
                   GL_FLOAT,            // Vertex type is GL_FLOAT
                   0,                   // Start position in referenced memory
                   color_vertices);     // Pointer to memory location to read from
    
    // Draw quad point planes: each 4 vertices
    glDrawArrays(GL_QUADS, 0, SCENE.size()/3.0);
    
    glFlush();            //Finish rendering
    glutSwapBuffers();
}

void idle_func() {
//    THETA = THETA + 0.3;
    display_func();
}

int main (int argc, char **argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    // Create a window with rendering context and everything else we need
    glutCreateWindow("Assignment 3");
    
    setup();
    init_camera();
    // Setting global variables SCENE and COLOR with actual values
    SCENE = init_scene();
    COLOR = init_color(SCENE);
    
    // Set up our display function
    glutDisplayFunc(display_func);
    glutIdleFunc(idle_func);
    // Render our world
    glutMainLoop();
    
    // Remember to call "delete" on your dynmically allocated arrays
    // such that you don't suffer from memory leaks. e.g.
    // delete arr;
    
    return 0;
}

