#include "app.h"
#include "default_meshes.h"
#include "shader.h"
#include "texture.h"
#include "input.h"
#include "program_handle.h"
#include "debug.h"
#include "game_object.h"
#include "transform.h"
#include "script_component.h"

static float totalTime = 0.f;
App_SetupTest* p_currentApp = nullptr;


float Box(const glm::vec3& p, const glm::vec3& b)
{
	glm::vec3 q = glm::abs(p) - b;
	return glm::length(glm::max(q, 0.f)) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.f);
}

glm::vec3 RepXZ(const glm::vec3 p, const glm::vec2& s)
{
	glm::vec2 q = s * glm::round(glm::vec2(p.x, p.z) / s);
	return p - glm::vec3(q.x, 0.f, q.y);
}

glm::vec3 RotX(const glm::vec3& p, float k)
{
	float k0 = glm::atan(p.y, p.z);
	float r = glm::length(glm::vec2(p.y, p.z));
	return glm::vec3(p.x, r * glm::sin(k0 + k), r * glm::cos(k0 + k));
}

glm::vec3 Fold(const glm::vec3& p, const glm::vec3& n)
{
	return p - 2.f * glm::min(0.f, glm::dot(p, n)) * n;
}

float Capsule(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, float r)
{
	glm::vec3 pa = p - a;
	glm::vec3 ba = b - a;
	float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.f, 1.f);
	return glm::length(pa - ba * h) - r;
}

float Tree(glm::vec3 p)
{
	glm::vec2 dim = glm::vec2(1.f, 8.f);
	float d = Capsule(p, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 1.f + dim.y, 0.f), dim.x);
	glm::vec3 scale = glm::vec3(1.f);
	glm::vec3 change = glm::vec3(0.7f, 0.68f, 0.7f);

	glm::vec3 n1 = normalize(glm::vec3(1.f + 0.1f * glm::cos(totalTime), 0.f, 1.f));
	glm::vec3 n2 = glm::vec3(n1.x, 0.f, -n1.z);
	glm::vec3 n3 = glm::vec3(-n1.x, 0.f, n1.z);

	for (int i = 0; i < 7; i++)
	{
		p = Fold(p, n1);
		p = Fold(p, n2);
		p = Fold(p, n3);

		p.y -= scale.y * dim.y;
		p.z = abs(p.z);
		p = RotX(p, 3.1415f * 0.25f);
		scale *= change;

		d = glm::min(d, Capsule(p, glm::vec3(0.f), glm::vec3(0.f, dim.y * scale.y, 0.), scale.x * dim.x));
	}

	return d;
}

float SmoothUnion(float d1, float d2, float k)
{
	float h = glm::clamp(0.5f + 0.5f * (d2 - d1) / k, 0.f, 1.f);
	return glm::mix(d2, d1, h) - k * h * (1.f - h);
}

float WorldSDF(const glm::vec3& p)
{
	//glm::vec3 q = p;
	//q.y -= 20. * cos(q.x * 0.01 + 1.5) * cos(q.z * 0.01 + 1.5);
	//float plane = q.y - 0.2f * glm::sin(q.x) * glm::sin(q.z);
	//float trees = Tree(RepXZ(q - glm::vec3(0.f, 1.f, 0.f), glm::vec2(40.f)));
	//float hills = glm::length(RepXZ(q + glm::vec3(0.f, 10.f, 0.f), glm::vec2(34.f))) - 12.f;
	//return SmoothUnion(plane, glm::min(trees, hills), 0.8f);

	return p_currentApp->worldSdfObject.p_program->Execute<float>(p);
}


SdfObject::SdfObject() :
	p_program(nullptr)
{
	for (size_t i = 0; i < fileWatchers.size(); i++)
		fileWatchers[i] = new Engine::FileWatcher();
}

SdfObject::~SdfObject()
{
	delete p_program;
	
	for (size_t i = 0; i < fileWatchers.size(); i++)
		delete fileWatchers[i];
}

