#include "Config.h"

#include <fnkdat.h>
#include <IconsFontAwesome4.h>

extern "C" {
    #include "lauxlib.h"
}

#include <inttypes.h>

bool hc::Config::init(Logger* logger) {
    _logger = logger;
    _logger->debug("%s:%u: %s()", __FILE__, __LINE__, __FUNCTION__);

    reset();

    if (fnkdat(NULL, 0, 0, FNKDAT_INIT) != 0) {
        _logger->error("Error initializing fnkdat");
        return false;
    }

    char path[1024];

    if (fnkdat("autorun.lua", path, sizeof(path), FNKDAT_USER | FNKDAT_CREAT) != 0) {
        _logger->error("Error getting the autorun.lua file path");
        return false;
    }

    _autorunPath = path;
    _logger->info("The autorun.lua file path is \"%s\"", path);

    if (fnkdat("system/", path, sizeof(path), FNKDAT_USER | FNKDAT_CREAT) != 0) {
        _logger->error("Error getting the system path");
        return false;
    }

    _systemPath = path;
    _logger->info("The system path is \"%s\"", path);

    if (fnkdat("assets/", path, sizeof(path), FNKDAT_USER | FNKDAT_CREAT) != 0) {
        _logger->error("Error getting the core assets path");
        return false;
    }

    _coreAssetsPath = path;
    _logger->info("The core assets path is \"%s\"", path);

    if (fnkdat("saves/", path, sizeof(path), FNKDAT_USER | FNKDAT_CREAT) != 0) {
        _logger->error("Error getting the saves path");
        return false;
    }

    _savePath = path;
    _logger->info("The save path is \"%s\"", path);

    if (fnkdat("cores/", path, sizeof(path), FNKDAT_USER | FNKDAT_CREAT) != 0) {
        _logger->error("Error getting the cores path");
        return false;
    }

    _coresPath = path;
    _logger->info("The cores path is \"%s\"", path);

    return true;
}

void hc::Config::destroy() {
    _logger->debug("%s:%u: %s()", __FILE__, __LINE__, __FUNCTION__);
    fnkdat(NULL, 0, 0, FNKDAT_UNINIT);
}

void hc::Config::reset() {
    _logger->debug("%s:%u: %s()", __FILE__, __LINE__, __FUNCTION__);

    _performanceLevel = 0;
    _supportsNoGame = false;
    _getCoreProc = nullptr;
    _supportAchievements = false;
    _serializationQuirks = 0;

    _subsystemInfo.clear();
    _memoryMap.clear();

    _coreOptions.clear();
    _coreMap.clear();
    _optionsUpdated = false;
}

void hc::Config::draw() {
    if (!ImGui::Begin(ICON_FA_WRENCH " Configuration")) {
        return;
    }

    for (auto& option : _coreOptions) {
        if (!option.visible) {
            continue;
        }

        static auto const getter = [](void* const data, int const idx, char const** const text) -> bool {
            auto const option = (CoreOption const*)data;

            if ((size_t)idx < option->values.size()) {
                *text = option->values[idx].label.c_str();
                return true;
            }

            return false;
        };

        int selected = static_cast<int>(option.selected);
        int old = selected;

        ImGui::Combo(option.label.c_str(), &selected, getter, (void*)&option, option.values.size());

        if (old != selected) {
            option.selected = static_cast<unsigned>(selected);
            _optionsUpdated = true;
            _logger->info("Variable \"%s\" changed to \"%s\"", option.key.c_str(), option.values[selected].value.c_str());
        }
    }

    ImGui::End();
}

bool hc::Config::setPerformanceLevel(unsigned level) {
    _logger->debug("%s:%u: %s(%u)", __FILE__, __LINE__, __FUNCTION__, level);
    _performanceLevel = level;
    _logger->info("Set performance level to %u", level);
    return true;
}

bool hc::Config::getSystemDirectory(char const** directory) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, directory);
    *directory = _systemPath.c_str();
    return true;
}

