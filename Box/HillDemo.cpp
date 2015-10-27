#include "d3dApp.h"


class HillsApp : public D3DApp
{
public:
	HillsApp();
	~HillsApp();

	bool Init();
	void OnResize();
	void UpdateScene();
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
};