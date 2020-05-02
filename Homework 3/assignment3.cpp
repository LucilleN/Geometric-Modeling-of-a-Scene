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
}

/**************************************************
 *            TRANSFORMATION HELPERS              *
 *************************************************/

// Given a matrix of points in Cartesian coordinates, returns a new matrix of
// points rotated in either the x, y, or z direction by theta
vector<GLfloat> rotate(vector<GLfloat> points, string axis, float theta) {
    vector<GLfloat> homogenous_original = to_homogeneous_coord(points);
    vector<GLfloat> homogenous_transformed = {};
    
    if (axis == "x") {
        homogenous_transformed = mat_mult(rotation_matrix_x(theta), homogenous_original);
    } else if (axis == "y") {
        homogenous_transformed = mat_mult(rotation_matrix_y(theta), homogenous_original);
    } else if (axis == "z") {
        homogenous_transformed = mat_mult(rotation_matrix_z(theta), homogenous_original);
    } else {  // throw error if not valid axis
        throw;
    }
    
    vector<GLfloat> result = to_cartesian_coord(homogenous_transformed);
    return result;
}

// Scales a matrix of points in Cartesian coordinates by some sx, sy, and sz.
vector<GLfloat> scale(vector<GLfloat> points, GLfloat sx, GLfloat sy, GLfloat sz) {
    vector<GLfloat> homogenous_original = to_homogeneous_coord(points);
    vector<GLfloat> homogenous_transformed = mat_mult(scaling_matrix(sx, sy, sz), homogenous_original);
    vector<GLfloat> result = to_cartesian_coord(homogenous_transformed);
    return result;
}

// Translates a matrix of points in Cartesian coordinates by some dx, dy, and dz.
vector<GLfloat> translate(vector<GLfloat> points, GLfloat dx, GLfloat dy, GLfloat dz) {
    vector<GLfloat> homogenous_original = to_homogeneous_coord(points);
    vector<GLfloat> homogenous_transformed = mat_mult(translation_matrix(dx, dy, dz), homogenous_original);
    vector<GLfloat> result = to_cartesian_coord(homogenous_transformed);
    return result;
}

// Concats each vector in a vector of components onto the end of the given result vector.
vector<GLfloat> concat_components(vector<GLfloat> &result, vector<vector<GLfloat>> &components) {
    for (vector<GLfloat> &component : components) {
        result.insert(result.end(), component.begin(), component.end());
    }
    return result;
}

/**************************************************
 *                 Object Modeling                *
 *                                                *
 *  create hierarchical compositional models by   *
 *  transforming the unit cube and appending      *
 *  sub-components together to form more complex  *
 *  models                                        *
 *                                                *
 *************************************************/

//////////////
//   SOFA   //
//////////////

// Builds a 0.1 x 0.2 x 0.1 sofa leg centered at the origin
vector<GLfloat> build_sofa_leg() {
    return scale(build_cube(), 0.1, 0.2, 0.1);
}

// Builds a 0.89 x 0.3 x 0.8 sofa seat cushion centered at the origin
vector<GLfloat> build_sofa_seat_cushion() {
    return scale(build_cube(), 0.89, 0.3, 0.8);
}

// Builds a slightly reclined 0.89 x 0.7 x 0.3 sofa seat cushion centered at the origin
vector<GLfloat> build_sofa_back_cushion() {
    return rotate(scale(build_cube(), 0.89, 0.7, 0.3), "x", -10);
}

// Builds a 3 x 0.5 x 0.8 sofa base centered at the origin
vector<GLfloat> build_sofa_base() {
    return scale(build_cube(), 3, 0.5, 0.8);
}

// Builds a slightly reclined 3.35 x 1.1 x 0.2 sofa back centered at the origin
vector<GLfloat> build_sofa_back() {
    return scale(build_cube(), 3.35, 1.1, 0.2);
}

// Builds a 0.3 x 1.2 x 1.1 sofa arm centered at the origin
vector<GLfloat> build_sofa_arm() {
    return scale(build_cube(), 0.3, 1.2, 1.1);
}