bool hc::Config::getVariable(retro_variable* variable) {
    _logger->debug("%s:%u: %s(%p) // variable->key = \"%s\"", __FILE__, __LINE__, __FUNCTION__, variable, variable->key);

    auto const found = _coreMap.find(variable->key);

    if (found != _coreMap.end()) {
        auto const& option = _coreOptions[found->second];
        variable->value = option.values[option.selected].value.c_str();
        _logger->info("Found value \"%s\" for variable \"%s\"", variable->value, variable->key);
        return true;
    }

    variable->value = nullptr;
    _logger->error("Variable \"%s\" not found", variable->key);
    return false;
}

bool hc::Config::getVariableUpdate(bool* const updated) {
    bool const previous = _optionsUpdated;
    _optionsUpdated = false;
    return previous;
}

bool hc::Config::setSupportNoGame(bool const supports) {
    _logger->debug("%s:%u: %s(%s)", __FILE__, __LINE__, __FUNCTION__, supports ? "true" : "false");
    _supportsNoGame = supports;
    _logger->info("Set supports no game to %s", supports ? "true" : "false");
    return true;
}

bool hc::Config::getLibretroPath(char const** path) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, path);
    *path = _coresPath.c_str();
    return true;
}

bool hc::Config::getCoreAssetsDirectory(char const** directory) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, directory);
    *directory = _coreAssetsPath.c_str();
    return true;
}

bool hc::Config::getSaveDirectory(char const** directory) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, directory);
    *directory = _savePath.c_str();
    return true;
}

bool hc::Config::setProcAddressCallback(retro_get_proc_address_interface const* callback) {
    _logger->debug(
        "%s:%u: %s(%p) // callback->get_proc_address = %p",
        __FILE__, __LINE__, __FUNCTION__, callback,
        callback->get_proc_address
    );

    _getCoreProc = callback->get_proc_address;
    _logger->info("Set get procedure address to %p", callback->get_proc_address);
    return true;
}

bool hc::Config::setSubsystemInfo(retro_subsystem_info const* info) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, info);
    _logger->info("Setting subsystem information");

    for (; info->desc != nullptr; info++) {
        _logger->info("    desc  = %s", info->desc);
        _logger->info("    ident = %s", info->ident);
        _logger->info("    id    = %u", info->id);

        SubsystemInfo tempinfo;
        tempinfo.description = info->desc;
        tempinfo.name = info->ident;
        tempinfo.identifier = info->id;

        for (unsigned i = 0; i < info->num_roms; i++) {
            retro_subsystem_rom_info const* const rom = info->roms + i;

            _logger->info("        roms[%u].desc             = %s", i, rom->desc);
            _logger->info("        roms[%u].valid_extensions = %s", i, rom->valid_extensions);
            _logger->info("        roms[%u].need_fullpath    = %d", i, rom->need_fullpath);
            _logger->info("        roms[%u].block_extract    = %d", i, rom->block_extract);
            _logger->info("        roms[%u].required         = %d", i, rom->required);

            SubsystemInfo::Rom temprom;
            temprom.description = rom->desc;
            temprom.validExtensions = rom->valid_extensions;
            temprom.needFullpath = rom->need_fullpath;
            temprom.blockExtract = rom->block_extract;
            temprom.required = rom->required;

            for (unsigned j = 0; j < info->roms[i].num_memory; j++) {
                retro_subsystem_memory_info const* const memory = rom->memory + j;

                _logger->info("            memory[%u].type         = %u", j, memory->type);
                _logger->info("            memory[%u].extension    = %s", j, memory->extension);

                SubsystemInfo::Rom::Memory tempmem;
                tempmem.extension = memory->extension;
                tempmem.type = memory->type;

                temprom.memory.emplace_back(tempmem);
            }

            tempinfo.roms.emplace_back(temprom);
        }

        _subsystemInfo.emplace_back(tempinfo);
    }

    return true;
}

