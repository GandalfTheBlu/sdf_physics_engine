#include "component.h"
#include "program_handle.h"
#include "file_watcher.h"

class ScriptComponent : public Component
{
private:
	Tolo::ProgramHandle* p_ownedScriptProgram;
	Engine::FileWatcher scriptFileWatcher;
	std::vector<Tolo::FunctionHandle> functions;
	std::vector<Tolo::StructHandle> structs;

	void Compile();

protected:
	virtual bool CanBeCastedTo(ComponentId id) override;

public:
	ScriptComponent(
		GameObject* _p_owner, 
		const std::string& scriptPath, 
		const std::vector<Tolo::FunctionHandle>& _functions, 
		const std::vector<Tolo::StructHandle>& _structs
	);
	virtual ~ScriptComponent();

	virtual void EarlyUpdate(float deltaTime) override;
};