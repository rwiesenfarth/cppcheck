/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "projectfile.h"
#include "common.h"

static const char ProjectElementName[] = "project";
static const char ProjectVersionAttrib[] = "version";
static const char ProjectFileVersion[] = "1";
static const char BuildDirElementName[] = "builddir";
static const char ImportProjectElementName[] = "importproject";
static const char AnalyzeAllVsConfigsElementName[] = "analyze-all-vs-configs";
static const char IncludeDirElementName[] = "includedir";
static const char DirElementName[] = "dir";
static const char DirNameAttrib[] = "name";
static const char DefinesElementName[] = "defines";
static const char DefineName[] = "define";
static const char DefineNameAttrib[] = "name";
static const char UndefinesElementName[] = "undefines";
static const char UndefineName[] = "undefine";
static const char PathsElementName[] = "paths";
static const char PathName[] = "dir";
static const char PathNameAttrib[] = "name";
static const char RootPathName[] = "root";
static const char RootPathNameAttrib[] = "name";
static const char IgnoreElementName[] = "ignore";
static const char IgnorePathName[] = "path";
static const char IgnorePathNameAttrib[] = "name";
static const char ExcludeElementName[] = "exclude";
static const char ExcludePathName[] = "path";
static const char ExcludePathNameAttrib[] = "name";
static const char LibrariesElementName[] = "libraries";
static const char LibraryElementName[] = "library";
static const char PlatformElementName[] = "platform";
static const char SuppressionsElementName[] = "suppressions";
static const char SuppressionElementName[] = "suppression";
static const char SuppressionFileNameAttrib[] = "fileName";
static const char SuppressionLineNumberAttrib[] = "lineNumber";
static const char SuppressionSymbolNameAttrib[] = "symbolName";
static const char AddonElementName[] = "addon";
static const char AddonsElementName[] = "addons";
static const char ToolElementName[] = "tool";
static const char ToolsElementName[] = "tools";
static const char TagsElementName[] = "tags";
static const char TagElementName[] = "tag";

ProjectFile::ProjectFile(QObject *parent) :
    QObject(parent)
{
    clear();
}

ProjectFile::ProjectFile(const QString &filename, QObject *parent) :
    QObject(parent),
    mFilename(filename)
{
    clear();
    read();
}

void ProjectFile::clear()
{
    mRootPath.clear();
    mBuildDir.clear();
    mImportProject.clear();
    mAnalyzeAllVsConfigs = true;
    mIncludeDirs.clear();
    mDefines.clear();
    mUndefines.clear();
    mPaths.clear();
    mExcludedPaths.clear();
    mLibraries.clear();
    mPlatform.clear();
    mSuppressions.clear();
    mAddons.clear();
    mClangAnalyzer = mClangTidy = false;
}

