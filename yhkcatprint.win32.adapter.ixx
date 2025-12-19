/*++

Copyright (C) 2025 Umamusume Polska

Module Name:
	yhkcatprint.win32.adapter.ixx

Abstract:
	Win32 implementation of IAdapter interface.

--*/

export module yhkcatprint.win32:adapter;

import std;
import yhkcatprint;

/**
 * @file yhkcatprint.win32.adapter.ixx
 * @brief Win32 implementation of IAdapter interface.
 * 
 * This module provides a Windows-specific implementation of the IAdapter
 * interface using the Windows Bluetooth API.
 */

export namespace yhkcatprint
{
	/**
	 * @brief Win32 implementation of IAdapter interface.
	 * 
	 * This class implements the IAdapter interface using the Windows Bluetooth API.
	 * It provides functionality to interact with Bluetooth adapters on Windows platforms.
	 * 
	 * @tparam TDevice Type of Bluetooth device to use, must derive from IDevice.
	 */
	template<std::derived_from<IDevice> TDevice>
	class AdapterWin32 : public IAdapter
	{
	public:
		/**
		 * @brief Constructs an AdapterWin32.
		 * 
		 * This constructor initializes the adapter with the given Bluetooth radio handle.
		 * 
		 * @param radioHandle Handle to the Bluetooth radio.
		 * Must be a valid handle obtained from the Windows Bluetooth API.
		 * 
		 * @warning The caller is responsible for ensuring the validity of the handle.
		 * Invalid handles may lead to undefined behavior.
		 */
		AdapterWin32(std::uintptr_t radioHandle);

		/**
		 * @brief Virtual destructor.
		 */
		virtual ~AdapterWin32() = default;

		/**
		 * @brief Retrieves information about the Bluetooth adapter.
		 *
		 * @return ADAPTER_INFO structure containing the adapter's address and name.
		 */
		ADAPTER_INFO getInfo() override;

		/**
		 * @brief Retrieves a list of paired Bluetooth devices.
		 *
		 * @return Vector of shared pointers to IDevice representing paired devices.
		 */
		std::vector<std::shared_ptr<TDevice>> getPairedDevices() override;

	private:
		/**
		 * @brief Implementation struct to hide platform-specific details.
		 */
		struct Impl;

		/**
		 * @brief Unique pointer to the implementation.
		 */
		std::unique_ptr<Impl> m_impl;
	};
}