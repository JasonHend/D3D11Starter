#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"

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
	CreateGeometry();

	// Initialize ImGui with it's backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

	// Style for ImGui (I always use dark mode)
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
	}

	// Instantiate UI variables
	{
		// Initialize background color
		backgroundColor = new float[4] { 0.0f, 0.3f, 0.5f, 1.0f };

		// Initialize default vsync state out of loop to be used in UI
		vsync = Graphics::VsyncState();

		// Color tint and offset vectors
		colorTint = new float[4] { 0.0f, 0.0f, 1.0f, 0.8f };
		offset = new float[3] { 0.0f, 0.0f, 0.0f };
	}

	// Create cameras
	{
		// Make initial positions and cameras
		XMFLOAT3 firstCamPos(0.0f, 0.0f, -5.0f);
		std::shared_ptr firstCam = std::make_shared<Camera>(Window::AspectRatio(), firstCamPos, 90.0f);

		XMFLOAT3 secondCamPos(2.0f, 0.0f, -3.0f);
		std::shared_ptr secondCam = std::make_shared<Camera>(Window::AspectRatio(), secondCamPos, 120.0f);

		XMFLOAT3 thirdCamPos(-2.0f, 3.0f, -10.0f);
		std::shared_ptr thirdCam = std::make_shared<Camera>(Window::AspectRatio(), thirdCamPos, 60.0f);

		// Push back and set main
		cameras.push_back(firstCam);
		cameras.push_back(secondCam);
		cameras.push_back(thirdCam);

		currentCamera = cameras[0];
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
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Load Shaders
	std::shared_ptr<SimpleVertexShader> vShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());

	std::shared_ptr<SimplePixelShader> pShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());

	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //Adding black for A03
	XMFLOAT4 pink = XMFLOAT4(0.96f, 0.33f, 0.73f, 1.0f); //Also adding pink

	// Create Materials
	std::shared_ptr<Material> redMaterial = std::make_shared<Material>(red, vShader, pShader);
	std::shared_ptr<Material> greenMaterial = std::make_shared<Material>(green, vShader, pShader);
	std::shared_ptr<Material> blueMaterial = std::make_shared<Material>(blue, vShader, pShader);

	// Create models
	std::shared_ptr<Mesh> sphere1 = std::make_shared<Mesh>(FixPath("../../Assets/Models/forJason.obj").c_str());
	meshes.push_back(sphere1);

	std::shared_ptr<GameEntity> sphereEntity = std::make_shared<GameEntity>(sphere1, redMaterial);
	entities.push_back(sphereEntity);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for (int i = 0; i < cameras.size(); i++)
	{
		if (cameras[i] != nullptr)
		{
			cameras[i]->UpdateProjectionMatrix(Window::AspectRatio());
		}
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Call helper method to update UI
	UpdateUIContext(deltaTime);
	CustomizeUIContext();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Update some entities
	entities[0]->GetTransform()->SetRotation(XMFLOAT3(0.0f, 90.0f, 0.0f));

	// Update the cameras
	currentCamera->Update(deltaTime);
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
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry, each mesh is drawn seperately as mesh class has been created
	for (UINT i = 0; i < entities.size(); i++)
	{
		entities[i]->Draw(*currentCamera);
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Render UI at the end of frame
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
	// Send new data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Reset input captures
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
}

// --------------------------------------------------------
// Creates all custom UI elements
// --------------------------------------------------------
void Game::CustomizeUIContext()
{
	// Begin UI and give it a custom name
	ImGui::Begin("Custom Context");

	// Tree for app data
	if (ImGui::TreeNode("Application Data"))
	{
		// Frame rate
		ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);

		// Window size
		ImGui::Text("Window Width: %d Height: %d", Window::Width(), Window::Height());

		// Color selector
		ImGui::ColorEdit4("Background color editor", backgroundColor);

		// Button to toggle vsync
		if (ImGui::Button("Toggle vsync"))
			vsync = !vsync;

		// Show the demo window
		if (showDemo)
		{
			ImGui::ShowDemoWindow();
		}

		// Button for user to toggle demo
		if (ImGui::Button("Toggle demo window"))
			showDemo = !showDemo;

		ImGui::TreePop();
	}

	// Create UI tree to describe each mesh being shown
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

	// UI tree for constant buffer data
	if (ImGui::TreeNode("Constant Buffer"))
	{
		ImGui::ColorEdit4("ColorTint", colorTint);
		ImGui::DragFloat3("Offset", offset);
		ImGui::TreePop();
	}

	// UI tree for game entities
	if (ImGui::TreeNode("Entities"))
	{
		for (UINT i = 0; i < entities.size(); i++)
		{
			ImGui::PushID(i);
			// Reference floats for position, rotation, and scale
			XMFLOAT3 position = entities[i]->GetTransform()->GetPosition();
			XMFLOAT3 rotation = entities[i]->GetTransform()->GetPitchYawRoll();
			XMFLOAT3 scale = entities[i]->GetTransform()->GetScale();

			if (ImGui::TreeNode("Entity", "Entity: %d", i))
			{
				// Display each float3
				if (ImGui::DragFloat3("Position", &position.x, 0.1f)) entities[i]->GetTransform()->SetPosition(position);
				if (ImGui::DragFloat3("Rotation", &rotation.x, 0.1f)) entities[i]->GetTransform()->SetRotation(rotation);
				if (ImGui::DragFloat3("Scale", &scale.x, 0.1f)) entities[i]->GetTransform()->SetScale(scale);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// UI for cameras
	if (ImGui::TreeNode("Cameras"))
	{
		// Selection area for current camera
		static int selected = 0;
		if (ImGui::RadioButton("Camera 1: 90FOV", &selected, 0))
		{
			currentCamera = cameras[0];
		}
		if (ImGui::RadioButton("Camera 2: 120FOV", &selected, 1))
		{
			currentCamera = cameras[1];
		}
		if (ImGui::RadioButton("Camera 3: 60FOV", &selected, 2))
		{
			currentCamera = cameras[2];
		}

		// Display information about each camera
		for (UINT i = 0; i < cameras.size(); i++)
		{
			XMFLOAT3 position = cameras[i]->GetTransform()->GetPosition();

			ImGui::PushID(i);
			if (ImGui::TreeNode("Camera", "Camera: %d", i))
			{
				ImGui::DragFloat3("Position", &position.x, 0.0f);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// End UI
	ImGui::End();
}