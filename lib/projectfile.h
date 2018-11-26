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

//---------------------------------------------------------------------------
#ifndef PROJECT_FILE_H
#define PROJECT_FILE_H
//---------------------------------------------------------------------------

#include "config.h"
#include "suppressions.h"

#include <string>
#include <vector>

namespace tinyxml2 {
    class XMLDocument;
    class XMLElement;
}

/// @addtogroup Core
/// @{


/**
* @brief A class that reads and writes project files.
* The project files contain project-specific settings for checking. For
* example a list of include paths.
*/
class CPPCHECKLIB ProjectFile {

public:
    explicit ProjectFile();
    explicit ProjectFile(const std::string &filename);

    /**
     * @brief Read the project file.
     * @param filename Filename (can be also given to constructor).
     */
    bool read(const std::string &filename = std::string());

    /**
     * @brief Get project root path.
     * @return project root path.
     */
    std::string getRootPath() const {
        return mRootPath;
    }

    std::string getBuildDir() const {
        return mBuildDir;
    }

    std::string getImportProject() const {
        return mImportProject;
    }

    bool getAnalyzeAllVsConfigs() const {
        return mAnalyzeAllVsConfigs;
    }

    /**
    * @brief Get list of include directories.
    * @return list of directories.
    */
    std::vector<std::string> getIncludeDirs() const {
        return ProjectFile::fromNativeSeparators(mIncludeDirs);
    }

    /**
    * @brief Get list of defines.
    * @return list of defines.
    */
    std::vector<std::string> getDefines() const {
        return mDefines;
    }

    /**
    * @brief Get list of undefines.
    * @return list of undefines.
    */
    std::vector<std::string> getUndefines() const {
        return mUndefines;
    }

    /**
    * @brief Get list of paths to check.
    * @return list of paths.
    */
    std::vector<std::string> getCheckPaths() const {
        return ProjectFile::fromNativeSeparators(mPaths);
    }

    /**
    * @brief Get list of paths to exclude from the check.
    * @return list of paths.
    */
    std::vector<std::string> getExcludedPaths() const {
        return ProjectFile::fromNativeSeparators(mExcludedPaths);
    }

    /**
    * @brief Get list libraries.
    * @return list of libraries.
    */
    std::vector<std::string> getLibraries() const {
        return mLibraries;
    }

    /**
     * @brief Get platform.
     * @return Current platform. If it ends with .xml then it is a file. Otherwise it must match one of the return values from @sa cppcheck::Platform::platformString() ("win32A", "unix32", ..)
     */
    std::string getPlatform() const {
        return mPlatform;
    }

    /**
    * @brief Get list suppressions.
    * @return list of suppressions.
    */
    std::vector<Suppressions::Suppression> getSuppressions() const {
        return mSuppressions;
    }

    /**
    * @brief Get list addons.
    * @return list of addons.
    */
    std::vector<std::string> getAddons() const {
        return mAddons;
    }

    /**
    * @brief Get list of addons and tools.
    * @return list of addons and tools.
    */
    std::vector<std::string> getAddonsAndTools() const;

    bool getClangAnalyzer() const {
        return false; //mClangAnalyzer;
    }

    void setClangAnalyzer(bool c) {
        mClangAnalyzer = c;
    }

    bool getClangTidy() const {
        return mClangTidy;
    }

    void setClangTidy(bool c) {
        mClangTidy = c;
    }

    std::vector<std::string> getTags() const {
        return mTags;
    }

    /**
    * @brief Get filename for the project file.
    * @return file name.
    */
    std::string getFilename() const {
        return mFilename;
    }

    /**
    * @brief Set project root path.
    * @param rootpath new project root path.
    */
    void setRootPath(const std::string &rootpath) {
        mRootPath = rootpath;
    }

    void setBuildDir(const std::string &buildDir) {
        mBuildDir = buildDir;
    }

    void setImportProject(const std::string &importProject) {
        mImportProject = importProject;
    }

    void setAnalyzeAllVsConfigs(bool b) {
        mAnalyzeAllVsConfigs = b;
    }

    /**
     * @brief Set list of includes.
     * @param includes List of defines.
     */
    void setIncludes(const std::vector<std::string> &includes);

