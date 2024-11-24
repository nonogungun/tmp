#pragma once

#include "Object.h"

class Object;

class Component
{
private:
	weak_ptr<Object> m_object;



	friend class Object;
};

