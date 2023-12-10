#include "script_component.h"
#include "game_object.h"
#include "debug.h"

ScriptComponent::ScriptComponent(
	GameObject* _p_owner,
	const std::string& scriptPath, 
	const std::vector<Tolo::FunctionHandle>& _functions, 
	const std::vector<Tolo::StructHandle>& _structs
) :
	Component(_p_owner),
	p_ownedScriptProgram(nullptr),
	functions(_functions),
	structs(_structs)
{
	scriptFileWatcher.Init(scriptPath);
	Compile();
}

ScriptComponent::~ScriptComponent()
{
	delete p_ownedScriptProgram;
}

bool ScriptComponent::CanBeCastedTo(ComponentId id)
{
	return id.id == GetTypeId<ScriptComponent>().id || Component::CanBeCastedTo(id);
}

void ScriptComponent::Compile()
{
	Tolo::ProgramHandle* p_newProgram = nullptr;
	Engine::Info("compiling script component '", scriptFileWatcher.filePath, "'");

	try
	{
		p_newProgram = new Tolo::ProgramHandle(scriptFileWatcher.filePath, 1024, "Update");

		for (const Tolo::StructHandle& str : structs)
			p_newProgram->AddStruct(str);

		for (const Tolo::FunctionHandle& func : functions)
			p_newProgram->AddFunction(func);

		p_newProgram->Compile();
	}
	catch (const Tolo::Error& error)
	{
		delete p_newProgram;
		error.Print();
		return;
	}

	delete p_ownedScriptProgram;
	p_ownedScriptProgram = p_newProgram;
}

void ScriptComponent::EarlyUpdate(float deltaTime)
{
	if (scriptFileWatcher.NewVersionAvailable())
		Compile();
	
	if (p_ownedScriptProgram != nullptr)
	{
		Tolo::Ptr thisPtr = reinterpret_cast<Tolo::Ptr>(this);
		p_ownedScriptProgram->Execute<void>(thisPtr, deltaTime);
	}
}