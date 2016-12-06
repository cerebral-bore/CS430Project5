#define GLFW_DLL 1
#define GLFW_INCLUDE_ES2 1

// Wasn't required for my PC but just a safeguard considering 12-6-16's issues with texdemo
#define GLFW_TRUE 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include "../glfw-3.2.1/deps/linmath.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

GLFWwindow* window;
int viewEditFlag = 0;

typedef struct {
  float Position[2];
  float TexCoord[2];
} Vertex;


const Vertex vertices[] = {
  {{-1, 1}, {0, 0}},
  {{1, 1},  {1, 0}},
  {{-1, -1},  {0, 1}},
  {{1, 1}, {1, 0}},

  {{1, -1},  {1, 1}},
  {{-1, -1},  {0, 1}}
};


const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};


typedef struct RGBPixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RGBPixel;

typedef struct PPMimage {
	int width;
	int height;
	int maxColorValue;
	unsigned char *data;
} PPMimage;

PPMimage *buffer;
PPMimage PPMRead(char *inputFilename);

PPMimage PPMRead(char *inputFilename) {
	buffer = (PPMimage*)malloc(sizeof(PPMimage));
	FILE* fh = fopen(inputFilename, "rb");
	if (fh == NULL) {
		fprintf(stderr, "Error: Could not open file. \n");
		exit(1);
	}
	int c = fgetc(fh);
	if (c != 'P') {
		fprintf(stderr, "Error: PPM file is not properly formatted. \n");
		exit(1);
	}
	c = fgetc(fh);
	char ppmVersionNum = c;
	if (ppmVersionNum != '3' && ppmVersionNum != '6') {
		fprintf(stderr, "Error: Only PPM3 and PPM6 are supported.\n");
		exit(1);
	}

	while (c != '\n') {
		c = fgetc(fh);
	}
	
	c = fgetc(fh);
	while (c == '#') {
		while (c != '\n') {
			c = fgetc(fh);
		}
		c = fgetc(fh);
	}
	
	ungetc(c, fh);

	int wh = fscanf(fh, "%d %d", &buffer->width, &buffer->height);
	if (wh != 2) {
		fprintf(stderr, "Error: PPM must have a width and height \n");
		exit(1);
	}
	int mcv = fscanf(fh, "%d", &buffer->maxColorValue);
	if (mcv != 1) {
		fprintf(stderr, "Error: Only one value allowed for max color size \n");
		exit(1);
	}
	if (buffer->maxColorValue != 255) {
		fprintf(stderr, "Error: PPM must be in 8-bit per channel color format\n");
		exit(1);
	}

	buffer->data = (unsigned char*)malloc(buffer->width*buffer->height*sizeof(RGBPixel));
	if (buffer == NULL) {
		fprintf(stderr, "Error: Memory could not be allocated \n");
		exit(1);
	}
	
	if (ppmVersionNum == '3') {
		int i, j;
		for (i = 0; i<buffer->height; i++) {
			for (j = 0; j<buffer->width; j++) {
				RGBPixel *pixel = (RGBPixel*)malloc(sizeof(RGBPixel));
				fscanf(fh, "%hhd %hhd %hhd", &pixel->r, &pixel->g, &pixel->b);
				buffer->data[i*buffer->width * 3 + j * 3] = pixel->r;
				buffer->data[i*buffer->width * 3 + j * 3 + 1] = pixel->g;
				buffer->data[i*buffer->width * 3 + j * 3 + 2] = pixel->b;
			}
		}
	} else if (ppmVersionNum == '6') {
		size_t s = fread(buffer->data, sizeof(RGBPixel), buffer->width*buffer->height, fh);
		if (s != buffer->width*buffer->height) {
			fprintf(stderr, "Error: Improper Size of ppm image.");
			exit(1);
		}
	} else {
		fprintf(stderr, "Error: the PPM version cannot be read. \n");
		exit(1);
	}
	fclose(fh);
	return *buffer;
}


static const char* vertex_shader_text =
	"uniform mat4 MVP;\n"
	"attribute vec2 TexCoordIn;\n"
	"attribute vec2 vPos;\n"
	"varying vec2 TexCoordOut;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
	"    TexCoordOut = TexCoordIn;\n"
	"}\n";

static const char* fragment_shader_text =
	"varying lowp vec2 TexCoordOut;\n"
	"uniform sampler2D Texture;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
	"}\n";

static void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

// Currently all the buttons do is just close the window
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	
	if(key == GLFW_KEY_1 && action == GLFW_PRESS){
		// Change a flag variable to select Translation
		viewEditFlag = 1;
		glfwSetWindowTitle(window, "CS460 Image Viewer - Translation");
	} else if (key == GLFW_KEY_2 && action == GLFW_PRESS){
		// Change a flag variable to select Rotation
		viewEditFlag = 2;
		glfwSetWindowTitle(window, "CS460 Image Viewer - Rotation");
	} else if (key == GLFW_KEY_3 && action == GLFW_PRESS){
		// Change a flag variable to select Scaling
		viewEditFlag = 3;
		glfwSetWindowTitle(window, "CS460 Image Viewer - Scaling");
	} else if (key == GLFW_KEY_4 && action == GLFW_PRESS){
		// Change a flag variable to select Shearing
		viewEditFlag = 4;
		glfwSetWindowTitle(window, "CS460 Image Viewer - Shearing");
	} else if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		// Pressing 'Esc' will close the window
        glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	} else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	} else if (key == GLFW_KEY_UP && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	} else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void glCompileShaderOrDie(GLuint shader) {
	
	GLint compiled;
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	
	if (!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		char* info = malloc(infoLen+1);
		GLint done;
		glGetShaderInfoLog(shader, infoLen, &done, info);
		printf("Unable to compile shader: %s\n", info);
		exit(1);
	}
}

