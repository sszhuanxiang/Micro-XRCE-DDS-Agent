// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef UXR_AGENT_AGENT_INSTANCE_HPP_
#define UXR_AGENT_AGENT_INSTANCE_HPP_

#include <uxr/agent/config.hpp>

#include <uxr/agent/utils/ArgumentParser.hpp>
#include <uxr/agent/utils/AgentServer.hpp>

#include <csignal>

namespace eprosima {
namespace uxr {

namespace middleware {
/**
 * @brief   Forward declaration.
 */
class CallbackFactory;
} // middleware

/**
 * @brief   Singleton class to manage the launch process of a MicroXRCE-DDS Agent.
 */
class AgentInstance
{
private:
    /**
     * @brief   Default constructor.
     */
    UXR_AGENT_EXPORT AgentInstance();
    /**
     * @brief   AgentInstance class shall not be copy constructible.
     */
    UXR_AGENT_EXPORT AgentInstance(
            const AgentInstance &) = delete;

    UXR_AGENT_EXPORT AgentInstance(
            AgentInstance &&) = delete;

    /**
     * @brief   AgentInstance class shall not be copy assignable.
     */
    UXR_AGENT_EXPORT AgentInstance& operator =(
            const AgentInstance &) = delete;

    UXR_AGENT_EXPORT AgentInstance& operator =(
            AgentInstance &&) = delete;

public:
    /**
     * @brief   Get instance associated to this class.
     * @return  Static reference to the singleton AgentInstance object.
     */
    UXR_AGENT_EXPORT static AgentInstance& getInstance();

    /**
     * @brief       Create an Agent instance, based on provided input parameters from user.
     * @param[in]   argc Number of available parameters introduced by the user.
     * @param[in]   argv List of parameters to be parsed to instantiate an Agent.
     * @return      Boolean value indicating if a Micro XRCE-DDS Agent was instantiated successfully.
     */
    UXR_AGENT_EXPORT bool create(
            int argc,
            char** argv);

    /**
     * @brief   Run the created agent until finished via user interrupt or process error.
     */
    UXR_AGENT_EXPORT void run();

    /**
     * @brief Sets a callback function for a specific create/delete middleware entity operation.
     *        Note that not some middlewares might not implement every defined operation, or even
     *        no operation at all.
     * @param middleware_kind   Enumeration class defining all the supported pluggable middlewares for the agent.
     * @param callback_kind     Enumeration class defining all the different operations available to which
     *                          set a callback to.
     * @param callback_function std::function rvalue variable implementing the callback logic. Desirable
     *                          to be implemented using lambda expressions wrapped inside a std::function descriptor.
     */
    template <typename ... Args>
    UXR_AGENT_EXPORT void add_middleware_callback(
            const Middleware::Kind& middleware_kind,
            const middleware::CallbackKind& callback_kind,
            std::function<void (Args ...)>&& callback_function);

private:
    std::thread agent_thread_;
#ifndef _WIN32
    sigset_t signals_;
#endif  // _WIN32
    middleware::CallbackFactory& callback_factory_;
};

template<typename AgentType>
class AgentInstanceAPI
{
public:
    UXR_AGENT_EXPORT AgentInstanceAPI(){};

    UXR_AGENT_EXPORT ~AgentInstanceAPI()
    {
        stop();
    };

	UXR_AGENT_EXPORT void configure(uint16_t port)
	{
		agent_.configure_agent_ipv4(port, Middleware::Kind::FASTDDS);
	}

	UXR_AGENT_EXPORT void configure(std::string dev, const std::string baudrate);
	UXR_AGENT_EXPORT void configure(std::vector<std::string> devs, const std::string baudrate);
	UXR_AGENT_EXPORT void configure(const std::string baudrate);
	
    UXR_AGENT_EXPORT void set_verbose_level(uint8_t verbose_level)
    {
        agent_.set_verbose_level(verbose_level);
    }

    UXR_AGENT_EXPORT void run()
    {
        std::thread agent_thread = std::thread([=]() -> void
        {
            agent_.start_agent();
        });

        agent_thread_ = std::move(agent_thread);
    }

    UXR_AGENT_EXPORT void stop()
    {
        if (agent_thread_.joinable())
        {
			agent_.stop_agent();
			agent_thread_.join();
        }
    };

    template <typename ... Args>
    UXR_AGENT_EXPORT void add_middleware_callback(
            const Middleware::Kind& middleware_kind,
            const middleware::CallbackKind& callback_kind,
            std::function<void (Args ...)>&& callback_function)
    {
        AgentInstance agent_instance = AgentInstance::getInstance();
        agent_instance.add_middleware_callback(middleware_kind, callback_kind, callback_function);
    }

private:
    agent::AgentServer<AgentType> agent_;
    std::thread agent_thread_;
};

template<> inline UXR_AGENT_EXPORT void AgentInstanceAPI<TermiosAgent>::configure(std::string dev, const std::string baudrate)
{
    agent_.configure_agent_serial(dev, baudrate, Middleware::Kind::FASTDDS);
}

template<> inline UXR_AGENT_EXPORT void AgentInstanceAPI<MultiTermiosAgent>::configure(std::vector<std::string> devs, const std::string baudrate)
{
    agent_.configure_agent_multiserial(devs, baudrate, Middleware::Kind::FASTDDS);
}

template<> inline UXR_AGENT_EXPORT void AgentInstanceAPI<PseudoTerminalAgent>::configure(const std::string baudrate)
{
    agent_.configure_agent_pseudoterminal(baudrate, Middleware::Kind::FASTDDS);
}

} // uxr
} // eprosima

#endif  // UXR_AGENT_AGENT_INSTANCE_HPP_