bool ProjectFile::read( const QString &filename )
{
    if(!filename.isEmpty())
        mFilename = filename;

    tinyxml2::XMLDocument xmlDoc;
    auto result = xmlDoc.LoadFile(mFilename.toUtf8().constData());
    if (result != tinyxml2::XML_SUCCESS)
        return false;

    clear();

    auto project = xmlDoc.FirstChildElement(ProjectElementName);
    if (!project)
        return false;

    auto root = project->FirstChildElement(RootPathName);
    if (root)
        mRootPath = QString::fromUtf8(root->Attribute(RootPathNameAttrib, ""));

    auto buildDir = project->FirstChildElement(BuildDirElementName);
    if (buildDir)
        mBuildDir = QString::fromUtf8(buildDir->GetText());

    auto platform = project->FirstChildElement(PlatformElementName);
    if (platform)
        mPlatform = QString::fromUtf8(platform->GetText());

    auto importProject = project->FirstChildElement(ImportProjectElementName);
    if (importProject)
        mImportProject = QString::fromUtf8(importProject->GetText());

    auto analyzeAllVsConfigs = project->FirstChildElement(AnalyzeAllVsConfigsElementName);
    if (analyzeAllVsConfigs)
        mAnalyzeAllVsConfigs = analyzeAllVsConfigs->BoolText();

    auto includeDirList = project->FirstChildElement(IncludeDirElementName);
    if (includeDirList) {
        auto includeDir = includeDirList->FirstChildElement(DirElementName);
        while (includeDir) {
            mIncludeDirs << QString::fromUtf8(includeDir->Attribute(DirNameAttrib, ""));
            includeDir = includeDir->NextSiblingElement(DirElementName);
        }
    }

    auto defineList = project->FirstChildElement(DefinesElementName);
    if (defineList) {
        auto define = defineList->FirstChildElement(DefineName);
        while (define) {
            mDefines << QString::fromUtf8(define->Attribute(DefineNameAttrib, ""));
            define = define->NextSiblingElement(DefineName);
        }
    }

    auto undefineList = project->FirstChildElement(UndefinesElementName);
    if (undefineList) {
        auto undefine = undefineList->FirstChildElement(UndefineName);
        while (undefine) {
            mUndefines << QString::fromUtf8(undefine->GetText());
            undefine = undefine->NextSiblingElement(UndefinesElementName);
        }
    }

    auto pathList = project->FirstChildElement(PathsElementName);
    if (pathList) {
        auto path = pathList->FirstChildElement(PathName);
        while (path) {
            mPaths << QString::fromUtf8(path->Attribute(PathNameAttrib, ""));
            path = path->NextSiblingElement(PathName);
        }
    }

    auto excludeList = project->FirstChildElement(ExcludeElementName);
    if (excludeList) {
        auto path = excludeList->FirstChildElement(ExcludePathName);
        while (path) {
            mExcludedPaths << QString::fromUtf8(path->Attribute(ExcludePathNameAttrib, ""));
            path = path->NextSiblingElement(ExcludePathName);
        }
    }

    auto libraryList = project->FirstChildElement(LibrariesElementName);
    if (libraryList) {
        auto library = libraryList->FirstChildElement(LibraryElementName);
        while (library) {
            mLibraries << QString::fromUtf8(library->GetText());
            library = library->NextSiblingElement(LibraryElementName);
        }
    }

    auto suppressionList = project->FirstChildElement(SuppressionsElementName);
    if (suppressionList) {
        auto suppression = suppressionList->FirstChildElement(SuppressionElementName);
        while (suppression) {
            Suppressions::Suppression sup;
            sup.errorId = suppression->GetText();
            sup.fileName = suppression->Attribute(SuppressionFileNameAttrib);
            sup.lineNumber = suppression->IntAttribute(SuppressionLineNumberAttrib, Suppressions::Suppression::NO_LINE);
            sup.symbolName = suppression->Attribute(SuppressionSymbolNameAttrib);
            mSuppressions << sup;
            suppression = suppression->NextSiblingElement(SuppressionElementName);
        }
    }

    auto addonList = project->FirstChildElement(AddonsElementName);
    if (addonList) {
        auto addon = addonList->FirstChildElement(AddonElementName);
        while (addon) {
            mAddons << QString::fromUtf8(addon->GetText());
            addon = addon->NextSiblingElement(AddonElementName);
        }
    }

    auto toolList = project->FirstChildElement(ToolsElementName);
    if (toolList) {
        auto tool = toolList->FirstChildElement(ToolElementName);
        while (tool) {
            auto toolName = tool->GetText();
            mClangAnalyzer |= (toolName == CLANG_ANALYZER);
            mClangTidy |= (toolName == CLANG_TIDY);
            tool = tool->NextSiblingElement(ToolElementName);
        }
    }

    auto tagList = project->FirstChildElement(TagsElementName);
    if (tagList) {
        auto tag = tagList->FirstChildElement(TagElementName);
        while (tag) {
            mTags << QString::fromUtf8(tag->GetText());
            tag = tag->NextSiblingElement(TagElementName);
        }
    }
}

void ProjectFile::setIncludes(const QStringList &includes)
{
    mIncludeDirs = includes;
}

void ProjectFile::setDefines(const QStringList &defines)
{
    mDefines = defines;
}

void ProjectFile::setUndefines(const QStringList &undefines)
{
    mUndefines = undefines;
}

void ProjectFile::setCheckPaths(const QStringList &paths)
{
    mPaths = paths;
}

void ProjectFile::setExcludedPaths(const QStringList &paths)
{
    mExcludedPaths = paths;
}

void ProjectFile::setLibraries(const QStringList &libraries)
{
    mLibraries = libraries;
}

void ProjectFile::setPlatform(const QString &platform)
{
    mPlatform = platform;
}

void ProjectFile::setSuppressions(const QList<Suppressions::Suppression> &suppressions)
{
    mSuppressions = suppressions;
}

void ProjectFile::setAddons(const QStringList &addons)
{
    mAddons = addons;
}

