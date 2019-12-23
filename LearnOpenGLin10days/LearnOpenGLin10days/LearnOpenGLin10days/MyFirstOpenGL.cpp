#include <iostream>
#include <map>
#include <iterator>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"


// functions declaration
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
float distanceFromCamera(glm::vec3 pos);
unsigned int loadCubemap(vector<std::string> faces);
void RenderScene(const Shader& shader);
void RenderCube();
void RenderQuad();
void SaveCubeMapToImage(GLuint texture, int width, int height);
glm::vec3 t_TBNCalculation(glm::vec2 delta_u, glm::vec2 delta_v, glm::vec3 edge1, glm::vec3 edge2);
void renderQuad2();
void RenderDeferredShadingScene(Shader shader);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// processInput
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
// exposure for hdr
float exposure = 0.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// global for function calling >> RenderScene(), RenderCube()
unsigned int planeVAO;
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
unsigned int quadVAO = 0;
unsigned int quadVBO;

//perspective projection
float nearPlane = 0.1f;
float farPlane = 100.0f;

std::vector<glm::vec3> lightPositions;
std::vector<glm::vec3> lightColors;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glDepthFunc(GL_LESS); // always pass the depth test (same effect as glDisable(GL_DEPTH_TEST))

	// build and compile shaders
	// -------------------------
	Shader quadShader("quadShader.vs", "quadShader.fs");
	Shader shadowmapQuadShader("shadowmapQuad.vs", "shadowmapQuad.fs");
	Shader shadowmapDepthShader("shadowmapDepth.vs", "shadowmapDepth.fs");
	Shader testsceneShader("testscene.vs", "testscene.fs");
	Shader shadowmapScene("shadowmapScene.vs", "shadowmapScene.fs");

	Shader point_GeometryShader("geometryShader.vs", "geometryShader.fs", "geometryShader.gs");
	Shader pointLightShadowMap("pointLightShadowMap.vs", "pointLightShadowMap.fs", "pointLightShadowMap.gs");
	Shader pointLightShadowMapScene("pointLightShadowMapScene.vs", "pointLightShadowMapScene.fs");

	Shader normalMappingInShadowMapShader("normalMappingInShadowMap.vs", "normalMappingInShadowMap.fs");

	Shader hdrShader("hdrShader.vs", "hdrShader.fs");

	Shader bloomShader("bloom.vs", "bloom.fs");
	Shader lightBloomerShader("bloom.vs", "lightBloomer.fs");
	Shader blurShader("blur.vs", "blur.fs");
	Shader bloom_finalShader("bloom_final.vs", "bloom_final.fs");
	Shader testBrightShader("quadShader.vs", "testBright.fs");

	Shader gBuffersShader("gBuffers.vs", "gBuffers.fs");
	Shader deferredLightingShader("deferredLighting.vs", "deferredLighting.fs");
	Shader forwardRenderingShader("forwardRendering.vs", "forwardRendering.fs");
	//Shader blurShader("blur.vs", "blur.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	float planeVertices[] = {
		// positions            // normals         // texcoords
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// load textures
	// -------------
	unsigned int cubeTexture = loadTexture("C:/Users/DELL EMC/Desktop/LearnOpenGL/allTextureFiles/textures/marble.jpg");
	unsigned int containerTexture = loadTexture("C:/Users/DELL EMC/Desktop/LearnOpenGL/allTextureFiles/textures/container.jpg");
	unsigned int container2Texture = loadTexture("C:/Users/DELL EMC/Desktop/LearnOpenGL/allTextureFiles/textures/container2.png");
	unsigned int container2Texture_specular = loadTexture("C:/Users/DELL EMC/Desktop/LearnOpenGL/allTextureFiles/textures/container2_specular.png");
	unsigned int floorTexture = loadTexture("C:/Users/DELL EMC/Desktop/LearnOpenGL/allTextureFiles/textures/metal.png");
	unsigned int woodTexture = loadTexture("C:/Users/DELL EMC/Desktop/LearnOpenGL/allTextureFiles/textures/wood.png");
	unsigned int brickTexture = loadTexture("C:/Users/DELL EMC/Desktop/LearnOpenGL/allTextureFiles/textures/bricks2.jpg");
	unsigned int brick_normalTexture = loadTexture("C:/Users/DELL EMC/Desktop/LearnOpenGL/allTextureFiles/textures/bricks2_normal.jpg");
	unsigned int brick_depthTexture = loadTexture("C:/Users/DELL EMC/Desktop/LearnOpenGL/allTextureFiles/textures/bricks2_disp.jpg");

	// shader configuration
	// --------------------

	// HDR ColorBuffer

	unsigned int hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

	unsigned int colorBuffers[2];
	glGenTextures(2, colorBuffers);

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	glBindTexture(GL_TEXTURE_2D, 0);
	unsigned int hdrRBO;
	glGenRenderbuffers(1, &hdrRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, hdrRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, hdrRBO); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now

	// unsigned int attactments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	// glDrawBuffers(3, attactments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// gBuffers
	unsigned int gBuffers;
	glGenFramebuffers(1, &gBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffers);

	unsigned int gPos, gNormal, gAlbedoSpec;

	glGenTextures(1, &gPos);
	glBindTexture(GL_TEXTURE_2D, gPos);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPos, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
	glBindTexture(GL_TEXTURE_2D, 0);


	unsigned int gRenderBuffer;
	glGenRenderbuffers(1, &gRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, gRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gRenderBuffer); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now

	unsigned int attactments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attactments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shadow transform

	glm::vec3 lightPos(0.0f, 4.0f, 3.0f);

	// lighting info for hdr testing (w/ exposure)
	// -------------
	// positions
	//std::vector<glm::vec3> lightPositions;
	lightPositions.push_back(glm::vec3(0.0f, 0.5f, 1.5f));
	lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
	lightPositions.push_back(glm::vec3(3.0f, 0.5f, 1.0f));
	lightPositions.push_back(glm::vec3(-.8f, 2.4f, -1.0f));
	// colors
	//std::vector<glm::vec3> lightColors;
	lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));
	lightColors.push_back(glm::vec3(10.0f, 0.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 0.0f, 15.0f));
	lightColors.push_back(glm::vec3(0.0f, 5.0f, 0.0f));

	// light volume
	float constant = 1.0f;
	float linear = 0.7f;
	float quadratic = 1.8f;
	const int nLights = lightPositions.size();
	float lightMax[4];
	for (int i = 0; i < nLights; i++)
	{
		lightMax[i] = fmax(fmax(lightColors[i].r, lightColors[i].g), lightColors[i].b);
	}
	
	float lightRadius[4];
	for (int i = 0; i < nLights; i++)
	{
		lightRadius[i] = (-linear + sqrt(linear * linear - 4 * quadratic * (constant - lightMax[i] * 256 / 5))) / (2 * quadratic);
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// std::cout << deltaTime << std::endl;

		// input
		// -----
		processInput(window);

		// render
		// --------------

		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_PROGRAM_POINT_SIZE);
	
		// 1st pass scene & brightFrag
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffers);

		//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, nearPlane, farPlane);

		// render all object

		gBuffersShader.use();
		/*uniform sampler2D diffuseTexture;
		uniform sampler2D specularTexture;*/
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, container2Texture);
		gBuffersShader.setInt("diffuseTexture", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, container2Texture_specular);
		gBuffersShader.setInt("specularTexture", 1);

		gBuffersShader.setMat4("view", view);
		gBuffersShader.setMat4("projection", projection);

		RenderDeferredShadingScene(gBuffersShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// all deferred lighting pass (2nd)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDisable(GL_DEPTH_TEST);
		deferredLightingShader.use();
		/* uniform sampler2D positionTexture;
		uniform sampler2D normalTexture;
		uniform sampler2D diffuseSpecTexture;
		uniform vec3 viewPos;
		uniform Light lights[4];*/
		
		// all uniform for deferred lighting shader
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPos);
		deferredLightingShader.setInt("positionTexture", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		deferredLightingShader.setInt("normalTexture", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		deferredLightingShader.setInt("albedoSpecTexture", 2);
	
		deferredLightingShader.setVec3("viewPos", camera.Position);

		for (int i = 0, n = lightPositions.size(); i < n; i++)
		{
			deferredLightingShader.setVec3("lights[" + std::to_string(i) + "].LightPos", lightPositions[i]);
			deferredLightingShader.setVec3("lights[" + std::to_string(i) + "].LightColor", lightColors[i]);
			deferredLightingShader.setFloat("lights[" + std::to_string(i) + "].Radius", lightRadius[i]);
		}

		deferredLightingShader.setFloat("exposure", exposure);

		RenderQuad();


		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffers);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		glBlitFramebuffer(
			0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// MVP and lightColor
		forwardRenderingShader.use();
		forwardRenderingShader.setMat4("view", view);
		forwardRenderingShader.setMat4("projection", projection);
		for (int i = 0, n = lightPositions.size(); i < n; i++)
		{
			glm::mat4 lightModelMatrix = glm::translate(glm::mat4(1.0f), lightPositions[i]);
			lightModelMatrix = glm::scale(lightModelMatrix, glm::vec3(0.2f));
			forwardRenderingShader.setMat4("model", lightModelMatrix);
			forwardRenderingShader.setVec3("lightColor", lightColors[i]);
			RenderCube();
		}



		// function for checking buffer
		//testsceneShader.use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, gNormal); // <---- change texture we wanna check here
		//testsceneShader.setInt("texture1", 0);
		//testsceneShader.setMat4("projection", glm::mat4(1.0f));
		//testsceneShader.setMat4("view", glm::mat4(1.0f));
		//testsceneShader.setMat4("model", glm::mat4(1.0f));
		//RenderQuad();
		// finished first pass ________________________________________________________________________________________
		// continue here ++++++++++++++++=



		// --------------------------------------------

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	/*glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);*/

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (exposure > 0.0f)
			exposure -= 0.001f;
		else
			exposure = 0.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		exposure += 0.001f;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

float distanceFromCamera(glm::vec3 pos)
{
	return glm::length(camera.Position - pos);
}

// renders the 3D scene
// --------------------
void RenderScene(const Shader& shader)
{
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	RenderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	RenderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	RenderCube();
}


// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------

void RenderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// Position			  // Normal		      // TexCoord
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};

		// calculation of TBN
		// delta_u = uv p2 - p1, v = p3 - p1, edge1 = p2 - p1, edge2 = p3 - p1
		float Nvertices = 0;  // temp buffer
		//float Nvertices = floor(vertices.size() / 8); // size of array----------------------------
		// int verticesSize = size(vertice)
		int verticesSize = 36 * 8;
	/*	unsigned int Ntriangles = floor(Nvertices / 3);*/

		//for (int i = 0; i < Ntriangles ; i++)
		//{
		//	// for each triangles calc delta_uv, e1, e2
		//	glm::vec2 delta_u = glm::vec2(vertices[6 + i], vertices[7 + i]);
		//	glm::vec3 t = glm::vec3 t_TBNCalculation(glm::vec2 delta_u, glm::vec2 delta_v, glm::vec3 edge1, glm::vec3 edge2);
		//}

		// for each triangles
		// p1 p2    p1  p3
		// edge 1 , edge 2
		// delta uv1, delta uv2
		//	return t
		// calculate n
		int NdataPerTriangle = 24;
		int NdataPerVertex = NdataPerTriangle / 3;
		int Ndata = floor(sizeof(vertices) / sizeof(float));
		int Ntriangles = Ndata / NdataPerTriangle;
		float verticesTBN[36 * (8 + 6)];
		int index = 0;
		for (int i = 0; i < Ntriangles; i++)
		{
			// delta 24 indices per each loop
			glm::vec3 edge1, edge2;
			glm::vec2 deltaUV1, deltaUV2;
			edge1 = glm::vec3(vertices[0 + 24 * i] - vertices[8 + 24 * i], vertices[1 + 24 * i] - vertices[9 + 24 * i], vertices[2 + 24 * i] - vertices[10 + 24 * i]);
			edge2 = glm::vec3(vertices[0 + 24 * i] - vertices[16 + 24 * i], vertices[1 + 24 * i] - vertices[17 + 24 * i], vertices[10 + 24 * i] - vertices[18 + 24 * i]);
			deltaUV1 = glm::vec2(vertices[6 + 24 * i] - vertices[14 + 24 * i], vertices[7 + 24 * i] - vertices[15 + 24 * i]);
			deltaUV2 = glm::vec2(vertices[6 + 24 * i] - vertices[22 + 24 * i], vertices[7 + 24 * i] - vertices[23 + 24 * i]);
			glm::vec3 T = t_TBNCalculation(deltaUV1, deltaUV2, edge1, edge2);
			if (T.x < 0.0001f && T.x > -0.0001f) T.x = 0.0f;
			if (T.y < 0.0001f && T.y > -0.0001f) T.y = 0.0f;
			if (T.z < 0.0001f && T.z > -0.0001f) T.z = 0.0f;
			glm::vec3 N = glm::normalize(glm::vec3(vertices[3 + 24 * i], vertices[4 + 24 * i], vertices[5 + 24 * i]));
			if (N.x < 0.0001f && N.x > -0.0001f) N.x = 0.0f;
			if (N.y < 0.0001f && N.y > -0.0001f) N.y = 0.0f;
			if (N.z < 0.0001f && N.z > -0.0001f) N.z = 0.0f;
			glm::vec3 B = glm::cross(N, T);

			// storing to new array
			for (int k = 0; k < 3; k++)
			{
				// each vertex setup
				for (int j = 0; j < 8; j++)
				{ 
					verticesTBN[index++] = vertices[j + 8 * k + 24 * i];
				}
				verticesTBN[index++] = T.x;
				verticesTBN[index++] = T.y;
				verticesTBN[index++] = T.z;
				verticesTBN[index++] = B.x;
				verticesTBN[index++] = B.y;
				verticesTBN[index++] = B.z;
			}

		}

		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);

		// set VBO value

		//glBindVertexArray(cubeVAO);
		//glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		//// set VAO attributes
		//glEnableVertexAttribArray(0);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		//glEnableVertexAttribArray(1);
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		//glEnableVertexAttribArray(2);
		//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		//glBindBuffer(GL_ARRAY_BUFFER, 0);

		// testing normal
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verticesTBN), verticesTBN, GL_STATIC_DRAW);
		// set VAO attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);					
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);					
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);					
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
void RenderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// Saving to file 

