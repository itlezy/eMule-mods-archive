#pragma once

template<typename T>
class SingletonEx 
{
public:
	static bool HasInstance() 
	{
		return (instance!=NULL);
	}

	static T* GetInstance() 
	{
		ASSERT(instance);
		return instance;
	}
	
	static void NewInstance(T* p) 
	{
		ASSERT(instance==NULL);
		if(instance)
			delete instance;
		instance = p;
//		instance = new T();
	}
	
	static void DeleteInstance() 
	{
		ASSERT(instance);
		if(instance)
			delete instance;
		instance = NULL;
	}

protected:
	static T* instance;
};

template<typename T>
T* SingletonEx<T>::instance = NULL;
