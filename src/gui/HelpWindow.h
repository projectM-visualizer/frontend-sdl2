#pragma once

#include <string>
#include <vector>

class HelpWindow
{
public:
    /**
     * @brief Displays the help window.
     */
    void Show();

    /**
     * @brief Draws the help window.
     */
    void Draw();

private:
    void FillCommandLineArgumentTable();

    void FillKeyboardShortcutsTable();

    bool _visible{false}; //!< window visibility flag.

    using CommandLineArgument = std::tuple<std::string, std::string, std::string>;
    using KeyboardShortcut = std::pair<std::string, std::string>;

    std::vector<CommandLineArgument> _commandLineArguments; //!< Command line argument help. Similar to running the application with --help.
    std::vector<KeyboardShortcut> _shortcuts; //!< Keyboard shortcuts
};
