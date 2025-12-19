/*++

Copyright (C) 2025 Umamusume Polska

Module Name:
	win32_adapter.cpp

Abstract:
	Implementation of AdapterWin32 methods.

--*/

module;

#include <WinSock2.h>
#include <ws2bth.h>
#include <BluetoothAPIs.h>

// Link with Bthprops.lib
#pragma comment(lib, "Bthprops.lib")

module yhkcatprint.win32:adapter;

import std;

/**
 * @file win32_adapter.cpp
 * @brief Implementation of AdapterWin32 methods.
 * 
 * This file provides the Windows-specific implementation of the AdapterWin32
 * class using the Windows Bluetooth API.
 */

namespace yhkcatprint
{
	template <std::derived_from<IDevice> TDevice>
	struct AdapterWin32<TDevice>::Impl
	{
		/**
		* @brief Bluetooth radio handle.
		*/
		HANDLE hRadio;

		/**
		* @brief Cached adapter information.
		*/
		ADAPTER_INFO cachedInfo;

		/**
		* @brief Flag indicating if adapter information is cached.
		*/
		bool infoCached = false;

		/**
		* @brief Reads adapter information from the Bluetooth radio.
		* 
		* @param hRadio Handle to the Bluetooth radio.
		* 
		* @return ADAPTER_INFO structure containing the adapter's address and name.
		*/
		ADAPTER_INFO readAdapterInfo(HANDLE hRadio)
		{
			BLUETOOTH_RADIO_INFO radioInfo = { sizeof(BLUETOOTH_RADIO_INFO) };
			DWORD result = BluetoothGetRadioInfo(hRadio, &radioInfo);
			if (result != ERROR_SUCCESS)
			{
				throw std::runtime_error("Failed to get Bluetooth radio info.");
			}
			ADAPTER_INFO info;
			info.address = formatBthAddr(radioInfo.address.ullLong);
			info.name = wideStringToString(radioInfo.szName);
			return info;
		}

		/**
		 * @brief Formats a Bluetooth address from ULONGLONG to string.
		 *
		 * @param ullLong Bluetooth address as ULONGLONG.
		 * @return Formatted Bluetooth address as string in "XX:XX:XX:XX:XX:XX" format.
		 */
		static std::string formatBthAddr(ULONGLONG ullLong)
		{
			std::ostringstream oss;
			oss << std::hex << std::setfill('0');
			for (int i = 5; i >= 0; --i)
			{
				oss << std::setw(2) << ((ullLong >> (i * 8)) & 0xFF);
				if (i != 0)
					oss << ":";
			}
			return oss.str();
		}

		/**
		 * @brief Converts a wide string (std::wstring) to a standard string (std::string).
		 * @param wideStr The wide string to convert.
		 * @return The converted standard string.
		 */
		static std::string wideStringToString(const wchar_t* wideStr)
		{
			using convert_typeX = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_typeX, wchar_t> converterX;

			return converterX.to_bytes(wideStr);
		}
	};

	template <std::derived_from<IDevice> TDevice>
	AdapterWin32<TDevice>::AdapterWin32(std::uintptr_t radioHandle)
		: pImpl(std::make_unique<Impl>())
	{
		pImpl->hRadio = reinterpret_cast<HANDLE>(radioHandle);
	}

	template <std::derived_from<IDevice> TDevice>
	ADAPTER_INFO AdapterWin32<TDevice>::getInfo()
	{
		return m_impl->infoCached
			? m_impl->cachedInfo
			: (
				m_impl->cachedInfo = m_impl->readAdapterInfo(m_impl->hRadio), 
				m_impl->infoCached = true, 
				m_impl->cachedInfo
			);
	}

	template <std::derived_from<IDevice> TDevice>
	std::vector<std::shared_ptr<TDevice>> AdapterWin32<TDevice>::getPairedDevices()
	{
		std::vector<std::shared_ptr<TDevice>> devices;
		BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = { sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS) };
		searchParams.hRadio = m_impl->hRadio;
		searchParams.fReturnAuthenticated = TRUE;
		searchParams.fReturnRemembered = TRUE;
		searchParams.fReturnUnknown = FALSE;
		searchParams.fReturnConnected = FALSE;
		searchParams.fIssueInquiry = FALSE;
		searchParams.cTimeoutMultiplier = 0;
		BLUETOOTH_DEVICE_INFO deviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO) };
		HBLUETOOTH_DEVICE_FIND hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
		if (hFind == nullptr)
		{
			DWORD error = GetLastError();
			BluetoothFindDeviceClose(hFind);
			throw std::runtime_error("Failed to find Bluetooth devices: " + std::to_string(error);
		}
		do
		{
			std::string address = m_impl->formatBthAddr(deviceInfo.Address.ullLong);
			std::string name = m_impl->wideStringToString(deviceInfo.szName);
			devices.push_back(std::make_shared<TDevice>(address, name));
		} while (BluetoothFindNextDevice(hFind, &deviceInfo));
		BluetoothFindDeviceClose(hFind);
		return devices;
	}
}