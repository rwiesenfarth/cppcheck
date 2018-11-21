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

bool ProjectFile::read(const QString &filename)
{
    if (!filename.isEmpty())
        mFilename = filename;

    QFile file(mFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    clear();

    QXmlStreamReader xmlReader(&file);
    bool insideProject = false;
    bool projectTagFound = false;
    while (!xmlReader.atEnd()) {
        switch (xmlReader.readNext()) {
        case QXmlStreamReader::StartElement:
            if (xmlReader.name() == ProjectElementName) {
                insideProject = true;
                projectTagFound = true;
            }
            // Read root path from inside project element
            if (insideProject && xmlReader.name() == RootPathName)
                readRootPath(xmlReader);

            // Read root path from inside project element
            if (insideProject && xmlReader.name() == BuildDirElementName)
                readBuildDir(xmlReader);

            // Find paths to check from inside project element
            if (insideProject && xmlReader.name() == PathsElementName)
                readCheckPaths(xmlReader);

            if (insideProject && xmlReader.name() == ImportProjectElementName)
                readImportProject(xmlReader);

            if (insideProject && xmlReader.name() == AnalyzeAllVsConfigsElementName)
                readAnalyzeAllVsConfigs(xmlReader);

            // Find include directory from inside project element
            if (insideProject && xmlReader.name() == IncludeDirElementName)
                readIncludeDirs(xmlReader);

            // Find preprocessor define from inside project element
            if (insideProject && xmlReader.name() == DefinesElementName)
                readDefines(xmlReader);

            // Find preprocessor define from inside project element
            if (insideProject && xmlReader.name() == UndefinesElementName)
                readStringList(mUndefines, xmlReader, UndefineName);

            // Find exclude list from inside project element
            if (insideProject && xmlReader.name() == ExcludeElementName)
                readExcludes(xmlReader);

            // Find ignore list from inside project element
            // These are read for compatibility
            if (insideProject && xmlReader.name() == IgnoreElementName)
                readExcludes(xmlReader);

            // Find libraries list from inside project element
            if (insideProject && xmlReader.name() == LibrariesElementName)
                readStringList(mLibraries, xmlReader,LibraryElementName);

            if (insideProject && xmlReader.name() == PlatformElementName)
                readPlatform(xmlReader);

            // Find suppressions list from inside project element
            if (insideProject && xmlReader.name() == SuppressionsElementName)
                readSuppressions(xmlReader);

            // Addons
            if (insideProject && xmlReader.name() == AddonsElementName)
                readStringList(mAddons, xmlReader, AddonElementName);

            // Tools
            if (insideProject && xmlReader.name() == ToolsElementName) {
                QStringList tools;
                readStringList(tools, xmlReader, ToolElementName);
                mClangAnalyzer = tools.contains(CLANG_ANALYZER);
                mClangTidy = tools.contains(CLANG_TIDY);
            }

            if (insideProject && xmlReader.name() == TagsElementName)
                readStringList(mTags, xmlReader, TagElementName);

            break;

        case QXmlStreamReader::EndElement:
            if (xmlReader.name() == ProjectElementName)
                insideProject = false;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    }

    file.close();
    return projectTagFound;
}

void ProjectFile::readRootPath(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attribs = reader.attributes();
    QString name = attribs.value(QString(), RootPathNameAttrib).toString();
    if (!name.isEmpty())
        mRootPath = name;
}

void ProjectFile::readBuildDir(QXmlStreamReader &reader)
{
    mBuildDir.clear();
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mBuildDir = reader.text().toString();
        case QXmlStreamReader::EndElement:
            return;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (1);
}

void ProjectFile::readImportProject(QXmlStreamReader &reader)
{
    mImportProject.clear();
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mImportProject = reader.text().toString();
        case QXmlStreamReader::EndElement:
            return;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (1);
}

void ProjectFile::readAnalyzeAllVsConfigs(QXmlStreamReader &reader)
{
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mAnalyzeAllVsConfigs = (reader.text().toString() == "true");
        case QXmlStreamReader::EndElement:
            return;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (1);
}

void ProjectFile::readIncludeDirs(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:

            // Read dir-elements
            if (reader.name().toString() == DirElementName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), DirNameAttrib).toString();
                if (!name.isEmpty())
                    mIncludeDirs << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == IncludeDirElementName)
                allRead = true;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (!allRead);
}

void ProjectFile::readDefines(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read define-elements
            if (reader.name().toString() == DefineName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), DefineNameAttrib).toString();
                if (!name.isEmpty())
                    mDefines << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == DefinesElementName)
                allRead = true;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (!allRead);
}

void ProjectFile::readCheckPaths(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:

            // Read dir-elements
            if (reader.name().toString() == PathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), PathNameAttrib).toString();
                if (!name.isEmpty())
                    mPaths << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == PathsElementName)
                allRead = true;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (!allRead);
}

