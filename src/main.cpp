#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility

//*************************************
// global constants
static const char*	window_name = "sphere";
static const char*	vert_shader_path = "../bin/shaders/sphere.vert";
static const char*	frag_shader_path = "../bin/shaders/sphere.frag";
uint				NUM_TESS = 50;		// initial tessellation factor of the circle as a polygon

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2(1280,720); // initial window size

//*************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

//*************************************
// global variables
int		frame = 0;						// index of rendering frames
int		color_xy = 3;					// (tc.xy,0) for initial color (1=.xxx, 2=.yyy)
float	t0 = 0.0f;						// current simulation parameter
float	t1 = 0.0f;
float	theta = 0.0f;
bool	b_rotate = false;				// rotate sphere?
#ifndef GL_ES_VERSION_2_0
bool	b_wireframe = false;
#endif

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_sphere_vertices;	// host-side vertices

//*************************************
void update()
{
	// update global simulation parameter
	t1 = float(glfwGetTime()) * 0.4f - t0;
	t0 = float(glfwGetTime()) * 0.4f;

	// tricky aspect correction matrix for non-square window
	float aspect = window_size.x/float(window_size.y);
	mat4 aspect_matrix = 
	{
		min(1 / aspect,1.0f), 0, 0, 0,
		0, min(aspect,1.0f), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 view_projection_matrix =
	{
		0,1,0,0,
		0,0,1,0,
		-1,0,0,1,
		0,0,0,1
	};

	// update common uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "color_xy");			if (uloc > -1) glUniform1i(uloc, color_xy);
	uloc = glGetUniformLocation( program, "aspect_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, aspect_matrix );
	uloc = glGetUniformLocation(program, "view_projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, view_projection_matrix);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// notify GL that we use our own program
	glUseProgram( program );

	// bind vertex array object
	glBindVertexArray( vertex_array );

	if (b_rotate) theta = theta + t1;
	float c = cos(theta), s = sin(theta);
	mat4 model_matrix =
	{
		c,-s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	GLint uloc;
	uloc = glGetUniformLocation(program, "model_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, model_matrix);
	
	glDrawElements(GL_TRIANGLES, NUM_TESS * (NUM_TESS/2-1) * 6, GL_UNSIGNED_INT, nullptr);

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
#ifndef GL_ES_VERSION_2_0
	printf( "- press 'w' to toggle wireframe\n" );
#endif
	printf("- press 'd' to toggle (tc.xy.0) > (tc.xxx) > (tc.yyy)\n");
	printf("- press 'r' to rotate the sphere\n");
	printf( "\n" );
}

std::vector<vertex> create_sphere_vertices()
{
	std::vector<vertex> v;
	for (uint i = 0; i <= NUM_TESS/2; i++)
	{
		float ti = PI*i / float(NUM_TESS/2), ci = cos(ti), si = sin(ti);
		for (uint j = 0; j <= NUM_TESS; j++)
		{
			float tj = PI * 2.0f * j / float(NUM_TESS), cj = cos(tj), sj = sin(tj);
			v.push_back({ vec3(si * cj, si * sj, ci), vec3(si * cj, si * sj, ci),vec2(tj/(2*PI),1-ti/PI) });
		}
	}

	return v;
}

void update_index_buffer( const std::vector<vertex>& vertices )
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if(index_buffer)	glDeleteBuffers( 1, &index_buffer );	index_buffer = 0;

	// check exceptions
	if(vertices.empty()){ printf("[error] vertices is empty.\n"); return; }

	// create buffer
	std::vector<uint> indices;
	for (uint i = 0; i < NUM_TESS/2; i++)
	{
		for (uint j = 0; j < NUM_TESS; j++)
		{
			if (i != 0)
			{
				indices.push_back(i * (NUM_TESS + 1) + j);
				indices.push_back((i + 1) * (NUM_TESS + 1) + j);
				indices.push_back(i * (NUM_TESS + 1) + j + 1);
			}

			if (i != NUM_TESS - 1)
			{
				indices.push_back(i * (NUM_TESS + 1) + j + 1);
				indices.push_back((i + 1) * (NUM_TESS + 1) + j);
				indices.push_back((i + 1) * (NUM_TESS + 1) + j + 1);
			}
		}
	}

	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (vertex_array) glDeleteVertexArrays(1, &vertex_array);
	vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if(key==GLFW_KEY_D)
		{
			color_xy = (color_xy == 3) ? 1 : color_xy + 1;
			printf( "> using (texcoord.");
			switch (color_xy) {
			case 1: printf("xxx) as color\n"); break;
			case 2: printf("yyy) as color\n"); break;
			case 3: printf("xy,0) as color\n");
			}
		}
		else if (key == GLFW_KEY_R)
		{
			b_rotate = !b_rotate;
		}
#ifndef GL_ES_VERSION_2_0
		else if(key==GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode( GL_FRONT_AND_BACK, b_wireframe ? GL_LINE:GL_FILL );
			printf( "> using %s mode\n", b_wireframe ? "wireframe" : "solid" );
		}
#endif
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS )
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		printf( "> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y) );
	}
}

void motion( GLFWwindow* window, double x, double y )
{
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glLineWidth( 1.0f );
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	
	// define the position of four corner vertices
	unit_sphere_vertices = std::move(create_sphere_vertices());

	// create vertex buffer; called again when index buffering mode is toggled
	update_index_buffer( unit_sphere_vertices );

	t0 = float(glfwGetTime()) * 0.4f;

	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
