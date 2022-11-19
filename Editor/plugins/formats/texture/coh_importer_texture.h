/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2022 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <core/plugin_interfaces/PluginDeclarations.h>

class StringName;

class ResourceLoaderCoHTexture : public QObject, public ResourceLoaderInterface {

    Q_PLUGIN_METADATA(IID "org.godot.CoHTextureLoader")
    Q_INTERFACES(ResourceLoaderInterface)
    Q_OBJECT

public:
    RES    load(StringView p_path, StringView p_original_path = StringView(), Error *r_error = nullptr,
                bool p_no_subresource_cache = false) override;
    void get_recognized_extensions(Vector<String> &p_extensions) const override;
    bool handles_type(StringView p_type) const override;
    String get_resource_type(StringView p_path) const override;

    ~ResourceLoaderCoHTexture() override = default;
};
