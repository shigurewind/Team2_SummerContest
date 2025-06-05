#include "FBXBox.h"
//#include "DirectGraphics.h"


static Box g_Box;




HRESULT InitBox(void)
{
	g_Box.model = new FbxMeshFile();
	if (g_Box.model->Load(
		"data\MODEL\MaterialBox.fbx",

		DirectGraphics::GetInstance()->GetDevice(),
		DirectGraphics::GetInstance()->GetVertexShader()) == false)
	{
		return E_FAIL;
	}

	g_Box.pos = Vector3(0.0f, 0.0f, 0.0f);
	g_Box.rot = Vector3(0.0f, 0.0f, 0.0f);
	g_Box.scl = Vector3(10.0f, 10.0f, 10.0f);



	return S_OK;
}


void UninitBox(void)
{
	delete g_Box.model;

	DirectGraphics::GetInstance()->Release();
}



void UpdateBox(void)
{

}


void DrawBox(void)
{
	DirectGraphics::GetInstance()->StartRendering();

	DirectGraphics::GetInstance()->SetUpContext();

	g_Box.model->Render(
		DirectGraphics::GetInstance(),
		g_Box.pos,
		g_Box.scl,
		g_Box.rot);


	DirectGraphics::GetInstance()->FinishRendering();
}



Box* GetBox(void)
{
	return &g_Box;
}