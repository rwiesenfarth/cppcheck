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
#include "path.h"

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

ProjectFile::ProjectFile()
{
    clear();
}

ProjectFile::ProjectFile(const std::string &filename) :
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

bool ProjectFile::read( const std::string &filename )
{
    if(!filename.empty())
        mFilename = filename;

    tinyxml2::XMLDocument xmlDoc;
    auto result = xmlDoc.LoadFile(mFilename.c_str());
    if (result != tinyxml2::XML_SUCCESS)
        return false;

    clear();

    auto project = xmlDoc.FirstChildElement(ProjectElementName);
    if (!project)
        return false;

    auto root = project->FirstChildElement(RootPathName);
    if (root)
        mRootPath = root->Attribute(RootPathNameAttrib, "");

    auto buildDir = project->FirstChildElement(BuildDirElementName);
    if (buildDir)
        mBuildDir = buildDir->GetText();

    auto platform = project->FirstChildElement(PlatformElementName);
    if (platform)
        mPlatform = platform->GetText();

    auto importProject = project->FirstChildElement(ImportProjectElementName);
    if (importProject)
        mImportProject = importProject->GetText();

    auto analyzeAllVsConfigs = project->FirstChildElement(AnalyzeAllVsConfigsElementName);
    if (analyzeAllVsConfigs)
        mAnalyzeAllVsConfigs = analyzeAllVsConfigs->BoolText();

    auto includeDirList = project->FirstChildElement(IncludeDirElementName);
    if (includeDirList) {
        auto includeDir = includeDirList->FirstChildElement(DirElementName);
        while (includeDir) {
            mIncludeDirs.push_back(includeDir->Attribute(DirNameAttrib, ""));
            includeDir = includeDir->NextSiblingElement(DirElementName);
        }
    }

    auto defineList = project->FirstChildElement(DefinesElementName);
    if (defineList) {
        auto define = defineList->FirstChildElement(DefineName);
        while (define) {
            mDefines.push_back(define->Attribute(DefineNameAttrib, ""));
            define = define->NextSiblingElement(DefineName);
        }
    }

    auto undefineList = project->FirstChildElement(UndefinesElementName);
    if (undefineList) {
        auto undefine = undefineList->FirstChildElement(UndefineName);
        while (undefine) {
            mUndefines.push_back(undefine->GetText());
            undefine = undefine->NextSiblingElement(UndefinesElementName);
        }
    }

    auto pathList = project->FirstChildElement(PathsElementName);
    if (pathList) {
        auto path = pathList->FirstChildElement(PathName);
        while (path) {
            mPaths.push_back(path->Attribute(PathNameAttrib, ""));
            path = path->NextSiblingElement(PathName);
        }
    }

    auto excludeList = project->FirstChildElement(ExcludeElementName);
    if (excludeList) {
        auto path = excludeList->FirstChildElement(ExcludePathName);
        while (path) {
            mExcludedPaths.push_back(path->Attribute(ExcludePathNameAttrib, ""));
            path = path->NextSiblingElement(ExcludePathName);
        }
    }

    auto libraryList = project->FirstChildElement(LibrariesElementName);
    if (libraryList) {
        auto library = libraryList->FirstChildElement(LibraryElementName);
        while (library) {
            mLibraries.push_back(library->GetText());
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
            mSuppressions.push_back(sup);
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
            std::string toolName(tool->GetText());
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

    return true;
}

void ProjectFile::setIncludes(const std::vector<std::string> &includes)
{
    mIncludeDirs = includes;
}

void ProjectFile::setDefines(const std::vector<std::string> &defines)
{
    mDefines = defines;
}

void ProjectFile::setUndefines(const std::vector<std::string> &undefines)
{
    mUndefines = undefines;
}

void ProjectFile::setCheckPaths(const std::vector<std::string> &paths)
{
    mPaths = paths;
}

void ProjectFile::setExcludedPaths(const std::vector<std::string> &paths)
{
    mExcludedPaths = paths;
}

void ProjectFile::setLibraries(const std::vector<std::string> &libraries)
{
    mLibraries = libraries;
}

void ProjectFile::setPlatform(const std::string &platform)
{
    mPlatform = platform;
}

void ProjectFile::setSuppressions(const std::vector<Suppressions::Suppression> &suppressions)
{
    mSuppressions = suppressions;
}

void ProjectFile::setAddons(const QStringList &addons)
{
    mAddons = addons;
}

bool ProjectFile::write(const std::string &filename)
{
    if (!filename.empty())
        mFilename = filename;

    tinyxml2::XMLDocument xmlDoc;
    xmlDoc.SetBOM(true);
    
    auto declaration = xmlDoc.NewDeclaration();
    xmlDoc.InsertFirstChild(declaration);

    auto project = xmlDoc.NewElement(ProjectElementName);
    project->SetAttribute(ProjectVersionAttrib, ProjectFileVersion);
    xmlDoc.InsertEndChild(project);

    if (!mRootPath.empty()) {
        auto root = xmlDoc.NewElement(RootPathName);
        root->SetAttribute(RootPathNameAttrib, mRootPath.c_str());
        project->InsertEndChild(root);
    }

    if (!mBuildDir.empty()) {
        auto buildDir = xmlDoc.NewElement(BuildDirElementName);
        buildDir->SetText(mBuildDir.c_str());
        project->InsertEndChild(buildDir);
    }

    if (!mPlatform.empty()) {
        auto platform = xmlDoc.NewElement(PlatformElementName);
        platform->SetText(mPlatform.c_str());
        project->InsertEndChild(platform);
    }

    if (!mImportProject.empty()) {
        auto importProject = xmlDoc.NewElement(ImportProjectElementName);
        importProject->SetText(mImportProject.c_str());
        project->InsertEndChild(importProject);
    }

    auto analyzeAllVsConfigs = xmlDoc.NewElement(AnalyzeAllVsConfigsElementName);
    analyzeAllVsConfigs->SetText(mAnalyzeAllVsConfigs ? "true" : "false");
    project->InsertEndChild(analyzeAllVsConfigs);

    if (!mIncludeDirs.empty()) {
        auto includeDirList = xmlDoc.NewElement(IncludeDirElementName);
        project->InsertEndChild(includeDirList);
        for (auto incdir : mIncludeDirs) {
            auto includeDir = xmlDoc.NewElement(DirElementName);
            includeDir->SetAttribute(DirNameAttrib, incdir.c_str());
            includeDirList->InsertEndChild(includeDir);
        }
    }

    if (!mDefines.empty()) {
        auto defineList = xmlDoc.NewElement(DefinesElementName);
        project->InsertEndChild(defineList);
        for (auto def : mDefines) {
            auto define = xmlDoc.NewElement(DefineName);
            define->SetAttribute(DefineNameAttrib, def.c_str());
            defineList->InsertEndChild(define);
        }
    }

    writeStringList(xmlDoc, *project, mUndefines, UndefinesElementName, UndefineName);

    if (!mPaths.empty()) {
        auto pathList = xmlDoc.NewElement(PathsElementName);
        project->InsertEndChild(pathList);
        for (auto p : mPaths) {
            auto path = xmlDoc.NewElement(PathName);
            path->SetAttribute(PathNameAttrib, p.c_str());
            pathList->InsertEndChild(path);
        }
    }

    if (!mExcludedPaths.empty()) {
        auto excludeList = xmlDoc.NewElement(ExcludeElementName);
        project->InsertEndChild(excludeList);
        for (auto p : mExcludedPaths) {
            auto path = xmlDoc.NewElement(ExcludePathName);
            path->SetAttribute(ExcludePathNameAttrib, p.c_str());
            excludeList->InsertEndChild(path);
        }
    }

    writeStringList(xmlDoc, *project, mLibraries, LibrariesElementName, LibraryElementName);

    if (!mSuppressions.empty()) {
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

    auto result = xmlDoc.SaveFile(mFilename.c_str());
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

void ProjectFile::writeStringList(tinyxml2::XMLDocument &xmlDoc, tinyxml2::XMLElement &parent, const std::vector<std::string> &stringlist, const char startelementname[], const char stringelementname[])
{
    if (stringlist.empty())
        return;

    auto list = xmlDoc.NewElement(startelementname);
    parent.InsertEndChild(list);
    for (auto str : stringlist) {
        auto element = xmlDoc.NewElement(stringelementname);
        element->SetText(str.c_str());
        list->InsertEndChild(element);
    }
}

std::vector<std::string> ProjectFile::fromNativeSeparators(const std::vector<std::string> &paths)
{
    std::vector<std::string> ret;
    for (const auto &path : paths)
        ret.push_back(Path::fromNativeSeparators(path));
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