void ProjectFile::readExcludes(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read exclude-elements
            if (reader.name().toString() == ExcludePathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), ExcludePathNameAttrib).toString();
                if (!name.isEmpty())
                    mExcludedPaths << name;
            }
            // Read ignore-elements - deprecated but support reading them
            else if (reader.name().toString() == IgnorePathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), IgnorePathNameAttrib).toString();
                if (!name.isEmpty())
                    mExcludedPaths << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == IgnoreElementName)
                allRead = true;
            if (reader.name().toString() == ExcludeElementName)
                allRead = true;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (!allRead);
}

void ProjectFile::readPlatform(QXmlStreamReader &reader)
{
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mPlatform = reader.text().toString();
        case QXmlStreamReader::EndElement:
            return;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (1);
}


void ProjectFile::readSuppressions(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == SuppressionElementName) {
                Suppressions::Suppression suppression;
                if (reader.attributes().hasAttribute(QString(),"fileName"))
                    suppression.fileName = reader.attributes().value(QString(),"fileName").toString().toStdString();
                if (reader.attributes().hasAttribute(QString(),"lineNumber"))
                    suppression.lineNumber = reader.attributes().value(QString(),"lineNumber").toInt();
                if (reader.attributes().hasAttribute(QString(),"symbolName"))
                    suppression.symbolName = reader.attributes().value(QString(),"symbolName").toString().toStdString();
                type = reader.readNext();
                if (type == QXmlStreamReader::Characters) {
                    suppression.errorId = reader.text().toString().toStdString();
                }
                mSuppressions << suppression;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != SuppressionElementName)
                return;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (true);
}


void ProjectFile::readStringList(QStringList &stringlist, QXmlStreamReader &reader, const char elementname[])
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == elementname) {
                type = reader.readNext();
                if (type == QXmlStreamReader::Characters) {
                    QString text = reader.text().toString();
                    stringlist << text;
                }
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != elementname)
                allRead = true;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (!allRead);
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
        root->SetAttribute(RootPathNameAttrib, mRootPath.toStdString().c_str());
        project->InsertEndChild(root);
    }

    if (!mBuildDir.isEmpty()) {
        auto buildDir = xmlDoc.NewElement(BuildDirElementName);
        buildDir->SetText(mBuildDir.toStdString().c_str());
        project->InsertEndChild(buildDir);
    }

    if (!mPlatform.isEmpty()) {
        auto platform = xmlDoc.NewElement(PlatformElementName);
        platform->SetText(mPlatform.toStdString().c_str());
        project->InsertEndChild(platform);
    }

    if (!mImportProject.isEmpty()) {
        auto importProject = xmlDoc.NewElement(ImportProjectElementName);
        importProject->SetText(mImportProject.toStdString().c_str());
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
            includeDir->SetAttribute(DirNameAttrib, incdir.toStdString().c_str());
            includeDirList->InsertEndChild(includeDir);
        }
    }

    if (!mDefines.isEmpty()) {
        auto defineList = xmlDoc.NewElement(DefinesElementName);
        project->InsertEndChild(defineList);
        for (auto def : mDefines) {
            auto define = xmlDoc.NewElement(DefineName);
            define->SetAttribute(DefineNameAttrib, def.toStdString().c_str());
            defineList->InsertEndChild(define);
        }
    }

    writeStringList(xmlDoc, *project, mUndefines, UndefinesElementName, UndefineName);

    if (!mPaths.isEmpty()) {
        auto pathList = xmlDoc.NewElement(PathsElementName);
        project->InsertEndChild(pathList);
        for (auto p : mPaths) {
            auto path = xmlDoc.NewElement(PathName);
            path->SetAttribute(PathNameAttrib, p.toStdString().c_str());
            pathList->InsertEndChild(path);
        }
    }

    if (!mExcludedPaths.isEmpty()) {
        auto excludeList = xmlDoc.NewElement(ExcludeElementName);
        project->InsertEndChild(excludeList);
        for (auto p : mExcludedPaths) {
            auto path = xmlDoc.NewElement(ExcludePathName);
            path->SetAttribute(ExcludePathNameAttrib, p.toStdString().c_str());
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
                suppression->SetAttribute("fileName", sup.fileName.c_str());
            if (sup.lineNumber > 0)
                suppression->SetAttribute("lineNumber", std::to_string(sup.lineNumber).c_str());
            if (!sup.symbolName.empty())
                suppression->SetAttribute("symbolName", sup.symbolName.c_str());
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

    auto result = xmlDoc.SaveFile(mFilename.toStdString().c_str());
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
        element->SetText(str.toStdString().c_str());
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
