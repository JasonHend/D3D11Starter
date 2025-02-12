#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "BufferStructs.h"
#include "GameEntity.h"

#include <DirectXMath.h>
#include <vector>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

//Include ImGui headers
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	//Initialize ImGui with it's backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

	//Style for ImGui (I always use dark mode)
	ImGui::StyleColorsDark();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	//Instantiate UI variables
	{
		//Initialize background color
		backgroundColor = new float[4] { 0.0f, 0.3f, 0.5f, 1.0f };

		//Initialize default vsync state out of loop to be used in UI
		vsync = Graphics::VsyncState();

		//Color tint and offset vectors
		colorTint = new float[4] { 0.0f, 0.0f, 1.0f, 0.8f };
		offset = new float[3] { 0.0f, 0.0f, 0.0f };
	}

	//Create constant buffer
	{
		//Defines size of the constant buffer (multiple of 16)
		unsigned int constBuffSize = sizeof(BufferStructs);
		constBuffSize = (constBuffSize + 15) / 16 * 16;

		//Intialize constant buffer
		D3D11_BUFFER_DESC constBuffDesc = {};
		constBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBuffDesc.ByteWidth = constBuffSize;
		constBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constBuffDesc.Usage = D3D11_USAGE_DYNAMIC;

		//Create buffer and bind it
		Graphics::Device->CreateBuffer(&constBuffDesc, 0, constBuffer.GetAddressOf());
		Graphics::Context->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());
	}
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	delete backgroundColor;
	backgroundColor = nullptr;

	delete colorTint;
	colorTint = nullptr;

	delete offset;
	offset = nullptr;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //Adding black for A03
	XMFLOAT4 pink = XMFLOAT4(0.96f, 0.33f, 0.73f, 1.0f); //Also adding pink

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };

	std::shared_ptr<Mesh>triangle = std::make_shared<Mesh>(vertices, ARRAYSIZE(vertices), indices, ARRAYSIZE(indices), "Default Triangle");

	//Start creating some custom meshes
	Vertex verticesQuad[] =
	{
		{ XMFLOAT3(+0.9f, +0.9f, +0.0f), red },
		{ XMFLOAT3(+0.9f, +0.7f, +0.0f), black },
		{ XMFLOAT3(+0.7f, +0.7f, +0.0f), blue },
		{ XMFLOAT3(+0.7f, +0.9f, +0.0f), black },
	};

	unsigned int indicesQuad[] = { 0, 1, 2, 0, 2, 3 };

	std::shared_ptr<Mesh>quad = std::make_shared<Mesh>(verticesQuad, ARRAYSIZE(verticesQuad), indicesQuad, ARRAYSIZE(indicesQuad), "Quad");

	//Make a mesh that is a face
	Vertex verticesFace[] =
	{
		//Left brow
		{ XMFLOAT3(-0.7f, +0.55f, +0.0f), black},
		{ XMFLOAT3(-0.65f, +0.70f, +0.0f), black},
		{ XMFLOAT3(-0.60f, +0.55f, +0.0f), black},

		//Right brow
		{ XMFLOAT3(-0.50f, +0.55f, +0.0f), black},
		{ XMFLOAT3(-0.45f, +0.70f, +0.0f), black},
		{ XMFLOAT3(-0.40f, +0.55f, +0.0f), black},

		//Left eye
		{ XMFLOAT3(-0.70f, +0.40f, +0.0f), black},
		{ XMFLOAT3(-0.70f, +0.50f, +0.0f), black},
		{ XMFLOAT3(-0.60f, +0.45f, +0.0f), black},
		{ XMFLOAT3(-0.60f, +0.40f, +0.0f), black},

		//Right eye
		{ XMFLOAT3(-0.50f, +0.40f, +0.0f), black},
		{ XMFLOAT3(-0.50f, +0.45f, +0.0f), black},
		{ XMFLOAT3(-0.40f, +0.50f, +0.0f), black},
		{ XMFLOAT3(-0.40f, +0.40f, +0.0f), black},

		//Nose
		{ XMFLOAT3(-0.575f, +0.30f, +0.0f), pink},
		{ XMFLOAT3(-0.55f, +0.35f, +0.0f), pink},
		{ XMFLOAT3(-0.525f, +0.30f, +0.0f), pink},
	};
	
	unsigned int indicesFace[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 9, 6, 10, 11, 12, 12, 13, 10, 14, 15, 16 };
	
	std::shared_ptr<Mesh> face = std::make_shared<Mesh>(verticesFace, ARRAYSIZE(verticesFace), indicesFace, ARRAYSIZE(indicesFace), "Face");

	//Push all meshes into the vector
	meshes.push_back(triangle);
	meshes.push_back(quad);
	meshes.push_back(face);

	//Create entities
	std::shared_ptr<GameEntity> triangleEntity = std::make_shared<GameEntity>(triangle);
	std::shared_ptr<GameEntity> quadEntity = std::make_shared<GameEntity>(quad);
	std::shared_ptr<GameEntity> faceEntity = std::make_shared<GameEntity>(face);
	std::shared_ptr<GameEntity> faceEntity2 = std::make_shared<GameEntity>(face);
	std::shared_ptr<GameEntity> quadEntity2 = std::make_shared<GameEntity>(quad);

	entities.push_back(triangleEntity);
	entities.push_back(quadEntity);
	entities.push_back(faceEntity);
	entities.push_back(faceEntity2);
	entities.push_back(quadEntity2);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//Call helper method to update UI
	UpdateUIContext(deltaTime);
	CustomizeUIContext();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	//Update some entities
	entities[0]->GetTransform()->SetRotation(0.0f, totalTime, 0.0f);
	entities[3]->GetTransform()->SetPosition((float)sin(totalTime), (float)sin(totalTime), 0.0f);
	entities[4]->GetTransform()->SetPosition(-0.75f, -0.75f, 0.0f);
	entities[4]->GetTransform()->SetScale((float)sin(totalTime), 1.0f, 1.0f);
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry, each mesh is drawn seperately as mesh class has been created
	for (UINT i = 0; i < entities.size(); i++)
	{
		entities[i]->Draw(constBuffer);
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		//Render UI at the end of frame
		ImGui::Render(); //Turn data into triangles to draw
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); //Draw triangles

		// Present at the end of the frame
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

