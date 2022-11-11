#include "../include/DEApplication.hpp"
#include "../include/DECore.hpp"
#include "../include/DEGUI.hpp"
#include "../include/DERendering.hpp"
#include "../include/DEThirdPartyMath.hpp"

void Func(DvigEngine2::demachword* arguments, DvigEngine2::deint32 jobIndex)
{
    std::cout << "Hello!" << std::endl;
}

int main()
{
    DvigEngine2::MemoryPoolProperty memoryPoolsData[2];
    memoryPoolsData[0].m_ByteWidth = 4 * DV_MEMORY_MiB;
    memoryPoolsData[1].m_ByteWidth = 24 * DV_MEMORY_KiB;

    DvigEngine2::EngineInputProperty engineInputData;
    engineInputData.m_Version = DV_ENGINE_VERSION_NUMBER;
    engineInputData.m_MemoryPoolsCount = 2u;
    engineInputData.m_MemoryPoolsData = memoryPoolsData;
    engineInputData.m_SystemMemoryPoolIndex = 0;
    engineInputData.m_ComponentStorageMemoryPoolIndex = 1;
    engineInputData.m_RequestedThreadCount = 1;

    // DvigEngine2::Engine::Init(&engineInputData);
    // DvigEngine2::Engine* engine = DvigEngine2::Engine::GetClassInstance();
    DvigEngine2::Engine engine(&engineInputData);
    DvigEngine2::Engine* pEngine = &engine;

    // Create window
    class AppWindow : public DvigEngine2::IWindow {
        public:
            void Update() override final {
                DvigEngine2::Application* app = this->GetApplication();
                DvigEngine2::RenderingSystem* renderSys = app->GetRenderingSystem();

                renderSys->BeginRender();

                renderSys->BeginBatch();
                renderSys->EndBatch();

                renderSys->EndRender();
            }
    };

    const char* windowCaption = "DvigEngine Test";
    glm::uvec2 windowSize(640, 480);

    DvigEngine2::Application* appSys = pEngine->Create <DvigEngine2::Application> ( "MyApplication_0" );
    appSys->Init();
    appSys->AddWindow <AppWindow> ( "MyTestWindow_0", &windowCaption[0], windowSize );

    class MyQueue : public DvigEngine2::IQueue { public: void Free() override final { } };
    DvigEngine2::demachword arguments[1];
    MyQueue* queue = pEngine->Create <MyQueue> ( "MyQueue_0" );
    queue->Init();
    queue->AddJob( &Func, &arguments[0], 0 );
    queue->DoJobs();

    appSys->Start();

    // DvigEngine2::RenderingSystem* renderSys = DvigEngine2::RenderingSystem::GetClassInstance();

    // AppWindow* window = engine->Create <AppWindow> ( "MyTestWindow_0" );
    // window->Init();

    // DvigEngine2::IWindow::Start();

    // engine->RegisterComponent <DvigEngine2::GeometryComponent> ();

    // // Vertices
    // DvigEngine2::deusize vertexCount = 9;
    // float vertices[9] = {
    //     -1.0f, -1.0f, 0.0f,
    //     0.0f, 1.0f, 0.0f,
    //     1.0f, -1.0f, 0.0f
    // };

    // DvigEngine2::GeometryComponent* geometryComp = engine->Create <DvigEngine2::GeometryComponent> ( "MyGeometryComponent_0" );
    // DvigEngine2::GeometryComponent* geometryComp2 = engine->Create <DvigEngine2::GeometryComponent> ( "MyGeometryComponent_2" );
    // geometryComp2->Init( nullptr, (void*)&vertices[0], 4 * vertexCount );
    // geometryComp->Init( nullptr, (void*)&vertices[0], 4 * vertexCount );
    // geometryComp->val = 255;

    // geometryComp2->Free();

    // DvigEngine2::IComponent* exComponent = (DvigEngine2::IComponent*)engine->GetExistingInstance( "MyGeometryComponent_0" );

    // DvigEngine2::INode* nodes = engine->Create <DvigEngine2::INode> ( "MyNode_0" );
    // nodes->Init();
    // engine->AddComponent <DvigEngine2::GeometryComponent> ( &nodes, geometryComp );

    // DvigEngine2::GeometryComponent* getComponent = (DvigEngine2::GeometryComponent*)nodes->GetComponent( "MyGeometryComponent_0" );
    // std::cout << getComponent->val << std::endl;

    // DvigEngine2::GeometryBatch::ClearGeometryBuffer();

    /*
        Node "House"
          GeometryComponent
        Node "House"
          GeometryComponent

        Draw()
          Bind vertex buffer
          Bind uniform buffer

        renderSys = RenderingSystem::Instance
        renderSys -> BeginBatch()
          renderSys -> Draw()
        renderSys -> EndBatch()
    */

    /*
        DvigEngine Info
        - Everything is based on objects
        - Object is something that has INTERNAL data and FUNCTIONALITY that operates on INTERNAL data
        - Component is a piece of data
        - System is an object that does not have external data but rather FUNCTIONALITY on SOMEWHAT data
        - HelperObject is an object that combines both INTERNAL data and FUNCTIONALITY on INTERNAL data

        Node
         Node
          GeometryComponent
          ScriptComponent
         Node
          GeometryComponent
    */

    std::cout << "Success!" << std::endl;

    return 0;
}