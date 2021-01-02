#pragma once

#include "Logger.h"

#include <Components.h>

#include <string>
#include <vector>
#include <unordered_map>

namespace hc {
    class Config: public lrcpp::Config {
    public:
        Config() : _logger(nullptr) {}
        virtual ~Config() {}

        bool init(Logger* logger);
        void destroy();
        void reset();
        void draw();

        void setCorePath(std::string const& corePath);

        virtual bool setPerformanceLevel(unsigned level) override;
        virtual bool getSystemDirectory(char const** directory) override;
        virtual bool getVariable(retro_variable* variable) override;
        virtual bool getVariableUpdate(bool* const updated) override;
        virtual bool setSupportNoGame(bool const supports) override;
        virtual bool getLibretroPath(char const** path) override;
        virtual bool getCoreAssetsDirectory(char const** directory) override;
        virtual bool getSaveDirectory(char const** directory) override;
        virtual bool setProcAddressCallback(retro_get_proc_address_interface const* callback) override;
        virtual bool setSubsystemInfo(retro_subsystem_info const* info) override;
        virtual bool setMemoryMaps(retro_memory_map const* map) override;
        virtual bool getUsername(char const** username) override;
        virtual bool getLanguage(unsigned* language) override;
        virtual bool setSupportAchievements(bool supports) override;
        virtual bool setSerializationQuirks(uint64_t quirks) override;
        virtual bool getAudioVideoEnable(int* enabled) override;
        virtual bool getFastForwarding(bool* is) override;
        virtual bool setCoreOptions(retro_core_option_definition const* options) override;
        virtual bool setCoreOptionsIntl(retro_core_options_intl const* intl) override;
        virtual bool setCoreOptionsDisplay(retro_core_option_display const* display) override;

    protected:
        struct SubsystemInfo {
            struct Rom {
                struct Memory {
                    std::string extension;
                    unsigned type;
                };

                std::string description;
                std::string validExtensions;
                bool needFullpath;
                bool blockExtract;
                bool required;
                std::vector<Memory> memory;
            };

            std::string description;
            std::string name;
            std::vector<Rom> roms;
            unsigned identifier;
        };

        struct MemoryDescriptor {
            uint64_t flags;
            void* pointer;
            size_t offset;
            size_t start;
            size_t select;
            size_t disconnect;
            size_t length;
            std::string addressSpace;
        };

        struct CoreOption {
            struct Value {
                std::string value;
                std::string label;
            };

            std::string key;
            std::string label;
            std::string sublabel;

            std::vector<Value> values;
            unsigned selected;
            bool visible;
        };

        Logger* _logger;

        std::string _systemPath;
        std::string _coreAssetsPath;
        std::string _savePath;
        std::string _corePath;

        unsigned _performanceLevel;
        bool _supportsNoGame;
        retro_get_proc_address_t _getCoreProc;
        bool _supportAchievements;
        uint64_t _serializationQuirks;

        std::vector<SubsystemInfo> _subsystemInfo;
        std::vector<MemoryDescriptor> _memoryMap;

        std::unordered_map<std::string, CoreOption> _coreOptions;
        bool _optionsUpdated;
    };
}