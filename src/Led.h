#pragma once

#include "Plugin.h"
#include "Logger.h"

#include <Components.h>

#include <vector>

namespace hc {
    class Led : public Plugin, public lrcpp::Led {
    public:
        Led();
        virtual ~Led() {}

        void init(hc::Logger* logger);

        // hc::Plugin
        virtual char const* getName() override;
        virtual char const* getVersion() override;
        virtual char const* getLicense() override;
        virtual char const* getCopyright() override;
        virtual char const* getUrl() override;

        virtual void onStarted() override;
        virtual void onConsoleLoaded() override;
        virtual void onGameLoaded() override;
        virtual void onGamePaused() override;
        virtual void onGameResumed() override;
        virtual void onGameReset() override;
        virtual void onFrame() override;
        virtual void onDraw() override;
        virtual void onGameUnloaded() override;
        virtual void onConsoleUnloaded() override;
        virtual void onQuit() override;

        // lrcpp::Audio
        virtual void setState(int led, int state) override;

    protected:
        hc::Logger* _logger;

        std::vector<int> _states;
    };
}
