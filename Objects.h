#pragma once
class Objects
{
public:
	Objects();
	virtual ~Objects();

	virtual void RenderInit() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;

	bool m_visible = true;
private:

};
