// rotating cube with two texture objects
// change textures with 1 and 2 keys
#define _CRT_SECURE_NO_DEPRECATE

#include "Angel.h"
#include "glui.h"

GLuint program;
GLuint vao[3];

typedef struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
}Image;

const int  NumTriangles = 12; // (6 faces)(2 triangles/face)
const int  NumVertices = 3 * NumTriangles;
const int  TextureSize = 64;

GLuint  ModelView, Projection;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

// Texture objects and storage for texture image
GLuint textures[3];

Image *imagetex1;
Image *imagetex2;
Image *imagetex3;

// Vertex data arrays
point4  points1[24];
vec2    tex_coords1[24];
point4  points2[6];
vec2    tex_coords2[6];
point4  points3[12];
vec2    tex_coords3[12];

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Yaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };
GLuint   theta;

//----------------------------------------------------------------------------

int Index = 0;

color4 material_ambient(0.0, 0.0, 0.0, 1.0);
color4 material_diffuse(1.0, 1.0, 1.0, 1.0);
color4 material_specular(1.0, 0.0, 1.0, 1.0);
float  material_shininess = 5.0;

point4 light_position;
color4 light_ambient;
color4 light_diffuse;
color4 light_specular;

point4 vertices[17] = {
		point4(-0.5, -0.5,  0.5, 1.0),
		point4(-0.5,  0.5,  0.5, 1.0),
		point4(0.5,  0.5,  0.5, 1.0),
		point4(0.5, -0.5,  0.5, 1.0),
		point4(-0.5, -0.5, -0.5, 1.0),
		point4(-0.5,  0.5, -0.5, 1.0),
		point4(0.5,  0.5, -0.5, 1.0),
		point4(0.5, -0.5, -0.5, 1.0),
		point4(1.0, -0.5,  1.0, 1.0),
		point4(-1.0, -0.5,  1.0, 1.0),
		point4(-1.0, -0.5, -1.0, 1.0),
		point4(1.0, -0.5, -1.0, 1.0),
		point4(0.0,  1.0, 0.0, 1.0),
		point4(0.5,  0.5, -0.5, 1.0),
		point4(-0.5,  0.5, -0.5, 1.0),
		point4(-0.5,  0.5,  0.5, 1.0),
		point4(0.5,  0.5,  0.5, 1.0)

	};

void
quad(int a, int b, int c, int d,point4 *points,vec2 *tex_coords)
{
	
	points[Index] = vertices[a];
	tex_coords[Index++] = vec2(0.0, 0.0);
	
	points[Index] = vertices[b];
	tex_coords[Index++] = vec2(0.0, 1.0);

	points[Index] = vertices[c];
	tex_coords[Index++] = vec2(1.0, 1.0);

	points[Index] = vertices[a];
	tex_coords[Index++] = vec2(0.0, 0.0);

	points[Index] = vertices[c];
	tex_coords[Index++] = vec2(1.0, 1.0);

	points[Index] = vertices[d];
	tex_coords[Index++] = vec2(1.0, 0.0);

}

//----------------------------------------------------------------------------

void
colorcube()
{
	quad(1, 0, 3, 2, points1, tex_coords1);
	quad(2, 3, 7, 6, points1, tex_coords1);
	quad(4, 5, 6, 7, points1, tex_coords1);
	quad(5, 4, 0, 1, points1, tex_coords1);
	Index = 0;
	quad(8, 9, 10, 11, points2, tex_coords2);
	Index = 0;
	quad(12, 13, 14, 15, points3, tex_coords3);
	quad(12, 13, 16, 15, points3, tex_coords3);
}

//----------------------------------------------------------------------------