bool hc::Config::setMemoryMaps(retro_memory_map const* map) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, map);
    _logger->info("Setting memory maps");

    auto const descriptors = static_cast<retro_memory_descriptor*>(malloc(map->num_descriptors * sizeof(retro_memory_descriptor)));

    if (descriptors == nullptr) {
        _logger->error("Out of memory setting memory descriptors");
        return false;
    }

    memcpy(descriptors, map->descriptors, sizeof(retro_memory_descriptor) * map->num_descriptors);
    
    if (!preprocessMemoryDescriptors(descriptors, map->num_descriptors)) {
        _logger->error("Error processing memory descriptors");
        free(static_cast<void*>(descriptors));
        return false;
    }

    _logger->info("    ndx flags  ptr      offset   start    select   disconn  len      addrspace");

    for (unsigned i = 0; i < map->num_descriptors; i++) {
        retro_memory_descriptor const* const descriptor = descriptors + i;

        char flags[7];
        flags[0] = 'M';
        flags[2] = 'A';
        flags[6] = 0;

        switch (descriptor->flags & (RETRO_MEMDESC_MINSIZE_2 | RETRO_MEMDESC_MINSIZE_4 | RETRO_MEMDESC_MINSIZE_8)) {
            case RETRO_MEMDESC_MINSIZE_2: flags[1] = '2'; break;
            case RETRO_MEMDESC_MINSIZE_4: flags[1] = '4'; break;
            case RETRO_MEMDESC_MINSIZE_8: flags[1] = '8'; break;
            default: flags[1] = '1'; break;
        }

        switch (descriptor->flags & (RETRO_MEMDESC_ALIGN_2 | RETRO_MEMDESC_ALIGN_4 | RETRO_MEMDESC_ALIGN_8)) {
            case RETRO_MEMDESC_ALIGN_2: flags[3] = '2'; break;
            case RETRO_MEMDESC_ALIGN_4: flags[3] = '4'; break;
            case RETRO_MEMDESC_ALIGN_8: flags[3] = '8'; break;
            default: flags[3] = '1'; break;
        }

        flags[4] = (descriptor->flags & RETRO_MEMDESC_BIGENDIAN) != 0 ? 'B' : 'b';
        flags[5] = (descriptor->flags & RETRO_MEMDESC_CONST) != 0 ? 'C' : 'c';

        _logger->info(
            "    %3u %s %p %08X %08X %08X %08X %08X %s",
            i, flags, descriptor->ptr, descriptor->offset, descriptor->start, descriptor->select,
            descriptor->disconnect, descriptor->len, descriptor->addrspace != nullptr ? descriptor->addrspace : ""
        );

        MemoryDescriptor tempdesc;
        tempdesc.flags = descriptor->flags;
        tempdesc.pointer = descriptor->ptr;
        tempdesc.offset = descriptor->offset;
        tempdesc.start = descriptor->start;
        tempdesc.select = descriptor->select;
        tempdesc.disconnect = descriptor->disconnect;
        tempdesc.length = descriptor->len;
        tempdesc.addressSpace = descriptor->addrspace != nullptr ? descriptor->addrspace : "";

        _memoryMap.emplace_back(tempdesc);
    }

    free(static_cast<void*>(descriptors));
    return true;
}

bool hc::Config::getUsername(char const** username) {
    static char const* const value = "hackcon";

    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, username);
    _logger->warn("Returning fixed username: %s", value);

    *username = value;
    return true;
}

bool hc::Config::getLanguage(unsigned* language) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, language);
    _logger->warn("Returning fixed language English");

    *language = RETRO_LANGUAGE_ENGLISH;
    return true;
}

bool hc::Config::setSupportAchievements(bool supports) {
    _logger->debug("%s:%u: %s(%s)", __FILE__, __LINE__, __FUNCTION__, supports ? "true" : "false");
    _supportAchievements = supports;
    _logger->info("Set support achievement to %s", supports ? "true" : "false");
    return true;
}

bool hc::Config::setSerializationQuirks(uint64_t quirks) {
    _logger->debug("%s:%u: %s(%" PRIu64 ")", __FILE__, __LINE__, __FUNCTION__, quirks);
    _serializationQuirks = quirks;
    _logger->info("Set serialization quirks to %" PRIu64, quirks);
    return true;
}

bool hc::Config::getAudioVideoEnable(int* enabled) {
    *enabled = 3;
    return true;
}

