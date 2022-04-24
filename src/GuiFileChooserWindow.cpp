#include "GuiFileChooserWindow.h"

#include "imgui.h"

#include <Poco/SortedDirectoryIterator.h>
#include <Poco/String.h>

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

    if (!_currentDir.isDirectory() || !Poco::File(_currentDir).exists())
    {
        _currentDir = Poco::Path::home();
    }

    if (ImGui::Begin("Load Preset", &_visible), ImGuiWindowFlags_NoCollapse)
    {
        DrawNavButtons();

        ImGui::Separator();

        ImGui::Text("%s", _currentDir.toString().c_str());

        if (ImGui::BeginListBox("##filelist", ImVec2(-1,-1)))
        {
            Poco::SortedDirectoryIterator directoryIterator(_currentDir);
            Poco::SortedDirectoryIterator directoryEnd;

            int index = 0;
            while (directoryIterator != directoryEnd)
            {
                bool isSelected = _selectedFileIndex == index;
                bool isDirectory = false;
                bool isHidden = false;
                try
                {
                    // This will throw for broken symlinks or if the file/dir isn't accessible
                    isHidden = directoryIterator->isHidden();
                    isDirectory = directoryIterator->isDirectory();
                }
                catch (...)
                {
                }

                if (!_showhidden && isHidden)
                {
                    ++directoryIterator;
                    continue;
                }

                auto filename = directoryIterator.name();

                if (isDirectory)
                {
                    filename.append("/");
                }
                else
                {
                    if (Poco::icompare(directoryIterator.path().getExtension(), "milk") != 0)
                    {
                        ++directoryIterator;
                        continue;
                    }
                }

                if (ImGui::Selectable(filename.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    _selectedFileIndex = index;

                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        if (isDirectory)
                        {
                            _currentDir = directoryIterator.path();
                            _currentDir.makeDirectory();
                            poco_debug_f1(_logger, "Changing dir to: %s", _currentDir.toString());
                        }
                        else
                        {
                            _selectedFile = directoryIterator.path();
                            poco_debug_f1(_logger, "User selected file: %s", _selectedFile.path());
                            fileSelected = true;
                            _visible = false;
                        }
                    }
                }

                ++directoryIterator;
                index++;
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
void GuiFileChooserWindow::DrawNavButtons()
{
    ImGui::Checkbox("Show hidden files", &_showhidden);

    // Root path buttons first
    std::vector<std::string> roots;
    Poco::Path::listRoots(roots);

    if (ImGui::Button("Up"))
    {
        _currentDir = _currentDir.parent();
        _currentDir.makeDirectory();
        poco_debug_f1(_logger, "Going one dir up: %s", _currentDir.toString());
    }

    ImGui::SameLine();

    if (ImGui::Button("Home"))
        {
            _currentDir = Poco::Path::home();
            poco_debug_f1(_logger, "Going to user's home dir: %s", _currentDir.toString());
        }

        for (const auto& root: roots)
        {
            ImGui::SameLine();

            if (ImGui::Button(root.c_str()))
            {
                _currentDir = root;

                poco_debug_f1(_logger, "Changing root/drive to: %s", _currentDir.toString());
            }
        }
}

const Poco::File& GuiFileChooserWindow::SelectedFile() const
{
    return _selectedFile;
}
