#include "GuiFileChooserWindow.h"

#include "imgui.h"

#include <Poco/SortedDirectoryIterator.h>
#include <Poco/String.h>

#include <algorithm>

void GuiFileChooserWindow::Show()
{
    _visible = true;
}

bool GuiFileChooserWindow::Draw()
{
    if (!_visible)
    {
        return false;
    }

    bool fileSelected{false};

    if (!_currentDir.isDirectory()
        || (!_currentDir.toString().empty() && !Poco::File(_currentDir).exists()))
    {
        ChangeDirectory(Poco::Path::home());
    }

    if (ImGui::Begin("Load Preset", &_visible), ImGuiWindowFlags_NoCollapse)
    {
        DrawNavButtons();

        ImGui::Separator();

        char pathBuffer[2048]{};
        strncpy(pathBuffer, _currentDir.toString().c_str(), std::min<size_t>(2047, _currentDir.toString().size()));

        if (ImGui::InputText("##path", &pathBuffer[0], IM_ARRAYSIZE(pathBuffer)), ImGuiInputTextFlags_EnterReturnsTrue)
        {
            ChangeDirectory(std::string(pathBuffer));
        }

        if (ImGui::BeginListBox("##filelist", ImVec2(-1, -1)))
        {
            if (_currentDir.toString().empty())
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No path entered");
            }
            else
            {
                Poco::File pathCheck(_currentDir);

                if (!pathCheck.exists())
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Directory does not exist");
                }
                else if (!pathCheck.canRead())
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Directory cannot be accessed");
                }
                else
                {
                    fileSelected = PopulateFileList();
                }
            }

            ImGui::EndListBox();
        }
    }
    else
    {
        _visible = false;
    }

    ImGui::End();

    return fileSelected;
}

const Poco::File& GuiFileChooserWindow::SelectedFile() const
{
    return _selectedFile;
}

void GuiFileChooserWindow::DrawNavButtons()
{
    ImGui::Checkbox("Show hidden files", &_showhidden);

    // Root path buttons first
    std::vector<std::string> roots;
    Poco::Path::listRoots(roots);

    if (ImGui::Button("Up"))
    {
        ChangeDirectory(_currentDir.parent());
        poco_debug_f1(_logger, "Going one dir up: %s", _currentDir.toString());
    }

    ImGui::SameLine();

    if (ImGui::Button("Home"))
    {
        ChangeDirectory(Poco::Path::home());
        poco_debug_f1(_logger, "Going to user's home dir: %s", _currentDir.toString());
    }

    for (const auto& root: roots)
    {
        ImGui::SameLine();

        if (ImGui::Button(root.c_str()))
        {
            ChangeDirectory(root);
            poco_debug_f1(_logger, "Changing root/drive to: %s", _currentDir.toString());
        }
    }
}

bool GuiFileChooserWindow::PopulateFileList()
{
    bool fileSelected{false};
    bool changeDir{false};
    Poco::Path newDir;

    int index = 0;
    for (const auto& file : _currentFileList)
    {
        bool isSelected = _selectedFileIndex == index;
        bool isDirectory = false;
        try
        {
            // This will throw for broken symlinks or if the file/dir isn't accessible
            isDirectory = file.isDirectory();
        }
        catch (...)
        {
        }

        Poco::Path filePath(file.path());
        auto filename = filePath.getFileName();

        if (isDirectory)
        {
            filename.append("/");
        }

        if (ImGui::Selectable(filename.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
        {
            _selectedFileIndex = index;

            if (ImGui::IsMouseDoubleClicked(0))
            {
                if (isDirectory)
                {
                    newDir = filePath;
                    changeDir = true;
                    poco_debug_f1(_logger, "Changing dir to: %s", _currentDir.toString());
                }
                else
                {
                    _selectedFile = filePath;
                    poco_debug_f1(_logger, "User selected file: %s", _selectedFile.path());
                    fileSelected = true;
                    _visible = false;
                }
            }
        }

        index++;
    }

    if (changeDir)
    {
        ChangeDirectory(newDir);
    }

    return fileSelected;
}

void GuiFileChooserWindow::ChangeDirectory(const Poco::Path& newDirectory)
{
    _currentDir = newDirectory;
    _currentDir.makeDirectory();

    _currentFileList.clear();

    if (_currentDir.toString().empty())
    {
        return;
    }

    Poco::File pathCheck(_currentDir);
    if (!pathCheck.exists() || !pathCheck.canRead())
    {
        return;
    }

    // Ideally, only use the directory iterator after cd'ing into a new dir and store the path list.
    Poco::SortedDirectoryIterator directoryIterator(_currentDir);
    Poco::SortedDirectoryIterator directoryEnd;

    while (directoryIterator != directoryEnd)
    {
        bool isHidden = false;
        bool isDirectory = false;
        try
        {
            // This will throw for broken symlinks or if the file/dir isn't accessible
            isHidden = directoryIterator->isHidden();
            isDirectory = directoryIterator->isDirectory();
        }
        catch (...)
        {
        }

        if (!isDirectory && Poco::icompare(directoryIterator.path().getExtension(), "milk") != 0)
        {
            ++directoryIterator;
            continue;
        }

        if (!isHidden || _showhidden)
        {
            _currentFileList.push_back(*directoryIterator);
        }

        ++directoryIterator;
    }
}