bool ProjectFile::write(const QString &filename)
{
    if (!filename.isEmpty())
        mFilename = filename;

    tinyxml2::XMLDocument xmlDoc;
    xmlDoc.SetBOM(true);
    
    auto declaration = xmlDoc.NewDeclaration();
    xmlDoc.InsertFirstChild(declaration);

    auto project = xmlDoc.NewElement(ProjectElementName);
    project->SetAttribute(ProjectVersionAttrib, ProjectFileVersion);
    xmlDoc.InsertEndChild(project);

    if (!mRootPath.isEmpty()) {
        auto root = xmlDoc.NewElement(RootPathName);
        root->SetAttribute(RootPathNameAttrib, mRootPath.toUtf8().constData());
        project->InsertEndChild(root);
    }

    if (!mBuildDir.isEmpty()) {
        auto buildDir = xmlDoc.NewElement(BuildDirElementName);
        buildDir->SetText(mBuildDir.toUtf8().constData());
        project->InsertEndChild(buildDir);
    }

    if (!mPlatform.isEmpty()) {
        auto platform = xmlDoc.NewElement(PlatformElementName);
        platform->SetText(mPlatform.toUtf8().constData());
        project->InsertEndChild(platform);
    }

    if (!mImportProject.isEmpty()) {
        auto importProject = xmlDoc.NewElement(ImportProjectElementName);
        importProject->SetText(mImportProject.toUtf8().constData());
        project->InsertEndChild(importProject);
    }

    auto analyzeAllVsConfigs = xmlDoc.NewElement(AnalyzeAllVsConfigsElementName);
    analyzeAllVsConfigs->SetText(mAnalyzeAllVsConfigs ? "true" : "false");
    project->InsertEndChild(analyzeAllVsConfigs);

    if (!mIncludeDirs.isEmpty()) {
        auto includeDirList = xmlDoc.NewElement(IncludeDirElementName);
        project->InsertEndChild(includeDirList);
        for (auto incdir : mIncludeDirs) {
            auto includeDir = xmlDoc.NewElement(DirElementName);
            includeDir->SetAttribute(DirNameAttrib, incdir.toUtf8().constData());
            includeDirList->InsertEndChild(includeDir);
        }
    }

    if (!mDefines.isEmpty()) {
        auto defineList = xmlDoc.NewElement(DefinesElementName);
        project->InsertEndChild(defineList);
        for (auto def : mDefines) {
            auto define = xmlDoc.NewElement(DefineName);
            define->SetAttribute(DefineNameAttrib, def.toUtf8().constData());
            defineList->InsertEndChild(define);
        }
    }

    writeStringList(xmlDoc, *project, mUndefines, UndefinesElementName, UndefineName);

    if (!mPaths.isEmpty()) {
        auto pathList = xmlDoc.NewElement(PathsElementName);
        project->InsertEndChild(pathList);
        for (auto p : mPaths) {
            auto path = xmlDoc.NewElement(PathName);
            path->SetAttribute(PathNameAttrib, p.toUtf8().constData());
            pathList->InsertEndChild(path);
        }
    }

    if (!mExcludedPaths.isEmpty()) {
        auto excludeList = xmlDoc.NewElement(ExcludeElementName);
        project->InsertEndChild(excludeList);
        for (auto p : mExcludedPaths) {
            auto path = xmlDoc.NewElement(ExcludePathName);
            path->SetAttribute(ExcludePathNameAttrib, p.toUtf8().constData());
            excludeList->InsertEndChild(path);
        }
    }

    writeStringList(xmlDoc, *project, mLibraries, LibrariesElementName, LibraryElementName);

    if (!mSuppressions.isEmpty()) {
        auto suppressionList = xmlDoc.NewElement(SuppressionsElementName);
        project->InsertEndChild(suppressionList);
        for (const auto &sup : mSuppressions) {
            auto suppression = xmlDoc.NewElement(SuppressionElementName);
            suppressionList->InsertEndChild(suppression);
            if (!sup.fileName.empty())
                suppression->SetAttribute(SuppressionFileNameAttrib, sup.fileName.c_str());
            if (sup.lineNumber > 0)
                suppression->SetAttribute(SuppressionLineNumberAttrib, std::to_string(sup.lineNumber).c_str());
            if (!sup.symbolName.empty())
                suppression->SetAttribute(SuppressionSymbolNameAttrib, sup.symbolName.c_str());
            if (!sup.errorId.empty())
                suppression->SetText(sup.errorId.c_str());
        }
    }

    writeStringList(xmlDoc, *project, mAddons, AddonsElementName, AddonElementName);

    {
        QStringList tools;
        if (mClangAnalyzer)
            tools << CLANG_ANALYZER;
        if (mClangTidy)
            tools << CLANG_TIDY;
        writeStringList(xmlDoc, *project, tools, ToolsElementName, ToolElementName);
    }

    writeStringList(xmlDoc, *project, mTags, TagsElementName, TagElementName);

    auto result = xmlDoc.SaveFile(mFilename.toUtf8().constData());
    return result == tinyxml2::XML_SUCCESS;
}

void ProjectFile::writeStringList(tinyxml2::XMLDocument &xmlDoc, tinyxml2::XMLElement &parent, const QStringList &stringlist, const char startelementname[], const char stringelementname[])
{
    if (stringlist.isEmpty())
        return;

    auto list = xmlDoc.NewElement(startelementname);
    parent.InsertEndChild(list);
    for (auto str : stringlist) {
        auto element = xmlDoc.NewElement(stringelementname);
        element->SetText(str.toUtf8().constData());
        list->InsertEndChild(element);
    }
}

QStringList ProjectFile::fromNativeSeparators(const QStringList &paths)
{
    QStringList ret;
    foreach (const QString &path, paths)
        ret << QDir::fromNativeSeparators(path);
    return ret;
}

QStringList ProjectFile::getAddonsAndTools() const
{
    QStringList ret(mAddons);
    if (mClangAnalyzer)
        ret << CLANG_ANALYZER;
    if (mClangTidy)
        ret << CLANG_TIDY;
    return ret;
}
