#pragma once
class Objects
{
public:
	Objects();
	~Objects();

	virtual void Update() = 0;
private:

};

Objects::Objects()
{
}

Objects::~Objects()
{
}