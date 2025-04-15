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
#include "WICTextureLoader.h"
#include "Lights.h"
#include "Sky.h"

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

	// Create lighting
	{
		ambientLight = XMFLOAT3(0.0f, 0.0f, 0.0f);

		directionalLight.type = LIGHT_TYPE_DIRECTIONAL;
		directionalLight.direction = XMFLOAT3(1, -1, 0);
		directionalLight.color = XMFLOAT3(0.56, 0.78, 0.43);
		directionalLight.intensity = 1.0;

		directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
		directionalLight2.direction = XMFLOAT3(-1, -1, 0);
		directionalLight2.color = XMFLOAT3(0.66, 0.66, 0.66);
		directionalLight2.intensity = 0.0;

		directionalLight3.type = LIGHT_TYPE_DIRECTIONAL;
		directionalLight3.direction = XMFLOAT3(1, 1, 0);
		directionalLight3.color = XMFLOAT3(0.32, 0.32, 0.32);
		directionalLight3.intensity = 0.0;

		pointLight1.type = LIGHT_TYPE_POINT;
		pointLight1.direction = XMFLOAT3(0, -1, 0);
		pointLight1.color = XMFLOAT3(1, 1, 1);
		pointLight1.intensity = 0.0;
		pointLight1.position = XMFLOAT3(0, 4, 0);
		pointLight1.range = 5;

		spotLight.type = LIGHT_TYPE_SPOT;
		spotLight.direction = XMFLOAT3(0, -1, 0);
		spotLight.color = XMFLOAT3(1, 0.68, 1);
		spotLight.intensity = 0.0;
		spotLight.position = XMFLOAT3(7, 2, 0);
		spotLight.spotInnerAngle = XMConvertToRadians(10);
		spotLight.spotOuterAngle = XMConvertToRadians(20);
		spotLight.range = 8;

		lights.push_back(directionalLight);
		lights.push_back(directionalLight2);
		lights.push_back(directionalLight3);
		lights.push_back(pointLight1);
		lights.push_back(spotLight);
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

	std::shared_ptr<SimplePixelShader> twoTexturesPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"TwoMaterialsPS.cso").c_str());

	std::shared_ptr<SimplePixelShader> debugUVPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugUVsPS.cso").c_str());

	std::shared_ptr<SimplePixelShader> debugNormalPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugNormalsPS.cso").c_str());

	std::shared_ptr<SimplePixelShader> customPS1 = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CustomPS1.cso").c_str());

	std::shared_ptr<SimpleVertexShader> skyVS = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"SkyVS.cso").c_str());

	std::shared_ptr<SimplePixelShader> skyPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"SkyPS.cso").c_str());

	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); // Adding black for A03
	XMFLOAT4 pink = XMFLOAT4(0.96f, 0.33f, 0.73f, 1.0f); // Also adding pink
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // Debug tint for uv and normals

	// Create sampler states
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampleState;
	D3D11_SAMPLER_DESC sampleDesc = {};
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device.Get()->CreateSamplerState(&sampleDesc, sampleState.GetAddressOf());

	// Load in albedos
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> onyxSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> snowSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Metal/albedo.png").c_str(), nullptr, metalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Onyx/albedo.png").c_str(), nullptr, onyxSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Snow/albedo.png").c_str(), nullptr, snowSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Wood/albedo.png").c_str(), nullptr, woodSRV.GetAddressOf());

	// Normals
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> onyxNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> snowNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodNormalSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Metal/normal.png").c_str(), nullptr, metalNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Onyx/normal.png").c_str(), nullptr, onyxNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Snow/normal.png").c_str(), nullptr, snowNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Wood/normal.png").c_str(), nullptr, woodNormalSRV.GetAddressOf());


	// Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> onyxRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> snowRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodRoughnessSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Metal/roughness.png").c_str(), nullptr, metalRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Onyx/roughness.png").c_str(), nullptr, onyxRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Snow/roughness.png").c_str(), nullptr, snowRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Wood/roughness.png").c_str(), nullptr, woodRoughnessSRV.GetAddressOf());
	
	// Metalness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalMetalnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> onyxMetalnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> snowMetalnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodMetalnessSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Metal/metalness.png").c_str(), nullptr, metalMetalnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Onyx/metalness.png").c_str(), nullptr, onyxMetalnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Snow/metalness.png").c_str(), nullptr, snowMetalnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Wood/metalness.png").c_str(), nullptr, woodMetalnessSRV.GetAddressOf());

	// Create Materials
	std::shared_ptr<Material> metalMaterial = std::make_shared<Material>(white, vShader, pShader, DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2(0, 0), 1.0f);
	std::shared_ptr<Material> onyxMaterial = std::make_shared<Material>(white, vShader, pShader, DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2(0, 0), 1.0f);
	std::shared_ptr<Material> snowMaterial = std::make_shared<Material>(white, vShader, pShader, DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2(0, 0), 1.0f);
	std::shared_ptr<Material> woodMaterial = std::make_shared<Material>(white, vShader, pShader, DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2(0, 0), 0.0f);
	std::shared_ptr<Material> twoTexturesMaterial = std::make_shared<Material>(white, vShader, pShader, DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2(0, 0), 0.68f);

	std::shared_ptr<Material> debugUV = std::make_shared<Material>(white, vShader, debugUVPS, DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2(0, 0), 0.0f);
	std::shared_ptr<Material> debugNormals = std::make_shared<Material>(white, vShader, debugNormalPS, DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2(0, 0), 0.5f);
	std::shared_ptr<Material> customMaterial = std::make_shared<Material>(white, vShader, customPS1, DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2(0, 0), 0.34f);

	materials.push_back(metalMaterial);
	materials.push_back(onyxMaterial);
	materials.push_back(snowMaterial);
	materials.push_back(woodMaterial);
	materials.push_back(twoTexturesMaterial);

	materials.push_back(debugUV);
	materials.push_back(debugNormals);
	materials.push_back(customMaterial);

	// Apply textures and normals to materials using pShader
	metalMaterial->AddTextureSRV("Albedo", metalSRV);
	metalMaterial->AddTextureSRV("NormalMap", metalNormalSRV);
	metalMaterial->AddTextureSRV("RoughnessMap", metalRoughnessSRV);
	metalMaterial->AddTextureSRV("MetalnessMap", metalMetalnessSRV);
	metalMaterial->AddSampler("BasicSampler", sampleState);

	onyxMaterial->AddTextureSRV("Albedo", onyxSRV);
	onyxMaterial->AddTextureSRV("NormalMap", onyxNormalSRV);
	onyxMaterial->AddTextureSRV("RoughnessMap", onyxRoughnessSRV);
	onyxMaterial->AddTextureSRV("MetalnessMap", onyxMetalnessSRV);
	onyxMaterial->AddSampler("BasicSampler", sampleState);

	snowMaterial->AddTextureSRV("Albedo", snowSRV);
	snowMaterial->AddTextureSRV("NormalMap", snowNormalSRV);
	snowMaterial->AddTextureSRV("RoughnessMap", snowRoughnessSRV);
	snowMaterial->AddTextureSRV("MetalnessMap", snowMetalnessSRV);
	snowMaterial->AddSampler("BasicSampler", sampleState);

	woodMaterial->AddTextureSRV("Albedo", woodSRV);
	woodMaterial->AddTextureSRV("NormalMap", woodNormalSRV);
	woodMaterial->AddTextureSRV("RoughnessMap", woodRoughnessSRV);
	woodMaterial->AddTextureSRV("MetalnessMap", woodMetalnessSRV);
	woodMaterial->AddSampler("BasicSampler", sampleState);

	twoTexturesMaterial->AddTextureSRV("SurfaceTexture", onyxSRV);
	twoTexturesMaterial->AddTextureSRV("SurfaceTexture2", snowSRV);
	twoTexturesMaterial->AddSampler("BasicSampler", sampleState);

	// Create meshes
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>(FixPath("../../Assets/Models/cube.obj").c_str());
	std::shared_ptr<Mesh> cylinder = std::make_shared<Mesh>(FixPath("../../Assets/Models/cylinder.obj").c_str());
	std::shared_ptr<Mesh> helix = std::make_shared<Mesh>(FixPath("../../Assets/Models/helix.obj").c_str());
	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>(FixPath("../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> torus = std::make_shared<Mesh>(FixPath("../../Assets/Models/torus.obj").c_str());
	std::shared_ptr<Mesh> quad = std::make_shared<Mesh>(FixPath("../../Assets/Models/quad.obj").c_str());
	std::shared_ptr<Mesh> quad2Side = std::make_shared<Mesh>(FixPath("../../Assets/Models/quad_double_sided.obj").c_str());

	meshes.push_back(cube);
	meshes.push_back(cylinder);
	meshes.push_back(helix);
	meshes.push_back(sphere);
	meshes.push_back(torus);
	meshes.push_back(quad);
	meshes.push_back(quad2Side);

	// Create entities
	std::shared_ptr<GameEntity> sphereEntity = std::make_shared<GameEntity>(sphere, metalMaterial);
	std::shared_ptr<GameEntity> sphereEntity2 = std::make_shared<GameEntity>(sphere, onyxMaterial);
	std::shared_ptr<GameEntity> sphereEntity3 = std::make_shared<GameEntity>(sphere, snowMaterial);
	std::shared_ptr<GameEntity> sphereEntity4 = std::make_shared<GameEntity>(sphere, woodMaterial);

	sphereEntity2->GetTransform()->SetPosition(-3.0f, 0.0f, 0.0f);
	sphereEntity3->GetTransform()->SetPosition(3.0f, 0.0f, 0.0f);
	sphereEntity4->GetTransform()->SetPosition(6.0f, 0.0f, 0.0f);

	entities.push_back(sphereEntity);
	entities.push_back(sphereEntity2);
	entities.push_back(sphereEntity3);
	entities.push_back(sphereEntity4);

	// Create skybox
	skybox = std::make_shared<Sky>(cube, sampleState, skyPS, skyVS, 
		FixPath(L"../../Assets/Textures/Skybox/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Skybox/back.png").c_str());
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
		const float black[4] = { 0, 0, 0, 0 };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), black);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry, each mesh is drawn seperately as mesh class has been created
	for (UINT i = 0; i < entities.size(); i++)
	{
		// Pass in the ambient light to each shader
		entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("ambientLight", ambientLight);
		entities[i]->GetMaterial()->GetPixelShader()->SetData(
			"lights",
			&lights[0],
			sizeof(Light) * (int)lights.size());
		entities[i]->Draw(*currentCamera);
	}

	// After drawing all geometry draw the sky
	skybox->Draw(*currentCamera);

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

	// Information about materials
	if (ImGui::TreeNode("Materials"))
	{
		// Display each material
		for (UINT i = 0; i < materials.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode("Material Node", "Material: %d", i))
			{
				// Displaying textures
				for (auto& texture : materials[i]->GetSRVs())
				{
					ImGui::Text(texture.first.c_str());
					ImGui::Image((ImTextureID)texture.second.Get(), ImVec2(256, 256));
				}

				// Color tint
				XMFLOAT4 color = materials[i]->GetColor();
				if (ImGui::ColorEdit4("Color Tint", &color.x))
					materials[i]->SetColor(color);
				
				// UV scale and offset
				XMFLOAT2 scale = materials[i]->GetScale();
				XMFLOAT2 offset = materials[i]->GetOffset();
				
				if (ImGui::DragFloat2("UV Scale", &scale.x, 0.2f, 1.0f, 10.0f)) materials[i]->SetScale(scale);
				if (ImGui::DragFloat2("UV Offset", &offset.x, 0.2f, -10.0f, 10.0f)) materials[i]->SetOffset(offset);

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// Color change for lights
	if (ImGui::TreeNode("Lights"))
	{
		// Display ambient light
		if (ImGui::TreeNode("Ambient light"))
		{
			ImGui::ColorEdit3("Ambient color", &ambientLight.x);
			ImGui::TreePop();
		}

		// Display each light
		for (UINT i = 0; i < lights.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode("Light Node", "Light %d", i))
			{
				ImGui::ColorEdit3("Color", &lights[i].color.x);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// End UI
	ImGui::End();
}