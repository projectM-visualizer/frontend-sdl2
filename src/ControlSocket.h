#pragma once

#include <Poco/JSON/Object.h>
#include <Poco/Logger.h>

#include <Poco/Util/Subsystem.h>

#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief Exposes application control over a UNIX domain socket.
 *
 * On initialization, this subsystem creates a UNIX domain socket (path is configurable via
 * the "controlSocket.path" setting) and listens for client connections on a background thread.
 *
 * Clients send newline-delimited JSON request objects and receive a newline-delimited JSON
 * response object for each. The protocol is documented in ControlSocket.cpp.
 *
 * Because libprojectM and its playlist are not thread-safe with respect to the render thread,
 * incoming commands are never executed on the socket thread. Instead they are queued and
 * executed by the render thread via ProcessPendingCommands(), which is called once per frame.
 * The socket thread blocks on a future until the render thread has produced the response.
 */
class ControlSocket : public Poco::Util::Subsystem
{
public:
    const char* name() const override;

    void initialize(Poco::Util::Application& app) override;

    void uninitialize() override;

    /**
     * @brief Executes all queued socket commands on the calling (render) thread.
     *
     * Must be called regularly from the render loop. Each executed command fulfills the
     * promise the originating socket thread is waiting on. Safe to call even if the socket
     * is disabled or unsupported on the current platform (it is then a no-op).
     */
    void ProcessPendingCommands();

private:
    /**
     * @brief A single queued request/response pair handed from a socket thread to the render thread.
     */
    struct Command
    {
        Poco::JSON::Object::Ptr request; //!< Parsed JSON request object.
        std::promise<std::string> response; //!< Fulfilled by the render thread with the serialized response line.
    };

    /**
     * @brief Background thread accepting incoming client connections.
     */
    void AcceptLoop();

    /**
     * @brief Handles a single connected client until it disconnects or shutdown is requested.
     * @param clientFd The accepted client socket file descriptor.
     */
    void HandleClient(int clientFd);

    /**
     * @brief Enqueues a request for the render thread and blocks until the response is ready.
     * @param request The parsed JSON request.
     * @return The serialized JSON response line (including trailing newline).
     */
    std::string DispatchToRenderThread(const Poco::JSON::Object::Ptr& request);

    /**
     * @brief Executes a single command on the render thread and builds the JSON response line.
     * @param request The parsed JSON request.
     * @return The serialized JSON response line (including trailing newline).
     */
    std::string ExecuteCommand(const Poco::JSON::Object::Ptr& request);

    /**
     * @brief Adds the current playback state (preset index/name, shuffle, lock, count) to a response.
     * @param response The JSON object to populate.
     */
    void AppendState(Poco::JSON::Object& response);

    std::string _socketPath; //!< Filesystem path of the UNIX domain socket.
    bool _enabled{false}; //!< True if the socket was successfully created and is being served.

    int _listenFd{-1}; //!< Listening socket file descriptor.
    int _shutdownPipe[2]{-1, -1}; //!< Self-pipe used to wake the accept thread on shutdown.

    std::atomic<bool> _running{false}; //!< Cleared on shutdown to stop accepting/processing.

    std::thread _acceptThread; //!< Thread running AcceptLoop().
    std::mutex _clientsMutex; //!< Guards _clientFds.
    std::vector<int> _clientFds; //!< Currently connected client file descriptors.
    std::atomic<int> _activeClients{0}; //!< Number of running (detached) client handler threads.

    std::mutex _queueMutex; //!< Guards _commandQueue and gates enqueueing against shutdown.
    std::queue<std::shared_ptr<Command>> _commandQueue; //!< Commands awaiting execution on the render thread.

    Poco::Logger& _logger{Poco::Logger::get("ControlSocket")}; //!< The class logger.
};
