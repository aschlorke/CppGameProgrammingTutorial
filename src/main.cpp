#include <iostream>
#include "core/application.hpp"
#include "core/window.hpp"
#include "core/memory.hpp"
#include "math/transform.hpp"
#include "rendering/renderContext.hpp"
#include "rendering/modelLoader.hpp"

#include "core/timing.hpp"
#include "tests.hpp"

#include "math/sphere.hpp"
#include "math/aabb.hpp"
#include "math/plane.hpp"
#include "math/intersects.hpp"
#include "gameEventHandler.hpp"
#include "core/input.hpp"
#include "ecs/ecs.hpp"
#include "motionIntegrators.hpp"
#include "gameRenderContext.hpp"
#include "gameComponentSystem/renderableMesh.hpp"
#include "gameComponentSystem/movementControl.hpp"
#include "gameComponentSystem/motion.hpp"
#include "gameComponentSystem/megaCube.hpp"

// NOTE: Profiling reveals that in the current instanced rendering system:
// - Updating the buffer takes more time than
// - Calculating the transforms which takes more time than
// - Performing the instanced draw
static int runApp(Application *app)
{
	Tests::runTests();
	Window window(*app, 800, 600, "My Window!");

	// Begin scene creation
	RenderDevice device(window);
	RenderTarget target(device);

	Array<IndexedModel> models;
	Array<uint32> modelMaterialIndices;
	Array<MaterialSpec> modelMaterials;
	ModelLoader::loadModels("./res/models/monkey3.obj", models,
							modelMaterialIndices, modelMaterials);

	ModelLoader::loadModels("./res/models/tinycube.obj", models,
							modelMaterialIndices, modelMaterials);
	//	IndexedModel model;
	//	model.allocateElement(3); // Positions
	//	model.allocateElement(2); // TexCoords
	//	model.allocateElement(3); // Normals
	//	model.allocateElement(3); // Tangents
	//	model.setInstancedElementStartIndex(4); // Begin instanced data
	//	model.allocateElement(16); // Transform matrix
	//
	//	model.addElement3f(0, -0.5f, -0.5f, 0.0f);
	//	model.addElement3f(0, 0.0f, 0.5f, 0.0f);
	//	model.addElement3f(0, 0.5f, -0.5f, 0.0f);
	//	model.addElement2f(1, 0.0f, 0.0f);
	//	model.addElement2f(1, 0.5f, 1.0f);
	//	model.addElement2f(1, 1.0f, 0.0f);
	//	model.addIndices3i(0, 1, 2);

	VertexArray vertexArray(device, models[0], RenderDevice::USAGE_STATIC_DRAW);
	VertexArray tinyCubeVertexArray(device, models[1], RenderDevice::USAGE_STATIC_DRAW);
	Sampler sampler(device, RenderDevice::FILTER_LINEAR_MIPMAP_LINEAR);
	//	ArrayBitmap bitmap;
	//	bitmap.set(0,0, Color::WHITE.toInt());
	//	if(!bitmap.load("./res/textures/bricks.jpg")) {
	//		DEBUG_LOG("Main", LOG_ERROR, "Could not load texture!");
	//		return 1;
	//	}
	//	Texture texture(device, bitmap, RenderDevice::FORMAT_RGB, true, false);
	DDSTexture ddsTexture;
	if (!ddsTexture.load("./res/textures/bricks.dds"))
	{
		DEBUG_LOG("Main", LOG_ERROR, "Could not load texture!");
		return 1;
	}
	Texture texture(device, ddsTexture);

	if (!ddsTexture.load("./res/textures/bricks2.dds"))
	{
		DEBUG_LOG("Main", LOG_ERROR, "Could not load texture!");
		return 1;
	}
	Texture bricks2Texture(device, ddsTexture);

	String shaderText;
	StringFuncs::loadTextFileWithIncludes(shaderText, "./res/shaders/basicShader.glsl", "#include");
	Shader shader(device, shaderText);
	shader.setSampler("diffuse", texture, sampler, 0);

	Matrix perspective(Matrix::perspective(Math::toRadians(70.0f / 2.0f),
										   4.0f / 3.0f, 0.1f, 1000.0f));
	Color color(0.0f, 0.15f, 0.3f);

	RenderDevice::DrawParams drawParams;
	drawParams.primitiveType = RenderDevice::PRIMITIVE_TRIANGLES;
	drawParams.faceCulling = RenderDevice::FACE_CULL_BACK;
	drawParams.shouldWriteDepth = true;
	drawParams.depthFunc = RenderDevice::DRAW_FUNC_LESS;
	//	drawParams.sourceBlend = RenderDevice::BLEND_FUNC_ONE;
	//	drawParams.destBlend = RenderDevice::BLEND_FUNC_ONE;
	// End scene creation

	GameRenderContext gameRenderContext(device, target, drawParams, shader, sampler, perspective);
	GameEventHandler eventHandler;
	InputControl horizontal;
	InputControl vertical;
	eventHandler.addKeyControl(Input::KEY_A, horizontal, -1.0f);
	eventHandler.addKeyControl(Input::KEY_D, horizontal, 1.0f);
	eventHandler.addKeyControl(Input::KEY_LEFT, horizontal, -1.0f);
	eventHandler.addKeyControl(Input::KEY_RIGHT, horizontal, 1.0f);

	eventHandler.addKeyControl(Input::KEY_S, vertical, -1.0f);
	eventHandler.addKeyControl(Input::KEY_W, vertical, 1.0f);
	eventHandler.addKeyControl(Input::KEY_DOWN, vertical, -1.0f);
	eventHandler.addKeyControl(Input::KEY_UP, vertical, 1.0f);

	ECS ecs;
	// Create Components
	TransformComponent transformComponent;
	transformComponent.transform.setTranslation(Vector3f(0.0f, 0.0f, 20.0f));

	MovementControlComponent movementControl;
	movementControl.movementControls.push_back(std::make_pair(Vector3f(1.0f, 0.0f, 0.0f) * 10.0f, &horizontal));
	movementControl.movementControls.push_back(std::make_pair(Vector3f(0.0f, 1.0f, 0.0f) * 10.0f, &vertical));

	RenderableMeshComponent renderableMesh;
	renderableMesh.vertexArray = &vertexArray;
	renderableMesh.texture = &texture;

	MotionComponent motionComponent;
	MegaCubeComponent megaCubeComp;

	// Create Entity
	ecs.makeEntity(transformComponent, movementControl, renderableMesh);
	for (uint32 i = 0; i < 100000; i++)
	{
		transformComponent.transform.setTranslation(Vector3f(Math::randf() * 10.0f - 5.0f, Math::randf() * 10.0f - 5.0f, 20.0f));
		renderableMesh.vertexArray = &tinyCubeVertexArray;
		renderableMesh.texture = Math::randf() > 0.5f ? &texture : &bricks2Texture;

		float vf = -4.0f;
		float af = 5.0f;

		motionComponent.acceleration = Vector3f(Math::randf(-af, af), Math::randf(-af, af), Math::randf(-af, af));
		motionComponent.velocity = motionComponent.acceleration * vf;
		for (uint32 i = 0; i < 3; i++)
		{
			megaCubeComp.pos[i] = transformComponent.transform.getTranslation()[i];
			megaCubeComp.vel[i] = motionComponent.velocity[i];
			megaCubeComp.acc[i] = motionComponent.acceleration[i];
			megaCubeComp.texIndex = Math::randf() > 0.5f ? 0 : 1;
		}
		ecs.makeEntity(megaCubeComp);
		// ecs.makeEntity(transformComponent, motionComponent, renderableMesh);
	}

	// Create the systems
	MovementControlSystem movementControlSystem;
	MegaCubeMotionSystem megaCubeMotionSystem;
	Texture *textures[] = {&texture, &bricks2Texture};
	MegaCubeRenderer megaCubeRenderer(gameRenderContext, tinyCubeVertexArray, textures, ARRAY_SIZE_IN_ELEMENTS(textures));
	MotionSystem motionSystem;
	RenderableMeshSystem renderableMeshSystem(gameRenderContext);

	ECSSystemList mainSystems;
	ECSSystemList renderingPipeline;
	mainSystems.addSystem(movementControlSystem);
	mainSystems.addSystem(megaCubeMotionSystem);
	mainSystems.addSystem(motionSystem);
	renderingPipeline.addSystem(renderableMeshSystem);
	renderingPipeline.addSystem(megaCubeRenderer);

	uint32 fps = 0;
	double lastTime = Time::getTime();
	double fpsTimeCounter = 0.0;
	double updateTimer = 1.0;
	float frameTime = 1.0 / 60.0;
	while (app->isRunning())
	{
		double currentTime = Time::getTime();
		double passedTime = currentTime - lastTime;
		lastTime = currentTime;

		fpsTimeCounter += passedTime;
		updateTimer += passedTime;

		if (fpsTimeCounter >= 1.0)
		{
			double msPerFrame = 1000.0 / (double)fps;
			DEBUG_LOG("FPS", "NONE", "%f ms (%d fps)", msPerFrame, fps);
			fpsTimeCounter = 0;
			fps = 0;
		}

		bool shouldRender = true;
		while (updateTimer >= frameTime)
		{
			app->processMessages(frameTime, eventHandler);
			// Begin scene update
			ecs.updateSystems(mainSystems, frameTime);
			// End scene update

			updateTimer -= frameTime;
			shouldRender = true;
		}

		if (shouldRender)
		{
			// Begin scene render
			gameRenderContext.clear(color, true);
			ecs.updateSystems(renderingPipeline, frameTime);
			gameRenderContext.flush();
			// End scene render

			window.present();
			fps++;
		}
		else
		{
			Time::sleep(1);
		}
	}
	return 0;
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
