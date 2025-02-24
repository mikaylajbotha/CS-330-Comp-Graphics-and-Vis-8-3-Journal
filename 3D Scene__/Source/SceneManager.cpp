
#include <sstream>  // Include this for stringstream

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declare the global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	// initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	// clear the allocated memory
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
	// destroy the created OpenGL textures
	DestroyGLTextures();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}
/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ,
	glm::vec3 offset)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ + offset);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}


 /***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// Load textures from files
	bReturn = CreateGLTexture("../textures/K_.jpg", "T");  
	bReturn = CreateGLTexture("../textures/R_.jpg", "M");
	bReturn = CreateGLTexture("../textures/R_.jpg", "S");
	bReturn = CreateGLTexture("../textures/I_.jpg", "B");
	bReturn = CreateGLTexture("../textures/A_.png", "F");
	bReturn = CreateGLTexture("../textures/M.jpg", "br");
	bReturn = CreateGLTexture("../textures/M_.jpg", "FLO");  
	bReturn = CreateGLTexture("../textures/P_.jpg", "N");
	bReturn = CreateGLTexture("../textures/E_.jpg", "H");
	bReturn = CreateGLTexture("../textures/L_.png", "I");
	bReturn = CreateGLTexture("../textures/Y_.png", "u");

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method defines the materials used in the 3D scene.
 *  The materials are created with specific diffuse, specular
 *  colors, and shininess values. These materials are then
 *  added to the `m_objectMaterials` list for later use in rendering
 *  the objects. This includes materials like gold, wood, glass,
 *  plate, and fabric.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL goldMaterial;
	goldMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	goldMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.6f);
	goldMaterial.shininess = 52.0;
	goldMaterial.tag = "metal";
	m_objectMaterials.push_back(goldMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.3f);
	woodMaterial.specularColor = glm::vec3(0.0f, 0.0f, 0.0f);
	woodMaterial.shininess = 0.1;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	glassMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glassMaterial.shininess = 95.0;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL plateMaterial;
	plateMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	plateMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	plateMaterial.shininess = 30.0;
	plateMaterial.tag = "plate";
	m_objectMaterials.push_back(plateMaterial);

	OBJECT_MATERIAL fabricMaterial;
	fabricMaterial.diffuseColor = glm::vec3(0.6f, 0.3f, 0.2f);
	fabricMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	fabricMaterial.shininess = 10.0;
	fabricMaterial.tag = "fabric";
	m_objectMaterials.push_back(fabricMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method configures the lighting system for the 3D scene.
 *  It sets up different types of lights, including directional,
 *  point, and spotlights. The lights are configured with
 *  appropriate position, ambient, diffuse, and specular colors.
 *  The method ensures that all the required lights are active
 *  for the scene, and debug output is printed to the console
 *  for verification.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	if (m_pShaderManager)
	{
		// Enable lighting in shader
		m_pShaderManager->setBoolValue("bUseLighting", true);  // Enable lighting

		// Setup Directional Light
		glm::vec3 directionalLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
		glm::vec3 directionalLightAmbient = glm::vec3(0.5f, 0.5f, 0.5f); 
		glm::vec3 directionalLightDiffuse = glm::vec3(1.0f, 1.0f, 1.0f); 
		glm::vec3 directionalLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

		m_pShaderManager->setVec3Value("directionalLight.direction", directionalLightDirection);
		m_pShaderManager->setVec3Value("directionalLight.ambient", directionalLightAmbient);
		m_pShaderManager->setVec3Value("directionalLight.diffuse", directionalLightDiffuse);
		m_pShaderManager->setVec3Value("directionalLight.specular", directionalLightSpecular);
		m_pShaderManager->setBoolValue("directionalLight.bActive", true);

		// Setup Point Lights 
		glm::vec3 pointLightPosition = glm::vec3(0.0f, 5.0f, 0.0f);
		glm::vec3 pointLightAmbient = glm::vec3(0.3f, 0.3f, 0.3f);  
		glm::vec3 pointLightDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 pointLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

		m_pShaderManager->setVec3Value("pointLights[0].position", pointLightPosition);
		m_pShaderManager->setVec3Value("pointLights[0].ambient", pointLightAmbient);
		m_pShaderManager->setVec3Value("pointLights[0].diffuse", pointLightDiffuse);
		m_pShaderManager->setVec3Value("pointLights[0].specular", pointLightSpecular);
		m_pShaderManager->setBoolValue("pointLights[0].bActive", true); 

		// Disable remaining point lights if any
		for (int i = 1; i < 5; i++) {
			m_pShaderManager->setBoolValue("pointLights[" + std::to_string(i) + "].bActive", false);
			std::cout << "Point Light [" << i << "] disabled." << std::endl;
		}

		// Add a secondary Point Light 
		glm::vec3 pointLightPosition2 = glm::vec3(5.0f, 3.0f, 5.0f); 
		glm::vec3 pointLightAmbient2 = glm::vec3(0.3f, 0.3f, 0.3f);
		glm::vec3 pointLightDiffuse2 = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 pointLightSpecular2 = glm::vec3(1.0f, 1.0f, 1.0f);

		m_pShaderManager->setVec3Value("pointLights[1].position", pointLightPosition2);
		m_pShaderManager->setVec3Value("pointLights[1].ambient", pointLightAmbient2);
		m_pShaderManager->setVec3Value("pointLights[1].diffuse", pointLightDiffuse2);
		m_pShaderManager->setVec3Value("pointLights[1].specular", pointLightSpecular2);
		m_pShaderManager->setBoolValue("pointLights[1].bActive", true);  // Enable second point light

		// Setup Spot Light with increased cut-off angles to cover more area
		glm::vec3 spotLightPosition = glm::vec3(0.0f, 4.0f, 5.0f);
		glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, -1.0f);
		float spotLightCutOff = glm::cos(glm::radians(20.0f)); // Wider angle
		float spotLightOuterCutOff = glm::cos(glm::radians(25.0f)); // Outer angle for broader coverage
		glm::vec3 spotLightAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
		glm::vec3 spotLightDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 spotLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

		m_pShaderManager->setVec3Value("spotLight.position", spotLightPosition);
		m_pShaderManager->setVec3Value("spotLight.direction", spotLightDirection);
		m_pShaderManager->setFloatValue("spotLight.cutOff", spotLightCutOff);
		m_pShaderManager->setFloatValue("spotLight.outerCutOff", spotLightOuterCutOff);
		m_pShaderManager->setVec3Value("spotLight.ambient", spotLightAmbient);
		m_pShaderManager->setVec3Value("spotLight.diffuse", spotLightDiffuse);
		m_pShaderManager->setVec3Value("spotLight.specular", spotLightSpecular);
		m_pShaderManager->setBoolValue("spotLight.bActive", true);
	}
}


/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// load the textures for the 3D scene
	LoadSceneTextures();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	  // Define materials
	DefineObjectMaterials();

	// Setup lights
	SetupSceneLights();

	m_basicMeshes->LoadPlaneMesh();     // For the table
	m_basicMeshes->LoadBoxMesh();       // For monitor, keyboard, books
	m_basicMeshes->LoadCylinderMesh();  // For the mug and pencil holder
	m_basicMeshes->LoadTorusMesh();     // For the mug handle
	m_basicMeshes->LoadSphereMesh();    // For the mouse
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering all objects in the 3D scene.
 *  It invokes the render functions for the table, monitor,
 *  keyboard, mouse, books, pencil holder, and pencils.
 *  It acts as the central function to initiate the rendering
 *  process for the entire scene.
 ***********************************************************/
