#pragma once

#include "MeshSync/msFoundation.h" //msSerializable

namespace ms {

enum class Handedness {
    Left,
    Right,
    LeftZUp,
    RightZUp,
};

enum class ColorSpace {
    Linear,
    sRGB
};

struct SceneSettings {
private:
    const std::string kScale_factor = "scale_factor";
    const std::string kHandedness = "handedness";
    const std::string kColorSpace = "colorSpace";
    std::map<std::string, std::string> settings;

    // Disabling for now because it breaks scenecache.
    // TODO make this work:
    //int material_sync_mode = 0;

public:

    float get_scale_factor() const {
        auto it = settings.find(kScale_factor);
        if (it != settings.end())
            return std::stof(it->second);
        return 1.0f;
    }

    void set_scale_factor(const float value) {
        settings[kScale_factor] = std::to_string(value);
    }

    Handedness get_handedness() const {
        auto it = settings.find(kHandedness);
        if (it != settings.end())
            return (Handedness)std::stoi(it->second);
        return Handedness::Left;
    }

    void set_handedness(const Handedness value) {
        settings[kHandedness] = std::to_string((int)value);
    }

    ColorSpace get_color_space() const {
        auto it = settings.find(kColorSpace);
        if (it != settings.end())
            return (ColorSpace)std::stoi(it->second);
        return ColorSpace::Linear;
    }

    void set_color_space(const ColorSpace value) {
        settings[kColorSpace] = std::to_string((int)value);
    }

    SceneSettings() {
        set_scale_factor(1.0f);
        set_handedness(Handedness::Left);
        set_color_space(ColorSpace::Linear);
    }

    SceneSettings(const SceneSettings& other) {
        settings = other.settings;
    }

    SceneSettings& operator=(const SceneSettings& other) {
        settings = other.settings;
        return *this;
    }

    SceneSettings(SceneSettings&& other) {
        settings = std::move(other.settings);
    }

    SceneSettings& operator=(SceneSettings&& other) {
        settings = std::move(other.settings);
        return *this;
    }
    
    void serialize(std::ostream& os) const {
        write(os, (int)settings.size());
        for (const auto& it : settings) {
            write(os, it.first);
            write(os, it.second);
        }
    }

    void deserialize(std::istream& is) {
        int settingsLength;
        read(is, settingsLength);

        for (int lineCount = 0; lineCount < settingsLength; lineCount++) {
            std::string key, value;
            read(is, key);
            read(is, value);

            settings[key] = value;
        }
    }
};

msSerializable(SceneSettings);

} // namespace ms
