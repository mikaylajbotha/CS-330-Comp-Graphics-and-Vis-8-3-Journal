///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// Manage the viewing of 3D objects within the viewport - camera, projection.
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "camera.h"

// GLFW library
#include "GLFW/glfw3.h"

class ViewManager
{
public:
	// Constructor
	ViewManager(ShaderManager* pShaderManager);
	// Destructor
	~ViewManager();

	// Mouse position callback for mouse interaction with the 3D scene
	static void Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos);

	// Mouse scroll wheel callback for zooming and movement speed
	static void Mouse_Scroll_Wheel_Callback(GLFWwindow* window, double x, double yScrollDistance);

	// Process keyboard events for user input
	void ProcessKeyboardEvents();

	// Create the initial OpenGL display window
	GLFWwindow* CreateDisplayWindow(const char* windowTitle);

	// Prepare the conversion from 3D object display to 2D scene display
	void PrepareSceneView();

private:
	// Pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// Active OpenGL display window
	GLFWwindow* m_pWindow;
	// Projection mode flag (true = orthographic, false = perspective)
	bool bOrthographicProjection;
};
