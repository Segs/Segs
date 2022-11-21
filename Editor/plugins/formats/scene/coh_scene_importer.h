/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2022 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once


#include <core/plugin_interfaces/PluginDeclarations.h>
#include <editor/plugin_interfaces/PluginDeclarations.h>
#include <editor/import/resource_importer_scene.h>
#include <scene/resources/scene_library.h>


#include "EASTL/unique_ptr.h"

class StringName;
namespace SEGS {
class SceneGraph;
}

class CoHSceneLibrary final : public QObject, public ResourceImporterInterface
{
    Q_PLUGIN_METADATA(IID "org.segs_engine.CoHSceneLibraryImporter")
    Q_INTERFACES(ResourceImporterInterface)
    Q_OBJECT

public:
    const char * get_importer_name() const override;
    const char * get_visible_name() const override;
    void get_recognized_extensions(Vector<String> &p_extensions) const override;
    bool can_import(StringView) const override;
    StringName get_save_extension() const override;
    StringName get_resource_type() const override;
    float get_priority() const override { return 1.0f; }
    int get_import_order() const override { return 0; }
    StringName get_option_group_file() const override { return StringName(); }
    int get_preset_count() const override { return 0; }
    StringName get_preset_name(int) const override;
    void get_import_options(Vector<ImportOption> * /*r_options*/, int /*p_preset*/) const override {}
    bool get_option_visibility(const StringName &p_option, const HashMap<StringName, Variant> &p_options) const override;
    Error import(StringView p_source_file, StringView p_save_path, const HashMap<StringName, Variant> &p_options,
                 Vector<String> &r_missing_deps, Vector<String> *r_platform_variants, Vector<String> *r_gen_files,
                 Variant *r_metadata) override;
    Error import_group_file(StringView p_group_file,
        const Map<String, HashMap<StringName, Variant>> &p_source_file_options,
        const Map<String, String> &p_base_paths) override
    {
        return ERR_UNAVAILABLE;
    }
    bool are_import_settings_valid(StringView /*p_path*/) const override { return true; }
    String get_import_settings_string() const override { return String(); }
};