void SdfObject::InitProgram(Tolo::ProgramHandle& program)
{
	program.AddStruct({
		"vec3",
		{
			{"float", "x"},
			{"float", "y"},
			{"float", "z"}
		}
	});
	program.AddFunction({ "vec3", "operator+", {"vec3", "vec3"}, [](Tolo::VirtualMachine& vm)
		{
			glm::vec3 a = Tolo::Pop<glm::vec3>(vm);
			glm::vec3 b = Tolo::Pop<glm::vec3>(vm);
			Tolo::PushStruct<glm::vec3>(vm, a + b);
		}
	});
	program.AddFunction({ "vec3", "operator-", {"vec3", "vec3"}, [](Tolo::VirtualMachine& vm)
		{
			glm::vec3 a = Tolo::Pop<glm::vec3>(vm);
			glm::vec3 b = Tolo::Pop<glm::vec3>(vm);
			Tolo::PushStruct<glm::vec3>(vm, a - b);
		}
	});
	program.AddFunction({ "vec3", "operator-", {"vec3"}, [](Tolo::VirtualMachine& vm)
		{
			glm::vec3 a = Tolo::Pop<glm::vec3>(vm);
			Tolo::PushStruct<glm::vec3>(vm, -a);
		}
	});
	program.AddFunction({ "vec3", "operator*", {"vec3", "float"}, [](Tolo::VirtualMachine& vm)
		{
			glm::vec3 a = Tolo::Pop<glm::vec3>(vm);
			float b = Tolo::Pop<float>(vm);
			Tolo::PushStruct<glm::vec3>(vm, a * b);
		}
	});
	program.AddFunction({ "float", "length", {"vec3"}, [](Tolo::VirtualMachine& vm)
		{
			glm::vec3 v = Tolo::Pop<glm::vec3>(vm);
			Tolo::Push<Tolo::Float>(vm, glm::length(v));
		}
	});
	program.AddFunction({ "float", "sin", {"float"}, [](Tolo::VirtualMachine& vm)
		{
			float a = Tolo::Pop<float>(vm);
			Tolo::Push<float>(vm, glm::sin(a));
		}
	});
	program.AddFunction({ "float", "cos", {"float"}, [](Tolo::VirtualMachine& vm)
		{
			float a = Tolo::Pop<float>(vm);
			Tolo::Push<float>(vm, glm::cos(a));
		}
	});
	program.AddFunction({ "float", "min", {"float", "float"}, [](Tolo::VirtualMachine& vm)
		{
			float a = Tolo::Pop<float>(vm);
			float b = Tolo::Pop<float>(vm);
			Tolo::Push<float>(vm, glm::min(a, b));
		} 
	});
	program.AddFunction({ "float", "max", {"float", "float"}, [](Tolo::VirtualMachine& vm)
		{
			float a = Tolo::Pop<float>(vm);
			float b = Tolo::Pop<float>(vm);
			Tolo::Push<float>(vm, glm::max(a, b));
		}
	});
	program.AddFunction({ "float", "Tree", {"vec3"}, [](Tolo::VirtualMachine& vm)
		{
			glm::vec3 p = Tolo::Pop<glm::vec3>(vm);
			Tolo::Push<float>(vm, Tree(p));
		}
	});
	program.AddFunction({ "vec3", "RepXZ", {"vec3", "float", "float"}, [](Tolo::VirtualMachine& vm)
		{
			glm::vec3 p = Tolo::Pop<glm::vec3>(vm);
			float x = Tolo::Pop<float>(vm);
			float y = Tolo::Pop<float>(vm);
			Tolo::PushStruct<glm::vec3>(vm, RepXZ(p, glm::vec2(x, y)));
		}
	});
	program.AddFunction({ "float", "SmoothUnion", {"float", "float", "float"}, [](Tolo::VirtualMachine& vm)
		{
			float a = Tolo::Pop<float>(vm);
			float b = Tolo::Pop<float>(vm);
			float k = Tolo::Pop<float>(vm);
			Tolo::Push<float>(vm, SmoothUnion(a, b, k));
		}
	});
}

void SdfObject::Init(const std::string& vertShaderPath, const std::string& fragShaderPath, const std::string& sdfPath)
{
	fileWatchers[0]->Init(vertShaderPath);
	fileWatchers[1]->Init(fragShaderPath);
	fileWatchers[2]->Init(sdfPath);

	Reload();
}

void SdfObject::Reload()
{
	Engine::Info("compiling sdf object");

	Tolo::ProgramHandle* p_newProgram = nullptr;
	std::string sdfCode;
	try
	{
		p_newProgram = new Tolo::ProgramHandle(fileWatchers[2]->filePath, 1024, "Sdf");
		InitProgram(*p_newProgram);
		p_newProgram->Compile(sdfCode);
	}
	catch (const Tolo::Error& error)
	{
		delete p_newProgram;
		error.Print();
		Engine::Info("failed to compile sdf object, keeping old version");
		return;
	}

	shader.Reload(fileWatchers[0]->filePath, fileWatchers[1]->filePath, { "__SDF__", sdfCode });

	delete p_program;
	p_program = p_newProgram;
}

