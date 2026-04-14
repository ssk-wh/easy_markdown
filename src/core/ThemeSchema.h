// src/core/ThemeSchema.h
//
// Spec: specs/模块-app/12-主题插件系统.md
// Spec: specs/横切关注点/30-主题系统.md (INV-6)
// Last synced: 2026-04-14
//
// 主题配置文件的字段命名与默认值中心化声明。
// UI 代码不直接依赖这里的字符串常量，只有 ThemeLoader 读写使用。
#pragma once

namespace theme_schema {

// 当前 schema 版本。不兼容的破坏性变更必须 bump major。
constexpr int kSchemaVersionMajor = 1;
constexpr int kSchemaVersionMinor = 0;

// section 名称
constexpr const char* kMetaSection      = "meta";
constexpr const char* kEditorSection    = "editor";
constexpr const char* kSyntaxSection    = "syntax";
constexpr const char* kPreviewSection   = "preview";

} // namespace theme_schema
