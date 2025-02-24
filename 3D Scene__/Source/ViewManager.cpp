///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport - camera, projection
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	// if orthographic projection is on, this value will be
	// true
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager* pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.5f, 5.5f, 10.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
	g_pCamera->MovementSpeed = 10;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// this callback is used to receive mouse scroll wheel events
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Wheel_Callback);

	// tell GLFW to capture all mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// Reduce sensitivity by applying a smaller scale factor
	float sensitivity = 0.1f;  // Reduce to 10% of default movement
	float xOffset = (xMousePos - gLastX) * sensitivity;
	float yOffset = (gLastY - yMousePos) * sensitivity; // Reversed since y-coordinates go bottom to top

	gLastX = xMousePos;
	gLastY = yMousePos;

	// Move the camera with reduced mouse movement sensitivity
	g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}



/***********************************************************
 *  Mouse_Scroll_Wheel_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse wheel is scrolled within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Wheel_Callback(GLFWwindow* window, double x, double yScrollDistance)
{
	if (g_pCamera)
	{
		// Reduce effect of scroll adjustments
		g_pCamera->MovementSpeed += yScrollDistance * 0.5f;  // Scale down speed adjustments

		// Clamp movement speed to prevent extreme values
		if (g_pCamera->MovementSpeed < 0.5f)  // Minimum speed is 0.5
			g_pCamera->MovementSpeed = 0.5f;
		if (g_pCamera->MovementSpeed > 20.0f) // Max speed is 20
			g_pCamera->MovementSpeed = 20.0f;
	}
}


/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// Close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// If the camera object is null, then exit this method
	if (g_pCamera == nullptr)
	{
		return;
	}

	// Reduce movement speed 
	float movementSpeed = (g_pCamera->MovementSpeed * 0.3f) * gDeltaTime;

	// Process camera movement (WASD for direction, QE for vertical movement)
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(FORWARD, movementSpeed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(BACKWARD, movementSpeed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(LEFT, movementSpeed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(RIGHT, movementSpeed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(UP, movementSpeed);
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
		g_pCamera->ProcessKeyboard(DOWN, movementSpeed);

	// Toggle between Perspective and Orthographic views
	static bool pKeyPressed = false;
	static bool oKeyPressed = false;

	// Switch to Perspective mode when "P" is pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS && !pKeyPressed)
	{
		bOrthographicProjection = false; // Enable Perspective mode
		g_pCamera->Position = glm::vec3(0.0f, 5.0f, 8.0f); // Adjust position
		g_pCamera->Front = glm::vec3(0.0f, -0.3f, -1.0f); // Tilt slightly downward
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Zoom = 80; // Restore original zoom
		pKeyPressed = true;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_RELEASE)
	{
		pKeyPressed = false;
	}

	// Switch to Orthographic mode when "O" is pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS && !oKeyPressed)
	{
		bOrthographicProjection = true; // Enable Orthographic mode
		g_pCamera->Position = glm::vec3(0.0f, 10.0f, 0.0f); // Overhead view
		g_pCamera->Front = glm::vec3(0.0f, -1.0f, 0.0f); // Look straight down
		g_pCamera->Up = glm::vec3(0.0f, 0.0f, -1.0f); // Adjust orientation
		g_pCamera->Zoom = 50; // Adjust zoom to fit the scene
		oKeyPressed = true;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_RELEASE)
	{
		oKeyPressed = false;
	}

	
	static bool key1Pressed = false;
	static bool key2Pressed = false;
	static bool key3Pressed = false;
	static bool key4Pressed = false;

	// Front View (Ortho)
	if (glfwGetKey(m_pWindow, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed)
	{
		bOrthographicProjection = true;
		g_pCamera->Position = glm::mix(g_pCamera->Position, glm::vec3(0.0f, 4.0f, 10.0f), 0.5f);
		g_pCamera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		key1Pressed = true;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_1) == GLFW_RELEASE)
	{
		key1Pressed = false;
	}

	// Side View (Ortho)
	if (glfwGetKey(m_pWindow, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed)
	{
		bOrthographicProjection = true;
		g_pCamera->Position = glm::mix(g_pCamera->Position, glm::vec3(10.0f, 4.0f, 0.0f), 0.5f);
		g_pCamera->Front = glm::vec3(-1.0f, 0.0f, 0.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		key2Pressed = true;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_2) == GLFW_RELEASE)
	{
		key2Pressed = false;
	}

	// Top View (Ortho)
	if (glfwGetKey(m_pWindow, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed)
	{
		bOrthographicProjection = true;
		g_pCamera->Position = glm::mix(g_pCamera->Position, glm::vec3(0.0f, 10.0f, 0.0f), 0.5f);
		g_pCamera->Front = glm::vec3(0.0f, -1.0f, 0.0f);
		g_pCamera->Up = glm::vec3(0.0f, 0.0f, -1.0f);
		key3Pressed = true;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_3) == GLFW_RELEASE)
	{
		key3Pressed = false;
	}

	// Perspective View (Same as P)
	if (glfwGetKey(m_pWindow, GLFW_KEY_4) == GLFW_PRESS && !key4Pressed)
	{
		bOrthographicProjection = false;
		g_pCamera->Position = glm::mix(g_pCamera->Position, glm::vec3(0.0f, 5.0f, 8.0f), 0.5f);
		g_pCamera->Front = glm::vec3(0.0f, -0.3f, -1.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Zoom = 80;
		key4Pressed = true;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_4) == GLFW_RELEASE)
	{
		key4Pressed = false;
	}
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// Define the current projection matrix based on selected mode
	if (bOrthographicProjection)
	{
		// Orthographic projection (2D-like view)
		float orthoSize = 10.0f;
		projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);
	}
	else
	{
		// Perspective projection (3D view)
		projection = glm::perspective(glm::radians(g_pCamera->Zoom),
			(GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}

	// If the shader manager object is valid
	if (m_pShaderManager != NULL)
	{
		// Set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);

		// Set the projection matrix into the shader
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);

		// Set the view position of the camera into the shader
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}
