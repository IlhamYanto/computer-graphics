#include <GL/glew.h>
#include <glfw3.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <common/controls.hpp>
#include <common/shader.hpp>
#include <grafkom.hpp>

using namespace std;

GLFWwindow* window;

int random (int imin, int imax) {
    return imin + rand () % (imax-imin+1); // NOLINT
}

float random (float fmin, float fmax) {
    return fmin + (float) rand () / (RAND_MAX / (fmax-fmin)); // NOLINT
}

void generateRandomTree (int n, float height, float planeSize, vector<Object*> &objects) {
    float space[] = {-planeSize/2, 5};
    for (int i = 0; i < n; i++) {
        float x, z;
        x = random (-planeSize/2, planeSize/2);
        
        float s = space[random (0, 1)];
        z = random (s, s+(planeSize/2)-5);
        
        int size = random (5, 10-5+1);
        objects.push_back (new Tree (x, height, z, size));
    }
}

int initGL (const char windowTitle[]) {
    // initialize GLFW. Exit on failed.
    if (!glfwInit ())
        return -1;
    
    glfwWindowHint (GLFW_SAMPLES, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Open a window and create its OpenGL context
    window = glfwCreateWindow (800, 600, windowTitle, nullptr, nullptr);
    
    if (window == nullptr) {
        glfwTerminate ();
        return -1;
    }
    
    glfwMakeContextCurrent (window);
    
    // Initialize GLEW
    glewExperimental = (GLboolean) true;
    if (glewInit () != GLEW_OK) {
        glfwTerminate ();
        return -1;
    }
    
    glfwSetInputMode (window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents ();
    glfwSetCursorPos (window, 1024/2, 768/2);
    glClearColor (150/255.0f, 207/255.0f, 234/255.0f, 0);
    
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    glEnable (GL_CULL_FACE);
    return 0;
}

int main () {
    srand (static_cast <unsigned> (time (0))); // NOLINT
    if (initGL ("RollerCoaster") == -1)
        return -1;
    
    GLuint programID = LoadShaders ("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");
    GLint MatrixID = glGetUniformLocation (programID, "MVP");
    
    // define all the objects here.
    vector<Object*> objects;
    
    // earth
    objects.push_back (new Plane (0, -0.25f, 0, 50));

    // rail
    auto *rail = new Box (-25, 25, -0.25f, 0, -1, 1);
    for (int i = 0; i < 6; i++)
        rail->setFaceColor (i, {64, 64, 64});
    rail->setFaceColor (TOP, {80, 80, 80});
    objects.push_back ((Object*) rail);
    
    Box box (-15, -10, 0, 0.25f, -0.5f, 0.5f);
    objects.push_back (&box);
    
    generateRandomTree (50, -0.25f, 50.0f, objects);
    Cube cube (-5, 2, 0, 4);
    Prism prism (10, 2, -10, 6, 20, 2);
    Cone cone (8, -6, 0, 2, 4);
    Pyramid pyramid (2, 5, 0, 4, 2, 4);
    Cylinder cylinder (-20, 20, 20, 1.75, 4);
    Sphere sphere (0, 5, 0, 3);
    
    Tree tree (0, -0.8f, 0, 4);

    objects.push_back (&cube);
    objects.push_back (&cone);
    objects.push_back (&pyramid);
    objects.push_back (&cylinder);
    objects.push_back (&prism);
    objects.push_back (&tree);
    objects.push_back (&sphere);
    objects.push_back (new Sun (0, 100));
    
    // axis
    objects.push_back (new Box (-100, 100, -0.01f, 0.01f, -0.01f, 0.01f));
    objects.push_back (new Box (-0.01f, 0.01f, -100, 100, -0.01f, 0.01f));
    objects.push_back (new Box (-0.01f, 0.01f, -0.01f, 0.01f, -100, 100));
    
//    objects.push_back (new Train (0, 0, 0, 4, 3.5));
    
    for (auto object : objects)
        object->render ();
    
    while (glfwGetKey (window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose (window) == 0) {
        box.rotateAt ({0, 0, 5}, {1, 0, 0.5});
        cube.orbit ({1, 0, 0}, {0, 0, 0});
        cone.orbit ({-1, 0, 0}, {0, 0, 0});
        pyramid.rotate ({0, 1, 0});
        cylinder.translateTo ({0, 0, 0}, 4);
        prism.scaleBy ({-0.0005f, -0.001f, 0});
        tree.translate ({0.05f, 0, 0});
        tree.rotate ({2, 0, 0});
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        glUseProgram (programID);
        
        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs ();
        glm::mat4 ProjectionMatrix = getProjectionMatrix ();
        glm::mat4 ViewMatrix = getViewMatrix ();
        glm::mat4 ModelMatrix = glm::mat4 (1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv (MatrixID, 1, GL_FALSE, &MVP[0][0]);
    
        for (auto object : objects)
            // render all objects defined
            object->render ();
        
        glfwSwapBuffers (window);
        glfwPollEvents ();
    }
    
    for (int i = 0; i < objects.size (); i++)
        objects[i]->onClean ();
    glDeleteProgram (programID);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate ();
    
    return 0;
}