// 4 x 4 image..
unsigned char image[] = {
  255, 0, 0, 255,
  255, 0, 0, 255,
  255, 0, 0, 255,
  255, 0, 0, 255,

  0, 255, 0, 255,
  0, 255, 0, 255,
  0, 255, 0, 255,
  0, 255, 0, 255,

  0, 0, 255, 255,
  0, 0, 255, 255,
  0, 0, 255, 255,
  0, 0, 255, 255,

  255, 0, 255, 255,
  255, 0, 255, 255,
  255, 0, 255, 255,
  255, 0, 255, 255
};





// Error checking function to minimize code in main()
int errCheck(int args, char *argv[]){
	
	// Initial check to see if there are 1 input argument on launch
	if ((args != 2) || (strlen(argv[1]) <= 4)){
		fprintf(stderr, "Error: Program requires usage: 'ezview input.ppm'");
		exit(1);
	}

	// Check the file extension of input file
	char *extIn;
	if(strrchr(argv[1],'.') != NULL){
		extIn = strrchr(argv[1],'.');
	} else {
		printf("Error: Program requires a PPM file");
		exit(1);
	}
	
	// Check to see if the input file is in .ppm format
	if(strcmp(extIn, ".ppm") != 0){
		printf("Error: Input file not a PPM");
		exit(1);
	}

	return(0);
}

int main(int args, char *argv[]) {
	
	// Perform startup argument error checking
	errCheck(args, argv);

	// Setup basic GL specific variables
	GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;


	// This function sets the error callback, which is called with an error code
	// and a human-readable description each time a GLFW error occurs.
	glfwSetErrorCallback(error_callback);

	// Initialize GLFW library
	if (!glfwInit())
		exit(EXIT_FAILURE);

	// We are using GL 2.0 instead of 3.3 like the tutorial shows
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	// This makes it so the window can't be resized
	glfwWindowHint(GLFW_RESIZABLE, 0);
	
	// Create and open a window
	// (x,y,string,"glfwGetPrimaryMonitor()",NULL) Makes the window into a fullscreen window
	window = glfwCreateWindow(640, 480, "CS460 Image Viewer", NULL, NULL);
	
	// If the window isnt initialized, will throw error
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	
	// Setup the keyboard command calling
	glfwSetKeyCallback(window, key_callback);

	// This function makes the OpenGL or OpenGL ES context of the specified
	// window current on the calling thread.
	// Essentially a "hey display this window's contents"
    glfwMakeContextCurrent(window);
	
    // gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

	// NOTE: OpenGL error checks have been omitted for brevity
	
	// Generate a single buffer name and place it in index_buffer
    glGenBuffers(1, &vertex_buffer);
	// Binds the index_buffer to the "vertex array indices" buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	// Populate the ELEMENT_ARRAY Buffer with data storage amount of "sizeof(Indices)"
	// and store the Indices array inside the buffer, then perform a GL_STATIC_DRAW with the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShaderOrDie(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShaderOrDie(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    // more error checking! glLinkProgramOrDie!

    mvp_location = glGetUniformLocation(program, "MVP");
    assert(mvp_location != -1);

    vpos_location = glGetAttribLocation(program, "vPos");
    assert(vpos_location != -1);

    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);

    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);

	// From what I understand, these EnableVertex calls allow for the program position and color
	// values to actually be used for rendering
    glEnableVertexAttribArray(vpos_location);
	// Define what vertex attributes to use for rendering
	// Corrolation with glEnableVertexAttribArray
    glVertexAttribPointer(vpos_location,
			  2,
			  GL_FLOAT,
			  GL_FALSE,
                          sizeof(Vertex),
			  (void*) 0);

    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location,
			  2,
			  GL_FLOAT,
			  GL_FALSE,
                          sizeof(Vertex),
			  (void*) (sizeof(float) * 2));
    
    int image_width = 4;
    int image_height = 4;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, 
		 GL_UNSIGNED_BYTE, image);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);
	

	// Repeat
	while (!glfwWindowShouldClose(window)){
		
		float ratio;
        int width, height;
        mat4x4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);

		// After a frame is rendered, the program needs to display another frame
		// This is done by swapping the front and back buffers to create a stream
		// Of rendered frames, because this program has no animation, all the frames look the same.
		glfwSwapBuffers(window);
		// Process events that have occured, such as keyboard events, window resize, etc.
		// Essentially is a "redraw" tool, has corrolation with 'glSwapBuffers()'
		glfwPollEvents();
		
		}
		
	// Cleanup functions
	// Close the window and terminate the rendering processes
	// Detachs the context from the main thread
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