// --------------------------------------------------------
// Pass values in to ImGui and reset/call frames
// --------------------------------------------------------
void Game::UpdateUIContext(float deltaTime)
{
	//Send new data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	//Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Reset input captures
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
}

// --------------------------------------------------------
// Creates all custom UI elements
// --------------------------------------------------------
void Game::CustomizeUIContext()
{
	//Begin UI and give it a custom name
	ImGui::Begin("Custom Context");

	//Tree for app data
	if (ImGui::TreeNode("Application Data"))
	{
		//Frame rate
		ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);

		//Window size
		ImGui::Text("Window Width: %d Height: %d", Window::Width(), Window::Height());

		//Color selector
		ImGui::ColorEdit4("Background color editor", backgroundColor);

		//Button to toggle vsync
		if (ImGui::Button("Toggle vsync"))
			vsync = !vsync;

		//Show the demo window
		if (showDemo)
		{
			ImGui::ShowDemoWindow();
		}

		//Button for user to toggle demo
		if (ImGui::Button("Toggle demo window"))
			showDemo = !showDemo;

		ImGui::TreePop();
	}

	//Create UI tree to describe each mesh being shown
	if (ImGui::TreeNode("Meshes"))
	{
		for (UINT i = 0; i < meshes.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode("Mesh", "Mesh: %s", meshes[i]->GetMeshName()))
			{
				ImGui::Text("Triangles: %d", meshes[i]->GetIndexCount() / 3);
				ImGui::Text("Vertices: %d", meshes[i]->GetVertexCount());
				ImGui::Text("Indices: %d", meshes[i]->GetIndexCount());
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	//UI tree for constant buffer data
	if (ImGui::TreeNode("Constant Buffer"))
	{
		ImGui::ColorEdit4("ColorTint", colorTint);
		ImGui::DragFloat3("Offset", offset);
		ImGui::TreePop();
	}

	//UI tree for game entities
	if (ImGui::TreeNode("Entities"))
	{
		for (UINT i = 0; i < entities.size(); i++)
		{
			ImGui::PushID(i);
			//Reference floats for position, rotation, and scale
			XMFLOAT3 position = entities[i]->GetTransform()->GetPosition();
			XMFLOAT3 rotation = entities[i]->GetTransform()->GetPitchYawRoll();
			XMFLOAT3 scale = entities[i]->GetTransform()->GetScale();

			if (ImGui::TreeNode("Entity", "Entity: %d", i))
			{
				//Display each float3
				if (ImGui::DragFloat3("Position", &position.x, 0.1f)) entities[i]->GetTransform()->SetPosition(position);
				if (ImGui::DragFloat3("Rotation", &rotation.x, 0.1f)) entities[i]->GetTransform()->SetRotation(rotation);
				if (ImGui::DragFloat3("Scale", &scale.x, 0.1f)) entities[i]->GetTransform()->SetScale(scale);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	//End UI
	ImGui::End();
}