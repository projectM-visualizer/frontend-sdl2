#pragma once

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Logger.h>

/**
 * @brief File/preset chooser dialog
 *
 * Displays a file browser that shows directories and .milk files from which the user can choose one.
 */
class FileChooser
{
public:

    void Show();

    /**
     * @brief Draws the dialog and returns if the user chose a file.
     * @return True if the user chose a file, false if not.
     */
    bool Draw();


    /**
     * @brief Returns the selected file
     * @return The selected file
     */
    const Poco::File& SelectedFile() const;

protected:
    /**
     * Draws the navigation buttons on top (up, home, root/drives)
     */
    void DrawNavButtons();

    /**
     * @brief Fills the file list with content and checks if one was chosen
     * @return True if the user chose a file.
     */
    bool PopulateFileList();

    /**
     * @brief Changes the currently displayed directory to the given path.
     *
     * The path must not necessarily exist or be accessible. A message is shown to the user if something is wrong.
     *
     * @param newDirectory The directory to chdir into
     */
    void ChangeDirectory(const Poco::Path& newDirectory);

    bool _visible{ false }; //!< File chooser window visible.
    bool _showhidden{ false }; //!< If true, hidden files/dirs are shown.
    Poco::Path _currentDir{ Poco::Path::current() }; //!< Current working dir.
    std::vector<Poco::File> _currentFileList; //!< File list of current directory
    Poco::File _selectedFile; //!< Currently selected file.
    int _selectedFileIndex{ 0 }; //!< Currently selected item in the file list.


    Poco::Logger& _logger{ Poco::Logger::get("GuiFileChooserWindow") };
};