void SceneManager::RenderScene() {
	RenderTable();
	RenderMonitor();
	RenderKeyboard();
	RenderMouse();
	RenderBooks();
	RenderPencilHolder();
	RenderPencils();
}

/***********************************************************
 *  RenderTable()
 *
 *  This method is used for rendering the table object in the scene.
 *  It sets the transformations for the table mesh and applies the
 *  appropriate materials and textures. The table is represented as
 *  a plane in the scene.
 ***********************************************************/
void SceneManager::RenderTable() {
	glm::vec3 scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	glm::vec3 positionXYZ = glm::vec3(0.0f, -0.5f, 0.0f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

	// Set material and texture for the table
	SetShaderMaterial("wood");  // Apply wood material to table
	SetShaderTexture("T");      // Apply the texture for the table
	SetTextureUVScale(5.0f, 5.0f);  // Texture tiling
	m_basicMeshes->DrawPlaneMesh();
}

/***********************************************************
 *  RenderMonitor()
 *
 *  This method is used for rendering the monitor object in the scene.
 *  It sets the transformations for the monitor mesh and applies the
 *  appropriate materials and textures. The monitor consists of a
 *  base, frame, and screen, each rendered separately.
 ***********************************************************/
void SceneManager::RenderMonitor() {
	glm::vec3 scaleXYZ = glm::vec3(2.0f, 0.2f, 0.5f);
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.1f, 0.0f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

	// Set material and texture for the monitor base
	SetShaderMaterial("metal");  // Apply metal material to monitor base
	SetShaderTexture("H");       // Apply the texture for the base
	m_basicMeshes->DrawBoxMesh();

	// Monitor frame
	scaleXYZ = glm::vec3(8.0f, 5.0f, 0.5f);
	positionXYZ = glm::vec3(0.0f, 3.0f, 0.0f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

	// Apply the same material and texture to the frame
	SetShaderMaterial("metal");
	SetShaderTexture("N");
	m_basicMeshes->DrawBoxMesh();

	// Monitor screen
	scaleXYZ = glm::vec3(7.5f, 4.5f, 0.1f);
	positionXYZ = glm::vec3(0.0f, 3.0f, 0.26f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

	// Apply glass material for the screen
	SetShaderMaterial("glass");
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);  // Set the screen color to white
	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  RenderKeyboard()
 *
 *  This method is used for rendering the keyboard object in the scene.
 *  It sets the transformations for the keyboard mesh, including the
 *  base and keys. Materials and textures are applied to each part
 *  of the keyboard.
 ***********************************************************/
void SceneManager::RenderKeyboard() {
	glm::vec3 scaleXYZ;
	glm::vec3 positionXYZ;
	float XrotationDegrees = 0.0f, YrotationDegrees = 0.0f, ZrotationDegrees = 0.0f;

	// Render the keyboard base (frame)
	scaleXYZ = glm::vec3(6.0f, 0.2f, 1.5f);  // Size of the keyboard base
	positionXYZ = glm::vec3(0.0f, -0.2f, 2.5f);  // Position in front of the monitor
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	// Set texture for the keyboard base
	SetShaderMaterial("metal");  // Apply metal material to base
	SetShaderTexture("N");       // Apply texture for the keyboard base
	m_basicMeshes->DrawBoxMesh();

	// Render the keyboard keys (grid of keys)
	glm::vec3 keyScaleXYZ = glm::vec3(0.35f, 0.1f, 0.35f);
	int rows = 4;
	int cols = 10;
	float keySpacingX = 0.4f;
	float keySpacingZ = 0.4f;
	glm::vec3 keyStartPosition = glm::vec3(-1.8f, -0.08f, 2.0f);
	SetTextureUVScale(4.0f, 4.0f);  // Apply tiling to key textures

	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			glm::vec3 keyPosition = keyStartPosition + glm::vec3(c * keySpacingX, 0.0f, r * keySpacingZ);
			SetTransformations(keyScaleXYZ, 0.0f, 0.0f, 0.0f, keyPosition);

			// Apply texture for each key
			SetShaderTexture("u");  // Apply texture for the key
			m_basicMeshes->DrawBoxMesh();
		}
	}

	// Reset UV scaling to prevent affecting other objects
	SetTextureUVScale(1.0f, 1.0f);
}

/***********************************************************
 *  RenderMouse()
 *
 *  This method is used for rendering the mouse object in the scene.
 *  It sets the transformations for the mouse mesh and applies the
 *  appropriate material and texture.
 ***********************************************************/
void SceneManager::RenderMouse() {
	glm::vec3 scaleXYZ = glm::vec3(0.5f, 0.3f, 0.8f);
	glm::vec3 positionXYZ = glm::vec3(3.5f, -0.15f, 2.5f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

	// Set material and texture for the mouse
	SetShaderMaterial("metal");
	SetShaderTexture("H");  // Apply texture to the mouse
	m_basicMeshes->DrawSphereMesh();
}

/***********************************************************
 *  RenderBooks()
 *
 *  This method is used for rendering books in the scene.
 *  It renders each book individually with slight position misalignments
 *  and applies textures and materials to each. The books are rendered
 *  as stacked boxes with randomized positioning and rotation for realism.
 ***********************************************************/
void SceneManager::RenderBooks() {
	glm::vec3 bookBaseScale = glm::vec3(2.8f, 0.3f, 1.8f);
	glm::vec3 bookPosition = glm::vec3(-6.5f, 0.2f, 0.0f);
	float bookSpacingY = 0.05f;

	std::vector<std::pair<std::string, std::string>> bookMaterials = {
		{"F", "fabric"},
		{"br", "wood"},
		{"FLO", "plate" }
	};

	for (int i = 0; i < bookMaterials.size(); i++) {
		glm::vec3 currentBookScale = bookBaseScale;
		if (i == 1) currentBookScale.y *= 1.2f;  // Thicker middle book
		if (i == 2) currentBookScale.y *= 1.1f;  // Thicker top book

		// Slight position misalignment for realism
		float xOffset = (i % 2 == 0) ? -0.1f : 0.1f;
		float zOffset = (i % 2 == 0) ? 0.05f : -0.05f;
		glm::vec3 misalignedPosition = bookPosition + glm::vec3(xOffset, 0.0f, zOffset);

		// Slight random rotation for top books
		float rotationAngle = (i > 0) ? ((i % 2 == 0) ? -5.0f : 5.0f) : 0.0f;

		SetTransformations(currentBookScale, 0.0f, rotationAngle, 0.0f, misalignedPosition);
		SetShaderMaterial(bookMaterials[i].second);
		SetShaderTexture(bookMaterials[i].first);
		m_basicMeshes->DrawBoxMesh();

		bookPosition.y += currentBookScale.y + bookSpacingY;
	}
}

/***********************************************************
 *  RenderPencilHolder()
 *
 *  This method is used for rendering the pencil holder object in the scene.
 *  It sets the transformations for the pencil holder mesh and applies
 *  the appropriate material and texture.
 ***********************************************************/
void SceneManager::RenderPencilHolder() {
	glm::vec3 holderScale = glm::vec3(0.6f, 1.2f, 0.6f);
	glm::vec3 holderPosition = glm::vec3(6.0f, 0.6f, 0.0f);
	SetTransformations(holderScale, 0.0f, 0.0f, 0.0f, holderPosition);
	SetShaderMaterial("metal");
	SetShaderTexture("B");
	m_basicMeshes->DrawCylinderMesh();
}


/***********************************************************
 *  RenderPencils()
 *
 *  This method is used for rendering multiple pencil objects in the scene.
 *  Each pencil is randomly positioned with slight rotations to simulate a
 *  realistic setup. Different colors are applied to each pencil to make them
 *  look distinct.
 ***********************************************************/
void SceneManager::RenderPencils() {
	glm::vec3 pencilScale = glm::vec3(0.05f, 1.8f, 0.05f);
	glm::vec3 basePosition = glm::vec3(6.0f, 1.4f, 0.0f);
	float rotationOffsets[] = { -10.0f, 5.0f, 15.0f, -20.0f, 10.0f };

	glm::vec3 pencilColors[] = {
		glm::vec3(1.0f, 0.0f, 0.0f),  // Red
		glm::vec3(1.0f, 1.0f, 0.0f),  // Yellow
		glm::vec3(0.0f, 0.0f, 1.0f),  // Blue
		glm::vec3(0.0f, 1.0f, 0.0f),  // Green
		glm::vec3(1.0f, 0.5f, 0.0f)   // Orange
	};

	for (int i = 0; i < 5; i++) {
		glm::vec3 pencilPosition = basePosition + glm::vec3(
			(i % 2 == 0) ? -0.1f : 0.1f,
			(i * 0.1f),
			(i % 2 == 0) ? -0.05f : 0.05f
		);

		float rotationAngle = rotationOffsets[i];
		glm::vec3 chosenColor = pencilColors[i];

		SetTransformations(pencilScale, rotationAngle, 0.0f, 0.0f, pencilPosition);
		SetShaderColor(chosenColor.r, chosenColor.g, chosenColor.b, 1.0f);
		m_basicMeshes->DrawCylinderMesh();
	}
}
