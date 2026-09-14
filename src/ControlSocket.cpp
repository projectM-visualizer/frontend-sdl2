#include "ControlSocket.h"

#include "ProjectMSDLApplication.h"
#include "ProjectMWrapper.h"

#include "notifications/PlaybackControlNotification.h"

#include <Poco/Environment.h>
#include <Poco/JSON/Parser.h>
#include <Poco/NotificationCenter.h>
#include <Poco/Path.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <sstream>

#ifndef _WIN32
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

/*
 * Control socket protocol
 * =======================
 *
 * Clients connect to the UNIX domain socket and exchange newline-delimited JSON. Each request is a
 * single JSON object on its own line; the server replies with a single JSON object line per request.
 * Multiple commands may be sent over one connection.
 *
 * Requests (the "command" field is required):
 *
 *   {"command": "next"}                      Advance to the next preset.
 *   {"command": "prev"}                      Go to the previous preset. ("previous" is also accepted.)
 *   {"command": "last"}                      Return to the previously displayed preset.
 *   {"command": "random"}                    Jump to a random preset.
 *   {"command": "shuffle"}                   Toggle shuffle mode.
 *   {"command": "shuffle", "enabled": true}  Set shuffle mode explicitly.
 *   {"command": "lock"}                      Toggle the preset lock.
 *   {"command": "lock", "enabled": true}     Set the preset lock explicitly.
 *   {"command": "select", "index": 5}        Jump to the preset at the given playlist index.
 *   {"command": "select", "name": "Foo"}     Jump to the first preset whose full path or file name matches.
 *   {"command": "status"}                    Query the current state without changing anything.
 *
 * The "next", "prev", "last", "random" and "select" commands accept an optional boolean
 * "smooth" field (default false) requesting a soft transition instead of a hard cut.
 *
 * Responses always carry a "status" field, either "ok" or "error":
 *
 *   {"status": "ok", "index": 5, "count": 1234, "preset": "/path/Foo.milk", "shuffle": true, "locked": false}
 *   {"status": "error", "error": "unknown command: foo"}
 *
 * Successful responses include the resulting playback state. Example client usage:
 *
 *   echo '{"command":"next"}' | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/projectMSDL.sock
 */

namespace
{
std::string SerializeResponse(Poco::JSON::Object& object)
{
    std::ostringstream stream;
    object.stringify(stream);
    stream << '\n';
    return stream.str();
}

std::string ErrorResponse(const std::string& message)
{
    Poco::JSON::Object response;
    response.set("status", "error");
    response.set("error", message);
    return SerializeResponse(response);
}
} // namespace

const char* ControlSocket::name() const
{
    return "Control Socket";
}

#ifndef _WIN32