int ImageLoad(char *filename, Image *image) {

	FILE *file;
	unsigned long size; // size of the image in bytes.
	unsigned long i; // standard counter.
	unsigned short int planes; // number of planes in image (must be 1)
	unsigned short int bpp; // number of bits per pixel (must be 24)
	char temp; // temporary color storage for bgr-rgb conversion.
			   // make sure the file is there.

	if ((file = fopen(filename, "rb")) == NULL) {
		printf("File Not Found : %s\n", filename);
		return 0;
	}

	// seek through the bmp header, up to the width/height:
	fseek(file, 18, SEEK_CUR);

	// read the width
	if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}

	//printf("Width of %s: %lu\n", filename, image->sizeX);
	// read the height
	if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}

	//printf("Height of %s: %lu\n", filename, image->sizeY);
	// calculate the size (assuming 24 bits or 3 bytes per pixel).
	size = image->sizeX * image->sizeY * 3;

	// read the planes
	if ((fread(&planes, 2, 1, file)) != 1) {
		printf("Error reading planes from %s.\n", filename);
		return 0;
	}

	if (planes != 1) {
		printf("Planes from %s is not 1: %u\n", filename, planes);
		return 0;
	}

	// read the bitsperpixel
	if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s.\n", filename);
		return 0;
	}

	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
	}

	// seek past the rest of the bitmap header.
	fseek(file, 24, SEEK_CUR);
	
	// read the data.
	image->data = (char *)malloc(size);

	if (image->data == NULL) {
		printf("Error allocating memory for color-corrected image data");
		return 0;
	}

	if ((i = fread(image->data, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		return 0;
	}

	for (i = 0; i<size; i += 3) { // reverse all of the colors. (bgr -> rgb)
		temp = image->data[i];
		image->data[i] = image->data[i + 2];
		image->data[i + 2] = temp;
	}

	// we're done.
	return 1;
}


Image * loadTexture(char *filename) {

	Image *image1;
	// allocate space for texture
	image1 = (Image *)malloc(sizeof(Image));

	if (image1 == NULL) {
		printf("Error allocating space for image");
		exit(0);
	}

	if (!ImageLoad(filename, image1)) {
		exit(1);
	}

	return image1;
}

void
init()
{
	colorcube();

	imagetex1 = loadTexture("red_brick.bmp");
	imagetex2 = loadTexture("ground.bmp");
	imagetex3 = loadTexture("green_tile.bmp");

	// Initialize texture objects
	glGenTextures(3, textures);

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imagetex1->sizeX, imagetex1->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, imagetex1->data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imagetex2->sizeX, imagetex2->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, imagetex2->data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imagetex3->sizeX, imagetex3->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, imagetex3->data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	// Create a vertex array object
	glGenVertexArrays(3, vao);
	glBindVertexArray(vao[0]);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(points1) + sizeof(tex_coords1),
		NULL, GL_STATIC_DRAW);

	// Specify an offset to keep track of where we're placing data in our
	//   vertex array buffer.  We'll use the same technique when we
	//   associate the offsets with vertex attribute pointers.
	GLintptr offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(points1), points1);
	offset += sizeof(points1);

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(tex_coords1), tex_coords1);

	// Load shaders and use the resulting shader program
	program = InitShader("vshader71.glsl", "fshader71.glsl");
	glUseProgram(program);

	// set up vertex arrays
	offset = 0;
	GLuint vPosition1 = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition1);
	glVertexAttribPointer(vPosition1, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));
	offset = sizeof(points1);

	GLuint vTexCoord1 = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord1);
	glVertexAttribPointer(vTexCoord1, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));

	// Set the value of the fragment shader texture sampler variable
	//   ("texture") to the the appropriate texture unit. In this case,
	//   zero, for GL_TEXTURE0 which was previously set by calling
	//   glActiveTexture().
	glUniform1i(glGetUniformLocation(program, "texture"), 0);

	glBindVertexArray(vao[1]);

	// Create and initialize a buffer object
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(points2) + sizeof(tex_coords2),
		NULL, GL_STATIC_DRAW);

	// Specify an offset to keep track of where we're placing data in our
	//   vertex array buffer.  We'll use the same technique when we
	//   associate the offsets with vertex attribute pointers.
	offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(points2), points2);
	offset += sizeof(points2);

	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(tex_coords2), tex_coords2);

	// Load shaders and use the resulting shader program
	program = InitShader("vshader71.glsl", "fshader71.glsl");
	glUseProgram(program);

	// set up vertex arrays
	offset = 0;
	GLuint vPosition2 = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition2);
	glVertexAttribPointer(vPosition2, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));
	offset = sizeof(points2);

	GLuint vTexCoord2 = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord2);
	glVertexAttribPointer(vTexCoord2, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));

	// Set the value of the fragment shader texture sampler variable
	//   ("texture") to the the appropriate texture unit. In this case,
	//   zero, for GL_TEXTURE0 which was previously set by calling
	//   glActiveTexture().
	glUniform1i(glGetUniformLocation(program, "texture"), 0);

	glBindVertexArray(vao[2]);

	// Create and initialize a buffer object
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(points3) + sizeof(tex_coords3),
		NULL, GL_STATIC_DRAW);

	// Specify an offset to keep track of where we're placing data in our
	//   vertex array buffer.  We'll use the same technique when we
	//   associate the offsets with vertex attribute pointers.
	offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(points3), points3);
	offset += sizeof(points3);

	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(tex_coords3), tex_coords3);

	// Load shaders and use the resulting shader program
	program = InitShader("vshader71.glsl", "fshader71.glsl");
	glUseProgram(program);

	// set up vertex arrays
	offset = 0;
	GLuint vPosition3 = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition3);
	glVertexAttribPointer(vPosition3, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));
	offset = sizeof(points3);

	GLuint vTexCoord3 = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord3);
	glVertexAttribPointer(vTexCoord3, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset));

	// Set the value of the fragment shader texture sampler variable
	//   ("texture") to the the appropriate texture unit. In this case,
	//   zero, for GL_TEXTURE0 which was previously set by calling
	//   glActiveTexture().
	glUniform1i(glGetUniformLocation(program, "texture"), 0);

	
	theta = glGetUniformLocation(program, "theta");

	// Initialize shader lighting parameters
	light_position = color4(0.0, 0.0, 2.0, 0.0);
	light_ambient = color4(1.0, 1.0, 1.0, 1.0);
	light_diffuse = color4(1.0, 1.0, 1.0, 1.0);
	light_specular = color4(1.0, 1.0, 1.0, 1.0);

	glUniform4fv(glGetUniformLocation(program, "LightPosition"),
		1, light_position);

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");

	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	point4 at(0.0,-0.9, -0.5, 1.0);
	point4 eye(0.0, 0.0, 2.0, 1.0);
	vec4   up(0.0, 1.0, 0.0, 0.0);

	mat4 model_view = LookAt(eye, at, up);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"),
		1, ambient_product);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"),
		1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"),
		1, specular_product);


	glUniform1f(glGetUniformLocation(program, "Shininess"),
		material_shininess);

	glUniform3fv(theta, 1, Theta);

	glBindVertexArray(vao[0]);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glDrawArrays(GL_TRIANGLES, 0, 24);

	glBindVertexArray(vao[1]);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(vao[2]);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDrawArrays(GL_TRIANGLES, 0, 12);

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
		case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
		case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
		}
	}
}

//----------------------------------------------------------------------------

void
idle(void)
{
	Theta[Axis] += 0.01;

	if (Theta[Axis] > 360.0) {
		Theta[Axis] -= 360.0;
	}

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int mousex, int mousey)
{
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	}

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	GLfloat left = -2.0, right = 2.0;
	GLfloat top = 2.0, bottom = -2.0;
	GLfloat zNear = -200.0, zFar = 200.0;

	GLfloat aspect = GLfloat(width) / height;

	if (aspect > 1.0) {
		left *= aspect;
		right *= aspect;
	}
	else {
		top /= aspect;
		bottom /= aspect;
	}

	GLUI_Master.auto_set_viewport();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-aspect*.08, aspect*.08, -.08, .08, .1, 15.0);

	mat4 projection = Ortho(left, right, bottom, top, zNear, zFar);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Villega");
	glewExperimental = TRUE;
	glewInit();

	init();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	

	glutMainLoop();
	return 0;
}