#include <iostream>
#include "rendering/modelLoader.hpp"
#include "tests.hpp"
#include "game.hpp"

// NOTE: Profiling reveals that in the current instanced rendering system:
// - Updating the buffer takes more time than
// - Calculating the transforms which takes more time than
// - Performing the instanced draw
static int runApp(Application *app)
{
	Tests::runTests();
	Window window(*app, 800, 600, "My Window!");
	RenderDevice device(window);

	Matrix perspective(Matrix::perspective(Math::toRadians(70.0f / 2.0f),
										   4.0f / 3.0f, 0.1f, 1000.0f));
	RenderDevice::DrawParams drawParams;
	drawParams.primitiveType = RenderDevice::PRIMITIVE_TRIANGLES;
	drawParams.faceCulling = RenderDevice::FACE_CULL_BACK;
	drawParams.shouldWriteDepth = true;
	drawParams.depthFunc = RenderDevice::DRAW_FUNC_LESS;
	//	drawParams.sourceBlend = RenderDevice::BLEND_FUNC_ONE;
	//	drawParams.destBlend = RenderDevice::BLEND_FUNC_ONE;
		
	RenderTarget target(device);

	// an error is thrown when creating a shader without having set a vertexArray
	// in the device. This code below doesn't do anything beyond that
	Array<IndexedModel> models;
	Array<uint32> modelMaterialIndices;
	Array<MaterialSpec> modelMaterials;
	ModelLoader::loadModels("./res/models/monkey3.obj", models,
							modelMaterialIndices, modelMaterials);

	VertexArray vertexArray(device, models[0], RenderDevice::USAGE_STATIC_DRAW);
	// See comment above

	String shaderText;
	StringFuncs::loadTextFileWithIncludes(shaderText, "./res/shaders/basicShader.glsl", "#include");
	Shader shader(device, shaderText);
	Sampler sampler(device, RenderDevice::FILTER_LINEAR_MIPMAP_LINEAR);

	GameRenderContext gameRenderContext(device, target, drawParams, shader, sampler, perspective);

	Game game(app, &window, &gameRenderContext);
	return game.loadAndRunScene(device);
}

#ifdef main
#undef main
#endif
int main(int argc, char **argv)
{
	Application *app = Application::create();
	int result = runApp(app);
	delete app;
	return result;
}
