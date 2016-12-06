#define GLFW_DLL 1
#define GLFW_INCLUDE_ES2 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

GLFWwindow* window;
int viewEditFlag = 0;

typedef struct {
  float position[3];
  float color[4];
} Vertex;


const Vertex Vertices[] = {
  {{1, -1, 0}, {1, 0, 0, 1}},
  {{1, 1, 0}, {0, 1, 0, 1}},
  {{-1, 1, 0}, {0, 0, 1, 1}},
  {{-1, -1, 0}, {0, 0, 0, 1}}
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
		fprintf(stderr, "Error: Not a PPM file. \n");
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
	}
	
	else if (ppmVersionNum == '6') {
		size_t s = fread(buffer->data, sizeof(RGBPixel), buffer->width*buffer->height, fh);
		if (s != buffer->width*buffer->height) {
			fprintf(stderr, "Error: Improper Size of ppm image.");
			exit(1);
		}
	}
	else {
		fprintf(stderr, "Error: the ppm version cannot be read. \n");
		exit(1);
	}
	fclose(fh);
	return *buffer;
}



char* vertex_shader_src =
  "attribute vec4 Position;\n"
  "attribute vec4 SourceColor;\n"
  "\n"
  "varying vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    DestinationColor = SourceColor;\n"
  "    gl_Position = Position;\n"
  "}\n";


char* fragment_shader_src =
  "varying lowp vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    gl_FragColor = DestinationColor;\n"
  "}\n";


GLint simple_shader(GLint shader_type, char* shader_src) {

  GLint compile_success = 0;

  int shader_id = glCreateShader(shader_type);

  glShaderSource(shader_id, 1, &shader_src, 0);

  glCompileShader(shader_id);

  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);

  if (compile_success == GL_FALSE) {
    GLchar message[256];
    glGetShaderInfoLog(shader_id, sizeof(message), 0, &message[0]);
    printf("glCompileShader Error: %s\n", message);
    exit(1);
  }

  return shader_id;
}


int simple_program() {

  GLint link_success = 0;

  GLint program_id = glCreateProgram();
  GLint vertex_shader = simple_shader(GL_VERTEX_SHADER, vertex_shader_src);
  GLint fragment_shader = simple_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

  glAttachShader(program_id, vertex_shader);
  glAttachShader(program_id, fragment_shader);

  glLinkProgram(program_id);

  glGetProgramiv(program_id, GL_LINK_STATUS, &link_success);

  if (link_success == GL_FALSE) {
    GLchar message[256];
    glGetProgramInfoLog(program_id, sizeof(message), 0, &message[0]);
    printf("glLinkProgram Error: %s\n", message);
    exit(1);
  }

  return program_id;
}


static void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

// Currently all the buttons do is just close the window
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	
	if(key == GLFW_KEY_1 && action == GLFW_PRESS){
		// Change a flag variable to select Translation
		viewEditFlag = 1;
		glfwSetWindowTitle(window, "Hello World - Translation");
	} else if (key == GLFW_KEY_2 && action == GLFW_PRESS){
		// Change a flag variable to select Rotation
		viewEditFlag = 2;
		glfwSetWindowTitle(window, "Hello World - Rotation");
	} else if (key == GLFW_KEY_3 && action == GLFW_PRESS){
		// Change a flag variable to select Scaling
		viewEditFlag = 3;
		glfwSetWindowTitle(window, "Hello World - Scaling");
	} else if (key == GLFW_KEY_4 && action == GLFW_PRESS){
		// Change a flag variable to select Shearing
		viewEditFlag = 4;
		glfwSetWindowTitle(window, "Hello World - Shearing");
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
	
	errCheck(args, argv);

	GLint program_id, position_slot, color_slot;
	GLuint vertex_buffer;
	GLuint index_buffer;

	// This function sets the error callback, which is called with an error code
	// and a human-readable description each time a GLFW error occurs.
	glfwSetErrorCallback(error_callback);

	// Initialize GLFW library
	if (!glfwInit())
	return -1;

		// We are using GL 2.0 instead of 3.3 like the tutorial shows
		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		// This makes it so the window can't be resized
		glfwWindowHint(GLFW_RESIZABLE, 0);
		
		// Create and open a window
		// (x,y,string,"glfwGetPrimaryMonitor()",NULL) Makes the window into a fullscreen window
		window = glfwCreateWindow(640,
								480,
								"Hello World",
								NULL,
								NULL);
		// If the window isnt initialized, will throw error
		if (!window) {
			glfwTerminate();
			printf("glfwCreateWindow Error\n");
			exit(1);
		}

	// This function makes the OpenGL or OpenGL ES context of the specified
	// window current on the calling thread.
	// Essentially a "hey display this window's contents"
	glfwMakeContextCurrent(window);

	// Create a GL program with attached shaders
	program_id = simple_program();

	// Designate a program for OpenGL to work with
	glUseProgram(program_id);

	// From what I understand, these EnableVertex calls allow for the program position and color
	// values to actually be used for rendering
	position_slot = glGetAttribLocation(program_id, "Position");
	color_slot = glGetAttribLocation(program_id, "SourceColor");
	glEnableVertexAttribArray(position_slot);
	glEnableVertexAttribArray(color_slot);

	// Create Buffer
	glGenBuffers(1, &vertex_buffer);

	// Map GL_ARRAY_BUFFER to this buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// Send the data
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	// Generate a single buffer name and place it in index_buffer
	glGenBuffers(1, &index_buffer);
	// Binds the index_buffer to the "vertex array indices" buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	// Populate the ELEMENT_ARRAY Buffer with data storage amount of "sizeof(Indices)"
	// and store the Indices array inside the buffer, then perform a GL_STATIC_DRAW with the buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
	// Setup the keyboard command calling
	glfwSetKeyCallback(window, key_callback);
	

	// Repeat
	while (!glfwWindowShouldClose(window)){
		
		// Set a 'background' color, or use this as a default window color essentially
		glClearColor(0, 104.0/255.0, 55.0/255.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Set the area of the window the drawn image can populate
		glViewport(0, 0, 640, 480);

		// Define what vertex attributes to use for rendering
		// Corrolation with glEnableVertexAttribArray
		glVertexAttribPointer(position_slot,
							  3,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  0);

		glVertexAttribPointer(color_slot,
							  4,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  (GLvoid*) (sizeof(float) * 3));

		// Render some primative shapes using the array data, triangles in this case
		glDrawElements(GL_TRIANGLES,
					   sizeof(Indices) / sizeof(GLubyte),
					   GL_UNSIGNED_BYTE, 0);

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

