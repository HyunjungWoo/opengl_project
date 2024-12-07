#pragma once

#include "GameObject.h"

class RotateObject : public GameObject
{
	float rotateSpeed;

public:
	RotateObject();
	virtual ~RotateObject();

	virtual void initilize() override;
	virtual void update(float elapsedTime) override;
	virtual void draw() const override;
	virtual void release() override;

};

