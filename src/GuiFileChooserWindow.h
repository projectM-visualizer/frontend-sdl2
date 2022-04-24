#pragma once

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Logger.h>

class GuiFileChooserWindow
{
public:

    void Show();

    /**
     * @brief Draws the dialog and returns if the user chose a file.
     * @return True if the user chose a file, false if not.
     */
    bool Draw();

    const Poco::File& SelectedFile() const;

protected:
    void DrawNavButtons();

    bool _visible{ false }; //!< File chooser window visible.
    bool _showhidden{ false }; //!< If true, hidden files/dirs are shown.
    Poco::Path _currentDir{ Poco::Path::current() }; //!< Current working dir.
    Poco::File _selectedFile; //!< Currently selected file.
    int _selectedFileIndex{ 0 }; //!< Currently selected item in the file list.

    Poco::Logger& _logger{ Poco::Logger::get("GuiFileChooserWindow") };
};