    /**
     * @brief Set list of defines.
     * @param defines List of defines.
     */
    void setDefines(const std::vector<std::string> &defines);

    /**
     * @brief Set list of undefines.
     * @param defines List of undefines.
     */
    void setUndefines(const std::vector<std::string> &undefines);

    /**
     * @brief Set list of paths to check.
     * @param paths List of paths.
     */
    void setCheckPaths(const std::vector<std::string> &paths);

    /**
     * @brief Set list of paths to exclude from the check.
     * @param paths List of paths.
     */
    void setExcludedPaths(const std::vector<std::string> &paths);

    /**
     * @brief Set list of libraries.
     * @param libraries List of libraries.
     */
    void setLibraries(const std::vector<std::string> &libraries);

    /**
     * @brief Set platform.
     * @param platform platform.
     */
    void setPlatform(const std::string &platform);

    /**
     * @brief Set list of suppressions.
     * @param suppressions List of suppressions.
     */
    void setSuppressions(const std::vector<Suppressions::Suppression> &suppressions);

    /**
     * @brief Set list of addons.
     * @param addons List of addons.
     */
    void setAddons(const std::vector<std::string> &addons);

    /**
     * @brief Set tags.
     * @param tags tag list
     */
    void setTags(const std::vector<std::string> &tags) {
        mTags = tags;
    }

    /**
     * @brief Write project file (to disk).
     * @param filename Filename to use.
     */
    bool write(const std::string &filename = std::string());

    /**
     * @brief Set filename for the project file.
     * @param filename Filename to use.
     */
    void setFilename(const std::string &filename) {
        mFilename = filename;
    }

protected:

    /**
     * @brief Write string list
     * @param xmlDoc xml document
     * @param parent parent xml element
     * @param stringlist string list to write
     * @param startelementname name of start element
     * @param stringelementname name of each string element
     */
    static void writeStringList(tinyxml2::XMLDocument &xmlDoc, tinyxml2::XMLElement &parent, const std::vector<std::string> &stringlist, const char startelementname[], const char stringelementname[]);

private:

    void clear();

    /**
     * @brief Create a std::string from a char pointer which might be nullptr
     * @param cString a character pointer that might be nullptr
     * @return The std::string equivalent, or an empty string if @arg cString was nullptr
     */
    static std::string charPointerToString(const char *cString);

    /**
     * @brief Convert paths
     */
    static std::vector<std::string> fromNativeSeparators(const std::vector<std::string> &paths);

    /**
     * @brief Filename (+path) of the project file.
     */
    std::string mFilename;

    /**
     * @brief Root path (optional) for the project.
     * This is the project root path. If it is present then all relative paths in
     * the project file are relative to this path. Otherwise paths are relative
     * to project file's path.
     */
    std::string mRootPath;

    /** Cppcheck build dir */
    std::string mBuildDir;

    /** Visual studio project/solution , compile database */
    std::string mImportProject;

    /**
     * Should all visual studio configurations be analyzed?
     * If this is false then only the Debug configuration
     * for the set platform is analyzed.
     */
    bool mAnalyzeAllVsConfigs;

    /**
     * @brief List of include directories used to search include files.
     */
    std::vector<std::string> mIncludeDirs;

    /**
     * @brief List of defines.
     */
    std::vector<std::string> mDefines;

    /**
     * @brief List of undefines.
     */
    std::vector<std::string> mUndefines;

    /**
     * @brief List of paths to check.
     */
    std::vector<std::string> mPaths;

    /**
     * @brief Paths excluded from the check.
     */
    std::vector<std::string> mExcludedPaths;

    /**
     * @brief List of libraries.
     */
    std::vector<std::string> mLibraries;

    /**
     * @brief Platform
     */
    std::string mPlatform;

    /**
     * @brief List of suppressions.
     */
    std::vector<Suppressions::Suppression> mSuppressions;

    /**
     * @brief List of addons.
     */
    std::vector<std::string> mAddons;

    /** @brief Execute clang analyzer? */
    bool mClangAnalyzer;

    /** @brief Execute clang-tidy? */
    bool mClangTidy;

    /**
     * @brief Warning tags
     */
    std::vector<std::string> mTags;
};
/// @}
#endif  // PROJECT_FILE_H
