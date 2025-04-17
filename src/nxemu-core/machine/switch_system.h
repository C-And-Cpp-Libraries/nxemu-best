#pragma once
#include <memory>
#include <nxemu-core/modules/modules.h>

class Nro;

class SwitchSystem :
    public ISwitchSystem
{
public:
    ~SwitchSystem();

    static bool Create(IRenderWindow & window);
    static void ShutDown();
    static SwitchSystem * GetInstance();

    bool LoadRom(const char * romFile);
    void StartEmulation(void);
    void StopEmulation(void);

    //ISwitchSystem
    IOperatingSystem & OperatingSystem();
    IVideo & Video(void);
    ICpu & Cpu(void);

private:
    SwitchSystem(const SwitchSystem &) = delete;
    SwitchSystem & operator=(const SwitchSystem &) = delete;

    SwitchSystem();

    bool Initialize(IRenderWindow & window);
    bool LoadNRO(const char * nroFile);

    static std::unique_ptr<SwitchSystem> m_instance;
    bool m_emulationRunning;
    Modules m_modules;
    std::unique_ptr<Nro> m_nro;
};