void SaveCubeMapToImage(GLuint texture, int width, int height)	// this save all 6 faces of cube map into images in harddrive
{
	float* tmpBuffer = new float[width * height];

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	for (int i = 0; i < 6; i++) {
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, GL_FLOAT, tmpBuffer);
		//			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
		//				SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	}

	delete tmpBuffer;

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}

// function for calculating t vector for TBN matrix
// we calculate n after by using cross(normal, tangential)
glm::vec3 t_TBNCalculation(glm::vec2 delta_uv1, glm::vec2 delta_uv2, glm::vec3 edge1, glm::vec3 edge2)
{
	float det = 1 / (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
	float tx, ty, tz;
	
	tx = det * delta_uv2.y * edge1.x - delta_uv1.y * edge2.x;
	ty = det * delta_uv2.y * edge1.y - delta_uv1.y * edge2.y;
	tz = det * delta_uv2.y * edge1.z - delta_uv1.y * edge2.z;

	return glm::normalize(glm::vec3(tx, ty, tz));
}

void RenderDeferredShadingScene(Shader shader)
{
	// create one large cube that acts as the floor
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
	model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
	shader.setMat4("model", model);
	RenderCube();
	// then create multiple cubes as the scenery
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	shader.setMat4("model", model);
	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
	model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(1.25));
	shader.setMat4("model", model);
	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
	model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	shader.setMat4("model", model);
	RenderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	RenderCube();
}

