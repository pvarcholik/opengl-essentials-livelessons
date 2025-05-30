#include "pch.h"
#include "EnvironmentMappingDemo.h"
#include "VectorHelper.h"
#include "ProxyModel.h"
#include "SpotLight.h"
#include "Camera.h"
#include "Game.h"
#include "Model.h"

using namespace glm;
using namespace std;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(EnvironmentMappingDemo)

	EnvironmentMappingDemo::EnvironmentMappingDemo(Game& game, shared_ptr<Camera> camera) :
		DrawableGameComponent(game, camera)
	{
	}

	EnvironmentMappingDemo::~EnvironmentMappingDemo()
	{
		glDeleteSamplers(1, &mEnvironmentMapSampler);
		glDeleteSamplers(1, &mColorTextureSampler);
		glDeleteTextures(1, &mEnvironmentMap);
		glDeleteTextures(1, &mColorTexture);
		glDeleteBuffers(1, &mIndexBuffer);
		glDeleteBuffers(1, &mVertexBuffer);
		glDeleteVertexArrays(1, &mVertexArrayObject);
	}

	void EnvironmentMappingDemo::Initialize()
	{
		// Build the shader program
		vector<ShaderDefinition> shaders;
		shaders.push_back(ShaderDefinition(GL_VERTEX_SHADER, "Content/Effects/EnvironmentMappingDemo.vert"));
		shaders.push_back(ShaderDefinition(GL_FRAGMENT_SHADER, "Content/Effects/EnvironmentMappingDemo.frag"));
		mShaderProgram.BuildProgram(shaders);
		
		// Load the model
		Model model("Content/Models/Sphere.obj", true);

		// Create the vertex and index buffers
		auto mesh = model.Meshes().at(0);
		VertexPositionTextureNormal::CreateVertexBuffer(*mesh, mVertexBuffer);
		mesh->CreateIndexBuffer(mIndexBuffer);
		mIndexCount = mesh->Indices().size();
		
		// Load the color texture
		mColorTexture = SOIL_load_OGL_texture("Content/Textures/Checkerboard.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		if (mColorTexture == 0)
		{
			throw runtime_error("SOIL_load_OGL_texture() failed.");
		}

		glGenSamplers(1, &mColorTextureSampler);
		glSamplerParameteri(mColorTextureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(mColorTextureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(mColorTextureSampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glSamplerParameteri(mColorTextureSampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Load the environment map	
		mEnvironmentMap = SOIL_load_OGL_cubemap("Content/Textures/Maskonaive2_1024/posx.jpg",
												"Content/Textures/Maskonaive2_1024/negx.jpg",
												"Content/Textures/Maskonaive2_1024/posy.jpg",
												"Content/Textures/Maskonaive2_1024/negy.jpg",
												"Content/Textures/Maskonaive2_1024/posz.jpg",
												"Content/Textures/Maskonaive2_1024/negz.jpg",
												SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		if (mEnvironmentMap == 0)
		{
			throw runtime_error("SOIL_load_OGL_cubemap() failed.");
		}

		glGenSamplers(1, &mEnvironmentMapSampler);
		glSamplerParameteri(mEnvironmentMapSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(mEnvironmentMapSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(mEnvironmentMapSampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glSamplerParameteri(mEnvironmentMapSampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glSamplerParameteri(mEnvironmentMapSampler, GL_TEXTURE_WRAP_R, GL_REPEAT);

		// Create the vertex array object
		glGenVertexArrays(1, &mVertexArrayObject);
		mShaderProgram.Initialize(mVertexArrayObject);
		glBindVertexArray(0);

		mAmbientLight = make_unique<Light>(*mGame);
		mAmbientLight->SetColor(ColorHelper::White);
	}

	void EnvironmentMappingDemo::Update(const GameTime& gameTime)
	{
		UpdateAmbientLight(gameTime);
		UpdateReflectionAmount(gameTime);
	}

	void EnvironmentMappingDemo::Draw(const GameTime& /*gameTime*/)
	{
		glBindVertexArray(mVertexArrayObject);
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		
		mShaderProgram.Use();

		mat4 wvp = mCamera->ViewProjectionMatrix() * mWorldMatrix;
		mShaderProgram.WorldViewProjection() << wvp;
		mShaderProgram.World() << mWorldMatrix;
		mShaderProgram.AmbientColor() << mAmbientLight->Color();
		mShaderProgram.EnvironmentColor() << mEnvironmentColor;
		mShaderProgram.ReflectionAmount() << mReflectionAmount;
		mShaderProgram.CameraPosition() << mCamera->Position();

		glBindSampler(0, mEnvironmentMapSampler);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvironmentMap);

		glBindSampler(1, mColorTextureSampler);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mColorTexture);
	
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void EnvironmentMappingDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = mAmbientLight->Color().r;

		if (glfwGetKey(mGame->Window(), GLFW_KEY_PAGE_UP) && ambientIntensity < 1.0f)
		{
			ambientIntensity += gameTime.ElapsedGameTimeSeconds().count();
			ambientIntensity = std::min(ambientIntensity, 1.0f);

			mAmbientLight->SetColor(vec4((vec3)ambientIntensity, 1.0f));
		}

		if (glfwGetKey(mGame->Window(), GLFW_KEY_PAGE_DOWN) && ambientIntensity > 0.0f)
		{
			ambientIntensity -= gameTime.ElapsedGameTimeSeconds().count();
			ambientIntensity = std::max(ambientIntensity, 0.0f);

			mAmbientLight->SetColor(vec4((vec3)ambientIntensity, 1.0f));
		}
	}

	void EnvironmentMappingDemo::UpdateReflectionAmount(const GameTime& gameTime)
	{
		static float reflectionAmount = 1.0f;

		if (glfwGetKey(mGame->Window(), GLFW_KEY_UP) && reflectionAmount < 1.0f)
		{
			reflectionAmount += gameTime.ElapsedGameTimeSeconds().count();
			reflectionAmount = std::min(reflectionAmount, 1.0f);

			mReflectionAmount = reflectionAmount;
		}

		if (glfwGetKey(mGame->Window(), GLFW_KEY_DOWN) && reflectionAmount > 0.0f)
		{
			reflectionAmount -= gameTime.ElapsedGameTimeSeconds().count();
			reflectionAmount = std::max(reflectionAmount, 0.0f);

			mReflectionAmount = reflectionAmount;
		}
	}
}