#pragma once
#include <vector>

class Component;

class GameObject
{
private:
	std::vector<Component*> ownedComponents;
	std::vector<size_t> componentsToDestroy;

	void DestroyPendingComponents();

public:
	GameObject();
	virtual ~GameObject();

	virtual void Start();
	virtual void Update(float deltaTime);

	template<typename T, typename... ARGS>
	T* AddComponent(ARGS&& ...args)
	{
		T* p_comp = new T(this, std::forward<ARGS>(args)...);
		ownedComponents.push_back(p_comp);
		return p_comp;
	}

	template<typename T>
	T* GetComponent()
	{
		for (Component* p_comp : ownedComponents)
		{
			if (p_comp->IsA<T>())
				return static_cast<T*>(p_comp);
		}
	}

	template<typename T>
	void GetComponents(std::vector<T*>& outComponents)
	{
		for (Component* p_comp : ownedComponents)
		{
			if (p_comp->IsA<T>())
				outComponents.push_back(static_cast<T*>(p_comp));
		}
	}

	void RemoveComponent(Component* p_component);
};