void SdfObject::Update()
{
	for (size_t i = 0; i < fileWatchers.size(); i++)
	{
		if (fileWatchers[i]->NewVersionAvailable())
		{
			Reload();
			break;
		}
	}
}


App_SetupTest::App_SetupTest()
{}

void App_SetupTest::Init()
{
	p_currentApp = this;

	window.Init(1200, 800, "setup_test");
	//window.Init(800, 600, "setup_test");
	window.SetMouseVisible(false);
	glClearColor(0.1f, 0.1f, 0.1f, 0.f);

	worldSdfObject.Init("assets/shaders/sdf_vert.glsl", "assets/shaders/sdf_frag.glsl", "assets/tolo/test.tolo");


	physicsWorld.Init(WorldSDF, { 1.f, 0.4f });
	physicsWorld.gravity = glm::vec3(0.f, -9.82f, 0.f);

	for (size_t i = 0; i < spheres.size(); i++)
	{
		float mass = 100.f;
		float radius = 1.f;

		Engine::Rigidbody& rb = spheres[i].rb;
		rb.centerOfMass = glm::vec3(0.f + glm::cos(float(i)), 20.f + i * 3.f, 30.f);
		rb.SetMass(mass);
		rb.SetInertiaTensor(Engine::Rigidbody::SphereInertiaTensor(radius, mass));

		spheres[i].collider.radius = radius;

		physicsWorld.AddObject(&spheres[i].collider, &rb, { 0.8f, 0.4f });
	}

	for (size_t i = 0; i < capsules.size(); i++)
	{
		float mass = 100.f;
		float radius = 1.f;
		float height = 2.f;

		Engine::Rigidbody& rb = capsules[i].rb;
		rb.centerOfMass = glm::vec3(0.f + glm::cos(float(i)), 20.f + i * 4.f, 10.f);
		rb.SetMass(mass);
		rb.SetInertiaTensor(Engine::Rigidbody::CylinderInertiaTensor(radius, height + radius, mass));
		rb.angularDamping = 0.9f;

		capsules[i].collider.radius = radius;
		capsules[i].collider.height = height;

		physicsWorld.AddObject(&capsules[i].collider, &rb, { 0.8f, 0.4f });
	}

	player.AddToPhysicsWorld(physicsWorld);
	player.rigidbody.SetMass(100.f);
	player.rigidbody.SetInertiaTensor(Engine::Rigidbody::CylinderInertiaTensor(1.f, 2.f, 100.f));
	player.rigidbody.centerOfMass = glm::vec3(4.f, 4.f, 0.f);
	player.rigidbody.SetLockRotation(true);
	player.camera.Init(70.f, (float)window.Width() / window.Height(), 0.3f, 500.f);

	physicsWorld.Start();
}