// Builds a sofa model whose *base* is centered at the origin
vector<GLfloat> build_sofa() {
    vector<GLfloat> result = {};
    GLfloat leg_offset_x = 1.52, leg_offset_y = -0.35, leg_offset_z = 0.36;
    vector<GLfloat> leg_front_left = translate(build_sofa_leg(), -leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_front_right = translate(build_sofa_leg(), leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_back_left = translate(build_sofa_leg(), -leg_offset_x, leg_offset_y, -leg_offset_z-0.22);
    vector<GLfloat> leg_back_right = translate(build_sofa_leg(), leg_offset_x, leg_offset_y, -leg_offset_z-0.22);
    
    vector<GLfloat> base = build_sofa_base();
    vector<GLfloat> back = translate(build_sofa_back(), 0, 0.3, -0.64);
    
    GLfloat seat_offset_x = 0.92, seat_offset_y = 0.35, seat_offset_z = 0.08;
    vector<GLfloat> seat_cushion_left = translate(build_sofa_seat_cushion(), -seat_offset_x, seat_offset_y, seat_offset_z);
    vector<GLfloat> seat_cushion_middle = translate(build_sofa_seat_cushion(), 0, seat_offset_y, seat_offset_z);
    vector<GLfloat> seat_cushion_right = translate(build_sofa_seat_cushion(), seat_offset_x, seat_offset_y, seat_offset_z);
    
    GLfloat back_cush_offset_y = 0.66, back_cush_offset_z = -0.45;
    vector<GLfloat> back_cushion_left = translate(build_sofa_back_cushion(), -seat_offset_x, back_cush_offset_y, back_cush_offset_z);
    vector<GLfloat> back_cushion_middle = translate(build_sofa_back_cushion(), 0, back_cush_offset_y, back_cush_offset_z);
    vector<GLfloat> back_cushion_right = translate(build_sofa_back_cushion(), seat_offset_x, back_cush_offset_y, back_cush_offset_z);
    
    GLfloat arm_offset_x = 1.55, arm_offset_z = -0.13;
    vector<GLfloat> arm_left = translate(build_sofa_arm(), -arm_offset_x, 0.3, arm_offset_z);
    vector<GLfloat> arm_right = translate(build_sofa_arm(), arm_offset_x, 0.3, arm_offset_z);

    vector<GLfloat> components[14] = {
        leg_front_left,
        leg_front_right,
        leg_back_left,
        leg_back_right,
        base,
        back,
        seat_cushion_left,
        seat_cushion_middle,
        seat_cushion_right,
        back_cushion_left,
        back_cushion_middle,
        back_cushion_right,
        arm_left,
        arm_right};
    
    for (vector<GLfloat> component : components) {
        result.insert(result.end(), component.begin(), component.end());
    }
    
    return result;
}

//////////////
//  TABLE   //
//////////////

// Builds a 0.1 x 0.7 x 0.1 table leg centered at the origin
vector<GLfloat> build_table_leg() {
    return scale(build_cube(), 0.1, 0.7, 0.1);
}

// Builds a 2.5 x 0.1 x 1.3 table top centered at the origin
vector<GLfloat> build_table_top() {
    return scale(build_cube(), 2.5, 0.1, 1.3);
}

// Builds a 2.3 x 0.07 x 1.1 table shelf centered at the origin
vector<GLfloat> build_table_shelf() {
    return scale(build_cube(), 2.3, 0.07, 1.1);
}

// Builds a 0.3 X 0.06 X 0.4 book centered at the origin
vector<GLfloat> build_table_book() {
    return scale(build_cube(), 0.3, 0.06, 0.4);
}

// Builds a table whose top is centered at the origin
vector<GLfloat> build_table() {
    vector<GLfloat> result = {};
    
    GLfloat leg_offset_x = 1.15, leg_offset_y = -0.35, leg_offset_z = 0.55;
    vector<GLfloat> leg_front_left = translate(build_table_leg(), -leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_front_right = translate(build_table_leg(), leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_back_left = translate(build_table_leg(), -leg_offset_x, leg_offset_y, -leg_offset_z);
    vector<GLfloat> leg_back_right = translate(build_table_leg(), leg_offset_x, leg_offset_y, -leg_offset_z);
    
    vector<GLfloat> top = build_table_top();
    vector<GLfloat> shelf = translate(build_table_shelf(), 0, -0.5, 0);
    
    GLfloat book_offset_x = -0.75, book_offset_y = 0.06, book_offset_z = 0.15;
    vector<GLfloat> book_1 = translate(rotate(build_table_book(), "y", 30), book_offset_x, book_offset_y, book_offset_z);
    vector<GLfloat> book_2 = translate(rotate(build_table_book(), "y", -5), book_offset_x + 0.01, 2 * book_offset_y, book_offset_z);
    vector<GLfloat> book_3 = translate(rotate(build_table_book(), "y", 8), book_offset_x - 0.01, 3 * book_offset_y, book_offset_z);
    
    
    vector<GLfloat> components[9] = {
        leg_front_left,
        leg_front_right,
        leg_back_left,
        leg_back_right,
        top,
        shelf,
        book_1,
        book_2,
        book_3
    };
    
    for (vector<GLfloat> component : components) {
        result.insert(result.end(), component.begin(), component.end());
    }
    
    return result;
}

//////////////
//   LAMP   //
//////////////

// Builds a trapezoidal volume (square pyramid with top cut off) centered at the origin
vector<GLfloat> build_trapezoidal_volume() {
    vector<GLfloat> result = {};
    
    vector<GLfloat> front = translate(init_plane(), 0, 0, 0.5);
    vector<GLfloat> back = translate(rotate(init_plane(), "y", 180), 0, 0, -0.5);

    vector<GLfloat> right = translate(rotate(init_plane(), "y", 90), 0.5, 0, 0);
    vector<GLfloat> left = translate(rotate(init_plane(), "y", -90), -0.5, 0, 0);

    vector<GLfloat> cube_sides[4] = {front, back, right, left};
    
    for (vector<GLfloat> face : cube_sides) {
        result.insert(result.end(), face.begin(), face.end());
    }
    
    vector<GLfloat> top = translate(rotate(init_plane(), "x", -90), 0, 0.5, 0);
    
    // Find all the points that are along the top of the cube and move them inwards;
    // these points will be at (0.5, 0.5, 0.5), (0.5, 0.5, -0.5), etc.
    for (int i = 0; i < result.size(); i += 3) {
        if (result[i+1] == 0.5) {
            result[i] = result[i] < 0 ? result[i] + 0.2 : result[i] - 0.2;
            result[i+2] = result[i+2] < 0 ? result[i+2] + 0.2 : result[i+2] - 0.2;
        }
    }
    
    return result;
}

// Builds a thin rod for the inside of the lamp shade
vector<GLfloat> build_shade_support() {
    return scale(build_cube(), 0.7, 0.01, 0.01);
}

// Builds a 0.07 x 2 x 0.07 lamp pole
vector<GLfloat> build_lamp_pole() {
    return scale(build_cube(), 0.07, 2, 0.07);
}

// Builds a 0.5 x 0.12 x 0.5 lamp base
vector<GLfloat> build_lamp_base() {
    return scale(build_cube(), 0.5, 0.12, 0.5);
}

vector<GLfloat> build_lamp() {
    vector<GLfloat> result = {};
    
    vector<GLfloat> shade = translate(scale(build_trapezoidal_volume(), 0.75, 0.5, 0.75), 0, 2, 0);
    vector<GLfloat> shade_support_1 = translate(rotate(build_shade_support(), "y", 45), 0, 2.1, 0);
    vector<GLfloat> shade_support_2 = translate(rotate(build_shade_support(), "y", -45), 0, 2.1, 0);
    vector<GLfloat> pole = translate(build_lamp_pole(), 0, 1, 0);
    vector<GLfloat> base = build_lamp_base();
    
    vector<GLfloat> components[5] = {
        shade,
        shade_support_1,
        shade_support_2,
        pole,
        base,
    };
    
    for (vector<GLfloat> component : components) {
        result.insert(result.end(), component.begin(), component.end());
    }
    
    return result;
}

//////////////
//  CHAIR   //
//////////////

// Builds a slightly tilted 1 x 0.15 x 0.9 chair seat
vector<GLfloat> build_chair_seat() {
    return rotate(scale(build_cube(), 1, 0.15, 0.9), "x", -8);
}

// Builds a slightly tilted 0.1 x 1 x 0.1 chair leg
vector<GLfloat> build_chair_leg() {
    return rotate(rotate(scale(build_cube(), 0.1, 0.8, 0.1), "x", 8), "z", -8);
}

// Builds a narrow rod for the various supports on the chair
vector<GLfloat> build_chair_support() {
    return scale(build_cube(), 0.95, 0.04, 0.04);
}

// Builds a vertical narrow rod for the back supports on the chair
vector<GLfloat> build_back_support() {
    return rotate(rotate(build_chair_support(), "z", 90), "x", -8);
}

// Builds the scaffolding at the top of the back of the chair
vector<GLfloat> build_chair_scaffold() {
    return rotate(scale(build_cube(), 1, 0.35, 0.12), "x", -8);
}

// Builds a rod/pole that goes on the sides of the chair back
vector<GLfloat> build_chair_back_component() {
    return rotate(scale(build_cube(), 0.08, 0.8, 0.08), "x", -8);
}

// Builds a chair arm with the arm rest portion centered at the origin
vector<GLfloat> build_chair_arm() {
    vector<GLfloat> unit_cube = build_cube();
    vector<GLfloat> arm_rest = rotate(scale(unit_cube, 0.13, 0.08, 0.8), "x", -8);
    vector<GLfloat> arm_support = translate(scale(unit_cube, 0.05, 0.4, 0.05), 0, -0.2, 0.3);
    arm_rest.insert(arm_rest.end(), arm_support.begin(), arm_support.end());
    return arm_rest;
}

// Builds a chair model whose seat is centered at the origin, facing the camera
vector<GLfloat> build_chair() {
    vector<GLfloat> result = {};
    
    vector<GLfloat> seat = build_chair_seat();
    
    GLfloat leg_offset_x = 0.45, leg_offset_y = -0.4, leg_offset_z = 0.45;
    vector<GLfloat> leg_front_left = translate(rotate(build_chair_leg(), "y", 90), -leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_front_right = translate(rotate(build_chair_leg(), "y", 180), leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_back_left = translate(build_chair_leg(), -leg_offset_x, leg_offset_y, -leg_offset_z);
    vector<GLfloat> leg_back_right = translate(rotate(build_chair_leg(), "y", -90), leg_offset_x, leg_offset_y, -leg_offset_z);

    vector<GLfloat> support_front = translate(build_chair_support(), 0, leg_offset_y, leg_offset_z);
    vector<GLfloat> support_back = translate(build_chair_support(), 0, leg_offset_y, -leg_offset_z);
    vector<GLfloat> support_left = translate(rotate(build_chair_support(), "y", 90), -leg_offset_x, leg_offset_y, 0);
    vector<GLfloat> support_right = translate(rotate(build_chair_support(), "y", 90), leg_offset_x, leg_offset_y, 0);

    vector<GLfloat> arm_left = translate(build_chair_arm(), -leg_offset_x, 0.4, 0);
    vector<GLfloat> arm_right = translate(build_chair_arm(), leg_offset_x, 0.4, 0);
    
    vector<GLfloat> back_scaffold = translate(build_chair_scaffold(), 0, 0.85, -0.5);
    
    GLfloat back_offset_x = 0.4, back_offset_y = 0.4, back_offset_z = -0.4;
    vector<GLfloat> back_left = translate(build_chair_back_component(), -back_offset_x, back_offset_y, back_offset_z);
    vector<GLfloat> back_right = translate(build_chair_back_component(), back_offset_x, back_offset_y, back_offset_z);

    GLfloat support_offset_x = 0.18;
    vector<GLfloat> back_support_left = translate(build_back_support(), -support_offset_x, back_offset_y, back_offset_z);
    vector<GLfloat> back_support_middle = translate(build_back_support(), 0, back_offset_y, back_offset_z);
    vector<GLfloat> back_support_right = translate(build_back_support(), support_offset_x, back_offset_y, back_offset_z);
    
    vector<GLfloat> components[17] = {
        seat,
        leg_front_left,
        leg_front_right,
        leg_back_left,
        leg_back_right,
        support_front,
        support_back,
        support_left,
        support_right,
        arm_left,
        arm_right,
        back_left,
        back_right,
        back_scaffold,
        back_support_left,
        back_support_middle,
        back_support_right
    };
    
    for (vector<GLfloat> component : components) {
        result.insert(result.end(), component.begin(), component.end());
    }
    
    return result;
}

//////////////
//SIDE TABLE//
//////////////

// Builds a 3 x 0.8 x 0.8 side table frame centered at the origin
vector<GLfloat> build_side_table_frame() {
    return scale(build_cube(), 3, 0.8, 0.8);
}

// Builds a 0.05 x 0.2 x 0.05 side table leg centered at the origin
vector<GLfloat> build_side_table_leg() {
    return scale(build_cube(), 0.05, 0.2, 0.05);
}

// Builds a 0.6 x 0.35 x 0.1 drawer face centered at the origin
vector<GLfloat> build_side_table_drawer() {
    return scale(build_cube(), 0.7, 0.35, 0.1);
}

// Builds a drawer knob with a dowel attached, centered at the origin
vector<GLfloat> build_side_table_knob() {
    vector<GLfloat> unit_cube = build_cube();
    vector<GLfloat> knob = scale(unit_cube, 0.06, 0.06, 0.03);
    vector<GLfloat> dowel = translate(scale(unit_cube, 0.04, 0.04, 0.05), 0, 0, -0.03);
    knob.insert(knob.end(), dowel.begin(), dowel.end());
    return knob;
}

// Builds a side table whose frame is centered at the origin and faces the camera.
vector<GLfloat> build_side_table() {
    vector<GLfloat> result = {};
    
    vector<GLfloat> frame = build_side_table_frame();
    
    GLfloat leg_offset_x = 1.4, leg_offset_y = -0.5, leg_offset_z = 0.36;
    vector<GLfloat> leg_front_left = translate(build_side_table_leg(), -leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_front_right = translate(build_side_table_leg(), leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_back_left = translate(build_side_table_leg(), -leg_offset_x, leg_offset_y, -leg_offset_z);
    vector<GLfloat> leg_back_right = translate(build_side_table_leg(), leg_offset_x, leg_offset_y, -leg_offset_z);

    GLfloat drawer_offset_x = 0.72, drawer_offset_y = 0.18, drawer_offset_z = 0.36;
    vector<GLfloat> drawer_top_1 = translate(build_side_table_drawer(), -1.5 * drawer_offset_x, drawer_offset_y, drawer_offset_z);
    vector<GLfloat> drawer_top_2 = translate(build_side_table_drawer(), -0.5 * drawer_offset_x, drawer_offset_y, drawer_offset_z);
    vector<GLfloat> drawer_top_3 = translate(build_side_table_drawer(), 0.5 * drawer_offset_x, drawer_offset_y, drawer_offset_z);
    vector<GLfloat> drawer_top_4 = translate(build_side_table_drawer(), 1.5 * drawer_offset_x, drawer_offset_y, drawer_offset_z);
    vector<GLfloat> drawer_bottom_1 = translate(build_side_table_drawer(), -1.5 * drawer_offset_x, -drawer_offset_y, drawer_offset_z);
    vector<GLfloat> drawer_bottom_2 = translate(build_side_table_drawer(), -0.5 * drawer_offset_x, -drawer_offset_y, drawer_offset_z);
    vector<GLfloat> drawer_bottom_3 = translate(build_side_table_drawer(), 0.5 * drawer_offset_x, -drawer_offset_y, drawer_offset_z);
    vector<GLfloat> drawer_bottom_4 = translate(build_side_table_drawer(), 1.5 * drawer_offset_x, -drawer_offset_y, drawer_offset_z);
    
    GLfloat knob_offset_z = 0.45;
    vector<GLfloat> knob_top_1 = translate(build_side_table_knob(), -1.5 * drawer_offset_x, drawer_offset_y, knob_offset_z);
    vector<GLfloat> knob_top_2 = translate(build_side_table_knob(), -0.5 * drawer_offset_x, drawer_offset_y, knob_offset_z);
    vector<GLfloat> knob_top_3 = translate(build_side_table_knob(), 0.5 * drawer_offset_x, drawer_offset_y, knob_offset_z);
    vector<GLfloat> knob_top_4 = translate(build_side_table_knob(), 1.5 * drawer_offset_x, drawer_offset_y, knob_offset_z);
    vector<GLfloat> knob_bottom_1 = translate(build_side_table_knob(), -1.5 * drawer_offset_x, -drawer_offset_y, knob_offset_z);
    vector<GLfloat> knob_bottom_2 = translate(build_side_table_knob(), -0.5 * drawer_offset_x, -drawer_offset_y, knob_offset_z);
    vector<GLfloat> knob_bottom_3 = translate(build_side_table_knob(), 0.5 * drawer_offset_x, -drawer_offset_y, knob_offset_z);
    vector<GLfloat> knob_bottom_4 = translate(build_side_table_knob(), 1.5 * drawer_offset_x, -drawer_offset_y, knob_offset_z);
    
    vector<GLfloat> book = translate(rotate(scale(build_table_book(), 1.5, 1.5, 1.5), "y", 10), -0.7, 0.44, 0);
    
    vector<GLfloat> components[22] = {
        frame,
        leg_front_left,
        leg_front_right,
        leg_back_left,
        leg_back_right,
        drawer_top_1,
        drawer_top_2,
        drawer_top_3,
        drawer_top_4,
        drawer_bottom_1,
        drawer_bottom_2,
        drawer_bottom_3,
        drawer_bottom_4,
        knob_top_1,
        knob_top_2,
        knob_top_3,
        knob_top_4,
        knob_bottom_1,
        knob_bottom_2,
        knob_bottom_3,
        knob_bottom_4,
        book
    };
    
    for (vector<GLfloat> component : components) {
        result.insert(result.end(), component.begin(), component.end());
    }
    
    return result;
}

//////////////
//FOOT STOOL//
//////////////

// Builds a 0.6 x 0.6 x 0.6 foot stool box frame centered at the origin
vector<GLfloat> build_foot_stool_frame() {
    return scale(build_cube(), 0.6, 0.6, 0.6);
}

// Builds a 0.05 x 0.08 x 0.05 foot stool leg centered at the origin
vector<GLfloat> build_foot_stool_leg() {
    return scale(build_cube(), 0.05, 0.08, 0.05);
}

// Builds a 0.55 x 0.2 x 0.55 drawer face centered at the origin
vector<GLfloat> build_foot_stool_cushion() {
    return scale(build_cube(), 0.55, 0.2, 0.55);
}

// Builds a foot stool whose frame is centered at the origin
vector<GLfloat> build_foot_stool() {
    vector<GLfloat> result = {};
    
    vector<GLfloat> frame = build_foot_stool_frame();
    vector<GLfloat> cushion = translate(build_foot_stool_cushion(), 0, 0.3, 0);
    
    GLfloat leg_offset_x = 0.25, leg_offset_y = -0.3, leg_offset_z = 0.25;
    vector<GLfloat> leg_front_left = translate(build_side_table_leg(), -leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_front_right = translate(build_side_table_leg(), leg_offset_x, leg_offset_y, leg_offset_z);
    vector<GLfloat> leg_back_left = translate(build_side_table_leg(), -leg_offset_x, leg_offset_y, -leg_offset_z);
    vector<GLfloat> leg_back_right = translate(build_side_table_leg(), leg_offset_x, leg_offset_y, -leg_offset_z);
    
    vector<vector<GLfloat>> components = {
        frame,
        cushion,
        leg_front_left,
        leg_front_right,
        leg_back_left,
        leg_back_right
    };
    
//    for (vector<GLfloat> component : components) {
//        result.insert(result.end(), component.begin(), component.end());
//    }
    
//    return result;
    
    return concat_components(result, components);
}

//////////////
//   ROOM   //
//////////////

// Arranges a couch, table, chair, lamp, side table, and other decorations in a room.
vector<GLfloat> build_room() {
    vector<GLfloat> result = {};
    
    vector<GLfloat> sofa = translate(build_sofa(), 0, 0.3, -2);
    vector<GLfloat> table = translate(build_table(), 0, 0.7, 0);
    vector<GLfloat> lamp = translate(rotate(build_lamp(), "y", 30), 2.3, 0, -2);
    vector<GLfloat> chair = translate(rotate(build_chair(), "y", 230), 2.2, 1, 1.5);
    vector<GLfloat> side_table = translate(rotate(build_side_table(), "y", 180), 0, 0.8, 3);
    vector<GLfloat> foot_stool_1 = translate(rotate(build_foot_stool(), "y", -10), -2.45, 0.45, 1);
    vector<GLfloat> foot_stool_2 = translate(rotate(build_foot_stool(), "y", 15), -2.5, 0.45, -0.5);
    
    vector<GLfloat> components[7] = {
        sofa,
        table,
        lamp,
        chair,
        side_table,
        foot_stool_1,
        foot_stool_2
    };
    
    for (vector<GLfloat> component : components) {
        result.insert(result.end(), component.begin(), component.end());
    }
    
    return result;
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
    
    // Define a 50 degree field of view, 1:1 aspect ratio, near and far planes at 1 and 12
    gluPerspective(50.0, 1.0, 1.0, 12.0);
    // Position camera at (2, 6, 5), attention at (0, 0, 0), up at (0, 1, 0)
    gluLookAt(2.0, 6.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
}

// Construct the scene using objects built from cubes/prisms
vector<GLfloat> init_scene() {
    vector<GLfloat> scene = build_room();
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
    
    vector<GLfloat> scene = rotate(SCENE, "y", THETA);
    
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
    THETA = THETA + 0.05;
    display_func();
}

int main (int argc, char **argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
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
    
    // TODO ASK HERE
    // Remember to call "delete" on your dynmically allocated arrays
    // such that you don't suffer from memory leaks. e.g.
    // delete arr;
    
    return 0;
}