void ControlSocket::initialize(Poco::Util::Application& app)
{
    if (!app.config().getBool("controlSocket.enabled", false))
    {
        poco_information(_logger, "Control socket is disabled by configuration.");
        return;
    }

    // Default to a per-user socket below the XDG runtime directory, falling back to /tmp.
    std::string defaultDir = Poco::Environment::get("XDG_RUNTIME_DIR", "/tmp");
    Poco::Path defaultPath(defaultDir);
    defaultPath.makeDirectory().setFileName("projectMSDL.sock");
    _socketPath = app.config().getString("controlSocket.path", defaultPath.toString());

    if (_socketPath.size() >= sizeof(sockaddr_un::sun_path))
    {
        poco_error_f1(_logger, "Control socket path is too long: %s", _socketPath);
        return;
    }

    if (::pipe(_shutdownPipe) != 0)
    {
        poco_error(_logger, "Failed to create control socket shutdown pipe.");
        return;
    }

    _listenFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (_listenFd < 0)
    {
        poco_error(_logger, "Failed to create control socket.");
        return;
    }

    // Remove a stale socket file left behind by a previous, unclean shutdown.
    ::unlink(_socketPath.c_str());

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::strncpy(address.sun_path, _socketPath.c_str(), sizeof(address.sun_path) - 1);

    if (::bind(_listenFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
    {
        poco_error_f1(_logger, "Failed to bind control socket to %s.", _socketPath);
        ::close(_listenFd);
        _listenFd = -1;
        return;
    }

    if (::listen(_listenFd, 4) != 0)
    {
        poco_error(_logger, "Failed to listen on control socket.");
        ::close(_listenFd);
        _listenFd = -1;
        ::unlink(_socketPath.c_str());
        return;
    }

    _running = true;
    _enabled = true;
    _acceptThread = std::thread(&ControlSocket::AcceptLoop, this);

    poco_information_f1(_logger, "Control socket listening on %s.", _socketPath);
}

void ControlSocket::uninitialize()
{
    if (!_enabled)
    {
        return;
    }

    _running = false;

    // Wake the accept thread out of poll().
    if (_shutdownPipe[1] != -1)
    {
        const char wake = 'x';
        ssize_t ignored = ::write(_shutdownPipe[1], &wake, 1);
        static_cast<void>(ignored);
    }

    // Fulfill any commands still queued so client threads blocked on their futures unblock.
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        while (!_commandQueue.empty())
        {
            auto command = _commandQueue.front();
            _commandQueue.pop();
            try
            {
                command->response.set_value(ErrorResponse("shutting down"));
            }
            catch (...)
            {
            }
        }
    }

    // Interrupt any blocking recv() in connected client threads.
    {
        std::lock_guard<std::mutex> lock(_clientsMutex);
        for (int fd : _clientFds)
        {
            ::shutdown(fd, SHUT_RDWR);
        }
    }

    if (_acceptThread.joinable())
    {
        _acceptThread.join();
    }

    // Detached client threads only access this object's members, which outlive them.
    // Wait (bounded) for them to finish before releasing the socket resources.
    for (int attempt = 0; _activeClients > 0 && attempt < 500; ++attempt)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (_listenFd != -1)
    {
        ::close(_listenFd);
        _listenFd = -1;
    }
    if (_shutdownPipe[0] != -1)
    {
        ::close(_shutdownPipe[0]);
        _shutdownPipe[0] = -1;
    }
    if (_shutdownPipe[1] != -1)
    {
        ::close(_shutdownPipe[1]);
        _shutdownPipe[1] = -1;
    }

    ::unlink(_socketPath.c_str());

    poco_information(_logger, "Control socket stopped.");
}

void ControlSocket::AcceptLoop()
{
    while (_running)
    {
        pollfd fds[2];
        fds[0].fd = _listenFd;
        fds[0].events = POLLIN;
        fds[0].revents = 0;
        fds[1].fd = _shutdownPipe[0];
        fds[1].events = POLLIN;
        fds[1].revents = 0;

        int result = ::poll(fds, 2, -1);
        if (result < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            poco_error(_logger, "Control socket poll() failed, stopping accept loop.");
            break;
        }

        if (fds[1].revents & POLLIN)
        {
            // Shutdown requested.
            break;
        }

        if (!(fds[0].revents & POLLIN))
        {
            continue;
        }

        int clientFd = ::accept(_listenFd, nullptr, nullptr);
        if (clientFd < 0)
        {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(_clientsMutex);
            if (!_running)
            {
                ::close(clientFd);
                break;
            }
            _clientFds.push_back(clientFd);
        }

        std::thread(&ControlSocket::HandleClient, this, clientFd).detach();
    }
}

void ControlSocket::HandleClient(int clientFd)
{
    ++_activeClients;

    std::string buffer;
    char chunk[1024];

    while (_running)
    {
        ssize_t received = ::recv(clientFd, chunk, sizeof(chunk), 0);
        if (received <= 0)
        {
            break;
        }

        buffer.append(chunk, static_cast<size_t>(received));

        // Process all complete, newline-delimited lines currently in the buffer.
        std::string::size_type newline;
        while ((newline = buffer.find('\n')) != std::string::npos)
        {
            std::string line = buffer.substr(0, newline);
            buffer.erase(0, newline + 1);

            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            // Trim leading/trailing whitespace; skip empty lines.
            const auto first = line.find_first_not_of(" \t");
            if (first == std::string::npos)
            {
                continue;
            }
            const auto last = line.find_last_not_of(" \t");
            line = line.substr(first, last - first + 1);

            std::string responseLine;
            try
            {
                Poco::JSON::Parser parser;
                auto parsed = parser.parse(line);
                auto request = parsed.extract<Poco::JSON::Object::Ptr>();
                responseLine = DispatchToRenderThread(request);
            }
            catch (const Poco::Exception& ex)
            {
                responseLine = ErrorResponse("invalid JSON request: " + ex.displayText());
            }
            catch (const std::exception& ex)
            {
                responseLine = ErrorResponse(std::string("invalid request: ") + ex.what());
            }

            // Write the full response, tolerating partial writes; bail if the client is gone.
            size_t written = 0;
            bool clientGone = false;
            while (written < responseLine.size())
            {
                ssize_t count = ::send(clientFd, responseLine.data() + written,
                                       responseLine.size() - written, MSG_NOSIGNAL);
                if (count <= 0)
                {
                    clientGone = true;
                    break;
                }
                written += static_cast<size_t>(count);
            }

            if (clientGone)
            {
                break;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(_clientsMutex);
        _clientFds.erase(std::remove(_clientFds.begin(), _clientFds.end(), clientFd), _clientFds.end());
    }
    ::close(clientFd);

    --_activeClients;
}

std::string ControlSocket::DispatchToRenderThread(const Poco::JSON::Object::Ptr& request)
{
    auto command = std::make_shared<Command>();
    command->request = request;

    std::future<std::string> future;
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        if (!_running)
        {
            return ErrorResponse("shutting down");
        }
        future = command->response.get_future();
        _commandQueue.push(command);
    }

    // Block until the render thread executes the command. The wait is interruptible so a
    // shutdown (which drains the queue) cannot leave this thread stuck forever.
    while (true)
    {
        if (future.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready)
        {
            return future.get();
        }
        if (!_running)
        {
            if (future.wait_for(std::chrono::milliseconds(500)) == std::future_status::ready)
            {
                return future.get();
            }
            return ErrorResponse("shutting down");
        }
    }
}

#else // _WIN32

void ControlSocket::initialize(Poco::Util::Application& app)
{
    static_cast<void>(app);
    poco_information(_logger, "Control socket is not supported on this platform.");
}

void ControlSocket::uninitialize()
{
}

void ControlSocket::AcceptLoop()
{
}

void ControlSocket::HandleClient(int clientFd)
{
    static_cast<void>(clientFd);
}

std::string ControlSocket::DispatchToRenderThread(const Poco::JSON::Object::Ptr& request)
{
    static_cast<void>(request);
    return ErrorResponse("control socket not supported on this platform");
}

#endif // _WIN32

void ControlSocket::ProcessPendingCommands()
{
    if (!_enabled)
    {
        return;
    }

    // Move the queued commands out under the lock, then execute without holding it.
    std::queue<std::shared_ptr<Command>> pending;
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        std::swap(pending, _commandQueue);
    }

    while (!pending.empty())
    {
        auto command = pending.front();
        pending.pop();
        try
        {
            command->response.set_value(ExecuteCommand(command->request));
        }
        catch (...)
        {
            try
            {
                command->response.set_value(ErrorResponse("internal error"));
            }
            catch (...)
            {
            }
        }
    }
}

void ControlSocket::AppendState(Poco::JSON::Object& response)
{
    auto& projectMWrapper = Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>();
    auto playlist = projectMWrapper.Playlist();
    auto projectM = projectMWrapper.ProjectM();

    uint32_t count = projectm_playlist_size(playlist);
    uint32_t index = projectm_playlist_get_position(playlist);

    response.set("index", index);
    response.set("count", count);
    response.set("shuffle", static_cast<bool>(projectm_playlist_get_shuffle(playlist)));
    response.set("locked", static_cast<bool>(projectm_get_preset_locked(projectM)));

    if (count > 0)
    {
        auto* presetName = projectm_playlist_item(playlist, index);
        if (presetName != nullptr)
        {
            response.set("preset", std::string(presetName));
            projectm_playlist_free_string(presetName);
        }
    }
}

std::string ControlSocket::ExecuteCommand(const Poco::JSON::Object::Ptr& request)
{
    if (request.isNull() || !request->has("command"))
    {
        return ErrorResponse("missing 'command' field");
    }

    const std::string command = request->getValue<std::string>("command");
    const bool smooth = request->optValue<bool>("smooth", false);

    auto& notificationCenter = Poco::NotificationCenter::defaultCenter();
    auto& projectMWrapper = Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>();
    auto playlist = projectMWrapper.Playlist();
    auto projectM = projectMWrapper.ProjectM();
    auto userConfig = ProjectMSDLApplication::instance().UserConfiguration();

    if (command == "next")
    {
        notificationCenter.postNotification(
            new PlaybackControlNotification(PlaybackControlNotification::Action::NextPreset, smooth));
    }
    else if (command == "prev" || command == "previous")
    {
        notificationCenter.postNotification(
            new PlaybackControlNotification(PlaybackControlNotification::Action::PreviousPreset, smooth));
    }
    else if (command == "last")
    {
        notificationCenter.postNotification(
            new PlaybackControlNotification(PlaybackControlNotification::Action::LastPreset, smooth));
    }
    else if (command == "random")
    {
        notificationCenter.postNotification(
            new PlaybackControlNotification(PlaybackControlNotification::Action::RandomPreset, smooth));
    }
    else if (command == "shuffle")
    {
        bool target = request->has("enabled")
                          ? request->getValue<bool>("enabled")
                          : !projectm_playlist_get_shuffle(playlist);
        // Route through the user configuration so the UI and persisted settings stay in sync.
        userConfig->setBool("projectM.shuffleEnabled", target);
    }
    else if (command == "lock")
    {
        bool target = request->has("enabled")
                          ? request->getValue<bool>("enabled")
                          : !projectm_get_preset_locked(projectM);
        userConfig->setBool("projectM.presetLocked", target);
    }
    else if (command == "select")
    {
        uint32_t count = projectm_playlist_size(playlist);

        if (request->has("index"))
        {
            int index = request->getValue<int>("index");
            if (index < 0 || static_cast<uint32_t>(index) >= count)
            {
                return ErrorResponse("index out of range");
            }
            projectm_playlist_set_position(playlist, static_cast<uint32_t>(index), !smooth);
        }
        else if (request->has("name"))
        {
            const std::string name = request->getValue<std::string>("name");
            bool found = false;
            for (uint32_t i = 0; i < count; ++i)
            {
                auto* item = projectm_playlist_item(playlist, i);
                if (item != nullptr)
                {
                    std::string itemPath(item);
                    projectm_playlist_free_string(item);

                    // Match either the full path or the trailing file name component.
                    auto slash = itemPath.find_last_of("/\\");
                    std::string fileName = slash == std::string::npos ? itemPath : itemPath.substr(slash + 1);
                    if (itemPath == name || fileName == name)
                    {
                        projectm_playlist_set_position(playlist, i, !smooth);
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
            {
                return ErrorResponse("no preset matching name: " + name);
            }
        }
        else
        {
            return ErrorResponse("'select' requires an 'index' or 'name' field");
        }
    }
    else if (command == "status")
    {
        // No state change; the response below reports the current state.
    }
    else
    {
        return ErrorResponse("unknown command: " + command);
    }

    Poco::JSON::Object response;
    response.set("status", "ok");
    AppendState(response);
    return SerializeResponse(response);
}