void App_SetupTest::UpdateLoop()
{
	Engine::RenderMesh screenQuad, cubeMesh, sphereMesh, capsuleMesh;
	Engine::GenerateUnitQuad(screenQuad);
	Engine::GenerateUnitCube(cubeMesh);
	Engine::GenerateUnitSphere(sphereMesh, {Engine::MeshGeneratorSettings::E_Normals | Engine::MeshGeneratorSettings::E_Uvs});
	Engine::GenerateUnitCapsule(capsuleMesh, { Engine::MeshGeneratorSettings::E_Normals | Engine::MeshGeneratorSettings::E_Uvs});

	Engine::Shader phongShader, skyboxShader, flatShader;
	phongShader.Reload("assets/shaders/phong_textured_vert.glsl", "assets/shaders/phong_textured_frag.glsl");
	skyboxShader.Reload("assets/shaders/skybox_vert.glsl", "assets/shaders/skybox_frag.glsl");
	flatShader.Reload("assets/shaders/flat_vert.glsl", "assets/shaders/flat_frag.glsl");

	Engine::TextureCube skyboxTexture;
	std::string skyboxTexturePaths[6]
	{
		"assets/textures/skybox/right.jpg",
		"assets/textures/skybox/left.jpg",
		"assets/textures/skybox/top.jpg",
		"assets/textures/skybox/bottom.jpg",
		"assets/textures/skybox/front.jpg",
		"assets/textures/skybox/back.jpg"
	};
	skyboxTexture.Reload(skyboxTexturePaths);

	Engine::Texture2D objectTexture;
	objectTexture.Reload("assets/textures/image.png");

	glm::vec3 lightDir(glm::normalize(glm::vec3(-0.5f, -1.f, 0.8f)));

	float pixelRadius = 0.5f * glm::length(glm::vec2(1.f / window.Width(), 1.f / window.Height()));
	float fixedDeltaTime = 1.f / 60.f;
	bool mouseVisible = false;
	std::vector<glm::vec4> particles;
	float particleLifeTime = 1.f;


	//
	
	GameObject go;
	go.AddComponent<Transform>();
	auto sc = go.AddComponent<ScriptComponent>(
		"assets/tolo/update_test.tolo",
		std::vector<Tolo::FunctionHandle>{ 
			{ "void", "UpdatePosition", { "ptr" , "vec3" }, [](Tolo::VirtualMachine& vm) {
				Tolo::Ptr thisPtr = Tolo::Pop<Tolo::Ptr>(vm);
				glm::vec3 newPos = Tolo::Pop<glm::vec3>(vm);

				ScriptComponent* p_this = reinterpret_cast<ScriptComponent*>(thisPtr);
				GameObject& obj = p_this->GetOwner();
				auto t = obj.GetComponent<Transform>();

				t->position = newPos;
			}
		}},
		std::vector<Tolo::StructHandle>({
			{"vec3", {
				{"float", "x"},
				{"float", "y"},
				{"float", "z"}
			}}
		})
	);

	go.Update(fixedDeltaTime);

	//



	while (!window.ShouldClose())
	{
		totalTime += fixedDeltaTime;
		window.BeginUpdate();

		worldSdfObject.Update();

		auto& IP = Engine::Input::Instance();
		if (IP.GetKey(GLFW_KEY_ESCAPE).WasPressed())
			window.SetMouseVisible(mouseVisible = !mouseVisible);

		physicsWorld.Update(fixedDeltaTime);

		for (Engine::Collision& collision : physicsWorld.collisions)
		{
			if (glm::length2(collision.impulse) * collision.p_object->p_rigidbody->inverseMass > 300.f)
			{
				particles.push_back(glm::vec4(collision.hitPoint + glm::vec3(0.1f, 0.f, 0.f), 0.1f));
				particles.push_back(glm::vec4(collision.hitPoint - glm::vec3(0.f, 0.1f, 0.f), 0.f));
				particles.push_back(glm::vec4(collision.hitPoint + glm::vec3(0.f, 0.f, 0.1f), 0.13f));
			}
		}

		for (int i=(int)particles.size()-1; i>=0; i--)
		{
			if (particles[i].w > particleLifeTime)
			{
				particles[i] = particles.back();
				particles.pop_back();
			}
			else
				particles[i] += glm::vec4(0.f, 10.f - 20.f * particles[i].w, 0.f, 1.f) * fixedDeltaTime;
		}
		
		player.Update(fixedDeltaTime);

		glm::mat4 P = player.camera.CalcP();
		glm::mat4 V = player.camera.CalcV(player.cameraTransform);
		glm::mat4 VP = P * V;
		glm::mat4 invVP = glm::inverse(VP);
		glm::mat4 skyboxVP = P * glm::mat4(glm::mat3(V));

		skyboxShader.Use();
		skyboxShader.SetMat4("u_VP", &skyboxVP[0][0]);
		skyboxTexture.Bind(GL_TEXTURE0);
		cubeMesh.Bind();
		glDepthMask(GL_FALSE);
		glCullFace(GL_FRONT);
		cubeMesh.Draw(0);
		glDepthMask(GL_TRUE);
		glCullFace(GL_BACK);
		cubeMesh.Unbind();
		skyboxTexture.Unbind(GL_TEXTURE0);
		skyboxShader.StopUsing();

		glm::vec2 nearFar(player.camera.GetNearPlane(), player.camera.GetFarPlane());

		Engine::Shader& sdfShader = worldSdfObject.shader;

		sdfShader.Use();
		sdfShader.SetVec2("u_nearFar", &nearFar[0]);
		sdfShader.SetMat4("u_invVP", &invVP[0][0]);
		sdfShader.SetVec3("u_camPos", &player.cameraTransform[3][0]);
		sdfShader.SetFloat("u_pixelRadius", pixelRadius);
		sdfShader.SetFloat("u_time", totalTime);
		screenQuad.Bind();
		screenQuad.Draw(0);
		screenQuad.Unbind();
		sdfShader.StopUsing();


		phongShader.Use();
		phongShader.SetVec3("u_camPos", &player.cameraTransform[3][0]);
		phongShader.SetVec3("u_lightDir", &lightDir[0]);
		glm::vec3 textureScale(1.f);
		phongShader.SetVec2("u_textureScale", &textureScale[0]);
		objectTexture.Bind(GL_TEXTURE0);

		for (size_t i = 0; i < spheres.size(); i++)
		{
			glm::mat4 M = glm::mat4(glm::mat3_cast(spheres[i].rb.rotation) * glm::mat3(spheres[i].collider.radius));
			M[3] = glm::vec4(spheres[i].rb.centerOfMass, 1.f);
			glm::mat4 MVP = VP * M;
			glm::mat3 N = glm::transpose(glm::inverse(glm::mat3(M)));
			glm::vec3 color(1.f, 0.9f, 0.9f);

			phongShader.SetMat4("u_MVP", &MVP[0][0]);
			phongShader.SetMat4("u_M", &M[0][0]);
			phongShader.SetMat3("u_N", &N[0][0]);
			phongShader.SetVec3("u_color", &color[0]);
			phongShader.SetFloat("u_roughness", 0.5f);
			sphereMesh.Bind();
			sphereMesh.Draw(0);
			sphereMesh.Unbind();
		}
		for (size_t i = 0; i < capsules.size(); i++)
		{
			glm::mat4 M = glm::mat4(glm::mat3_cast(capsules[i].rb.rotation) * glm::mat3(
				capsules[i].collider.radius, 0.f, 0.f,
				0.f, capsules[i].collider.height/2.f, 0.f,
				0.f, 0.f, capsules[i].collider.radius
			));

			M[3] = glm::vec4(capsules[i].rb.centerOfMass, 1.f);
			glm::mat4 MVP = VP * M;
			glm::mat3 N = glm::transpose(glm::inverse(glm::mat3(M)));
			glm::vec3 color(0.9f, 1.f, 1.f);

			phongShader.SetMat4("u_MVP", &MVP[0][0]);
			phongShader.SetMat4("u_M", &M[0][0]);
			phongShader.SetMat3("u_N", &N[0][0]);
			phongShader.SetVec3("u_color", &color[0]);
			phongShader.SetFloat("u_roughness", 0.5f);
			capsuleMesh.Bind();
			capsuleMesh.Draw(0);
			capsuleMesh.Unbind();
		}
		objectTexture.Unbind(GL_TEXTURE0);
		phongShader.StopUsing();

		flatShader.Use();
		{
			glm::mat4 M(1.f);
			M[0][0] = 4.f / window.Width();
			M[1][1] = 4.f / window.Height();
			glm::vec3 color(0.9f, 0.7f, 0.f);

			flatShader.SetMat4("u_MVP", &M[0][0]);
			flatShader.SetVec3("u_color", &color[0]);
			screenQuad.Bind();
			screenQuad.Draw(0);
			screenQuad.Unbind();
		}
		flatShader.StopUsing();

		
		flatShader.Use();
		for (glm::vec4& particle : particles)
		{
			float alpha = particle.w / particleLifeTime;
			glm::mat4 M(0.3f * (1.f - alpha));
			M[3] = glm::vec4(glm::vec3(particle), 1.f);

			M = M * glm::mat4_cast(glm::angleAxis(3.14f * alpha, glm::vec3(0.f, 0.f, 1.f)));

			glm::mat3 scale = player.cameraTransform * M;
			M[0] = glm::vec4(scale[0], 0.f);
			M[1] = glm::vec4(scale[1], 0.f);
			M[2] = glm::vec4(scale[2], 0.f);

			glm::mat4 MVP(VP * M);

			glm::vec3 color(1.f);

			flatShader.SetMat4("u_MVP", &MVP[0][0]);
			flatShader.SetVec3("u_color", &color[0]);
			screenQuad.Bind();
			screenQuad.Draw(0);
			screenQuad.Unbind();
		}
		flatShader.StopUsing();

		window.EndUpdate();
	}
}

void App_SetupTest::Deinit()
{
	window.Deinit();
}
