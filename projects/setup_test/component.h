#pragma once

class GameObject;

class Component
{
private:
	GameObject* p_owner;

	Component() = delete;

protected:
	Component(GameObject* _p_owner);

	struct ComponentId
	{
		int id;
	};

	static ComponentId nextId;

	template<typename T>
	static ComponentId GetTypeId()
	{
		static ComponentId typeId = { nextId.id++ };
		return typeId;
	}

	virtual bool CanBeCastedTo(ComponentId id);

public:
	virtual ~Component();

	virtual void EarlyUpdate(float deltaTime);
	virtual void LateUpdate(float deltaTime);

	void Destroy();
	GameObject& GetOwner();

	template<typename T>
	bool IsA()
	{
		return CanBeCastedTo(GetTypeId<T>());
	}
};