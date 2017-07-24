#pragma once

#include "../common/nonstd/type_traits.hpp"
#include "../common/assert.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace core
{
	struct subsystem
	{
		subsystem() = default;
		subsystem(const subsystem&) = delete;
		subsystem& operator = (const subsystem&) = delete;
        virtual ~subsystem();

		//-----------------------------------------------------------------------------
		//  Name : initialize (virtual )
		/// <summary>
		/// 
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
        virtual bool initialize();

		//-----------------------------------------------------------------------------
		//  Name : dispose (virtual )
		/// <summary>
		/// 
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
        virtual void dispose();
	};

	struct subsystem_context
	{
		//-----------------------------------------------------------------------------
		//  Name : initialize ()
		/// <summary>
		/// 
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
		bool initialize();

		//-----------------------------------------------------------------------------
		//  Name : dispose ()
		/// <summary>
		/// 
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
		void dispose();

		//-----------------------------------------------------------------------------
		//  Name : add_subsystem ()
		/// <summary>
		/// Spawn a new subsystem with type S and construct arguments
		/// </summary>
		//-----------------------------------------------------------------------------
		template<typename S, typename ... Args> 
		S& add_subsystem(Args&& ...);

		//-----------------------------------------------------------------------------
		//  Name : get_subsystem ()
		/// <summary>
		/// Retrieve the registered system instance, existence should be guaranteed
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
		template<typename S> 
		S& get_subsystem();

		//-----------------------------------------------------------------------------
		//  Name : remove_subsystem ()
		/// <summary>
		/// Release and unregistered a subsystem from our context
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
		template<typename S> 
		void remove_subsystem();

		//-----------------------------------------------------------------------------
		//  Name : has_subsystems ()
		/// <summary>
		/// Check if we have specified subsystems
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
		template<typename S>
		bool has_subsystems() const;

		//-----------------------------------------------------------------------------
		//  Name : has_subsystems ()
		/// <summary>
		/// Check if we have specified subsystems
		/// </summary>
		//-----------------------------------------------------------------------------
		template<typename S1, typename S2, typename ... Args> 
		bool has_subsystems() const;

	protected:
		/// 
		std::vector<std::size_t> _orders;
		///
		std::unordered_map<std::size_t, std::unique_ptr<subsystem>> _subsystems;
	};

	//
	// IMPLEMENTATIONS of SUBSYSTEMS
	template<typename S, typename ... Args>
	S& subsystem_context::add_subsystem(Args&& ... args)
	{
		auto index = rtti::type_id<S>().hash_code();
		expects(!has_subsystems<S>() && "duplicated subsystem");
	
		//auto sys = new (std::nothrow) S(std::forward<Args>(args)...);
		_orders.push_back(index);
        _subsystems.emplace(std::make_pair(index, std::make_unique<S>(std::forward<Args>(args)...)));
		auto& sys = static_cast<S&>(*_subsystems[index].get());
		expects(sys.initialize() && "failed to initialize subsystem.");
		
		return sys;
	}

	template<typename S>
	S& subsystem_context::get_subsystem()
	{
		expects(has_subsystems<S>() && "failed to find system");
		const auto index = rtti::type_id<S>().hash_code();
		return static_cast<S&>(*_subsystems[index].get());
	}

	template<typename S>
	void subsystem_context::remove_subsystem()
	{
		expects(has_subsystems<S>() && "failed to find system");
		const auto index = rtti::type_id<S>().hash_code();
		_subsystems.erase(index);
		_orders.erase(index);
	}

	template<typename S>
	bool subsystem_context::has_subsystems() const
	{
		const auto index = rtti::type_id<S>().hash_code();
		return _subsystems.find(index) != _subsystems.end();
	}

	template<typename S1, typename S2, typename ... Args> 
	bool subsystem_context::has_subsystems() const
	{
		return has_subsystems<S1>() && has_subsystems<S2, Args...>();
	}

	// retrieve the registered system instance, existence should be guaranteed
	template<typename S>
	S& get_subsystem();

	// spawn a new subsystem with type S and construct arguments
	template<typename S, typename ... Args> 
	S& add_subsystem(Args&& ... args);

	// release and unregistered a subsystem from our context
	template<typename S>
	void remove_subsystem();

	// check if we have specified subsystems
	template<typename ... Args> 
	bool has_subsystems();

	struct subsystem_context;
	namespace details
	{
		enum class internal_status : uint8_t
		{
			idle,
			running,
			disposed
		};

		//-----------------------------------------------------------------------------
		//  Name : initialize ()
		/// <summary>
		/// 
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
		bool initialize();

		//-----------------------------------------------------------------------------
		//  Name : status ()
		/// <summary>
		/// 
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
		internal_status& status();

		//-----------------------------------------------------------------------------
		//  Name : dispose ()
		/// <summary>
		/// 
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
		void dispose();

		//-----------------------------------------------------------------------------
		//  Name : context ()
		/// <summary>
		/// 
		/// 
		/// 
		/// </summary>
		//-----------------------------------------------------------------------------
		subsystem_context& context();
	}

	template<typename S> 
	S& get_subsystem()
	{
		expects(details::status() == details::internal_status::running && "details::context must be initialized");
		return details::context().get_subsystem<S>();
	}

	template<typename S, typename ... Args> 
	S& add_subsystem(Args&& ... args)
	{
		expects(details::status() == details::internal_status::running && "details::context must be initialized");
		return details::context().add_subsystem<S>(std::forward<Args>(args)...);
	}

	template<typename S>
	void remove_subsystem()
	{
		if (details::status() == details::internal_status::running)
			details::context().remove_subsystem<S>();
	}

	template<typename ... Args> 
	bool has_subsystems()
	{
		return details::status() != details::internal_status::running ?
			false : details::context().has_subsystems<Args...>();
	}
}