//void renderQuad2()
//{
//	if (quadVAO == 0)
//	{
//		// positions
//		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
//		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
//		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
//		glm::vec3 pos4(1.0f, 1.0f, 0.0f);
//		// texture coordinates
//		glm::vec2 uv1(0.0f, 1.0f);
//		glm::vec2 uv2(0.0f, 0.0f);
//		glm::vec2 uv3(1.0f, 0.0f);
//		glm::vec2 uv4(1.0f, 1.0f);
//		// normal vector
//		glm::vec3 nm(0.0f, 0.0f, 1.0f);
//
//		// calculate tangent/bitangent vectors of both triangles
//		glm::vec3 tangent1, bitangent1;
//		glm::vec3 tangent2, bitangent2;
//		// triangle 1
//		// ----------
//		glm::vec3 edge1 = pos2 - pos1;
//		glm::vec3 edge2 = pos3 - pos1;
//		glm::vec2 deltaUV1 = uv2 - uv1;
//		glm::vec2 deltaUV2 = uv3 - uv1;
//
//		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
//
//		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
//		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
//		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
//		tangent1 = glm::normalize(tangent1);
//
//		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
//		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
//		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
//		bitangent1 = glm::normalize(bitangent1);
//
//		// triangle 2
//		// ----------
//		edge1 = pos3 - pos1;
//		edge2 = pos4 - pos1;
//		deltaUV1 = uv3 - uv1;
//		deltaUV2 = uv4 - uv1;
//
//		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
//
//		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
//		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
//		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
//		tangent2 = glm::normalize(tangent2);
//
//
//		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
//		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
//		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
//		bitangent2 = glm::normalize(bitangent2);
//
//
//		float quadVertices[] = {
//			// positions            // normal         // texcoords  // tangent                          // bitangent
//			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
//			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
//			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
//
//			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
//			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
//			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
//		};
//		// configure plane VAO
//		glGenVertexArrays(1, &quadVAO);
//		glGenBuffers(1, &quadVBO);
//		glBindVertexArray(quadVAO);
//		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
//		glEnableVertexAttribArray(0);
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
//		glEnableVertexAttribArray(1);
//		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
//		glEnableVertexAttribArray(2);
//		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
//		glEnableVertexAttribArray(3);
//		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
//		glEnableVertexAttribArray(4);
//		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
//	}
//	glBindVertexArray(quadVAO);
//	glDrawArrays(GL_TRIANGLES, 0, 6);
//	glBindVertexArray(0);
//}