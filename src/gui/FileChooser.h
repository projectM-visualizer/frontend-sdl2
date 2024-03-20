#pragma once

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Logger.h>

#include <set>

/**
 * @brief File/preset chooser dialog
 *
 * Displays a file browser that shows directories and files from which the user can choose.
 */
class FileChooser
{
public:
    enum class Mode {
        File,
        Directory,
        Both
    };

    FileChooser() = delete;

    /**
     * Creates a new file or directory chooser dialog with the given mode.
     * @param mode The dialog mode to operate in.
     */
    explicit FileChooser(Mode mode);

    /**
     * Creates a new file or directory chooser dialog.
     * @param title The title of the dialog window.
     * @param initialDirectory The initial directory to display.
     * @param mode The dialog mode to operate in.
     */
    explicit FileChooser(std::string title, const std::string& initialDirectory, Mode mode);

    /**
     * @brief Sets the dialog title.
     * @param title The title text of the choose dialog.
     */
    void Title(const std::string& title);

    /**
     * @brief Sets the current directory shown in the chooser dialog.
     * @param path Sets the new current directory for the chooser dialog.
     */
    void CurrentDirectory(const std::string& path);

    /**
     * @brief Sets the current file chooser context of the caller.
     * This can be anything the caller needs to identify where to put the result of the dialog, e.g.
     * an ID or property name.
     * @param context The context data.
     */
    void Context(const std::string& context);

    /**
     * @brief Returns the current file chooser context.
     * @return The current context string value.
     */
    const std::string& Context() const;

    /**
     * @brief Sets a list of allowed file extensions.
     * Not used in directory mode. Extensions are always matched case-insensitively.
     * @param extensions A list of file extensions, without the leading dot.
     */
    void AllowedExtensions(std::vector<std::string> extensions);

    /**
     * @brief Returns the list of currently allowed file extensions.
     * @return The list of currently allowed file extensions. If empty, all extensions are allowed.
     */
    const std::vector<std::string>& AllowedExtensions() const;

    /**
     * @brief Sets the multi select mode.
     * @param enabled true if multiple selection will be allowed, false if not.
     */
    void MultiSelect(bool enabled);

    /**
     * @brief Returns if the multi select mode is enabled.
     * @return true if multiple selection is allowed, false if not.
     */
    bool MultiSelect() const;

    /**
     * @brief Displays the file chooser window.
     */
    void Show();

    /**
     * @brief Closes the file chooser window.
     */
    void Close();

    /**
     * @brief Draws the dialog and returns if the user chose a file.
     * @return True if the user chose a file, false if not.
     */
    bool Draw();

    /**
     * @brief Returns the selected file(s)
     * @return The selected files.
     */
    const std::vector<Poco::File>& SelectedFiles() const;

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
    void ChangeDirectory(Poco::Path newDirectory);

    /**
     * @brief Updates the current list selection for the given index.
     *
     * How the selection is updated is based on modifier keys being pressed, multiple
     * selection being allowed, and, if a shift key is held down, on the previously clicked
     * entry.
     *
     * It's not as sophisticated as the selection handling of most OS file browsers, but should
     * perform well for most users.
     *
     * @param index The current list index being clicked on.
     * @param isSelected true if the current list item was already selected.
     */
    void UpdateListSelection(int index, bool isSelected);

    std::string _title; //!< The window title.
    std::string _context; //!< Context data for the caller.
    std::vector<std::string> _extensions; //!< File extensions to filter.
    Mode _mode{Mode::File}; //!< Chooser mode, either file or directory.
    bool _visible{ false }; //!< File chooser window visible.
    bool _showHidden{ false }; //!< If true, hidden files/dirs are shown.
    bool _multiSelect{ false }; //!< If true, selecting multiple files/directories is allowed.
    Poco::Path _currentDir{ Poco::Path::current() }; //!< Current working dir.
    std::vector<Poco::File> _currentFileList; //!< File list of current directory
    std::vector<Poco::File> _selectedFiles; //!< Currently selected file(s).
    int _selectedFileIndex{ 0 }; //!< Last selected item in the file list.
    std::set<int> _selectedFileIndices; //!< Set of selected file indices in the list


    Poco::Logger& _logger{ Poco::Logger::get("GuiFileChooserWindow") };
};