bool hc::Config::getFastForwarding(bool* is) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, is);
    _logger->warn("Returning false for is fast forwarding");

    *is = false;
    return true;
}
bool hc::Config::setCoreOptions(retro_core_option_definition const* options) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, options);
    _logger->info("Setting core options");

    for (; options->key != nullptr; options++) {
        _logger->info("    key     = %s", options->key);
        _logger->info("    desc    = %s", options->desc);
        _logger->info("    info    = %s", options->info);
        _logger->info("    default = %s", options->default_value);

        CoreOption tempopt;
        tempopt.key = options->key;
        tempopt.label = options->desc;
        tempopt.sublabel = options->info != nullptr ? options->info : "";
        tempopt.selected = 0;
        tempopt.visible = true;

        for (unsigned i = 0; options->values[i].value != nullptr; i++) {
            retro_core_option_value const* const value = options->values + i;

            _logger->debug("        value = %s", value->value);
            _logger->debug("        label = %s", value->label);

            CoreOption::Value tempval;
            tempval.value = value->value;
            tempval.label = value->label != nullptr ? value->label : value->value;

            tempopt.values.emplace_back(tempval);
        }

        _coreMap.emplace(tempopt.key, _coreOptions.size());
        _coreOptions.emplace_back(tempopt);
    }

    return true;
}

bool hc::Config::setCoreOptionsIntl(retro_core_options_intl const* intl) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, intl);
    _logger->warn("Using English for the core options");
    return setCoreOptions(intl->us);
}

bool hc::Config::setCoreOptionsDisplay(retro_core_option_display const* display) {
    _logger->debug("%s:%u: %s(%p)", __FILE__, __LINE__, __FUNCTION__, display);
    _logger->info("Setting core options display");

    for (; display->key != nullptr; display++) {
        _logger->info("    \"%s\" is %s", display->key, display->visible ? "visible" : "invisible");

        auto found = _coreMap.find(display->key);

        if (found != _coreMap.end()) {
            auto& option = _coreOptions[found->second];
            option.visible = display->visible;
        }
        else {
            _logger->warn("        key not found in core options");
        }
    }

    return true;
}

bool hc::Config::getSupportNoGame() const {
    return _supportsNoGame;
}

const std::string& hc::Config::getRootPath() const {
    return _rootPath;
}

const std::string& hc::Config::getAutorunPath() const {
    return _autorunPath;
}

int hc::Config::push(lua_State* L) {
    auto const self = static_cast<Config**>(lua_newuserdata(L, sizeof(Config*)));
    *self = this;

    if (luaL_newmetatable(L, "hc::Config")) {
        static luaL_Reg const methods[] = {
            {"getRootPath", l_getRootPath},
            {"getAutorunPath", l_getAutorunPath},
            {"getSystemPath", l_getSystemPath},
            {"getCoreAssetsPath", l_getCoreAssetsPath},
            {"getSavePath", l_getSavePath},
            {"getCoresPath", l_getCoresPath},
            {nullptr, nullptr}
        };

        luaL_newlib(L, methods);
        lua_setfield(L, -2, "__index");
    }

    lua_setmetatable(L, -2);
    return 1;
}

hc::Config* hc::Config::check(lua_State* L, int index) {
    return *(Config**)luaL_checkudata(L, index, "hc::Config");
}

int hc::Config::l_getRootPath(lua_State* L) {
    Config* const self = check(L, 1);
    lua_pushlstring(L, self->_rootPath.c_str(), self->_rootPath.length());
    return 1;
}

int hc::Config::l_getAutorunPath(lua_State* L) {
    Config* const self = check(L, 1);
    lua_pushlstring(L, self->_autorunPath.c_str(), self->_autorunPath.length());
    return 1;
}

int hc::Config::l_getSystemPath(lua_State* L) {
    Config* const self = check(L, 1);
    lua_pushlstring(L, self->_systemPath.c_str(), self->_systemPath.length());
    return 1;
}

int hc::Config::l_getCoreAssetsPath(lua_State* L) {
    Config* const self = check(L, 1);
    lua_pushlstring(L, self->_coreAssetsPath.c_str(), self->_coreAssetsPath.length());
    return 1;
}

int hc::Config::l_getSavePath(lua_State* L) {
    Config* const self = check(L, 1);
    lua_pushlstring(L, self->_savePath.c_str(), self->_savePath.length());
    return 1;
}

int hc::Config::l_getCoresPath(lua_State* L) {
    Config* const self = check(L, 1);
    lua_pushlstring(L, self->_coresPath.c_str(), self->_coresPath.length());
    return 1;
}
